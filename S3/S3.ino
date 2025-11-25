#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"
#include <WiFiClientSecure.h>
#include <Servo.h>
#include "led.h"

const char* brokerURL = BROKER_URL;
const int brokerPort = BROKER_PORT;

const int ULTRA_ECHO3 = 18;
const int ULTRA_TRIG3 = 19;

const int LED_R = 14;
const int LED_G = 26;
const int LED_B = 25;

// CONFIGURAÇÃO DE PWM
const int PWM_FREQ = 5000;
const int PWM_RES = 8;

const int CH_R = 0;
const int CH_G = 1;
const int CH_B = 2;

Servo SERVO_1;
Servo SERVO_2;

volatile int comandoServo = 0;

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

  statusLED(0);

  // Verifica se é um comando para os servos
  if (msgRecebida == "1") {
      comandoServo = 1;
    } else if (msgRecebida == "2") {
      comandoServo = 2;
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

  // PWM RGB
  ledcSetup(CH_R, PWM_FREQ, PWM_RES);
  ledcSetup(CH_G, PWM_FREQ, PWM_RES);
  ledcSetup(CH_B, PWM_FREQ, PWM_RES);
  
  ledcAttachPin(LED_R, CH_R);
  ledcAttachPin(LED_G, CH_G);
  ledcAttachPin(LED_B, CH_B);

  SERVO_1.attach(22);
  SERVO_2.attach(23);

  // Conexão Wi-Fi
  statusLED(1);
  wifi_connect(ENV_SSID, ENV_PASS);
  
  // Conexão MQTT
  statusLED(2);
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

  if (comandoServo == 1) {
    SERVO_1.write(180);
    delay(700);
    SERVO_1.write(0);
    comandoServo = 0;
  } else if (comandoServo == 2) {
    SERVO_2.write(180);
    delay(700);
    SERVO_2.write(0);
    comandoServo = 0;
  }

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
