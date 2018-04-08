#include <FS.h>
#include <ArduinoJson.h>
#include "Scale.h"
#include "web_server_config.h"

ScaleClass Scale(16,14);		/*  gpio16 gpio0  */

ScaleClass::ScaleClass(byte dout, byte pd_sck) : HX711(dout, pd_sck) , ExponentialFilter<long>(){
	_server = NULL;	
	_authenticated = false;	
	saveWeight.isSave = false;
	saveWeight.value = 0.0;
}

ScaleClass::~ScaleClass(){}
	
void ScaleClass::setup(BrowserServerClass *server){
	init();
	_server = server;
	//_server->on("/wt",HTTP_GET, std::bind(&ScaleClass::handleWeight, Scale, std::placeholders::_1));						/* Получить вес и заряд. */
	/*_server->on("/cb",HTTP_GET,[this](AsyncWebServerRequest * request){
		Serial.println("saveValueCalibratedHttp GET");
		saveValueCalibratedHttp(request);	
	});*/
	/*#if HTML_PROGMEM
		_server->on(PAGE_FILE,[](AsyncWebServerRequest * reguest){	reguest->send_P(200,"text/html",calibr_html);});
	#else
	
	#endif*/
	_server->on(PAGE_FILE, [this](AsyncWebServerRequest * request) {								/* Открыть страницу калибровки.*/
		if(!request->authenticate(_scales_value.user.c_str(), _scales_value.password.c_str()))
			if (!_server->checkAdminAuth(request)){
				return request->requestAuthentication();
			}
		_authenticated = true;
		saveValueCalibratedHttp(request);
	});
	_server->on("/av", [this](AsyncWebServerRequest * request){									/* Получить значение АЦП усредненное. */
		request->send(200, TEXT_HTML, String(readAverage()));
	});
	_server->on("/tp", [this](AsyncWebServerRequest * request){									/* Установить тару. */
		tare();
		request->send(204, TEXT_HTML, "");
	});
	_server->on("/sl", handleSeal);									/* Опломбировать */	
}

void ScaleClass::init(){
	reset();
	_downloadValue();
	mathRound();
	tare();
	SetCurrent(readAverage());
}

void ScaleClass::mathRound(){
	_round = pow(10.0, _scales_value.accuracy) / _scales_value.step; // множитель для округления}
	_stable_step = 1/_round;
}

void ScaleClass::saveValueCalibratedHttp(AsyncWebServerRequest * request) {
	if (request->args() > 0) {
		if (request->hasArg("update")){
			_scales_value.accuracy = request->arg("weightAccuracy").toInt();
			_scales_value.step = request->arg("weightStep").toInt();
			setAverage(request->arg("weightAverage").toInt());
			SetFilterWeight(request->arg("weightFilter").toInt());
			_scales_value.max = request->arg("weightMax").toInt();
			mathRound();
			if (saveDate()){
				goto ok;
			}
			goto err;
		}
		
		if (request->hasArg("zero")){
			_scales_value.offset = readAverage();
		}
		
		if (request->hasArg("weightCal")){
			float rw = request->arg("weightCal").toFloat();			
			long cw = readAverage();
			mathScale(rw,cw);
		}
		
		if (request->hasArg("user")){
			_scales_value.user = request->arg("user");
			_scales_value.password = request->arg("pass");
			if (saveDate()){
				goto url;
			}
			goto err;
		}
		
		ok:
			return request->send(200, TEXT_HTML, "");
		err:
			return request->send(400, TEXT_HTML, "Ошибка ");	
	}
	url:
	#if HTML_PROGMEM
		request->send_P(200,F(TEXT_HTML), calibr_html);
	#else
		request->send(SPIFFS, request->url());
	#endif		
	
	
	//}
}

void ScaleClass::fetchWeight(){
	//char buffer[10];
	//float w = Scale.forTest(ESP.getFreeHeap());
	float w = getWeight();
	formatValue(w,_buffer);
	detectStable(w);
	//_weight = w;
	//ws.textAll(String("{\"w\":\""+String(Scale.getBuffer())+"\",\"c\":"+String(CORE.getCharge())+",\"s\":"+String(Scale.getStableWeight())+"}"));
	//_weight = String(buffer).toFloat();
	//taskPower.updateCache();
	/*char buffer[10];
	float w = getWeight();
	formatValue(w,buffer);
	detectStable(w);
	
	_weight = String(buffer);*/
	//_weight = String(ESP.getFreeHeap());
	//_weight = s.toFloat();
	//setTest(random(100,millis()));
	//setTest(s);
	//_weight = float(11.23);
	//taskPower.updateCache();	
}

