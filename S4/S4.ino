#include <WiFi.h>
#include <PubSubClient.h>
#include "env.h"
#include <Servo.h>

// --- WiFi & MQTT Configuration ---
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;
// Controle da velocidade
const char* mqttTopic = "TREM/VEL";  // Tópico usado para publish/subscribe

Servo motorF;
Servo motorT;

WiFiClient client;
PubSubClient mqtt(client);

const int ledPin        = 2;  // Pino do LED embutido
const int ledVermelho   = 21;
const int ledVerde      = 18;

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
    motorF.write(180);
    Serial.println("Motor Frente mexido 180 graus via MQTT");
    mqtt.publish(mqttTopic, "Motor Frente mexido via MQTT");

    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledVermelho, LOW);

  } else if (msgRecebida == "0") {
    motorT.write(180);
    Serial.println("Motor TRAS mexido 180 graus via MQTT");
    mqtt.publish(mqttTopic, "Motor TRAS mexido 180 graus via MQTT");
    
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, HIGH);

  } else if (msgRecebida == "2") {
    motorT.write(0);
    motorF.write(0);
    Serial.println("Trem parado via MQTT");

    digitalWrite(ledVerde, LOW);
    digitalWrite(ledVermelho, LOW);
    
  }
}

void setup() {
  Serial.begin(115200);
  WifiClient.setInsecure();
  
  // Configura os LEDs
  pinMode(ledPin, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledVerde, OUTPUT);

  // Configura os Servos
  motorF.attach(13);
  motorT.attach(12);

  // Conexão Wi-Fi
  WiFi.begin(SSID, PASS);
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
  String boardID = "S4-" + String(random(0xffff), HEX);

  while (!mqtt.connect(boardID.c_str())) {
    Serial.print(".");
    delay(200);
  }

  mqtt.subscribe(mqttTopic);  // Inscreve-se no tópico
  Serial.println("\nConectado ao broker MQTT e inscrito no tópico");
}

void loop() {
  mqtt.loop();  // Mantém a conexão MQTT ativa

  // Lê comandos pela serial
//   if (Serial.available()) {
//     String comando = Serial.readStringUntil('\n');
//     comando.trim();  // Remove espaços em branco

//     if (comando == "1") {
//       digitalWrite(ledPin, HIGH);
//       Serial.println("LED ligado via Serial");
//       mqtt.publish(mqttTopic, "LED ligado via Serial");
//     } else if (comando == "0") {
//       digitalWrite(ledPin, LOW);
//       Serial.println("LED desligado via Serial");
//       mqtt.publish(mqttTopic, "LED desligado via Serial");
//     } else {
//       Serial.println("Comando inválido. Use 1 (ligar) ou 0 (desligar).");
//     }
//   }
}