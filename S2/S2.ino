#include <Servo.h>

enum Estado { IDLE, RUNNING, APPROACH_STATION, STOPPED_AT_STATION, DEPARTING, SWITCHING };

const int PIN_MOTOR_PWM = 5;
const int PIN_MOTOR_DIR = 4;
const int PIN_SENSOR_A = 2;
const int PIN_SENSOR_B = 3;
const int PIN_BUTTON_START = 8;
const int PIN_LED_RED = 6;
const int PIN_LED_GREEN = 7;
const int PIN_SERVO = 9;

Servo chave;
Estado estado = IDLE;

int velocidadeAlvo = 180;
int velocidadeAtual = 0;
unsigned long ultimaRampa = 0;
const int passoRampa = 5;
const unsigned long intervaloRampa = 40;

bool rotaAlternativa = false;
unsigned long tempoParada = 4000;
unsigned long inicioParada = 0;

struct Debounce { int pin; int estado; int ultimoLeitura; unsigned long ultimaMudanca; unsigned long atraso; };
Debounce dbSensorA{PIN_SENSOR_A, HIGH, HIGH, 0, 30};
Debounce dbSensorB{PIN_SENSOR_B, HIGH, HIGH, 0, 30};
Debounce dbBotao{PIN_BUTTON_START, HIGH, HIGH, 0, 50};

void configuraPinos() {
  pinMode(PIN_MOTOR_PWM, OUTPUT);
  pinMode(PIN_MOTOR_DIR, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_SENSOR_A, INPUT_PULLUP);
  pinMode(PIN_SENSOR_B, INPUT_PULLUP);
  pinMode(PIN_BUTTON_START, INPUT_PULLUP);
}

int lerDebounce(Debounce &d) {
  int leitura = digitalRead(d.pin);
  if (leitura != d.ultimoLeitura) { d.ultimaMudanca = millis(); d.ultimoLeitura = leitura; }
  if (millis() - d.ultimaMudanca > d.atraso) d.estado = leitura;
  return d.estado;
}

bool bordaDescida(Debounce &d) {
  static int prevA = dbSensorA.estado, prevB = dbSensorB.estado, prevBtn = dbBotao.estado;
  int prev;
  if (&d == &dbSensorA) prev = prevA; else if (&d == &dbSensorB) prev = prevB; else prev = prevBtn;
  int atual = lerDebounce(d);
  bool borda = (prev == HIGH && atual == LOW);
  if (&d == &dbSensorA) prevA = atual; else if (&d == &dbSensorB) prevB = atual; else prevBtn = atual;
  return borda;
}

void motor(int velocidade) {
  if (velocidade < 0) velocidade = 0; if (velocidade > 255) velocidade = 255;
  digitalWrite(PIN_MOTOR_DIR, HIGH);
  analogWrite(PIN_MOTOR_PWM, velocidade);
}

void sinais(bool aberto) {
  digitalWrite(PIN_LED_GREEN, aberto ? HIGH : LOW);
  digitalWrite(PIN_LED_RED, aberto ? LOW : HIGH);
}

void chavePosicao(bool alt) {
  chave.write(alt ? 100 : 0);
}

void rampa() {
  if (millis() - ultimaRampa >= intervaloRampa) {
    ultimaRampa = millis();
    if (velocidadeAtual < velocidadeAlvo) velocidadeAtual += passoRampa;
    else if (velocidadeAtual > velocidadeAlvo) velocidadeAtual -= passoRampa;
    if (velocidadeAtual < 0) velocidadeAtual = 0; if (velocidadeAtual > 255) velocidadeAtual = 255;
    motor(velocidadeAtual);
  }
}

void comandoSerial() {
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s.startsWith("S")) { int v = s.substring(1).toInt(); if (v >= 0 && v <= 255) velocidadeAlvo = v; }
    else if (s.equalsIgnoreCase("GO")) { estado = RUNNING; }
    else if (s.equalsIgnoreCase("STOP")) { estado = IDLE; }
    else if (s.startsWith("R")) { int r = s.substring(1).toInt(); rotaAlternativa = r == 1; chavePosicao(rotaAlternativa); }
    else if (s.startsWith("T")) { unsigned long t = s.substring(1).toInt(); if (t >= 1000 && t <= 30000) tempoParada = t; }
  }
}

void atualizaEstado() {
  int sa = lerDebounce(dbSensorA);
  int sb = lerDebounce(dbSensorB);
  if (bordaDescida(dbBotao)) {
    if (estado == IDLE) estado = RUNNING; else estado = IDLE;
  }
  switch (estado) {
    case IDLE:
      velocidadeAlvo = 0;
      sinais(false);
      break;
    case RUNNING:
      sinais(true);
      velocidadeAlvo = 180;
      if (sa == LOW) estado = APPROACH_STATION;
      break;
    case APPROACH_STATION:
      velocidadeAlvo = 120;
      if (sb == LOW) { estado = STOPPED_AT_STATION; inicioParada = millis(); velocidadeAlvo = 0; }
      break;
    case STOPPED_AT_STATION:
      velocidadeAlvo = 0;
      sinais(false);
      if (millis() - inicioParada >= tempoParada) estado = DEPARTING;
      break;
    case DEPARTING:
      sinais(true);
      velocidadeAlvo = 160;
      if (sa == HIGH && sb == HIGH) estado = RUNNING;
      break;
    case SWITCHING:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  configuraPinos();
  chave.attach(PIN_SERVO);
  chavePosicao(rotaAlternativa);
  sinais(false);
  motor(0);
}

void loop() {
  comandoSerial();
  atualizaEstado();
  rampa();
}
