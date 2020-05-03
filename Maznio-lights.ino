#include <avr/sleep.h>

#define DEBUG
#define BREAKPEDAL 10
#define DAYLIGHTS 9
#define LIGHTS 8
#define HIGHBEAMS 7
#define ACC 6
#define IGNITION 5
#define START 4
#define intPin 2
#define LIGHTSENSOR A0

// States
int daylights = 0;
int lights = 0;
int highbeams = 0;
int acc = 0;
int ignition = 0;
int lightsensor = 0;
int break_pedal = 0;

bool car_started = false;
bool button_long_active= false;
bool button_active = false;
long button_timer = 0;
long button_long_time = 2000;
long starter_started = 0;
long start_lights = 0;
long stop_lights = 0;

int loop_count = 0;


void setup() {
#ifdef DEBUG
	Serial.begin(38400);
#endif
	pinMode(13, OUTPUT);  // Set lepPin - 9 pin as an output
	pinMode(LIGHTSENSOR, INPUT);// Set pResistor - A0 pin as an input (optional)

	pinMode(DAYLIGHTS, OUTPUT);
	pinMode(LIGHTS, OUTPUT);
	pinMode(HIGHBEAMS, OUTPUT);
	pinMode(ACC, OUTPUT);
	pinMode(IGNITION, OUTPUT);
	pinMode(START, INPUT);
	pinMode(BREAKPEDAL, INPUT);

	pinMode(intPin, INPUT_PULLUP);
}

bool have_light() {
	lightsensor = analogRead(LIGHTSENSOR);
  
	//You can change value "25"
#ifdef DEBUG
	Serial.println(lightsensor);
#endif
	if (lightsensor > 300){
#ifdef DEBUG
		Serial.println("Turn OFF");
#endif
		return true;
	} else{
#ifdef DEBUG
		Serial.println("Turn ON");
#endif
		return false;
	}
}

void wakeup() {
	sleep_disable();	// Disable sleep mode
	detachInterrupt(0);	// Detach the interrupt from intPin(d2) to prevent a loop of interrupts
	start_car();
}

void go_to_sleep() {
	lights_off();
	sleep_enable();
	attachInterrupt(0, wakeup, LOW); 		// attaching an interrupt to intPin(d2)
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);	// Full sleep
	delay(1000);
	sleep_cpu();
}

void lights_on() {
	if (millis() - start_lights < 7)
		return;
	if (!daylights) {
		digitalWrite(DAYLIGHTS, HIGH);
		daylights = 1;
#ifdef DEBUG
		Serial.println("Turrning daylights ON");
#endif
	}
	if (!lights && !have_light()) {
		digitalWrite(LIGHTS, HIGH);
		lights = 1;
#ifdef DEBUG
		Serial.println("Turrning lights ON");
#endif
	}
}

void check_lights() {
	if (!have_light() && !lights) {
		digitalWrite(LIGHTS, HIGH);
#ifdef DEBUG
		Serial.println("Turrning lights ON - checker");
#endif
		lights = 1;
	}
}

void lights_off() {
	if (lights) {
		digitalWrite(LIGHTS, LOW);
		lights = 0;
#ifdef DEBUG
		Serial.println("Turrning lights OFF");
#endif
	}
	if (daylights) {
		digitalWrite(DAYLIGHTS, LOW);
		daylights = 0;
#ifdef DEBUG
		Serial.println("Turrning daylights OFF");
#endif
	}
}

bool start_button() {
	if (digitalRead(START) == HIGH) {
		return true;
	} else {
		return false;
	}
}

void start_car() {
	if (car_started)
		return;
	if (digitalRead(BREAKPEDAL) == HIGH) {
		digitalWrite(ACC, HIGH);
#ifdef DEBUG
		Serial.println("ACC ON");
#endif
		digitalWrite(IGNITION, HIGH);
#ifdef DEBUG
		Serial.println("Ignition ON");
#endif
		starter_started = millis();
		start_lights = millis();
	}
	car_started = true;

}

void stop_starter() {
	if (millis() - starter_started > 7) {
		digitalWrite(IGNITION, LOW);
		starter_started = 0;
#ifdef DEBUG
		Serial.println("Ignition OFF");
#endif
	}
}

void stop_car() {
	// Do not stop already stopped car
	if (!car_started)
		return;
	if (acc) {
		digitalWrite(ACC, LOW);
#ifdef DEBUG
		Serial.println("Engine - turn OFF");
#endif
	} else {
#ifdef DEBUG
		Serial.println("Engine - already OFF");
#endif
	}
	car_started = false;
	stop_lights = millis();
}

void check_button() {
	if (digitalRead(START) == HIGH) {
		if (button_active == false) {
			button_active = true;
			button_timer = millis();
		}
		if ((millis() - button_timer > button_long_time) && (button_long_active == false)) {
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
	lights_on();

	loop_count++;
	delay(1000); // 1s delay
}
