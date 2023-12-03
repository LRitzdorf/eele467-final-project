#!/bin/sh

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
    return 1
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
    return 2
fi


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
