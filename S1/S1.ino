#include <WiFi.h>

const String SSID = "FIESC_IOT_EDU";
const String PASS = "8120gv08";

void setup() {
  Serial.begin(115200); //configura a placa para mostrar
  WiFi.begin(SSID, PASS);// tenta conectar na rede
  Serial.println("Conectando no Wifi");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.print("Conectado com sucesso");

  

}

void loop() {
  // put your main code here, to run repeatedly:

}
