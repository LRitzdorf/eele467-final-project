/* Advanced controller program.
 * Adjusts PWM duty cycles based on ADC and accelerometer data.
 * Lucas Ritzdorf
 * 12/01/2023
 * EELE 467
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include <fcntl.h>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <signal.h>
#include <math.h>
#include <stdarg.h>

// Configuration constants
#define SYSID_VERSION 0x3ADC37ED
#define ADC_PATH "/sys/class/misc/adc_controller"
#define PWM_PATH "/sys/class/misc/hps_multi_pwm"
#define ACCEL_INPUT_DEV "/dev/input/event0"
#define PERIOD 0x100 // 2ms
#define NUM_CHANNELS 3


// Preprocessor macros for stringification
#define xstr(s) str(s)
#define str(s) #s

// Interrupt tracker for main loop
static volatile sig_atomic_t interrupted = false;
// And its associated handler function
static void ctrl_c(int _) {
    (void)_;
    interrupted = true;
}

// Helper function, so we don't have to seek and flush repeatedly
int dev_fprintf(FILE *restrict stream, const char *restrict format, ...) {
    rewind(stream);
    va_list args;
    va_start(args, format);
    int bytes = vfprintf(stream, format, args);
    va_end(args);
    fflush(stream);
    return bytes;
}

// Helper function, so we don't have to seek repeatedly
int dev_fscanf(FILE *restrict stream, const char *restrict format, ...) {
    freopen(NULL, "r", stream);  // Must re-open device file to update its contents
    va_list args;
    va_start(args, format);
    int result = vfscanf(stream, format, args);
    va_end(args);
    return result;
}

// HSL to RGB conversion helper
void hsl2rgb(const float *hsl, float *rgb) {
    // Based on https://en.wikipedia.org/wiki/HSL_and_HSV#HSL_to_RGB
    float chroma = (1 - fabsf(2 * hsl[2] - 1)) * hsl[1];
    float h_prime = hsl[0] / (M_PI/3);
    float other_comp = chroma * (1 - fabsf(fmodf(h_prime, 2) - 1));
    // Assign color components
    if        (h_prime < 1) {
        rgb[0] = chroma;
        rgb[1] = other_comp;
        rgb[2] = 0;
    } else if (h_prime < 2) {
        rgb[0] = other_comp;
        rgb[1] = chroma;
        rgb[2] = 0;
    } else if (h_prime < 3) {
        rgb[0] = 0;
        rgb[1] = chroma;
        rgb[2] = other_comp;
    } else if (h_prime < 4) {
        rgb[0] = 0;
        rgb[1] = other_comp;
        rgb[2] = chroma;
    } else if (h_prime < 5) {
        rgb[0] = other_comp;
        rgb[1] = 0;
        rgb[2] = chroma;
    } else  /* h_prime < 6 */ {
        rgb[0] = chroma;
        rgb[1] = 0;
        rgb[2] = other_comp;
    }
    // Match lightness
    float m = hsl[2] - (chroma/2);
    rgb[0] += m;
    rgb[1] += m;
    rgb[2] += m;
    return;
}


