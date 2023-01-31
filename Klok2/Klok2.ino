#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

// const char* ssid = "Howest.guests";
// const char* password = "welkombijhowest";
const char* ssid = "himpes boven 2.4";
const char* password = "Vandenabeele11";

// MQTT Broker
const char *mqtt_broker = "test.mosquitto.org";
const char *topic = "WH_ESP2";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

int starttime = 0;
bool timing = false;
int buttonIn = 13;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
 // Set software serial baud to 115200;
 Serial.begin(115200);
 // connecting to a WiFi network
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
 }
 // publish and subscribe
 client.subscribe("WH_TO-ESP2");
 Wire.begin (0, 4);
 matrix.begin(0x70);
 pinMode(buttonIn,INPUT);
}

void callback(char *topic, byte *payload, unsigned int length) {
  
  bool ok = true;
  if(timing == false)
  {
    Serial.println("a");
    char word[5] = {'s', 't', 'a', 'r', 't'};
    for (int i = 0; i < 5; i++) {
      if( word[i] != (char) payload[i] )
      {
        ok = false;
      }
    }
    if(ok)
    {
      timing = true;
      starttime = millis();
    }
  }
  else
  {
    Serial.println("b");
    char word[4] = {'s', 't', 'o', 'p'};
    for (int i = 0; i < 4; i++) {
      if( word[i] != (char) payload[i] )
      {
        ok = false;
      }
    }
    
    if(ok)
    {
      timing = false;
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!client.connected())
  {
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public emqx mqtt broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    client.subscribe("WH_TO-ESP2");
  }
  client.loop();
  if(timing == true)
  {
    int milliseconds = millis() - starttime;
    if(milliseconds > 60000){
      int mins = milliseconds / 60000;
      int secs = (milliseconds / 1000) % 60;
      matrix.writeDigitNum(0, (mins / 10));
      matrix.writeDigitNum(1, mins % 10);
      matrix.drawColon(true);
      matrix.writeDigitNum(3, (secs / 10));
      matrix.writeDigitNum(4, secs % 10);
      matrix.writeDisplay();
    }
    else
    {
      matrix.println(milliseconds/10);
      matrix.drawColon(true);
      matrix.writeDisplay();
    }
    if(digitalRead(buttonIn)==HIGH)
    {
      char cstr[16];
      itoa(milliseconds, cstr, 10);
      client.publish(topic,cstr);
      Serial.println(touchRead(T2));
      timing = false;
      delay(200);
    }
  }
}
