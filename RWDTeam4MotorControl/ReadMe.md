This file is made with the intent of replacing the original motor controller. It follows an algorithm that attempts to minimize the amount of time reading serial commands will interrupt the general function of running the pulse widths for each motor. The only reason I'm doing this is to make it possible to calculated encoder values for each motor.

Generalized Logic
1) Check for Disabled - if disabled do nothing at all
2) Get current time
3) Reset Command read structure if command completed processing
4) Check current time against each motor pulse width timer
5) Read a character into the buffer if available
6) If a single motor command is ready, process the command
7) Set appropriate motor with the command sent (Stop, foward, reverse, duty cycle)
8) Flip pulse values (High/Low) and reset timers
9) Send motor direction shift chip output

Future iterations will need to translate pin state of encoders into readings for the primary hub to process. Currently open loop control
-Robert
