#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "Seeed_BME280.h"

void setup();
void loop();
void getTempAndHum();
void getSensorInfo();
void notFound();
void goToDeepSleep();

//WiFi Connection configuration
const char *ssid = "ESPMasterController";
const char *password = "[the secure password]";

ESP8266WebServer server(80);
BME280 sensor;
static bool goToDS = false;
static int updatePeriod = 2 * 1000;
static unsigned long time_now = 0;

void setup() {
  delay(1000);
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
  
  server.on("/getTempAndHum", getTempAndHum); 
  server.on("/getSensorInfo", getSensorInfo); 
  server.onNotFound(notFound);
  server.begin();

  sensor.init();
}

void loop() {
  server.handleClient();
  if (!goToDS) {
	  time_now = millis() + updatePeriod + 1000;
  }
  if (millis() > time_now + updatePeriod && goToDS) {
	  time_now = millis();
	  goToDeepSleep();
  }
  if (millis() > 60 * 1000) {
	  Serial.println("To long awake, restart!");
	  ESP.restart();
  }

}

void getTempAndHum() {
  server.send(200, "text/json", String() +   F("{\"Intern\": {\"temperature\": ") + String(sensor.getTemperature()) +  F(", \"humidity\": ") + String(sensor.getHumidity()) + F(", \"pressure\": ") + String((float)sensor.getPressure() / (float)100.00) + F("}}"));
  goToDS = true;
}

void getSensorInfo() {
  server.send(200, "text/json", String() +   F("{\"function\": \"getTempAndHum\", \"version\": \"v2\"}"));
}

void notFound(){
  server.send(404, "text/plain", "404 Not Found");
}

void goToDeepSleep(){
  Serial.println("Data send, going to Deep Sleep");
  delay(1);
  ESP.deepSleep(1000000*60*10); // 10 Minutes Deep Sleep
}
