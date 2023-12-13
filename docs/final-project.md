# Final Project: Advanced RGB LED Control


## Introduction

This system consists of two primary units:
- A basic functional unit, including:
  - Analog-to-digital converter hardware and associated driver
  - Custom multi-channel PWM hardware and associated driver
  - Userspace control code which pipes ADC readings to the PWM controller
- An advanced functional unit, including:
  - A custom bootloader, tuned to enable I2C on the Cyclone V SoC's hard processor
  - A custom kernel with modular support for the ADXL34x I2C accelerometer
  - Userspace control code which uses accelerometer data to derive RGB colors for display by the PWM controller

In addition, the advanced functional mode encompasses all features of the basic mode as well.
Transitioning between modes is accomplished via the accelerometer's tap detection feature.


## Body

### Basic Operational Flow

In its basic functional mode, the system's operational flow begins with the DE10-Nano board's ADC chip.
This is managed by an in-fabric IP block from Intel, which provides an memory-mappable Avalon interface for communication with the hard processor system.
Proper interaction with userspace code is accomplished via a custom Linux device driver (implemented as a loadable kernel module, in typical fashion), which provides a sysfs-level interface for reading ADC channels and controlling update functionality.
(The ADC controller's design includes some questionable features, such as write-only registers. Where possible, these were mitigated in the driver.)

Once in userspace, the ADC readings are processed into a suitable form for the other element of the basic functional mode: a multi-channel pulse width modulation controller.
Processed values from userspace control code are passed to a second custom device driver, which commands the hardware PWM controller to operate accordingly.
In this case, the module's three PWM outputs are connected to an RGB LED (through appropriately-sized resistors), so that each color channel is managed by a corresponding PWM channel.
Since these PWM channels are themselves controlled (via userspace software) by ADC readings, the system as a whole allows ADC inputs to independently control the LED's three color channels.

### Hardware and Software Details

**PWM Controller:**
The PWM controller is a custom hardware block, implemented in VHDL, which contains multiple PWM generator modules (three in this case, though a generic allows for easy customization).
A PWM module's functionality is relatively straightforward: at its core lies a counter, which rolls over at a particular (computed) value to yield the user's desired output period.
Within each period, the modulated output is determined by comparison between the counter and a second threshold, which is computed based on the period and the user's specified duty cycle.
In addition, PWM modules are designed such that these thresholds are recalculated only at the end of each period — thus, updates to the period and duty cycle can safely be made at any time, and will only take effect once the module has completed its current period.
This is intended to ensure that unexpectedly high-frequency pulses are not generated, even if inputs are changed just after the beginning of a new period.
If immediate updates are desired, the hardware designer may add appropriate reset logic for each module.
All PWM modules within the PWM controller are driven with a shared period, but have individually assigned duty cycles.
Each of these parameters (including the period) is exposed via a memory-mappable Avalon interface, which is designed to interface with the Cyclone V hard processor's lightweight data bus.

**PWM Driver:**
While the PWM controller's registers are exposed on the lightweight Avalon bus, a Linux device driver is needed to facilitate proper interaction from userspace code.
This driver is relatively basic, relative to device drivers in general — it simply exposes each of the four control registers as a corresponding file in the Linux sysfs.
Userspace code may read or write to these pseudo-files, which commands the driver to read or write to the corresponding hardware register.

**ADC Driver:**
While the ADC hardware module is an IP block provided by Intel, it also requires a device driver for proper interaction with Linux userspace.
This driver primarily exposes hardware registers, just as the PWM driver does.
However, one register in particular has something of a twist: the "auto-update" register is write-only in hardware, and thus could be exposed as a write-only sysfs entry.
In this case, though, clever software design mostly mitigates this unusual hardware choice — the driver begins tracking the state of this register upon its first write, allowing for read operations which draw from driver memory rather than actually reading the underlying hardware register.

### Accelerometer Functionality

TODO


## Conclusion

Make a conclusion and relate your experience regarding the creation and execution of your project.
What would you improve regarding the course?
