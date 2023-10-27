# Final Project Proposal


## Hardware

- Required
  - 12.5 MHz PLL
  - ADC controller
  - Color LED controller (3x PWM)
- Additional
  - Encoder interface
    - Track current position, with option to zero
    - Pass through current (debounced) button state
  - Accelerometer
    - Use built-in IMU on DE10-Nano
    - Need to check how this interfaces with the SoC


## Software

- Modes:
  - Potentiometer control
    - As described in project requirements
  - Accelerometer control
    - Read acceleration from IMU, perform double integration to find position along all axes
    - Use position to control LED channels
    - Scale "Δx to ΔLED" conversion using encoder
  - Transition between modes on encoder button press


## Extra Credit

- Start with acceleration-based LED control, implement double integration as extra credit?
- Add relative encoder interface mode
  - Extra control register to switch between modes
  - Accumulate number of steps and net direction since last read
  - Accumulate number of button presses
