#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <AsyncJson.h>
#include <functional>
#include "BrowserServer.h"
#include "tools.h"
#include "Core.h"
#include "Version.h"
#include "DateTime.h"
#include "HttpUpdater.h"
#include "web_server_config.h"

/* */
//ESP8266HTTPUpdateServer httpUpdater;
/* Soft AP network parameters */
IPAddress apIP(192,168,4,1);
IPAddress netMsk(255, 255, 255, 0);

IPAddress lanIp;			// Надо сделать настройки ip адреса
IPAddress gateway;

BrowserServerClass browserServer(80);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;
//holds the current upload
//File fsUploadFile;

/* hostname for mDNS. Should work at least on windows. Try http://esp8266.local */


BrowserServerClass::BrowserServerClass(uint16_t port) : AsyncWebServer(port) {}

BrowserServerClass::~BrowserServerClass(){}
	
void BrowserServerClass::begin() {
	SPIFFS.begin();
	/* Setup the DNS server redirecting all the domains to the apIP */
	dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer.start(DNS_PORT, "*", apIP);	
	_downloadHTTPAuth();
	ws.onEvent(onWsEvent);
	addHandler(&ws);
	CORE = new CoreClass(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str());
	addHandler(CORE);
	addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
	addHandler(new SPIFFSEditor(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str()));	
	addHandler(new HttpUpdaterClass("sa", "654321"));
	init();
	AsyncWebServer::begin(); // Web server start
}

void BrowserServerClass::init(){
	#if HTML_PROGMEM
		on("/",[](AsyncWebServerRequest * reguest){	reguest->send_P(200,F("text/html"),index_html);});
		on("/settings.html",[](AsyncWebServerRequest * reguest){	reguest->send_P(200,F("text/html"),settings_html);});	
		serveStatic("/", SPIFFS, "/");		
	#else
		on("/settings.html", HTTP_ANY, std::bind(&CoreClass::saveValueSettingsHttp, CORE, std::placeholders::_1));					/* Открыть страницу настроек или сохранить значения. */
		//on("/sn",WebRequestMethod::HTTP_GET,handleAccessPoint);						/* Установить Настройки точки доступа */
		//on("/sn",WebRequestMethod::HTTP_POST, std::bind(&CoreClass::handleSetAccessPoint, CORE, std::placeholders::_1));					/* Установить Настройки точки доступа */
		serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
	#endif // PROGMEM_PAGE	
		
	on("/wt",HTTP_GET, [](AsyncWebServerRequest * request){					/* Получить вес и заряд. */
		request->send(200, "text/html", String("{\"w\":\""+String(Scale.getBuffer())+"\",\"c\":"+String(BATTERY.getCharge())+",\"s\":"+String(Scale.getStableWeight())+"}"));	
	});		
	on("/rc", reconnectWifi);									/* Пересоединиться по WiFi. */
		
	on("/settings.json", handleFileReadAuth);
	on("/sv", handleScaleProp);									/* Получить значения. */
	on("/admin.html", std::bind(&BrowserServerClass::send_wwwauth_configuration_html, this, std::placeholders::_1));
	on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});
	//on("/secret.json",handleFileReadAdmin);
	//serveStatic("/sc", SPIFFS, "/").setDefaultFile("secret.json").setAuthentication(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str());
												
	/*on("/",[&](){												/ * Главная страница. * /
		handleFileRead(uri());
		taskPower.resume();
	});		
	
	//on("/generate_204", [this](){if (!handleFileRead("/index.html"))	this->send(404, "text/plain", "FileNotFound");});  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
	//on("/fwlink", [this](){if (!handleFileRead("/index.html"))	this->send(404, "text/plain", "FileNotFound");});  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.	
	
	const char * headerkeys[] = {"User-Agent","Cookie"/ *,"x-SETNET"* /} ;
	size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);
	//ask server to track these headers
	collectHeaders(headerkeys, headerkeyssize );*/
	
	//serveStatic("/", SPIFFS, "/").setDefaultFile("index-ap.html").setFilter(ON_AP_FILTER);
	//rewrite("/", "index.html").setFilter(ON_STA_FILTER);
	//rewrite("/", "index-ap.html").setFilter(ON_AP_FILTER);
	
	onNotFound([](AsyncWebServerRequest *request){
		request->send(404);
	});
}

void BrowserServerClass::send_wwwauth_configuration_html(AsyncWebServerRequest *request) {
	if (!checkAdminAuth(request))
		return request->requestAuthentication();
	if (request->args() > 0){  // Save Settings
		if (request->hasArg("wwwuser")){
			_httpAuth.wwwUsername = request->arg("wwwuser");
			_httpAuth.wwwPassword = request->arg("wwwpass");
		}		
		_saveHTTPAuth();
	}
	request->send(SPIFFS, request->url());
}

