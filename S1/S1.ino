// BIBLIOTECAS
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "env.h"
#include "led.h"
#include "wifi_connection.h"


// DEFINIÇÃO DE PINOS
#define DHT_PIN 4
#define DHT_TYPE DHT11

#define TRIG 22
#define ECHO 23

#define LED_PIN 19
#define LDR_PIN 34

// LED RGB
#define LED_R 14
#define LED_G 26
#define LED_B 25


// CONFIGURAÇÃO DE PWM
#define PWM_FREQ 5000
#define PWM_RES 8

#define CH_R 0
#define CH_G 1
#define CH_B 2


// OBJETOS
WiFiClientSecure client;
PubSubClient mqtt(client);
DHT dht(DHT_PIN, DHT_TYPE);


// REDE / BROKER
const char* WIFI_SSID = ENV_SSID;
const char* WIFI_PASS = ENV_PASS;

const char* BROKER = BROKER_URL;
const int BROKER_PORT = ENV_BROKER_PORT;
const char* TOPIC = TOPIC1;

// CALLBACK MQTT
void callback(char* topic, byte* payload, unsigned int length) {

  String msg = "";
  for (unsigned int i = 0; i < length; i++)
    msg += (char)payload[i];

  Serial.print("Recebi do MQTT: ");
  Serial.println(msg);

  statusLED(0);

  if (msg == "1") {
    digitalWrite(LED_PIN, HIGH);
    mqtt.publish(TOPIC, "LED ligado");
  }
  else if (msg == "0") {
    digitalWrite(LED_PIN, LOW);
    mqtt.publish(TOPIC, "LED desligado");
  }
}

// SETUP
void setup() {

  Serial.begin(115200);
  client.setInsecure(); 

  dht.begin();
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LDR_PIN, INPUT);

  // PWM RGB
  ledcSetup(CH_R, PWM_FREQ, PWM_RES);
  ledcSetup(CH_G, PWM_FREQ, PWM_RES);
  ledcSetup(CH_B, PWM_FREQ, PWM_RES);

  ledcAttachPin(LED_R, CH_R);
  ledcAttachPin(LED_G, CH_G);
  ledcAttachPin(LED_B, CH_B);


  // CONECTA WIFI
  statusLED(1);
  wifi_connect(WIFI_SSID, WIFI_PASS);
  Serial.println(WiFi.localIP());


  // CONECTA MQTT
  statusLED(2);

  mqtt.setServer(BROKER, BROKER_PORT);
  mqtt.setCallback(callback);

  String id = "ESP32-" + String(random(0xffff), HEX);

  while (!mqtt.connect(id.c_str(), BROKER_USR_NAME, BROKER_USR_PASS)) {
    Serial.print(".");
    delay(300);
  }

  mqtt.subscribe(TOPIC);
  Serial.println("\nMQTT conectado!");
  statusLED(3);
}



// LOOP
void loop() {

  mqtt.loop();

  static unsigned long lastRead = 0;

  if (millis() - lastRead > 5000) {

    lastRead = millis();

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();
    int luz = analogRead(LDR_PIN);


    // ULTRASSÔNICO
    digitalWrite(TRIG, LOW);
    delayMicroseconds(5);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    long duracao = pulseIn(ECHO, HIGH, 30000);
    float dist = (duracao == 0) ? -1 : duracao * 0.034 / 2;


    Serial.printf(
      "Temp: %.1f°C | Umid: %.1f%% | Luz: %d | Distância: %.1f cm\n",
      temp, umid, luz, dist
    );


    char msg[150];
    snprintf(msg, sizeof(msg),
      "{\"temp\":%.1f,\"umid\":%.1f,\"luz\":%d,\"dist\":%.1f}",
      temp, umid, luz, dist
    );

    mqtt.publish(TOPIC, msg);
  }
}
