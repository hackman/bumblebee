This is the code for Arduino, for controlling the lights and starting my car.

Two different modes of operation:
- If the car is already running
  When the start button is pressed, the controller should:
   - turn off ACC
   - turn off lights(low beams)
   - wait for 5min and turn off daylights
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


Requirements:
 - light measurements
 - voltage measurements
 - detection of break pedal
 - is the car in gear(any gear)
