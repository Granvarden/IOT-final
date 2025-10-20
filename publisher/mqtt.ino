#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <WiFiS3.h>
#include <PubSubClient.h>

// สร้าง object สำหรับ sensor
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ---- WiFi Config ----
const char* ssid = "Your Wifi";
const char* password = "Your Password";

// ---- MQTT Config ----
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic = "66070291/final";

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ArduinoR4Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  delay(1000);

  // ---- เริ่มต้น I2C และ SHT31 ----
  if (!sht31.begin(0x44)) {  // default address 0x44
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }

  // ---- เชื่อมต่อ WiFi ----
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // ---- ตั้งค่า MQTT ----
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temp = sht31.readTemperature();
  float hum = sht31.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    char payload[100];
    snprintf(payload, sizeof(payload), "{\"temperature\": %.2f, \"humidity\": %.2f}", temp, hum);
    
    Serial.print("Publishing: ");
    Serial.println(payload);
    
    client.publish(mqtt_topic, payload);
  } else {
    Serial.println("Sensor read failed");
  }

  delay(2000);  // ส่งข้อมูลทุก 5 วินาที
}