bool ScaleClass::_downloadValue(){
	_scales_value.average = 1;
	_scales_value.step = 1;
	_scales_value.accuracy = 0;
	_scales_value.offset = 0;
	_scales_value.max = 1000;
	_scales_value.scale = 1;
	SetFilterWeight(80);
	_scales_value.user = "admin";
	_scales_value.password = "1234";
	File dateFile;
	if (SPIFFS.exists(CDATE_FILE)){
		dateFile = SPIFFS.open(CDATE_FILE, "r");
	}else{
		dateFile = SPIFFS.open(CDATE_FILE, "w+");
	}
	if (!dateFile) {
		dateFile.close();
		return false;
	}
	size_t size = dateFile.size();
		
	std::unique_ptr<char[]> buf(new char[size]);
		
	dateFile.readBytes(buf.get(), size);
	dateFile.close();
	DynamicJsonBuffer jsonBuffer(size);
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		return false;
	}
	_scales_value.max = json[WEIGHT_MAX_JSON];
	_scales_value.offset = json[OFFSET_JSON];
	setAverage(json[AVERAGE_JSON]);
	_scales_value.step = json[STEP_JSON];
	_scales_value.accuracy = json[ACCURACY_JSON];
	_scales_value.scale = json[SCALE_JSON];
	SetFilterWeight(json[FILTER_JSON]);
	_scales_value.seal = json[SEAL_JSON];
	if (!json.containsKey(USER_JSON)){
		_scales_value.user = "admin";
		_scales_value.password = "1234";	
	}else{
		_scales_value.user = json[USER_JSON].as<String>();
		_scales_value.password = json[PASS_JSON].as<String>();
	}
	
	return true;
	
}

bool ScaleClass::saveDate() {
	File cdateFile = SPIFFS.open(CDATE_FILE, "w+");
	if (!cdateFile) {
		cdateFile.close();
		return false;
	}
	DynamicJsonBuffer jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();

	if (!json.success()) {
		return false;
	}
	
	json[STEP_JSON] = _scales_value.step;
	json[AVERAGE_JSON] = _scales_value.average;
	json[WEIGHT_MAX_JSON] = _scales_value.max;
	json[OFFSET_JSON] = _scales_value.offset;
	json[ACCURACY_JSON] = _scales_value.accuracy;
	json[SCALE_JSON] = _scales_value.scale;
	json[FILTER_JSON] = GetFilterWeight();
	json[SEAL_JSON] = _scales_value.seal;
	json[USER_JSON] = _scales_value.user;
	json[PASS_JSON] = _scales_value.password;
	
	json.printTo(cdateFile);
	cdateFile.flush();
	cdateFile.close();
	return true;
}

long ScaleClass::readAverage() {
	long long sum = 0;
	for (byte i = 0; i < _scales_value.average; i++) {
		sum += read();
	}
	return _scales_value.average == 0?sum / 1:sum / _scales_value.average;
}

long ScaleClass::getValue() {
	Filter(readAverage());
	return Current() - _scales_value.offset;
}

float ScaleClass::getUnits() {
	float v = getValue();
	return (v * _scales_value.scale);
}

float ScaleClass::getWeight(){
	return round(getUnits() * _round) / _round; 
}

float ScaleClass::forTest(uint32_t h){
	Filter(h);
	float v = Current();
	v*= _scales_value.scale;
	v = round(v * _round) / _round;
	return v;
}

void ScaleClass::tare() {
	long sum = readAverage();
	setOffset(sum);
}

void ScaleClass::setAverage(unsigned char a){
	_scales_value.average = constrain(a, 1, 5);
}

void ScaleClass::mathScale(float referenceW, long calibrateW){
	_scales_value.scale = referenceW / float(calibrateW - _scales_value.offset);
}

/*! Функция для форматирования значения веса
	value - Форматируемое значение
	digits - Количество знаков после запятой
	accuracy - Точновть шага значений (1, 2, 5, ...)
	string - Выходная строка отфармотронова значение 
*/
void ScaleClass::formatValue(float value, char* string){
	dtostrf(value, 6-_scales_value.accuracy, _scales_value.accuracy, string);
}

/* */
void ScaleClass::detectStable(float w){
	static float weight_temp;
	static unsigned char stable_num;
		if (weight_temp == w) {
			//if (stable_num <= STABLE_NUM_MAX){
			if (stable_num > STABLE_NUM_MAX) {
				if (!stableWeight){
					if(fabs(w) > _stable_step){
						saveWeight.isSave = true;
						saveWeight.value = w;
						//CORE.saveEvent("weight", String(w)+"_kg");
					}
					stableWeight = true;
				}
				return;
			}
			stable_num++;
			//}
			} else {
			stable_num = 0;
			stableWeight = false;
		}
		weight_temp = w;
}

/*
void ScaleClass::handleWeight(AsyncWebServerRequest * request){
	/ *char buffer[10];
	float w = Scale.getWeight();
	Scale.formatValue(w, buffer	);
	Scale.detectStable(w);
	
	taskPower.updateCache();* /
	request->send(200, "text/plain", String("{\"w\":\""+String(Scale.getTest())+"\",\"c\":"+String(BATTERY.getCharge())+",\"s\":"+String(Scale.getStableWeight())+"}"));	
}*/

void handleSeal(AsyncWebServerRequest * request){
	randomSeed(Scale.readAverage());
	Scale.setSeal(random(1000));
	
	if (Scale.saveDate()){
		request->send(200, TEXT_HTML, String(Scale.getSeal()));
	}
}





