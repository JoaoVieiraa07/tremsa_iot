#include <WiFiSecure.h>
#include "wifi_connection.h"
#include <PubSubClient.h>
#include "env.h"

// --- WiFi & MQTT Configuration ---
const char* SSID = "FIESC_IOT_EDU";
const char* PASS = "8120gv08";

const char* brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;
// Controle da velocidade
const char* mqttTopic = "TREM/VEL";  // Tópico usado para publish/subscribe

WiFiClient client;
PubSubClient mqtt(client);

const int motorF  = 18;
const int motorT  = 19;

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
    digitalWrite(motorF, HIGH);
    digitalWrite(motorT, LOW);

  } else if (msgRecebida == "0") {
    digitalWrite(motorF, LOW);
    digitalWrite(motorT, HIGH);

  } else if (msgRecebida == "2") {
    digitalWrite(motorF, LOW);
    digitalWrite(motorT, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  WifiClient.setInsecure();

  pinMode(motorF, OUTPUT);
  pinMode(motorT, OUTPUT);

  // Conexão Wi-Fi
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
