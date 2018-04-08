
#ifndef _TERMINAL_h
#define _TERMINAL_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <functional>

#define CR						0xd
#define LF						0xa
#define szCR_LF					"\x0D\x0A"

#define STABLE_NUM_MAX 100
#define STABLE_DELTA_STEP 10

class TerminalClass{
	protected:
		String _name;
		String inputString = "";							/*  */
		//double _weight, _weight_temp;
		int _weight=-1, _weight_temp;
		unsigned char _stable_num = 0;
		bool isStable = false;
		
		void detectStable();		

	public:
		TerminalClass(String);
		~TerminalClass();
		String getName(){return _name;};
		//String getweight();
		void init();		
};


#endif







