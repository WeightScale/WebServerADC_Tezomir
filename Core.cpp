#include <ESP8266HTTPClient.h>
#include <FS.h>
#include <ArduinoJson.h>
#include "Core.h"
#include "DateTime.h"
#include "BrowserServer.h"
#include "HttpUpdater.h"
#include "web_server_config.h"

CoreClass * CORE;
BatteryClass BATTERY;

CoreClass::CoreClass(const String& username, const String& password):
_username(username),
_password(password),
_authenticated(false){
	begin();
}

CoreClass::~CoreClass(){}

bool CoreClass::canHandle(AsyncWebServerRequest *request){	
	if(request->url().equalsIgnoreCase("/settings.html")){
		goto auth;
	}
#if HTML_PROGMEM
	else if(request->url().equalsIgnoreCase("/sn")){
		goto auth;
	}
#endif
	else
		return false;
	auth:
		if (!request->authenticate(_settings.scaleName.c_str(), _settings.scalePass.c_str())){
			if(!request->authenticate(_username.c_str(), _password.c_str())){
				request->requestAuthentication();
				return false;
			}
		}
		return true;
}

void CoreClass::handleRequest(AsyncWebServerRequest *request){
	if (request->args() > 0){		
		String message = " ";
		if (request->hasArg("ssid")){
			_settings.autoIp = false;
			if (request->hasArg("auto"))
				_settings.autoIp = true;
			else
				_settings.autoIp = false;
			_settings.scaleLanIp = request->arg("lan_ip");			
			_settings.scaleGateway = request->arg("gateway");
			_settings.scaleSubnet = request->arg("subnet");
			_settings.scaleWlanSSID = request->arg("ssid");
			_settings.scaleWlanKey = request->arg("key");	
			goto save;
		}		
		if(request->hasArg("data")){
			DateTimeClass DateTime(request->arg("data"));
			Rtc.SetDateTime(DateTime.toRtcDateTime());
			request->send(200, TEXT_HTML, getDateTime());
			return;
		}
		if (request->hasArg("host")){
			_settings.hostUrl = request->arg("host");
			_settings.hostPin = request->arg("pin").toInt();
			goto save;	
		}
		if (request->hasArg("n_admin")){
			_settings.scaleName = request->arg("n_admin");
			_settings.scalePass = request->arg("p_admin");
			goto save;
		}		
		save:
		if (saveSettings()){
			if (request->hasArg("n_admin")){
				goto url;
			}
			return request->send(200, TEXT_HTML, "OK");
		}
		return request->send(400);
	}
	url:	
	#if HTML_PROGMEM
		request->send_P(200,F(TEXT_HTML), settings_html);
	#else
		if(request->url().equalsIgnoreCase("/sn")){
			request->send_P(200, F(TEXT_HTML), netIndex);
		}else
			request->send(SPIFFS, request->url());
	#endif
		
}

void CoreClass::begin(){		
	Rtc.Begin();
	_downloadSettings();
	BATTERY.setMax(_settings.bat_max);
	if(BATTERY.callibrated()){		
		_settings.bat_max = BATTERY.getMax();
		saveSettings();	
	};	
}

bool CoreClass::saveEvent(const String& event, const String& value) {
	String date = getDateTime();
	bool flag = WiFi.status() == WL_CONNECTED?eventToServer(date, event, value):false;
	File readFile;
	readFile = SPIFFS.open("/events.json", "r+");
    if (!readFile) {        
        readFile.close();
		if (!SPIFFS.exists("/events.json")){
			readFile = SPIFFS.open("/events.json", "w+");	
		}else{
			return false;	
		}
    }
	
    size_t size = readFile.size(); 	
    std::unique_ptr<char[]> buf(new char[size]);
    readFile.readBytes(buf.get(), size);	
    readFile.close();
		
    //DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(110));
	DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(110));
	JsonObject& json = jsonBuffer.parseObject(buf.get());

    if (!json.containsKey(EVENTS_JSON)) {	
		json["cur_num"] = 0;
		json["max_events"] = MAX_EVENTS;
		JsonArray& events = json.createNestedArray(EVENTS_JSON);
		for(int i = 0; i < MAX_EVENTS; i++){
			JsonObject& ev = jsonBuffer.createObject();
			ev["d"] = "";
			ev["v"] = "";
			ev["s"] = false;
			events.add(ev);	
		}		
		/*if (!json.success())
			return false;*/
    }
	
	long n = json["cur_num"];
	
	json[EVENTS_JSON][n]["d"] = date;
	json[EVENTS_JSON][n]["v"] = value;	
	json[EVENTS_JSON][n]["s"] = flag;
		
	if ( ++n == MAX_EVENTS){
		n = 0;
	}
	json["max_events"] = MAX_EVENTS;
	json["cur_num"] = n;
	//TODO add AP data to html
	File saveFile = SPIFFS.open("/events.json", "w");
	if (!saveFile) {
		SPIFFS.remove("/events.json");
		//saveFile.close();
		return false;
	}

	json.printTo(saveFile);
	saveFile.flush();
	saveFile.close();
	return true;
}



