
#include "Terminal.h"
#include "Scales.h"


TerminalClass::TerminalClass(String name) : _name(name){
	
}

TerminalClass::~TerminalClass(){}
	
/*  */
void TerminalClass::detectStable(){	
	if (_weight_temp - STABLE_DELTA_STEP <= _weight && _weight_temp + STABLE_DELTA_STEP >= _weight && _weight != 0) {
		if (_stable_num <= STABLE_NUM_MAX){
			if (_stable_num == STABLE_NUM_MAX) {			
				if (!isStable){									
					SCALES.saveEvent("weight", String(SCALES.getWeight())+"_kg");
					isStable = true;	
				}
				return;
			}
			_stable_num++;
		}
	} else {
		_stable_num=0;
		isStable = false;
	}
	_weight_temp = _weight;
}



void TerminalClass::init(){
}