bool BrowserServerClass::_saveHTTPAuth() {
	
	DynamicJsonBuffer jsonBuffer(256);
	//StaticJsonBuffer<256> jsonBuffer;
	JsonObject& json = jsonBuffer.createObject();
	json["user"] = _httpAuth.wwwUsername;
	json["pass"] = _httpAuth.wwwPassword;

	//TODO add AP data to html
	File configFile = SPIFFS.open(SECRET_FILE, "w");
	if (!configFile) {
		configFile.close();
		return false;
	}

	json.printTo(configFile);
	configFile.flush();
	configFile.close();
	return true;
}

bool BrowserServerClass::_downloadHTTPAuth() {
	_httpAuth.wwwUsername = "sa";
	_httpAuth.wwwPassword = "343434";
	File configFile = SPIFFS.open(SECRET_FILE, "r");
	if (!configFile) {
		configFile.close();
		return false;
	}
	size_t size = configFile.size();

	std::unique_ptr<char[]> buf(new char[size]);
	
	configFile.readBytes(buf.get(), size);
	configFile.close();
	DynamicJsonBuffer jsonBuffer(256);
	JsonObject& json = jsonBuffer.parseObject(buf.get());

	if (!json.success()) {
		return false;
	}	
	_httpAuth.wwwUsername = json["user"].as<String>();
	_httpAuth.wwwPassword = json["pass"].as<String>();
	return true;
}

bool BrowserServerClass::checkAdminAuth(AsyncWebServerRequest * r) {	
	return r->authenticate(_httpAuth.wwwUsername.c_str(), _httpAuth.wwwPassword.c_str());
}

/*
void BrowserServerClass::restart_esp() {
	String message = "<meta name='viewport' content='width=device-width, initial-scale=1, maximum-scale=1'/>";
	message += "<meta http-equiv='refresh' content='10; URL=/admin.html'>Please Wait....Configuring and Restarting.";
	send(200, "text/html", message);
	SPIFFS.end(); // SPIFFS.end();
	delay(1000);
	ESP.restart();
}*/

bool BrowserServerClass::isAuthentified(AsyncWebServerRequest * request){
	if (!request->authenticate(CORE->getNameAdmin().c_str(), CORE->getPassAdmin().c_str())){
		if (!checkAdminAuth(request)){
			return false;
		}
	}
	return true;
}

void handleFileReadAuth(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request)){
		return request->requestAuthentication();
	}
	request->send(SPIFFS, request->url());
}

void handleScaleProp(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request))
		return request->requestAuthentication();
	AsyncJsonResponse * response = new AsyncJsonResponse();
	JsonObject& root = response->getRoot();
	root["id_date"] = getDateTime();
	root["id_local_host"] = String(MY_HOST_NAME);
	root["id_ap_ssid"] = String(SOFT_AP_SSID);
	root["id_ap_ip"] = toStringIp(WiFi.softAPIP());
	root["id_ip"] = toStringIp(WiFi.localIP());
	root["sl_id"] = String(Scale.getSeal());
	root["chip_v"] = String(ESP.getCpuFreqMHz());
	response->setLength();
	request->send(response);
	/*String values = "";
	values += "id_date|" + getDateTime() + "|div\n";
	values += "id_local_host|"+String(MY_HOST_NAME)+"/|div\n";
	values += "id_ap_ssid|" + String(SOFT_AP_SSID) + "|div\n";
	values += "id_ap_ip|" + toStringIp(WiFi.softAPIP()) + "|div\n";
	values += "id_ip|" + toStringIp(WiFi.localIP()) + "|div\n";
	values += "sl_id|" + String(Scale.getSeal()) + "|div\n";
	
	request->send(200, "text/plain", values);*/
}

/*
#if! HTML_PROGMEM
void handleAccessPoint(AsyncWebServerRequest * request){
	if (!browserServer.isAuthentified(request))
		return request->requestAuthentication();
	request->send_P(200, F(TEXT_HTML), netIndex);	
}
#endif*/

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len){
	if(type == WS_EVT_CONNECT){	
		client->ping();	
	}else if(type == WS_EVT_DATA){
		String msg = "";
		for(size_t i=0; i < len; i++) {
			msg += (char) data[i];
		}
		if (msg.equals("/wt")){
			DynamicJsonBuffer jsonBuffer;
			JsonObject& json = jsonBuffer.createObject();
			json["w"]= String(Scale.getBuffer());
			json["c"]= BATTERY.getCharge();
			json["s"]= Scale.getStableWeight();
			size_t len = json.measureLength();
			AsyncWebSocketMessageBuffer * buffer = ws.makeBuffer(len);
			if (buffer) {
				json.printTo((char *)buffer->get(), len + 1);
				if (client) {
					client->text(buffer);
				}
			}	
			//taskPower.updateCache();	
			//client->text(String("{\"w\":\""+String(Scale.getBuffer())+"\",\"c\":"+String(BATTERY.getCharge())+",\"s\":"+String(Scale.getStableWeight())+"}"));
		}
	}
}