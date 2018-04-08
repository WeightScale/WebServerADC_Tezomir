#ifndef _HX711_h
#define _HX711_h

#if ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//typedef float d_type;

class HX711 {
	private:
		
		byte PD_SCK;	// Power Down and Serial Clock Input Pin
		byte DOUT;		// Serial Data Output Pin
		byte GAIN;		// amplification factor				
		bool pinsConfigured;

	public:
		// define clock and data pin, channel, and gain factor
		// channel selection is made by passing the appropriate gain: 128 or 64 for channel A, 32 for channel B
		// gain: 128 or 64 for channel A; channel B works with 32 gain factor only
		HX711(byte dout, byte pd_sck, byte gain = 128);
		virtual ~HX711();
		bool is_ready();
		void set_gain(byte gain = 128);
		long read();
		void power_down();
		void power_up();		
		void reset();
};

#endif /* HX711_h */






