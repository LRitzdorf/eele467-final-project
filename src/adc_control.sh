#!/bin/bash

# Basic controller script to set PWM duty cycles based on ADC readings
# Lucas Ritzdorf
# 12/01/2023
# EELE 467

# Configuration variables
SYSID_VERSION=0x3ADC37ED
ADC_PATH=/sys/class/misc/adc_controller
PWM_PATH=/sys/class/misc/hps_multi_pwm
PERIOD=0x100 # 2ms
LOOP_DELAY=0.1


# Check System ID

# Scan for valid device files
shopt -s nullglob  # Return empty list if glob fails to match
sysids=( /sys/bus/platform/devices/*.sysid/sysid/id )
if [ "${#sysids[@]}" -lt 1 ]
then
    echo "No System ID device files found!"
    exit 1
fi

# Check device files for matching ID
sysid_match=false
for id in "${sysids[@]}"
do
    if [ $(< $id) -eq $(printf "%d" "$SYSID_VERSION") ]
    then
        sysid_match=true
        break
    fi
done

# Exit if nothing matched
if [ ! $sysid_match ]
then
    printf "No matching System ID found! (Expected 0x%X)\n" "$SYSID_VERSION"
    exit 2
fi


# Initialization
echo "$PERIOD" > "$PWM_PATH"/period

# Prepare to catch interrupts
interrupted=false
trap ctrl_c INT
function ctrl_c() { interrupted=true; }

# Main control loop
echo "Control loop running; interrupt to exit..."
while [ $interrupted = false ]
do
    # Both register sets are fixed-point, and happen to have the same number of
    # fractional bits. Were this not the case, bit shifting would be needed.
    cat "$ADC_PATH"/channel_0 > "$PWM_PATH"/duty_cycle_1
    cat "$ADC_PATH"/channel_1 > "$PWM_PATH"/duty_cycle_2
    cat "$ADC_PATH"/channel_2 > "$PWM_PATH"/duty_cycle_3
    # Don't eat the CPU for breakfast
    sleep "$LOOP_DELAY"
done

# Cleanup
echo 0 > "$PWM_PATH"/period

exit 0
