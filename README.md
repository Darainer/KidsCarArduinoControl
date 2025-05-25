# Four-Wheel-Drive Kids Car (24 V, 4 × BTS7960)

Class-based Arduino driver for the wiring shown in  
![schematic](docs/schematic.png)

## Features
* Drives four BTS7960 H-bridges in parallel
* Three-mode toggle (fast, slow, reverse)
* Throttle via Hall / pot pedal
* Soft start/stop ramp
* Easily portable to any AVR (Nano, Pro Mini, UNO…)

## Quick start

```bash
git clone git@github.com:Darainer/KidsCarArduinoControl.git
cd KidsCarArduinoControl

# compile / upload using the make file
make                           # compile locally → build/FourWD_Demo.hex
make upload PORT=/dev/ttyUSB0  # flash Nano (override PORT if needed)
make monitor PORT=/dev/ttyUSB0 # open 115200‑baud serial console