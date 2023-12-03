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
int dev_fprintf(FILE *stream, const char *format, ...) {
    rewind(stream);
    va_list args;
    va_start(args, format);
    int bytes = vfprintf(stream, format, args);
    va_end(args);
    fflush(stream);
    return bytes;
}


int main(int argc, char** argv) {

    { // TODO: Check System ID
    }

    // Initialization
    FILE *period_f = fopen(PWM_PATH "/period", "r");
    if (period_f == NULL) {
        perror("Failed to open PWM period file");
        return 3;
    }
    FILE *channels[NUM_CHANNELS] = {NULL};
    for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
        // TODO: Loop-open channels and duty cycles
        // On open failure, goto cleanup_files
    }
    dev_fprintf(period_f, xstr(PERIOD));

    // Prepare to catch interrupts
    signal(SIGINT, ctrl_c);

    // Main control loop
    printf("Control loop running; interrupt to exit...");
    while (!interrupted) {
        /* Both register sets are fixed-point, and happen to have the same
         * number of fractional bits. Were this not the case, bit shifting
         * would be needed.
         */
        for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
            // TODO: Loop-write channels and duty cycles
        }
        // NOTE: No waiting here. Time to eat the CPU for breakfast!
    }

    // Cleanup
    dev_fprintf(period_f, "0");
cleanup_files:
    for (unsigned int i = 0; i < NUM_CHANNELS; i++) {
        // TODO: Loop-close channels and duty cycles
    }
    fclose(period_f);
    return 0;
}
