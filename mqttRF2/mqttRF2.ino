#include "config.h"

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <NewRemoteReceiver.h>


WiFiClient wClient;
PubSubClient mqttClient(wClient);
String data = "";
bool published = true;
unsigned long lastMsgTime;

void setup() {
  Serial.begin(115200);

  setup_wifi();
  mqttClient.setServer(mqtt_server, mqtt_port);  
  
  
  NewRemoteReceiver::init(2, 2, showCode);
  Serial.println("Receiver initialized");    
  
  
}

void loop() {
 if (!mqttClient.loop()) {
  reconnect();
 }

 if(published == false) {
   mqttClient.publish(sendTopic,(char *)data.c_str());
   published = true;
   
 }

 
}

// Callback function is called only when a valid code is received.
void showCode(unsigned int period, unsigned long address, unsigned long groupBit, unsigned long unit, unsigned long switchType) {

  // Print the received code.
  Serial.print("Code: ");
  Serial.print(address);
  Serial.print(" Period: ");
  Serial.println(period);
  Serial.print(" unit: ");
  Serial.println(unit);
  Serial.print(" groupBit: ");
  Serial.println(groupBit);
  Serial.print(" switchType: ");
  Serial.println(switchType);

  String newData = String(address) + "," + String(unit) + "," + String(switchType);
  // new data or repeated data older than 2sec
  // TODO this will fail, if millis overflows
  if(newData != data || millis() > lastMsgTime + 2000) {
    data = newData;
    published = false;
    lastMsgTime = millis();
  }

}

void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(wifi_ssid, wifi_password);
    
  Serial.println(WiFi.macAddress()); 
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(WiFi.localIP());
  
}

boolean reconnect() {
  Serial.println("Reconnecting");
  // Loop until we're reconnected
  while (!mqttClient.connected()) {

      if (!mqttClient.connect(device_name, mqtt_user, mqtt_password)) {
        Serial.println("Connection to mqtt failed");
        // Wait 5 seconds before retrying
        delay(5000);
    }
  }
  Serial.println("Connected");
  mqttClient.publish(statusTopic, "up");
  
  return mqttClient.connected();
}
