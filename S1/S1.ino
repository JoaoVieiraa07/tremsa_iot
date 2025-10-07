#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;  //cria objeto p/ wifi
PubSubClient mqtt(client); //cria objeto p/ mqtt usando WiFi

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

const String brokerURL = "test.mosquitto.org";
const int brokerPort = 1883;

const String brokerUser = ""; //variável para o user do broker
const String brokerPass = ""; //variável para a senha do broker

void setup() {
  Serial.begin(115200); //configura a placa para mostrar
  WiFi.begin(SSID, PASS);// tenta conectar na rede
  Serial.println("Conectando no Wifi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso");

  String boardID = "S1-";
  boardID += String(random(0xffff),HEX);

  mqtt.setServer(brokerURL.c_str(), brokerPort);
  Serial.println("Conectando no Broker");

  while(!mqtt.connect(boardID.c_str())){
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado com sucesso ao broker!");
}

void loop() {
  // put your main code here, to run repeatedly:

}
