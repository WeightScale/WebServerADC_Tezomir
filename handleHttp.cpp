#include "handleHttp.h"
#include "tools.h"
#include "Core.h"
#include "DateTime.h"
#include "BrowserServer.h"

void handleScaleProp(){	
	if (!browserServer.isAuthentified())
		return browserServer.requestAuthentication();
	String values = "";
	values += "id_date|" + getDateTime() + "|div\n";	
	values += "id_local_host|"+String(MY_HOST_NAME)+"/|div\n";
	values += "id_ap_ssid|" + String(SOFT_AP_SSID) + "|div\n";
	values += "id_ap_ip|" + toStringIp(WiFi.softAPIP()) + "|div\n";
	values += "id_ip|" + toStringIp(WiFi.localIP()) + "|div\n";
	values += "sl_id|" + String(Scale.getSeal()) + "|div\n";
	browserServer.send(200, "text/plain", values);
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void handleSettingsHtml() {	
	if (!browserServer.isAuthentified())
		return browserServer.requestAuthentication();
	if (browserServer.args() > 0) // Save Settings
		return CORE.saveValueSettingsHttp("OK");
	handleFileRead(browserServer.uri());
}



