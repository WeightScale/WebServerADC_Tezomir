/*
 * web_server_config.h
 *
 * Created: 01.04.2018 8:50:55
 *  Author: Kostya
 */ 


#ifndef WEB_SERVER_CONFIG_H_
#define WEB_SERVER_CONFIG_H_

//Использовать веб страницы из flash памяти
#define HTML_PROGMEM 1
#if HTML_PROGMEM
	#include "Page.h"
#endif


#endif /* WEB_SERVER_CONFIG_H_ */