#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"
#include <WiFiClientSecure.h>
#include <Servo.h>

const char* brokerURL = BROKER_URL;
const int brokerPort = BROKER_PORT;

const int ULTRA_ECHO3 = 18;
const int ULTRA_TRIG3 = 19;

Servo SERVO_1;
Servo SERVO_2;

// ENV_TOPIC_PRESENCA subscribe
// ENV_TOPIC_PRESENCA publish
// ENV_TOPIC_SERVO1 publish
// ENV_TOPIC_SERVO2 publish

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

  // Verifica se é um comando para os servos
  if (msgRecebida == "1") {
      SERVO_1.write(180);
      delay(1000);
      SERVO_1.write(0);
    } else if (msgRecebida == "2") {
      SERVO_2.write(180);
      delay(1000);
      SERVO_2.write(0);
    } else {
      Serial.printf("Comando invalido");
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
  
  // Configura as entradas/saidas
  pinMode(ledPin, OUTPUT);
  pinMode(ULTRA_TRIG3, OUTPUT);
  pinMode(ULTRA_ECHO3, INPUT);

  SERVO_1.attach(22);
  SERVO_2.attach(23);

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

  mqtt.subscribe(ENV_TOPIC_PRESENCA);  // Inscreve-se no tópico
  Serial.println("\nConectado ao broker MQTT e inscrito no tópico");
}

void loop() {
  mqtt.loop();  // Mantém a conexão MQTT ativa

  long distancia = lerDistancia();

  Serial.printf("Distância: %ld cm.\n", distancia);

  if (distancia > 0 && distancia < 5) {
    mqtt.publish(ENV_TOPIC_PRESENCA, "1");
    Serial.println("Trem detectado!");
    delay(300);
  }

  // Lê comandos pela serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();  // Remove espaços em branco
    
    if (cmd == "1") {
      SERVO_1.write(180);
      delay(1000);
      SERVO_1.write(0);
    } else if (cmd == "2") {
      SERVO_2.write(180);
      delay(1000);
      SERVO_2.write(0);
    } else {
      Serial.printf("Comando invalido");
    }
  }
}
