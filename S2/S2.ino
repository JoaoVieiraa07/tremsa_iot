#include <WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include "wifi_connection.h"

// WIFI e MQTT
const char* ssid     = "";
const char* password = "";

const char* mqtt_server = "";
WiFiClient espClient;
PubSubClient client(espClient);

// PINOS
#define PIN_PRESENCA1  32
#define PIN_PRESENCA2  33
#define PIN_SERVO      14
#define PIN_LED_R      25
#define PIN_LED_G      26
#define PIN_LED_B      27



Servo servoS2;

int presenca1_ant = HIGH;
int presenca2_ant = HIGH;


void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("S2")) {
      Serial.println(" conectado!");

      // tópicos que a S2 ESCUTA
      client.subscribe("S2/Servo1");
      client.subscribe("S2/LED");

    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente ...");
      delay(2000);
    }
  }
}


void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)message[i];

  Serial.print("Recebido [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(msg);

  // SERVO
  if (String(topic) == "S2/Servo1") {
    int pos = msg.toInt();
    if (pos >= 0 && pos <= 180) {
      servoS2.write(pos);
    }
  }

  // LED
  if (String(topic) == "S2/LED") {
    // mensagem no formato: R,G,B (ex: 255,0,100)
    int p1 = msg.indexOf(',');
    int p2 = msg.indexOf(',', p1 + 1);

    int r = msg.substring(0, p1).toInt();
    int g = msg.substring(p1 + 1, p2).toInt();
    int b = msg.substring(p2 + 1).toInt();

    analogWrite(PIN_LED_R, r);
    analogWrite(PIN_LED_G, g);
    analogWrite(PIN_LED_B, b);
  }
}

void setup() {
  Serial.begin(115200);

  
  pinMode(PIN_PRESENCA1, INPUT_PULLUP);
  pinMode(PIN_PRESENCA2, INPUT_PULLUP);

  
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

 
  servoS2.attach(PIN_SERVO);
  servoS2.write(90); // inicial

  // WiFi
  wifi_connect(ssid, password);

  // MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  int p1 = digitalRead(PIN_PRESENCA1);
  int p2 = digitalRead(PIN_PRESENCA2);

  if (p1 != presenca1_ant) {
    presenca1_ant = p1;
    client.publish("S2/Presenca1", p1 == LOW ? "1" : "0");
  }

  if (p2 != presenca2_ant) {
    presenca2_ant = p2;
    client.publish("S2/Presenca2", p2 == LOW ? "1" : "0");
  }

  delay(50);
}