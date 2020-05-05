#include <avr/sleep.h>

#define VERSION "0.4"
#define DEBUG
#define INGEAR 11
#define BREAKPEDAL 10
#define DAYLIGHTS 9
#define LIGHTS 8
#define HIGHBEAMS 7
#define ACC 6
#define IGNITION 5
#define START 2
#define INTPIN 2
#define LIGHTSENSOR A0
#define VOLTAGE A1
#define MIN_VOLTAGE 13.0
#define STARTER_TIME 4000
#define LONG_PRESS 1500
#define R1 100000.0 // resistance of R1 (100K) connected to +
#define R2 10000.0  // resistance of R2 (10K) connected to GND on the board and on the voltage source GND
//#define LIGHTS_TURN_OFF_TIME 300000	// 5 minutes
#define LIGHTS_TURN_OFF_TIME 5000
#define DAYLIGHT_VALUE 300

// States
bool daylights = false;
bool lights = false;
bool highbeams = false;
bool acc = false;
bool ignition = false;
bool break_pedal = false;
bool car_started = false;
bool turn_lights_off = false;
bool button_long_active = false;
bool button_active = false;

int  lightsensor = 0;
long button_timer = 0;
long starter_timer = 0;
long stop_lights_timer = 0;

void wakeup() {
	sleep_disable();	// Disable sleep mode
	detachInterrupt(0);	// Detach the interrupt from INTPIN(d2 - INT 0) to prevent a loop of interrupts
#ifdef DEBUG
	Serial.println("Interrupt disabled");
#endif
}

void initController() {
	sleep_enable();
	attachInterrupt(0, wakeup, LOW); 		// attaching an interrupt to INTPIN(d2 - INT 0)
#ifdef DEBUG
	Serial.println("Putting the controller to sleep");
#endif
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);	// Full sleep
	delay(1000);
	sleep_cpu();
#ifdef DEBUG
	Serial.println("Resetting values");
#endif
	// Reset values
	ignition = false;
	acc = false;
	turn_lights_off = false;
	lightsensor = 0;
	button_timer = 0;
	starter_timer = 0;
	stop_lights_timer = 0;
	button_active = false;
	button_long_active = false;
	button_timer = 0;
}

void setup() {
	Serial.begin(38400);
	Serial.println("Starting Maznio ECU1 version " VERSION);
	// Setup inputs
	pinMode(LIGHTSENSOR, INPUT);
	pinMode(INGEAR, INPUT_PULLUP);
	pinMode(BREAKPEDAL, INPUT_PULLUP);
	pinMode(VOLTAGE, INPUT);
	pinMode(INTPIN, INPUT_PULLUP);

	// Setup outputs
	pinMode(DAYLIGHTS, OUTPUT);
	pinMode(LIGHTS, OUTPUT);
	pinMode(HIGHBEAMS, OUTPUT);
	pinMode(ACC, OUTPUT);
	pinMode(IGNITION, OUTPUT);

	initController();
}

bool isCarStarted() {
	int value = 0;
	float result = 0.0;
	value = analogRead(VOLTAGE);

	float voltage = (value * 5.0) / 1023.0; // 5.0V is the voltage of the controller. Here we are converting the analog integer value into the float value of the voltage
	// Because we use a voltage devider, we need to calculate the correct value that we are reading.
	// With R1(100k) and R2(10k) we can measure between 0-55V, so we need to adjust the value to match our reading of 0-5V
	result = voltage / (R2 / (R1 + R2));

	if (result < MIN_VOLTAGE) {
#ifdef DEBUG
		Serial.print("Engine did not start. Voltage: ");
		Serial.println(result);
#endif
		return false;
	} else {
#ifdef DEBUG
		Serial.print("Engine running, voltage: ");
		Serial.println(result);
#endif
		car_started = true;
		return true;
	}
}

bool have_light() {
	lightsensor = analogRead(LIGHTSENSOR);

	if (lightsensor > DAYLIGHT_VALUE){
#ifdef DEBUG
		Serial.print("Daylight detected. Value: ");
		Serial.println(lightsensor);
#endif
		return true;
	} else{
		return false;
	}
}