String CoreClass::getIp(){	
	HTTPClient http;	
	http.begin("http://sdb.net.ua/ip.php");
	http.setTimeout(_settings.timeout);	
	int httpCode = http.GET();
	String ip = http.getString();
	http.end();	
	if(httpCode == HTTP_CODE_OK){		
		return ip;
	}	
	return String(httpCode);
}

/* */	
bool CoreClass::eventToServer(const String& date, const String& type, const String& value){
	if (_settings.hostPin == 0){
		return false;
	}
	HTTPClient http;
	String message = "http://";
	message += _settings.hostUrl.c_str();
	String hash = getHash(_settings.hostPin, date, type, value);	
	message += "/scales.php?hash=" + hash;
	http.begin(message);
	http.setTimeout(_settings.timeout);
	int httpCode = http.GET();
	http.end();
	if(httpCode == HTTP_CODE_OK) {
		return true;
	}
	return false;
}


/*
void CoreClass::handleSetAccessPoint(AsyncWebServerRequest * request){	
	if (request->hasArg("ssids")){
		_settings.autoIp = true;
		_settings.scaleWlanSSID = request->arg("ssids");
		_settings.scaleWlanKey = request->arg("key");
	}
	AsyncWebServerResponse *response;	
	if (saveSettings()){
		response = request->beginResponse(200, TEXT_HTML, successResponse);
		response->addHeader("Connection", "close");
		request->onDisconnect([](){ESP.reset();});
	}else{
		response = request->beginResponse(400);
	}
	request->send(response);	
}*/

#if! HTML_PROGMEM
void CoreClass::saveValueSettingsHttp(AsyncWebServerRequest *request) {	
	if (!browserServer.isAuthentified(request))
		return request->requestAuthentication();
	if (request->args() > 0){	// Save Settings
		String message = " ";
		if (request->hasArg("ssid")){
			_settings.autoIp = false;
			if (request->hasArg("auto"))
				_settings.autoIp = true;
			else
				_settings.autoIp = false;
			_settings.scaleLanIp = request->arg("lan_ip");			
			_settings.scaleGateway = request->arg("gateway");
			_settings.scaleSubnet = request->arg("subnet");
			//Serial.println("Save ssid");
			//CORE.setSSID(request->arg("ssid"));		
			_settings.scaleWlanSSID = request->arg("ssid");			
			_settings.scaleWlanKey = request->arg("key");	
			goto save;
		}
		
		if(request->hasArg("data")){
			DateTimeClass DateTime(request->arg("data"));
			Rtc.SetDateTime(DateTime.toRtcDateTime());
			request->send(200, TEXT_HTML, getDateTime());
			return;	
		}
		if (request->hasArg("host")){
			_settings.hostUrl = request->arg("host");
			_settings.hostPin = request->arg("pin");
			goto save;	
		}
		if (request->hasArg("n_admin")){
			_settings.scaleName = request->arg("n_admin");
			_settings.scalePass = request->arg("p_admin");
			goto save;
		}		
		save:
		if (saveSettings()){
			return request->send(200, TEXT_HTML, "OK");
		}
		return request->send(400);	
	} 		
	request->send(SPIFFS, request->url());
}
#endif


String CoreClass::getHash(const int code, const String& date, const String& type, const String& value){
	
	String event = String(code);
	event += "\t" + date + "\t" + type + "\t" + value;
	int s = 0;
	for (int i = 0; i < event.length(); i++)
		s += event[i];
	event += (char) (s % 256);
	String hash = "";
	for (int i = 0; i < event.length(); i++){
		int c = (event[i] - (i == 0? 0 : event[i - 1]) + 256) % 256;
		int c1 = c / 16; int c2 = c % 16;
		char d1 = c1 < 10? '0' + c1 : 'a' + c1 - 10;
		char d2 = c2 < 10? '0' + c2 : 'a' + c2 - 10;
		hash += "%"; hash += d1; hash += d2;
	} 
	return hash;
}

