# TallyRelayBox
RS-422 Tally relay box using the TSL protocol


## Hardware

This design is based on a ATMega324PB.

Power supply can be in the range of 8 to 24V (change relays to your operating voltage!)

## Compiling and flashing

I use avr-gcc 10.2.0 with GNU make on Linux for compiling this project. Your build environment may vary.
You may have to source the device definitions for the Mega324PB from Microchip to be able to compile this project.

See setup_fuses.sh for the fuse setting of the controller.