int main(int argc, char** argv) {

    { // Check System ID
        // Scan for valid device files
        glob_t globbuf;
        if (glob("/sys/bus/platform/devices/*.sysid/sysid/id", GLOB_NOSORT, NULL, &globbuf) == GLOB_NOMATCH) {
            fprintf(stderr, "No System ID device files found!\n");
            return 1;
        }

        // Check device files for matching ID
        bool sysid_match = false;
        for (unsigned int i = 0; i < globbuf.gl_pathc; i++) {
            // Open matched file and check for success
            FILE *id_f = fopen(globbuf.gl_pathv[i], "r");
            if (id_f == NULL) continue;
            // Parse and compare ID
            unsigned int id = 0;
            fscanf(id_f, "%u", &id);
            fclose(id_f);
            if (id == SYSID_VERSION) {
                sysid_match = true;
                break;
            }
        }
        globfree(&globbuf);

        // Exit if nothing matched
        if (!sysid_match) {
            fprintf(stderr, "No matching System ID found! (Expected 0x%X)\n", SYSID_VERSION);
            return 2;
        }
        printf("Found matching System ID 0x%X\n", SYSID_VERSION);
    }

    struct libevdev *accel = NULL;
    { // Verify accelerometer presence
        int accel_fd = open(ACCEL_INPUT_DEV, O_RDONLY|O_NONBLOCK);
        if (libevdev_new_from_fd(accel_fd, &accel) < 0) {
            fprintf(stderr, "Failed to initialize libedvev interface for " ACCEL_INPUT_DEV "!\n");
            return 3;
        }
        if (!libevdev_has_event_code(accel, EV_ABS, ABS_X) ||
            !libevdev_has_event_code(accel, EV_ABS, ABS_Y) ||
            !libevdev_has_event_code(accel, EV_ABS, ABS_Z) ||
            !libevdev_has_event_type(accel, EV_KEY)
           ) {
            fprintf(stderr, "Input device does not look like an accelerometer!\n");
            return 2;
        }
        printf("Found suitable accelerometer \"%s\" on " ACCEL_INPUT_DEV "\n", libevdev_get_name(accel));
    }

    // Initialization
    FILE *period_f = fopen(PWM_PATH "/period", "w");
    if (period_f == NULL) {
        perror("Failed to open PWM period file");
        return 3;
    }
    int rc = 0;
    FILE *channels[NUM_CHANNELS] = {NULL};
    FILE *duty_cycles[NUM_CHANNELS] = {NULL};
    for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
        char adcfile[sizeof(ADC_PATH) + 20];
        char pwmfile[sizeof(PWM_PATH) + 20];
        // Loop-open channel files...
        snprintf(adcfile, sizeof(adcfile), ADC_PATH "/channel_%d", i);
        channels[i] = fopen(adcfile, "r");
        if (channels[i] == NULL) {
            perror("Failed to open ADC channel");
            rc = 3;
            goto cleanup_files;
        }
        // ...and duty cycle files
        snprintf(pwmfile, sizeof(pwmfile), PWM_PATH "/duty_cycle_%d", i+1);
        duty_cycles[i] = fopen(pwmfile, "w");
        if (duty_cycles[i] == NULL) {
            perror("Failed to open PWM interface");
            rc = 3;
            goto cleanup_files;
        }
    }

    // Initialize hardware
    bool accel_mode = false;
    libevdev_disable_event_type(accel, EV_ABS); // No accelerometer updates for now
    dev_fprintf(period_f, xstr(PERIOD));

    // Prepare to catch interrupts
    signal(SIGINT, ctrl_c);

    printf("Control loop running; interrupt to exit...\n");
    printf("ADC mode\r"); fflush(stdout);
    int accel_vec[3] = {0};
    // Main control loop
    while (!interrupted) {

        // Handle any pending accelerometer events
        while (libevdev_has_event_pending(accel)) {
            struct input_event event;
            libevdev_next_event(accel, LIBEVDEV_READ_FLAG_NORMAL, &event);
            switch (event.type) {
                case EV_KEY:
                    // Tap event; switch control modes
                    if (event.value == 1) {
                        if (accel_mode) {
                            accel_mode = false;
                            libevdev_disable_event_type(accel, EV_ABS);
                            printf("ADC mode  \r"); fflush(stdout);
                        } else {
                            accel_mode = true;
                            libevdev_enable_event_type(accel, EV_ABS);
                            printf("Accel mode\r"); fflush(stdout);
                        }
                    }
                    break;
                case EV_ABS:
                    // Accelerometer event; record updated values
                    /* This won't trigger if we're not in "accelerometer mode,"
                     * since we disable the corresponding event type.
                     */
                    switch (event.code) {
                        case ABS_X:
                            accel_vec[0] = event.value;
                            break;
                        case ABS_Y:
                            accel_vec[1] = event.value;
                            break;
                        case ABS_Z:
                            accel_vec[2] = event.value;
                            break;
                        default: break;
                    }
                default: break;
            }
        }

        // Control the PWM module
        if (accel_mode) {
            // Calculate roll and pitch angles
            float roll = atan2(-(float)accel_vec[1], (float)accel_vec[2]);
            float pitch = atan2(-(float)accel_vec[0], sqrt(powf(accel_vec[1], 2) + powf(accel_vec[2], 2)));
            // Transform to HSL...
            float hsl[3], rgb[3];
            // roll -> hue, saturation = 1, pitch -> lightness
            hsl[0] = roll + M_PI;
            hsl[1] = 1;
            hsl[2] = (pitch / (M_PI)) + 0.5;
            // ...and then to RGB
            hsl2rgb(hsl, rgb);
            // Write RGB values to PWM controller
            for (unsigned int i = 0; i < 3; i++) {
                dev_fprintf(duty_cycles[i], "%u", (int)(rgb[i] * pow(2, 12)));
            }
        } else {
            /* Both register sets are fixed-point, and happen to have the same
             * number of fractional bits. Were this not the case, bit shifting
             * would be needed.
             */
            for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
                // Loop-write channels and duty cycles
                unsigned int reading = 0;
                dev_fscanf(channels[i], "%i", &reading);
                dev_fprintf(duty_cycles[i], "%u", reading);
            }
        }

        // NOTE: No waiting here. Time to eat the CPU for breakfast!
    }
    printf("\nCaught interrupt; exiting...\n");

    // Cleanup
    dev_fprintf(period_f, "0");
cleanup_files:
    for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
        // Loop-close channels and duty cycles
        if (channels[i] != NULL) fclose(channels[i]);
        if (duty_cycles[i] != NULL) fclose(duty_cycles[i]);
    }
    fclose(period_f);
    return rc;
}
