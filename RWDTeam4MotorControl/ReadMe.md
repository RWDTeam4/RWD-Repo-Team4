This file is made with the intent of replacing the original motor controller. It follows an algorithm that attempts to minimize the amount of time reading serial commands will interrupt the general function of running the pulse widths for each motor.

Algorithm:
1) Check for Disabled - if disabled do nothing at all
2) Check current time against motor timer pulse width
3) ... To be completed when I'm not exhausted


Future iterations will need to translate pin state of encoders into readings for the primary hub to process. Currently open loop control
-Robert
