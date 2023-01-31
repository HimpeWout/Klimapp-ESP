#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5  // ESP32 pin GIOP5 
#define RST_PIN 27 // ESP32 pin GIOP27
// const char* ssid = "Howest.guests";
// const char* password = "welkombijhowest";
const char* ssid = "himpes boven 2.4";
const char* password = "Vandenabeele11";

const char *mqtt_broker = "test.mosquitto.org";
const char *topic = "PI_PLAYERRFID";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;
const int buzzer = 2;

WiFiClient espClient;
PubSubClient client(espClient);

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
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
 client.subscribe("WH_TO-ESP");
 client.subscribe("WH_TO-ESP2");

  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  pinMode(buzzer, OUTPUT);
}
void callback(char *topic, byte *payload, unsigned int length) {
  char word[5] = {'s', 't', 'a', 'r', 't'};
  bool ok = true;
  for (int i = 0; i < 5; i++) {
    if( word[i] != (char) payload[i] )
    {
      ok = false;
    }
  }
  if(ok)
  {
    digitalWrite(buzzer, HIGH); // Set GPIO22 active high
    delay(1000);  // delay of one second
    digitalWrite(buzzer, LOW);
  }
}

void loop() {
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
    client.subscribe("WH_TO-ESP");
    client.subscribe("WH_TO-ESP2");
  }
  client.loop();
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      String hexstring = "";
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? "0":"");
        Serial.print(rfid.uid.uidByte[i], HEX);
        if(rfid.uid.uidByte[i] < 0x10) {
          hexstring += '0';
        }

        hexstring += String(rfid.uid.uidByte[i], HEX);
      }
      
      Serial.println();
      client.publish(topic,hexstring.c_str());
      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
    delay(1000);
  }
}
