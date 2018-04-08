
#ifndef _SCALE_h
#define _SCALE_h

/*
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif*/
#include "HX711.h"
#include "Filter.h"
#include "BrowserServer.h"

#define PAGE_FILE			"/calibr.html"
#define CDATE_FILE			"/cdate.json"

#define WEIGHT_MAX_JSON		"mw_id"
#define OFFSET_JSON			"ofs"
#define AVERAGE_JSON		"av_id"
#define STEP_JSON			"st_id"
#define ACCURACY_JSON		"ac_id"
#define SCALE_JSON			"scale"
#define FILTER_JSON			"fl_id"
#define SEAL_JSON			"sl_id"
#define USER_JSON			"us_id"
#define PASS_JSON			"ps_id"

typedef struct {
	long offset;		/*  */
	unsigned char average;				/*  */
	unsigned char step;					/*  */
	int accuracy;					/*  */
	//unsigned char w_filter; /*! Значение для фильтра от 1-100 % */
	unsigned int max;					/*  */
	float scale;
	int seal;
	String user;
	String password;
}t_scales_value;

typedef struct{
	bool isSave;
	float value;
}t_save_value;

class BrowserServerClass;

class ScaleClass : public HX711 , public ExponentialFilter<long>{
	private:
		float _weight;
		char _buffer[10];
	protected:
		BrowserServerClass *_server;
		//char _buffer[10];
		bool _authenticated;
		bool stableWeight;
		t_save_value saveWeight;
		t_scales_value _scales_value;
		float _round;						/* множитиль для округления */
		float _stable_step;					/* шаг для стабилизации */
		//float _referenceWeight;				/*  */
		//long _calibrateWeightValue;			/*  */
				
			
		bool _downloadValue();

	public:
		ScaleClass(byte, byte);
		~ScaleClass();
		void setup(BrowserServerClass *server/*, const char * username, const char * password*/);	
		bool saveDate();
		void saveValueCalibratedHttp(AsyncWebServerRequest *);
		//void handleWeight(AsyncWebServerRequest*);
		void fetchWeight();
		void mathScale(float referenceW, long calibrateW);
		void mathRound();
		//void setScale(d_type scale = 1.f){_scales_value.scale = scale;};
		//d_type getScale(){return _scales_value.scale;};
		char * getBuffer(){return _buffer;};
		float getTest(){return _weight;};
		void setTest(float f){_weight = f;};	
		void setOffset(long offset = 0){_scales_value.offset = offset;};
		long getOffset(){return _scales_value.offset;};
		void init();	
		long readAverage();
		long getValue();
		void setAverage(unsigned char);
		unsigned char getAverage(){return _scales_value.average;};	
		void setSeal(int s){_scales_value.seal = s; };
		int getSeal(){ return _scales_value.seal;};	
		BrowserServerClass *getServer(){ return _server;};
		void detectStable(float);
		
		float getUnits();
		float getWeight();
		void tare();
		
		void formatValue(float value, char* string);
		float getStableStep(){return _stable_step;};
		
		float getRound(){return _round;};
		bool getStableWeight(){return stableWeight;};
		void setStableWeight(bool s){stableWeight = s;};
			
		float isSave(){return saveWeight.isSave;};
		float getSaveValue(){return saveWeight.value;};
		void setIsSave(bool s){saveWeight.isSave = s;};
		
		float forTest(uint32_t h);		
};

extern ScaleClass Scale;
void handleSeal(AsyncWebServerRequest*);
void handleSlave(AsyncWebServerRequest*);

#endif

