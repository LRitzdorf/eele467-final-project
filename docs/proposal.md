# Final Project Proposal


## Hardware

- Encoder interface
  - Track current position, with option to zero
  - Pass through current (debounced) button state
- IMU
  - Use built-in accelerometer on DE10-Nano
  - Need to check how this interfaces with the SoC


## Software

- Modes:
  - Potentiometer control
    - As described in project requirements
  - IMU control
    - Read from IMU, perform double integration to find position along all axes
    - Use position to control LED channels
    - Scale position-to-LED-channel conversion using encoder
  - Transition between modes based on encoder button


## Extra Credit

- Add relative encoder interface mode
  - Extra control register to switch between modes
  - Accumulate number of steps and net direction since last read
  - Accumulate number of button presses
- Start with acceleration-based LED control, implement double integration as extra credit?
