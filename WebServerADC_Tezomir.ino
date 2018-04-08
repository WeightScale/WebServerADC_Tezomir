#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "BrowserServer.h"
#include "ESP8266NetBIOS.h" 
#include "Core.h"
#include "Task.h"
#include "HttpUpdater.h"

/*
 * This example serves a "hello world" on a WLAN and a SoftAP at the same time.
 * The SoftAP allow you to configure WLAN parameters at run time. They are not setup in the sketch but saved on EEPROM.
 * 
 * Connect your computer or cell phone to wifi network ESP_ap with password 12345678. A popup may appear and it allow you to go to WLAN config. If it does not then navigate to http://192.168.4.1/wifi and config it there.
 * Then wait for the module to connect to your wifi and take note of the WLAN IP it got. Then you can disconnect from ESP_ap and return to your regular WLAN.
 * 
 * Now the ESP8266 is in your network. You can reach it through http://192.168.x.x/ (the IP you took note of) or maybe at http://esp8266.local too.
 * 
 * This is a captive portal because through the softAP it will redirect any http request to http://192.168.4.1/
 */

void onStationModeConnected(const WiFiEventStationModeConnected& evt);
void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt);
void takeBlink();
void takeBattery();
//void powerSwitchInterrupt();
void connectWifi();
//
TaskController taskController = TaskController();		/*  */
Task taskBlink(takeBlink, 500);							/*  */
Task taskBattery(takeBattery, 20000);					/* 20 Обновляем заряд батареи */
//Task taskPower(powerOff, 1200000);						/* 10 минут бездействия и выключаем */
Task taskConnectWiFi(connectWifi, 60000);				/* Пытаемся соедениться с точкой доступа каждые 60 секунд */
WiFiEventHandler stationModeConnectedHandler;
WiFiEventHandler stationModeDisconnectedHandler;

unsigned int COUNT_FLASH = 500;
unsigned int COUNT_BLINK = 500;

void connectWifi();

void setup() {
	//pinMode(EN_NCP, OUTPUT);
	//digitalWrite(EN_NCP, HIGH);
	pinMode(LED, OUTPUT);	

	/*while (digitalRead(PWR_SW) == HIGH){
		delay(100);
	};*/
	
	CORE.begin();
	delay(1000);	
	takeBattery();
  
	taskController.add(&taskBlink);
	taskController.add(&taskBattery);
	taskController.add(&taskConnectWiFi);
	taskConnectWiFi.pause();
	//taskController.add(&taskPower);	

	stationModeConnectedHandler = WiFi.onStationModeConnected(&onStationModeConnected);	
	stationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationModeDisconnected);
  
	WiFi.persistent(false);
	WiFi.hostname(MY_HOST_NAME);
	WiFi.softAPConfig(apIP, apIP, netMsk);
	WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD);
	delay(500); 
	
	//ESP.eraseConfig();
	connectWifi();
	browserServer.begin();
	NBNS.begin("ESP");
	httpUpdater.setup(&browserServer,"sa","343434");
	Scale.setup(&browserServer); 
	//Scale.init();  
	
	CORE.saveEvent("power", "ON");	
	Scale.tare();
}

/*********************************/
/* */
/*********************************/
void takeBlink() {
	bool led = !digitalRead(LED);
	digitalWrite(LED, led);	
	taskBlink.setInterval(led ? COUNT_BLINK : COUNT_FLASH/Scale.getAverage());	
}

/**/
void takeBattery(){
	CORE.getBattery(1);		
}

/*
void powerSwitchInterrupt(){
	unsigned long t = millis();
	//delay(100);
	if(digitalRead(PWR_SW)==HIGH){ //
		digitalWrite(LED, HIGH);
		while(digitalRead(PWR_SW)==HIGH){ // 
			delay(100);	
			if(t + 4000 < millis()){ // 
				digitalWrite(LED, HIGH);
				while(digitalRead(PWR_SW) == HIGH){delay(10);};// 
				powerOff();			
				break;
			}
			digitalWrite(LED, !digitalRead(LED));
		}
	}
}*/

void connectWifi() {	
	WiFi.disconnect(false);
	/*!  */
	int n = WiFi.scanNetworks();	
	if (n > 0){
		for (int i = 0; i < n; ++i)	{			
			if(WiFi.SSID(i) == CORE.getSSID().c_str()){
				WiFi.begin ( CORE.getSSID().c_str(), CORE.getPASS().c_str());
				if (!CORE.isAuto()){
					if (lanIp.fromString(CORE.getLanIp()) && gateway.fromString(CORE.getGateway())){
						WiFi.config(lanIp,gateway, netMsk);									// Надо сделать настройки ip адреса		
					}
				}				
				WiFi.waitForConnectResult();
				CORE.saveEvent("ip", CORE.getIp());				
				return;
			}
		}
	}	
}

void loop() {
	taskController.run();	
	//DNS
	dnsServer.processNextRequest();
	//HTTP
	browserServer.handleClient();	
	
	//powerSwitchInterrupt();	
}

void onStationModeConnected(const WiFiEventStationModeConnected& evt) {
	taskConnectWiFi.pause();
	WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD, evt.channel); //Устанавливаем канал как роутера
	// Setup MDNS responder
	if (MDNS.begin(MY_HOST_NAME, WiFi.localIP())) {
		// Add service to MDNS-SD
		MDNS.addService("http", "tcp", 80);
	}
	COUNT_FLASH = 50;
	COUNT_BLINK = 3000;	
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt) {	
	taskConnectWiFi.resume();
	COUNT_FLASH = 500;
	COUNT_BLINK = 500;
}