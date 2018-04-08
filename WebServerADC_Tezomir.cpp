//#define PROGMEM_PAGE
#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
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
void takeWeight();
void powerSwitchInterrupt();
void connectWifi();
//
TaskController taskController = TaskController();		/*  */
Task taskBlink(takeBlink, 500);							/*  */
Task taskBattery(takeBattery, 20000);					/* 20 Обновляем заряд батареи */
//Task taskPower(powerOff, 2400000);						/* 10 минут бездействия и выключаем */
Task taskConnectWiFi(connectWifi, 20000);				/* Пытаемся соедениться с точкой доступа каждые 20 секунд */
Task taskWeight(takeWeight,100);
WiFiEventHandler stationModeConnectedHandler;
WiFiEventHandler stationModeDisconnectedHandler;

unsigned int COUNT_FLASH = 500;
unsigned int COUNT_BLINK = 500;
#define USE_SERIAL Serial

void connectWifi();

void setup() {
	//pinMode(EN_NCP, OUTPUT);
	//digitalWrite(EN_NCP, HIGH);
	pinMode(LED, OUTPUT);	
	//Serial.begin(115200);
	/*while (digitalRead(PWR_SW) == HIGH){
		delay(100);
	};*/
	
	//CORE.begin();
	//delay(1000);
	browserServer.begin();	
	Scale.setup(&browserServer);	
	takeBattery();
  
	taskController.add(&taskBlink);
	taskController.add(&taskBattery);
	taskController.add(&taskWeight);
	taskController.add(&taskConnectWiFi);
	//taskController.add(&taskPower);	

	stationModeConnectedHandler = WiFi.onStationModeConnected(&onStationModeConnected);	
	stationModeDisconnectedHandler = WiFi.onStationModeDisconnected(&onStationModeDisconnected);
  
	//ESP.eraseConfig();
	WiFi.persistent(false);
	WiFi.setAutoConnect(true);
	WiFi.setAutoReconnect(true);
	//WiFi.smartConfigDone();
	WiFi.mode(WIFI_AP_STA);
	WiFi.hostname(MY_HOST_NAME);
	WiFi.softAPConfig(apIP, apIP, netMsk);
	WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD);
	delay(500); 	
	connectWifi();	
	//CORE.saveEvent("power", "ON");
}

/*********************************/
/* */
/*********************************/
void takeBlink() {
	bool led = !digitalRead(LED);
	digitalWrite(LED, led);	
	taskBlink.setInterval(led ? COUNT_BLINK : COUNT_FLASH);
}

/**/
void takeBattery(){
	BATTERY.fetchCharge(1);
}

void takeWeight(){
	Scale.fetchWeight();
	taskWeight.updateCache();
}

/*
void powerSwitchInterrupt(){
	if(digitalRead(PWR_SW)==HIGH){
		unsigned long t = millis();
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
	//USE_SERIAL.println("Connecting...");
	WiFi.disconnect(false);
	/*!  */
	int n = WiFi.scanComplete();
	if(n == -2){
		n = WiFi.scanNetworks();
		if (n <= 0){
			goto scan;
		}
	}else if (n > 0){
		goto connect;
	}else{
		goto scan;
	}
	connect:
		for (int i = 0; i < n; ++i)	{
			if(WiFi.SSID(i) == CORE->getSSID().c_str()){
				//USE_SERIAL.println(CORE.getSSID());
				String ssid_scan;
				int32_t rssi_scan;
				uint8_t sec_scan;
				uint8_t* BSSID_scan;
				int32_t chan_scan;
				bool hidden_scan;

				WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan, hidden_scan);
				if (!CORE->isAuto()){
					if (lanIp.fromString(CORE->getLanIp()) && gateway.fromString(CORE->getGateway())){
						WiFi.config(lanIp,gateway, netMsk);									// Надо сделать настройки ip адреса
					}
				}
				//Serial.println(String(chan_scan));
				WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD, chan_scan); //Устанавливаем канал как роутера
				WiFi.begin ( CORE->getSSID().c_str(), CORE->getPASS().c_str(),chan_scan,BSSID_scan);
				//WiFi.begin ( CORE.getSSID().c_str(), CORE.getPASS().c_str());
				//USE_SERIAL.println("waitForConnectResult");
				int status = WiFi.waitForConnectResult();
				//USE_SERIAL.println("ConnectResult ");
				if(status == WL_CONNECTED ){					
					NBNS.begin(MY_HOST_NAME);
					//CORE.saveEvent("ip", CORE.getIp());
				}
				//USE_SERIAL.println(String(status));
				return;
			}
		}
	scan:
	WiFi.scanDelete();
	WiFi.scanNetworks(true);
	
}

void loop() {
	taskController.run();	
	//DNS
	dnsServer.processNextRequest();
	//HTTP
	if (Scale.isSave()){
		CORE->saveEvent("weight", String(Scale.getSaveValue())+"_kg");
		Scale.setIsSave(false);
	}
	//powerSwitchInterrupt();
}

void onStationModeConnected(const WiFiEventStationModeConnected& evt) {
	taskConnectWiFi.pause();
	//Serial.println(String(evt.channel));
	//WiFi.softAPdisconnect();
	//WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD, evt.channel); //Устанавливаем канал как роутера
	COUNT_FLASH = 50;
	COUNT_BLINK = 3000;
}

void onStationModeDisconnected(const WiFiEventStationModeDisconnected& evt) {
	//Serial.println("DisconnectStation");
	WiFi.scanDelete();
	WiFi.scanNetworks(true);
	taskConnectWiFi.resume();
	COUNT_FLASH = 500;
	COUNT_BLINK = 500;
}
