#include "HX711.h"

HX711::HX711(byte dout, byte pd_sck, byte gain) {
	PD_SCK 	= pd_sck;
	DOUT 	= dout;

	GAIN = 1;
	pinsConfigured = false;
}

HX711::~HX711(){

}

bool HX711::is_ready() {
	if (!pinsConfigured) {
		// We need to set the pin mode once, but not in the constructor
		pinMode(PD_SCK, OUTPUT);
		pinMode(DOUT, INPUT);
		pinsConfigured = true;
	}
	return digitalRead(DOUT) == LOW;
}

void HX711::set_gain(byte gain) {
	switch (gain) {
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
	}

	digitalWrite(PD_SCK, LOW);
	read();
}

/*
long HX711::read() {
	// wait for the chip to become ready
	while (!is_ready());

	unsigned long value = 0;
	byte data[3] = { 0 };
	byte filler = 0x00;

	// pulse the clock pin 24 times to read the data
	data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// set the channel and the gain factor for the next reading using the clock pin
	for (unsigned int i = 0; i < GAIN; i++) {
		digitalWrite(PD_SCK, HIGH);
		digitalWrite(PD_SCK, LOW);
	}

	// Data sheet indicates the value is returned as a two's complement value
	// Flip all the bits
	//data[2] = ~data[2];
	//data[1] = ~data[1];
	//data[0] = ~data[0];

	// Replicate the most significant bit to pad out a 32-bit signed integer
	if ( data[2] & 0x80 ) {
		filler = 0xFF;
	} else if ((0x7F == data[2]) && (0xFF == data[1]) && (0xFF == data[0])) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( static_cast<unsigned long>(filler) << 24
	| static_cast<unsigned long>(data[2]) << 16
	| static_cast<unsigned long>(data[1]) << 8
	| static_cast<unsigned long>(data[0]) );

	// ... and add 1
	return static_cast<long>(++value);
}*/

long HX711::read() {
	// wait for the chip to become ready
	while (!is_ready());

    unsigned long value = 0;
    byte data[3] = { 0 };
    byte filler = 0x00;

	// pulse the clock pin 24 times to read the data
    data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
    data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
    data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// set the channel and the gain factor for the next reading using the clock pin
	for (unsigned int i = 0; i < GAIN; i++) {
		digitalWrite(PD_SCK, HIGH);
		digitalWrite(PD_SCK, LOW);
	}
	
	data[2] ^= 0x80;
	return ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];    
}

void HX711::power_down() {
	digitalWrite(PD_SCK, HIGH);
}

void HX711::power_up() {
	digitalWrite(PD_SCK, LOW);
}

void HX711::reset(){
	power_down();
	delayMicroseconds(100);
	power_up();
}







