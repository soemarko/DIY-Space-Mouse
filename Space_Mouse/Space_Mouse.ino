#include <Mouse.h>
#include <Keyboard.h>

struct Joystick {
	const uint8_t VCCpin;
	const uint8_t GNDpin;
	const uint8_t Xpin;
	const uint8_t Ypin;
	const uint8_t SWpin;
};

Joystick joy = {15, A3, A2, A1, A0}; // VCC is throwaway, it's connected directly to VCC PIN

const int calibration = 220;	// adjust speed, lower value means faster movement
#define THRESHOLD 5
int yOffset, xOffset;
int yValue, xValue;
bool swState, swStatePrev = HIGH;
bool isPanning = false;

struct Encoder {
	const uint8_t CLK;
	const uint8_t DT;
	const uint8_t SW;
	const uint8_t VCC;
	const uint8_t GND;
};

Encoder enc = {3, 4, 2, 5, 6}; // CLK and SW needs to be on 2 and 3, for attach interrupt

bool currentStateCLK, lastStateCLK;

void encoder_scroll() {
	currentStateCLK = digitalRead(enc.CLK);

	if (currentStateCLK != lastStateCLK  && currentStateCLK == HIGH){
		if (digitalRead(enc.DT) != currentStateCLK) {
			Serial.println("scroll up");
			Mouse.move(0, 0, 1);
		}
		else {
			Serial.println("scroll down");
			Mouse.move(0, 0, -1);
		}
	}

	lastStateCLK = currentStateCLK;
}

void encoder_click() {
	Serial.println("encoder click");
	Mouse.click(MOUSE_MIDDLE);			// double middle click mouse
	Mouse.click(MOUSE_MIDDLE);			// zoom to fit
}

void setup() {
//	Serial.begin(115200);
//	Serial.println();

	// put your setup code here, to run once:
	pinMode(joy.VCCpin, OUTPUT);
	pinMode(joy.GNDpin, OUTPUT);
	digitalWrite(joy.VCCpin, HIGH);
	digitalWrite(joy.GNDpin, LOW);

	pinMode(joy.Xpin, INPUT);
	pinMode(joy.Ypin, INPUT);
	pinMode(joy.SWpin, INPUT_PULLUP);

	pinMode(enc.VCC, OUTPUT);
	pinMode(enc.GND, OUTPUT);
	digitalWrite(enc.VCC, HIGH);
	digitalWrite(enc.GND, LOW);

	pinMode (enc.CLK, INPUT);
	pinMode (enc.DT, INPUT);
	pinMode (enc.SW, INPUT_PULLUP);

	attachInterrupt(digitalPinToInterrupt(enc.CLK), encoder_scroll, CHANGE);
	attachInterrupt(digitalPinToInterrupt(enc.SW), encoder_click, FALLING);

	delay(10);

	yOffset = analogRead(joy.Ypin);		// read center values
	xOffset = analogRead(joy.Xpin);

	Mouse.begin();
	Keyboard.begin();
}

void loop() {
	// put your main code here, to run repeatedly:
	yValue = analogRead(joy.Ypin) - yOffset;
	xValue = analogRead(joy.Xpin) - xOffset;

//	Serial.println(digitalRead(joy.SWpin));
//	Serial.print(xValue);
//	Serial.print(",");
//	Serial.println(yValue);

	swState = digitalRead(joy.SWpin);
	if(swState == HIGH && swStatePrev == LOW) {
		Serial.println("joystick click");
		isPanning = !isPanning;		// toggle panning / orbiting
	}
	swStatePrev = swState;

	if (xValue > THRESHOLD || xValue < -THRESHOLD) {
		if(!isPanning) Keyboard.press(KEY_LEFT_SHIFT);	// orbiting
		Mouse.press(MOUSE_MIDDLE);
		Mouse.move(xValue/calibration, 0, 0);
	}

	if (yValue > THRESHOLD || yValue < -THRESHOLD) {
		if(!isPanning) Keyboard.press(KEY_LEFT_SHIFT);
		Mouse.press(MOUSE_MIDDLE);
		Mouse.move(0, yValue/calibration, 0);
	}

	if (yValue <= THRESHOLD && yValue >= -THRESHOLD &&
		 xValue <= THRESHOLD && xValue >= -THRESHOLD) {
		Keyboard.releaseAll();
		Mouse.release(MOUSE_MIDDLE);
	}

	delay(10);
}
