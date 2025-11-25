//BIBLIOTECAS 
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "env.h"


//DEFINIÇÃO DE PINOS
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


//CONFIGURAÇÃO DE PWM
#define PWM_FREQ 5000
#define PWM_RES 8

// Canais de PWM para cada cor do LED RGB
#define CH_R 0
#define CH_G 1
#define CH_B 2


//OBJETOS
WiFiClientSecure client;
PubSubClient mqtt(client);
DHT dht(DHT_PIN, DHT_TYPE);

//rede
const char* WIFI_SSID = ENV_SSID;
const char* WIFI_PASS = ENV_PASS;

const char* BROKER = BROKER_URL;
const int BROKER_PORT = BROKER_PORT;
const char* TOPIC = TOPIC1;


//FUNÇÃO PARA CONTROLAR CORES
void setLED(byte r, byte g, byte b) {
  ledcWrite(CH_R, r);
  ledcWrite(CH_G, g);
  ledcWrite(CH_B, b);
}


//LED RGB INDICANDO ESTADOS
void statusLED(byte estado) {

  setLED(0, 0, 0); // Sempre apaga antes

  switch (estado) {

    case 1: // Amarelo → conectando WiFi
      setLED(255, 255, 0);
      break;

    case 2: // Roxo → conectando ao Broker
      setLED(180, 0, 255);
      break;

    case 3: // Verde → tudo OK
      setLED(0, 255, 0);
      break;

    case 254: // Vermelho → erro
      setLED(255, 0, 0);
      break;

    default: // Pisca azul → recebeu mensagem
      for (int i = 0; i < 4; i++) {
        setLED(0, 0, 255);
        delay(100);
        setLED(0, 0, 0);
        delay(100);
      }
      break;
  }
}
