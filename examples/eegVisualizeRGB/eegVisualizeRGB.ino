/* A simple sketch to demonstrate reading brainwave data via Neurosky headset and
visuialize result using RGB LED
*/
#include "EEGheadset.h"

/*
Quality of EEG signal. Must be less than 200
*/
const uint8_t POOR_QUALITY = 200;
const uint8_t PIN_LED_RGB[3] = {6, 5, 3};

/*
Thresholds for used algorithm. thresholdFirst must be less than thresholdSecond
*/
const uint8_t thresholdFirst = 40;
const uint8_t thresholdSecond = 70;

/*
used metrics: poorQuality, attention, meditation
*/
typedef struct METRIC {
	uint8_t lastValue, currentValue;
} METRIC;

METRIC poorQuality = {0};
METRIC attention = {0};
METRIC meditation = {0};

EEGheadset eeg_headset(Serial1);

void traffic() {
	if(attention.lastValue==100) setRGB(HIGH, HIGH, HIGH);
		else if(attention.lastValue>=thresholdSecond) setRGB(LOW, LOW, HIGH);
			else if(attention.lastValue>=thresholdFirst) setRGB(LOW, HIGH, LOW);
				else setRGB(HIGH, LOW, LOW);
}

void setRGB(uint8_t R, uint8_t G, uint8_t B) {
	digitalWrite(PIN_LED_RGB[0], R);
	digitalWrite(PIN_LED_RGB[1], G);
	digitalWrite(PIN_LED_RGB[2], B);
}

void setup() {
	eeg_headset.begin(115200);
	eeg_headset.useAutoConnect = true;

	for(uint8_t i=0;i<3;i++) 
		pinMode(PIN_LED_RGB[i], OUTPUT);

	setRGB(LOW, LOW, LOW);
}

void loop() {
	eeg_headset.getData();
	poorQuality.lastValue = eeg_headset.getPoorQuality();
	if(poorQuality.lastValue<POOR_QUALITY) {
		attention.currentValue = eeg_headset.getAttention();
		if(attention.currentValue!=attention.lastValue) {
			attention.lastValue = attention.currentValue;
			traffic();
		} else attention.currentValue = 0;	
	} else setRGB(LOW, LOW, LOW);
}

