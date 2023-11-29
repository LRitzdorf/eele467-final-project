# EELE 467 Final Project

**Author:** Lucas Ritzdorf


## Summary

This repository is the home of an FPGA-based final project for Montana State University's SoC FPGAs I course.

This project implements:
- Basic ADC and PWM control
  - A driver for an ADC interface module (Intel's IP) in the FPGA fabric
  - A custom multi-channel PWM module in the FPGA fabric
  - A driver for the PWM module
  - Userspace programs (C and shell) to drive the PWM module based on ADC channel values
- Advanced accelerometer-based control
  - Hardware reconfiguration steps to enable the DE10-Nano's on-board accelerometer
  - Userspace control programs (C and shell)


## Structure

- 📁`hw`: VHDL hardware design files
- 📁`quartus`: Intel Quartus Prime project files
- 📁`boot`: Documentation and scripts related to the HPS boot process
- 📁`linux`: Sources and scripts related to the Linux kernel and device tree
- 📁`src`: Software sources and scripts to facilitate interaction with hardware
- 📁`docs`: Project documentation, homework write-ups, etc.
- 📁`figures`: Graphical resources, usually for reference from within `docs`