bool CoreClass::saveSettings() {	
	File serverFile = SPIFFS.open(SETTINGS_FILE, "w+");
	if (!serverFile) {
		serverFile.close();
		return false;
	}
	
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	if (!json.containsKey(SCALE_JSON)) {
		JsonObject& scale = json.createNestedObject(SCALE_JSON);
	}
	
	json[SCALE_JSON]["id_n_admin"] = _settings.scaleName;
	json[SCALE_JSON]["id_p_admin"] = _settings.scalePass;
	json[SCALE_JSON]["id_auto"] = _settings.autoIp;
	json[SCALE_JSON]["id_lan_ip"] = _settings.scaleLanIp;
	json[SCALE_JSON]["id_gateway"] = _settings.scaleGateway;
	json[SCALE_JSON]["id_subnet"] = _settings.scaleSubnet;
	json[SCALE_JSON]["id_ssid"] = _settings.scaleWlanSSID;
	json[SCALE_JSON]["id_key"] = _settings.scaleWlanKey;
	json[SCALE_JSON]["bat_max"] = _settings.bat_max;	
	
	if (!json.containsKey(SERVER_JSON)) {
		JsonObject& server = json.createNestedObject(SERVER_JSON);
	}
	
	json[SERVER_JSON]["id_host"] = _settings.hostUrl;
	json[SERVER_JSON]["id_pin"] = _settings.hostPin;
	json[SERVER_JSON]["timeout"] = _settings.timeout;

	json.printTo(serverFile);
	serverFile.flush();
	serverFile.close();
	return true;
}

bool CoreClass::_downloadSettings() {
	_settings.scaleName = "admin";
	_settings.scalePass = "1234";
	_settings.autoIp = true;
	_settings.scaleLanIp = "192.168.1.100";
	_settings.scaleGateway = "192.168.1.1";
	_settings.scaleSubnet = "255.255.255.0";
	_settings.hostUrl = HOST_URL;
	_settings.hostPin = 0;
	_settings.timeout = TIMEOUT_HTTP;
	_settings.bat_max = MIN_CHG;
	File serverFile;
	if (SPIFFS.exists(SETTINGS_FILE)){
		serverFile = SPIFFS.open(SETTINGS_FILE, "r");	
	}else{
		serverFile = SPIFFS.open(SETTINGS_FILE, "w+");
	}
	
	if (!serverFile) {			
		serverFile.close();
		return false;
	}
	size_t size = serverFile.size();

	// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);
	
	serverFile.readBytes(buf.get(), size);
	serverFile.close();
	DynamicJsonBuffer jsonBuffer(size);
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		return false;
	}
	if (json.containsKey(SCALE_JSON)){
		_settings.scaleName = json[SCALE_JSON]["id_n_admin"].as<String>();
		_settings.scalePass = json[SCALE_JSON]["id_p_admin"].as<String>();
		_settings.autoIp = json[SCALE_JSON]["id_auto"];
		_settings.scaleLanIp = json[SCALE_JSON]["id_lan_ip"].as<String>();
		_settings.scaleGateway = json[SCALE_JSON]["id_gateway"].as<String>();
		_settings.scaleSubnet = json[SCALE_JSON]["id_subnet"].as<String>();
		_settings.scaleWlanSSID = json[SCALE_JSON]["id_ssid"].as<String>();
		_settings.scaleWlanKey = json[SCALE_JSON]["id_key"].as<String>();
		_settings.bat_max = json[SCALE_JSON]["bat_max"];	
	}
	if (json.containsKey(SERVER_JSON)){
		_settings.hostUrl = json[SERVER_JSON]["id_host"].as<String>();
		_settings.hostPin = json[SERVER_JSON]["id_pin"];	
		_settings.timeout = json[SERVER_JSON]["timeout"];	
	}	
	return true;
}



/*
void powerOff(){
	browserServer.stop();
	SPIFFS.end();
	Scale.power_down(); /// Выключаем ацп
	digitalWrite(EN_NCP, LOW); /// Выключаем стабилизатор
	ESP.reset();
}*/

void reconnectWifi(AsyncWebServerRequest * request){
	AsyncWebServerResponse *response = request->beginResponse_P(200, PSTR(TEXT_HTML), "RECONNECT...");
	response->addHeader("Connection", "close");
	request->onDisconnect([](){
		SPIFFS.end();
		ESP.reset();});
	request->send(response);
}

int BatteryClass::fetchCharge(int times){
	_charge = _get_adc(times);
	_charge = constrain(_charge, MIN_CHG, _max);
	_charge = map(_charge, MIN_CHG, _max, 0, 100);
	return _charge;
}

int BatteryClass::_get_adc(byte times){
	long sum = 0;
	for (byte i = 0; i < times; i++) {
		sum += analogRead(A0);
	}
	return times == 0?sum :sum / times;	
}

bool BatteryClass::callibrated(){
	bool flag = false;
	int charge = _get_adc();	
	int t = _max;
	_max = constrain(t, MIN_CHG, 1024);
	if(t != _max){
		flag = true;	
	}
	charge = constrain(charge, MIN_CHG, 1024);
	if (_max < charge){
		_max = charge;	
		flag = true;
	}
	return flag;
}






