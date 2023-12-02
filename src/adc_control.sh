#!/bin/sh

# Basic controller script to set PWM duty cycles based on ADC readings
# Lucas Ritzdorf
# 12/01/2023
# EELE 467

# Configuration variables
ADC_PATH=/sys/class/misc/adc_controller
PWM_PATH=/sys/class/misc/hps_multi_pwm
PERIOD=0x100 # 2ms
LOOP_DELAY=0.1

# Initialization
echo "$PERIOD" > "$PWM_PATH"/period
# Main control loop
echo "Control loop running; interrupt to exit..."
while true
do
    cat "$ADC_PATH"/channel_0 > "$PWM_PATH"/duty_cycle_1
    cat "$ADC_PATH"/channel_1 > "$PWM_PATH"/duty_cycle_2
    cat "$ADC_PATH"/channel_2 > "$PWM_PATH"/duty_cycle_3
    sleep "$LOOP_DELAY"
done
# Cleanup
echo 0 > "$PWM_PATH"/period
