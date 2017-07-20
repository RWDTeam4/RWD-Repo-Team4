# RWD-Repo
A code repo for Arduino code written to control the Robot

This robot consists of three main modules:
* Bluetooth
* Master Control
* Motor Control

Circuitry diagrams will hopefully soon follow to show anyone who discovers this repository what the actual build situation is.

## Libraries Required
* https://github.com/adafruit/Adafruit_NeoPixel
* https://github.com/felis/USB_Host_Shield_2.0


## Control Scheme
* Left Stick -  Mecanum movement on a plane
* Right Stick - X-axis turn left and turn right
* R1 -          Turn on electromagnet if it's not disabled (will self disable if you don't turn it off)
* L1 -          Turn off electromagnet if it's not already disabled
* R2 -          Un-assigned
* L2 -          Un-assigned
* R3 -          Un-assigned
* L3 -          Un-assigned
* Start -       Un-assigned
* Select -      Un-assigned
* Triangle -    Trigger Servo (will disengage clutch after 50 ms itself)
* Square -      Toggle AC Spinup
* X -           Un-assigned
* Circle -      Un-assigned
* PS3 Button -  Disconnect the Controller
