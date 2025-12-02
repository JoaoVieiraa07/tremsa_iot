#include <WiFi.h>
#include <PubSubClient.h>
#include "wifi_connection.h"

// ====== WIFI E MQTT ======
const char* ssid     = "";
const char* password = "";
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);

// Ultrassônico 1
#define TRIG1 32
#define ECHO1 33

// Ultrassônico 2
#define TRIG2 25
#define ECHO2 26

// LED RGB
#define LED_R 14
#define LED_G 27
#define LED_B 12

// led normal
#define LED_SIMPLE 13


// leitura sensor ultrassonico
long readUltrasonic(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH, 30000);  
  long distance = duration * 0.034 / 2; // em cm

  if (distance == 0) distance = -1; // erro

  return distance;
}


// mqtt roconnect
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("S2")) {
      Serial.println(" conectado!");

      client.subscribe("S2/LED_RGB");
      client.subscribe("S2/LED_SIMPLE");

    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente ...");
      delay(2000);
    }
  }
}


// CALLBACK MQTT
void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)message[i];

  Serial.printf("Mensagem [%s]: %s\n", topic, msg.c_str());

  // LED RGB
  if (String(topic) == "S2/LED_RGB") {
    int p1 = msg.indexOf(',');
    int p2 = msg.indexOf(',', p1 + 1);

    int r = msg.substring(0, p1).toInt();
    int g = msg.substring(p1 + 1, p2).toInt();
    int b = msg.substring(p2 + 1).toInt();

    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);
  }

  // LED simples (0 ou 1)
  if (String(topic) == "S2/LED_SIMPLE") {
    digitalWrite(LED_SIMPLE, msg == "1" ? HIGH : LOW);
  }
}


// SETUP
void setup() {
  Serial.begin(115200);

  // Ultrassônicos
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  // LEDs
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_SIMPLE, OUTPUT);

  // WiFi
  wifi_connect(ssid, password);

  // MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


// LOOP
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Leitura dos sensores
  long dist1 = readUltrasonic(TRIG1, ECHO1);
  long dist2 = readUltrasonic(TRIG2, ECHO2);

  // Publica no MQTT
  char buffer[10];

  sprintf(buffer, "%ld", dist1);
  client.publish("S2/Distancia1", buffer);

  sprintf(buffer, "%ld", dist2);
  client.publish("S2/Distancia2", buffer);

  delay(300);
}
