# TallyRelayBox
RS-422 Tally relay box using the TSL protocol

![assembled board](images/tallybox.jpg)



## Hardware

This design is based on a ATMega324PB.

Power supply can be in the range of 8 to 24V (change relays to your operating voltage!)

## Component sourcing

Most components in this design are widely available standard parts (Mouser etc are your friends for the switchmode regulator and inductor).
If used at 12V a 7805 linear regulator can be fitted as an alternative.
The provided CAD data allows easy adaption to your
local or preferred supplier. The board can be edited in the eagle freeware version if it is shortened by a few millimeters.
Current board size is designed to fit into a cheap plastic enclosure available from Reichelt Electronik in Germany 
(https://www.reichelt.de/kleingehaeuse-135-x-95-x-45-mm-eurobox-sw-p50429.html?&nbc=1). Similar enclosures should be readily available
from several suppliers. The relays used can either be TE  OJE-SS-112LMH or other types with a compatible footprint (like SonChuan 307 series).


## Compiling and flashing

I use avr-gcc 10.2.0 with GNU make on Linux for compiling this project. Your build environment may vary.
You may have to source the device definitions for the Mega324PB from Microchip to be able to compile this project.

See setup_fuses.sh for the fuse setting of the controller.
