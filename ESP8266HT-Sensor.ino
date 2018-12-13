#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include "Seeed_BME280.h"

void setup();
void loop();

//WiFi Connection configuration
const char *ssid = "ESP32Master";
const char *password = "[password of the master]";

HTTPClient http;
BME280 sensor;
const int bufferSize = 36;
static char buff[bufferSize];

void setup() {
	Serial.begin(115200);
	Serial.println();

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	Serial.println("");

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	sensor.init();

	delay(1000);
	sendTempAndHum();
}

void loop() {

}

void sendTempAndHum() {
	float t = sensor.getTemperature();
	float p = (float)(sensor.getPressure()) / (float)100.00;
	float h = sensor.getHumidity();
	http.begin("http://" + WiFi.gatewayIP().toString() + "/payload");      //Specify request destination
	String payload = "MAC=" + WiFi.macAddress() + "&Version=v2&Payload=T_" + String(t) + "H_" + String(h) + "P_" + String(p);
	Serial.println("Try to send to: http://" + WiFi.gatewayIP().toString() + "/payload");
	Serial.println("payload: " + payload);
	Serial.println("Content-Length: " + String(payload.length()));
	http.addHeader("Content-Type", "text/plain");
	http.addHeader("Content-Length", String(payload.length()));
	int httpCode = http.POST(payload);
	String get = http.getString();                  //Get the response payload
	Serial.println(httpCode);   //Print HTTP return code
	Serial.println(get);    //Print request response payload
	http.end();  //Close connection
	if (httpCode == 200) {
		goToDeepSleep(10);
	}
	else {
		goToDeepSleep(2);
	}
}

void goToDeepSleep(int min) {
	Serial.println("Data send, going to Deep Sleep");
	delay(1);
	ESP.deepSleep(1000000 * 60 * min); // 10 Minutes Deep Sleep
}
