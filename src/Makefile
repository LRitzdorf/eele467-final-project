# Support cross-compilation
CROSS_COMPILE ?= arm-linux-gnueabihf-
CC = $(CROSS_COMPILE)gcc

CFLAGS += -Wall
BUILD_DIR ?= bin/

.PHONY: clean


all: adc_control accel_control

adc_control: adc_control.c | builddir
	$(CC) $(CFLAGS) adc_control.c -o $(BUILD_DIR)adc_control

accel_control: accel_control.c | builddir
	$(CC) $(CFLAGS) accel_control.c -levdev -lm -o $(BUILD_DIR)accel_control

builddir:
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
