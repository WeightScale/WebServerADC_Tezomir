// scales.h

#ifndef _CORE_h
#define _CORE_h

#include "TaskController.h"
#include "Task.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <ArduinoJson.h>
#include "Scale.h"
//using namespace ArduinoJson;

#define SETTINGS_FILE "/settings.json"
#define HOST_URL "sdb.net.ua"
#define TIMEOUT_HTTP 3000
#define STABLE_NUM_MAX 10
#define MAX_EVENTS 100
//#define MAX_CHG 1013//980	//делитель U2=U*(R2/(R1+R2)) 0.25
#define MIN_CHG 880			//ADC = (Vin * 1024)/Vref  Vref = 1V

#define EN_NCP  12							/* сигнал включения питания  */
#define PWR_SW  13							/* сигнал от кнопки питания */
#define LED  2								/* индикатор работы */

#define SCALE_JSON		"scale"
#define SERVER_JSON		"server"
//#define DATE_JSON		"date"
#define EVENTS_JSON		"events"

extern TaskController taskController;		/*  */
extern Task taskBlink;								/*  */
extern Task taskBattery;							/*  */
//extern Task taskPower;
extern void connectWifi();

const char netIndex[] PROGMEM = R"(<html><meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'/>
										<body><form method='POST'>
										<input name='ssid'><br/>
										<input type='password' name='key'><br/>
										<input type='submit' value='СОХРАНИТЬ'>
										</form></body></html>)";

typedef struct {	
	bool autoIp;
	String scaleName;
	String scalePass;
	String scaleLanIp;
	String scaleGateway;
	String scaleSubnet;
	String scaleWlanSSID;
	String scaleWlanKey;
	String hostUrl;
	int hostPin;
	int timeout;
	int bat_max;	
} settings_t;

class CoreClass : public AsyncWebHandler{
	private:
	settings_t _settings;
	
	String _username;
	String _password;
	bool _authenticated;
	
	bool saveAuth();
	bool loadAuth();		
	bool _downloadSettings();
			

	public:			
		CoreClass(const String& username, const String& password);
		~CoreClass();
		void begin();
		bool saveSettings();
		String& getNameAdmin(){return _settings.scaleName;};
		String& getPassAdmin(){return _settings.scalePass;};
		String& getSSID(){return _settings.scaleWlanSSID;};
		String& getLanIp(){return _settings.scaleLanIp;};
		String& getGateway(){return _settings.scaleGateway;};
		void setSSID(const String& ssid){_settings.scaleWlanSSID = ssid;};
		void setPASS(const String& pass){_settings.scaleWlanKey = pass;};	
		String& getPASS(){return _settings.scaleWlanKey;};
		bool saveEvent(const String&, const String&);
		void setBatMax(int m){_settings.bat_max = m;};
		String getIp();
		bool eventToServer(const String&, const String&, const String&);
		#if! HTML_PROGMEM
			void saveValueSettingsHttp(AsyncWebServerRequest*);
		#endif			
		void handleSetAccessPoint(AsyncWebServerRequest*);	
		String getHash(const int, const String&, const String&, const String&);
		
		bool isAuto(){return _settings.autoIp;};		
		virtual bool canHandle(AsyncWebServerRequest *request) override final;
		virtual void handleRequest(AsyncWebServerRequest *request) override final;
		virtual bool isRequestHandlerTrivial() override final {return false;}
		
		
};

class BatteryClass{	
	protected:
		unsigned int _charge;
		int _max;
		int _get_adc(byte times = 1);
	
	public:
		BatteryClass(){};
		~BatteryClass(){};
		int fetchCharge(int);
		bool callibrated();		
		void setCharge(unsigned int ch){_charge = ch;};
		unsigned int getCharge(){return _charge;};
		void setMax(int m){_max = m;};	
		int getMax(){return _max;};
};



//void powerOff();
void reconnectWifi(AsyncWebServerRequest*);
extern CoreClass * CORE;
extern BatteryClass BATTERY;

#endif //_CORE_h







