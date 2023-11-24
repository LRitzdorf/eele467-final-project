# Final Project Proposal


## Hardware

- Required
  - 12.5 MHz PLL
  - ADC controller
  - Color LED controller (3x PWM)
- Additional
  - Accelerometer
    - Use built-in IMU on DE10-Nano
    - Need to check how this interfaces with the SoC


## Software

- Modes:
  - Potentiometer control
    - As described in project requirements
  - Accelerometer control
    - Read acceleration updates from IMU using the Linux kernel's input system
    - Perform double integration to find position along all axes
    - Use position to control LED channels
- Generate custom boot image
  - Custom U-Boot build required to enable IMU interfacing
- Fully integrate kernel modules with the custom boot image
  - i.e. perform automatic module loading at boot time


## Extra Credit

- Start with acceleration-based LED control, implement double integration as extra credit?
