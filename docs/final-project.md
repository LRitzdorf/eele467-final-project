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

**Userspace Control Software:**
The basic control program, which runs in userspace, is provided in two forms: a Bash script, and a C program.
Both implement essentially the same functionality.
The control programs begin by searching for available Altera System ID devices and checking that a matching ID is present, which allows them to be sure that the expected hardware has indeed been programmed into the FPGA fabric.
They then enter a main loop, in which they simply retrieve readings from the ADC driver and relay them to the PWM driver (via their respective sysfs interfaces).
This effectively allows the user to control the RGB LED's three color channels by varying the ADC's voltage inputs.

### Accelerometer Functionality

In addition to its basic functional mode, the system can operate in an advanced mode which incorporates accelerometer-based control.
When the advanced control program is running, it monitors the accelerometer's event stream for "key" events, which are sent when the accelerometer detects motion consistent with a tap.
This event cues the control program to switch into its advanced control mode, where it:
- Translates raw accelerometer data into attitude parameters (i.e. roll and pitch);
- Composes an [HSL](https://en.m.wikipedia.org/wiki/HSL_and_HSV) color with hue and lightness based on the device's attitude;
- Converts the HSL color to RGB format and relays the resulting color channels to the PWM module.
Subsequent tap events continue to toggle between operational modes.

This project element required that specific pin multiplexing settings within the hard processor system be changed.
This, in turn, required the creation of a custom U-Boot preloader and bootable SD card image.
The details of this process, in the form of a step-by-step guide, are recorded in the [`boot` README](/boot/README.md).


## Conclusion

Given my prior experience with Linux systems in general, and the ease with which I was able to complete the instructional component of this course, implementing the basic functionality of this project was rather straightforward.
The advanced functional mode was more involved, primarily due to the need to recreate an entire bootable SD card image; however, I particularly enjoyed this element of the project.
In my opinion, it was an ideal blend of familiar and novel material, which allowed me to learn far more about the HPS boot process while not being dangerously out of my depth.
In addition, the resulting boot-image guide (linked above) should help to eliminate a source of technical debt from this class, both for present and future instructors.

I do have one minor suggestion regarding the starter material and project templates provided in this course.
While I recognize that our eventual goal (in SoC FPGAs II) is to interface with the AudioMini add-on board, many of the templates used here include cruft from the AudioMini project, which ranges from simply annoying to actively confusing for students.
The GPIO signal naming conventions in the top-level `DE10Nano_System.vhd` file are a key example of the latter case.
If possible, removing or otherwise cleaning up these elements might be helpful for future students.
