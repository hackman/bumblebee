The bumblebee project is a controller for automating the control of the lights(daylights, low and high beams) of a car
and also implementing start/stop functionality with a button.

Currently the crank time is hardcoded, it may be possible to be replaced by constant
measurement of the voltage, while the starter motor is cranking, but this has to be
live tested, before I implement that.



This is the code for Arduino, for controlling the lights and starting my car.

When the controller first starts, it is immediately put to sleep. This is to ensure
consistancy between clicks of the start/stop button.

Two different modes of operation:
- If the car is already running
  When the start button is pressed, the controller should:
   - turn off ACC
   - wait for 5min and turn off daylights and lights(low beams)
   - go to sleep
- If the car is not running
  When the start button is pressed, the controller should:
   - if the controller is in sleep mode handle the interrupt and continue
   - check and if ACC is not on and turn it on
   - if the engine is not running and the break pedal is pressed it should turn on the ignition for 7sec
   - if the ignition is not on, turn the daylights on
   - check the photoresistor sensor, if it is determined that it is dark outside, it should turn on the lights(low beams)

Definition for the car is running:
 - the voltage measured from the car is above 13.0V
 - this assumption is based on the fact, that alternator is supposed to charge the battery with 14+ volts

Bill of materials:
- Arduino Uno or Arduino Mini Pro
- Photoresistor
- Button for start/stop
- Button for the break pedal
- 2x 10k Ohm resistor
- 1x 100k Ohm resistor
- One board with 4 relays


Requirements for this project:
 - light measurements
 - voltage measurements
 - detection of break pedal
 - is the shift lever in gear(any gear)
