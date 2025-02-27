/* Basic controller program to set PWM duty cycles based on ADC readings
 * Lucas Ritzdorf
 * 12/01/2023
 * EELE 467
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <glob.h>
#include <signal.h>
#include <stdarg.h>

// Configuration constants
#define SYSID_VERSION 0x3ADC37ED
#define ADC_PATH "/sys/class/misc/adc_controller"
#define PWM_PATH "/sys/class/misc/hps_multi_pwm"
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
    }

    // Initialization
    FILE *period_f = fopen(PWM_PATH "/period", "w");
    if (period_f == NULL) {
        perror("Failed to open PWM period file");
        return 3;
    }
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
            goto cleanup_files;
        }
        // ...and duty cycle files
        snprintf(pwmfile, sizeof(pwmfile), PWM_PATH "/duty_cycle_%d", i+1);
        duty_cycles[i] = fopen(pwmfile, "w");
        if (duty_cycles[i] == NULL) {
            perror("Failed to open PWM interface");
            goto cleanup_files;
        }
    }
    dev_fprintf(period_f, xstr(PERIOD));

    // Prepare to catch interrupts
    signal(SIGINT, ctrl_c);

    // Main control loop
    printf("Control loop running; interrupt to exit...\n");
    fflush(stdout);
    while (!interrupted) {
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
        // NOTE: No waiting here. Time to eat the CPU for breakfast!
    }

    // Cleanup
    dev_fprintf(period_f, "0");
cleanup_files:
    for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
        // Loop-close channels and duty cycles
        if (channels[i] != NULL) fclose(channels[i]);
        if (duty_cycles[i] != NULL) fclose(duty_cycles[i]);
    }
    fclose(period_f);
    return 0;
}