void lights_on() {
	if (!daylights) {
		digitalWrite(DAYLIGHTS, HIGH);
		daylights = true;
#ifdef DEBUG
		Serial.println("Turrning daylights ON");
#endif
	}
	if (car_started && !lights && !have_light()) {
		digitalWrite(LIGHTS, HIGH);
		lights = true;
#ifdef DEBUG
		Serial.println("Turrning low beams ON");
#endif
	}
}

void check_lights() {
	if (car_started && !have_light() && !lights) {
		digitalWrite(LIGHTS, HIGH);
#ifdef DEBUG
		Serial.println("Turrning low beams ON - checker");
#endif
		lights = true;
	}
}

void lights_off() {
	// Do not try to stop the lights if turn_lights_off is not set by stop_car()
	if (!turn_lights_off)
		return;

	if (millis() - stop_lights_timer > LIGHTS_TURN_OFF_TIME) {
		stop_lights_timer = 0;
		if (lights) {
			digitalWrite(LIGHTS, LOW);
			lights = false;
#ifdef DEBUG
			Serial.println("Turrning lights OFF");
#endif
		}
		if (daylights) {
			digitalWrite(DAYLIGHTS, LOW);
			daylights = false;
#ifdef DEBUG
			Serial.println("Turrning daylights OFF");
#endif
		}
		// Do not put the controller to sleep if the car is still started
		if (!car_started)
			initController();
	}
}

void start_car() {
	// If we try to start the car, during the stop_lights_timer we have to clear the timer
	if (turn_lights_off) {
		stop_lights_timer = 0;
		turn_lights_off = false;
	}
	if (acc) {
#ifdef DEBUG
		Serial.println("ACC already ON");
#endif
	} else {
#ifdef DEBUG
		Serial.println("Turning ACC ON");
#endif
		digitalWrite(ACC, HIGH);
		acc = true;
	}

	// If we are in the ignition phase, don't continue
	if (ignition) {
#ifdef DEBUG
		Serial.println("Ignition motor already ON");
#endif
		return;
	}

	if (car_started) {
#ifdef DEBUG
		Serial.println("Car is already started");
#endif
		return;
	}

	if (digitalRead(BREAKPEDAL) == LOW) {
		if (daylights) {
#ifdef DEBUG
			Serial.println("Turn Daylights OFF");
#endif
			digitalWrite(DAYLIGHTS, LOW);
			daylights = false;
		}

#ifdef DEBUG
    Serial.println("Turning Ignition ON");
#endif
		digitalWrite(IGNITION, HIGH);
		starter_timer = millis();
		ignition = true;
	}
}

void stop_starter() {
	// If the starter motor is not working at the moment, we have nothing to do here
	if (!ignition)
		return;

	// Check if more then STARTER_TIME has passed since the started motor been turned on
	if (millis() - starter_timer > STARTER_TIME) {
		digitalWrite(IGNITION, LOW);
		starter_timer = 0;
		ignition = false;
#ifdef DEBUG
		Serial.println("Turning ignition OFF");
#endif
		if (isCarStarted()) {
			lights_on();
		}
	}
}

void stop_car() {
	if (acc) {
		digitalWrite(ACC, LOW);
		acc = false;
#ifdef DEBUG
		Serial.println("Engine turn OFF");
#endif
	} else {
#ifdef DEBUG
		Serial.println("Engine already OFF");
#endif
	}

	car_started = false;
	turn_lights_off = true;
	delay(1500);
	stop_lights_timer = millis();
}

void check_button() {
	if (digitalRead(START) == LOW) {
		if (button_active == false) {
			button_active = true;
			button_timer = millis();
		}
		if ((millis() - button_timer > LONG_PRESS) && (button_long_active == false)) {
#ifdef DEBUG
			Serial.println("Long press detected");
#endif
			button_long_active = true;
			stop_car();
		}
	} else {
		if (button_active == true) {
			if (button_long_active == true) {
				button_long_active = false;
			} else {
				start_car();
			}
			button_active = false;
		}
	}
}

void loop() {
	check_button();
	stop_starter();
	check_lights();
	lights_off();
}
