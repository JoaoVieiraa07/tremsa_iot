#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"
#include <WiFiClientSecure.h>

const char* brokerURL = BROKER_URL;
const int brokerPort = BROKER_PORT;

// TOPIC_PRESENCA subscribe
// TOPIC_PRESENCA publish
// TOPIC_SERVO1 publish
// TOPIC_SERVO2 publish

WiFiClientSecure client;
PubSubClient mqtt(client);

const int ledPin = 2;  // Pino do LED embutido

// --- Função para receber mensagens MQTT ---
void callback(char* topic, byte* payload, unsigned int length) {
  String msgRecebida = "";
  for (unsigned int i = 0; i < length; i++) {
    msgRecebida += (char)payload[i];
  }

  Serial.print("Mensagem recebida via MQTT: ");
  Serial.println(msgRecebida);

  // Verifica se é um comando para o LED
  if (msgRecebida == "1") {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED ligado via MQTT");
    mqtt.publish(mqttTopic, "LED ligado via MQTT");
  } else if (msgRecebida == "0") {
    digitalWrite(ledPin, LOW);
    Serial.println("LED desligado via MQTT");
    mqtt.publish(mqttTopic, "LED desligado via MQTT");
  }
}

long lerDistancia() {
  digitalWrite(ULTRA_TRIG3, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRA_TRIG3, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRA_TRIG3, LOW);
  
  long duracao = pulseIn(ULTRA_ECHO3, HIGH);
  long distancia = duracao * 349.24 / 2 / 10000;
  
  return distancia;
}

void setup() {
  Serial.begin(115200);
  client.setInsecure();
  
  // Configura o LED como saída
  pinMode(ledPin, OUTPUT);

  // Conexão Wi-Fi
  WiFi.begin(ENV_SSID, ENV_PASS);
  Serial.print("Conectando no WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso ao WiFi");

  // Conexão MQTT
  mqtt.setServer(brokerURL, brokerPort);
  mqtt.setCallback(callback);

  Serial.println("Conectando ao broker MQTT...");
  String boardID = "S3-" + String(random(0xffff), HEX);

  while (!mqtt.connect(boardID.c_str(), BROKER_USR_NAME, BROKER_USR_PASS)) {
    Serial.print(".");
    delay(200);
  }

  mqtt.subscribe(TOPIC_PRESENCA);  // Inscreve-se no tópico
  Serial.println("\nConectado ao broker MQTT e inscrito no tópico");
}

void loop() {
  mqtt.loop();  // Mantém a conexão MQTT ativa

  // Lê comandos pela serial
  if (Serial.available()) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();  // Remove espaços em branco

    if (comando == "1") {
      digitalWrite(ledPin, HIGH);
      Serial.println("LED ligado via Serial");
      mqtt.publish(mqttTopic, "LED ligado via Serial");
    } else if (comando == "0") {
      digitalWrite(ledPin, LOW);
      Serial.println("LED desligado via Serial");
      mqtt.publish(mqttTopic, "LED desligado via Serial");
    } else {
      Serial.println("Comando inválido. Use 1 (ligar) ou 0 (desligar).");
    }
  }
}
