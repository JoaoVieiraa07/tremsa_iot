#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "env.h"  


// DEFINIÇÕES DE PINOS

#define DHT_PIN 4
#define DHT_TYPE DHT11  
#define ULTRA_TRIG 22
#define ULTRA_ECHO 23
#define LED_PIN 19
#define LDR_PIN 34
#define LED_R_PIN 14
#define LED_G_PIN 26
#define LED_B_PIN 25

#define PWM_FREQ 5000
#define PWM_RES 8  // 8 bits (0-255)


// VARIÁVEIS GLOBAIS

WiFiClientSecure client;
PubSubClient mqtt(client);
DHT dht(DHT_PIN, DHT_TYPE);

const char* WIFI_SSID = ENV_SSID;
const char* WIFI_PASS = ENV_PASS;
const char* brokerURL = BROKER_URL;
const int brokerPort = BROKER_PORT;

// Tópicos MQTT do env.h
const char* topicTemperatura = ENV_TOPIC_TEMPERATURA;
const char* topicUmidade = ENV_TOPIC_UMIDADE;
const char* topicLuminosidade = ENV_TOPIC_LUMINOSIDADE;
const char* topicPresenca = ENV_TOPIC_PRESENCA1;


// FUNÇÕES DE CONTROLE RGB

void setLEDColor(byte r, byte g, byte b) {
  ledcWrite(LED_R_PIN, r);
  ledcWrite(LED_G_PIN, g);
  ledcWrite(LED_B_PIN, b);
}

void turnOffLEDs() {
  setLEDColor(0, 0, 0);
}

void statusLED(byte status) {
  turnOffLEDs();
  switch (status) {
    case 254:  // Vermelho - Erro
      setLEDColor(255, 0, 0);
      break;
    case 1:  // Amarelo - Conectando WiFi
      setLEDColor(150, 255, 0);
      break;
    case 2:  // Rosa - Conectando Broker
      setLEDColor(150, 0, 255);
      break;
    case 3:  // Verde - Tudo OK
      setLEDColor(0, 255, 0);
      break;
    default:  // Pisca azul - Mensagem recebida
      for (byte i = 0; i < 4; i++) {
        setLEDColor(0, 0, 255);
        delay(100);
        turnOffLEDs();
        delay(100);
      }
      break;
  }
}


// CALLBACK MQTT

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("Mensagem recebida no tópico ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);

  statusLED(0);  // Pisca azul

  if (msg == "1") {
    digitalWrite(LED_PIN, HIGH);
    mqtt.publish(topicPresenca, "LED ligado via MQTT");
  } else if (msg == "0") {
    digitalWrite(LED_PIN, LOW);
    mqtt.publish(topicPresenca, "LED desligado via MQTT");
  }
}


// SETUP

void setup() {
  Serial.begin(115200);
  client.setInsecure();
  dht.begin();

  pinMode(LED_PIN, OUTPUT);
  pinMode(ULTRA_TRIG, OUTPUT);
  pinMode(ULTRA_ECHO, INPUT);
  pinMode(LDR_PIN, INPUT);


  // ledcAttach(pino, frequência, resolução)
  if (!ledcAttach(LED_R_PIN, PWM_FREQ, PWM_RES)) {
    Serial.println("Erro ao configurar LED vermelho!");
  }
  if (!ledcAttach(LED_G_PIN, PWM_FREQ, PWM_RES)) {
    Serial.println("Erro ao configurar LED verde!");
  }
  if (!ledcAttach(LED_B_PIN, PWM_FREQ, PWM_RES)) {
    Serial.println("Erro ao configurar LED azul!");
  }

  // Conectando WiFi 
  statusLED(1);
  Serial.print("Conectando WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n✅ WiFi conectado!");
  Serial.println(WiFi.localIP());

  // Conectando MQTT
  statusLED(2);
  mqtt.setServer(brokerURL, brokerPort);
  mqtt.setCallback(callback);

  String boardID = "ESP32-" + String(random(0xffff), HEX);
  while (!mqtt.connect(boardID.c_str(), BROKER_USR_NAME, BROKER_USR_PASS)) {
    Serial.print(".");
    delay(500);
  }
  
  // Subscreve aos tópicos definidos no env.h
  mqtt.subscribe(topicTemperatura);
  mqtt.subscribe(topicUmidade);
  mqtt.subscribe(topicLuminosidade);
  mqtt.subscribe(topicPresenca);
  
  Serial.println("\n✅ Broker MQTT conectado!");
  Serial.println("Tópicos subscritos:");
  Serial.println(topicTemperatura);
  Serial.println(topicUmidade);
  Serial.println(topicLuminosidade);
  Serial.println(topicPresenca);
  
  statusLED(3);
}


// LOOP PRINCIPAL

void loop() {
  mqtt.loop();

  // Leitura periódica dos sensores
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 5000) {
    lastRead = millis();

    float temperatura = dht.readTemperature();
    float umidade = dht.readHumidity();
    int luz = analogRead(LDR_PIN);

    // Sensor ultrassônico
    digitalWrite(ULTRA_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRA_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRA_TRIG, LOW);
    float duracao = pulseIn(ULTRA_ECHO, HIGH);
    float distancia = duracao * 0.034 / 2;

    Serial.printf("Temp: %.1f°C | Umid: %.1f%% | Luz: %d | Dist: %.1f cm\n",
                  temperatura, umidade, luz, distancia);

    // Publica em tópicos separados
    char msgTemp[20];
    snprintf(msgTemp, sizeof(msgTemp), "%.1f", temperatura);
    mqtt.publish(topicTemperatura, msgTemp);

    char msgUmid[20];
    snprintf(msgUmid, sizeof(msgUmid), "%.1f", umidade);
    mqtt.publish(topicUmidade, msgUmid);

    char msgLuz[20];
    snprintf(msgLuz, sizeof(msgLuz), "%d", luz);
    mqtt.publish(topicLuminosidade, msgLuz);

    // Detecção de presença (distância < 50cm)
    if (distancia < 50.0 && distancia > 0) {
      mqtt.publish(topicPresenca, "1");
    } else {
      mqtt.publish(topicPresenca, "0");
    }
  }
}