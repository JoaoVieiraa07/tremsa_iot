// FUNÇÃO RGB
void setLED(byte r, byte g, byte b) {
  ledcWrite(CH_R, r);
  ledcWrite(CH_G, g);
  ledcWrite(CH_B, b);
}

// LED RGB INDICANDO ESTADOS
void statusLED(byte estado) {

  setLED(0, 0, 0); // apaga tudo

  switch (estado) {
    case 1: setLED(255, 255, 0); break;   // amarelo
    case 2: setLED(180, 0, 255); break;   // roxo
    case 3: setLED(0, 255, 0); break;     // verde
    case 254: setLED(255, 0, 0); break;   // vermelho

    default: // pisca azul
      for (int i = 0; i < 4; i++) {
        setLED(0, 0, 255);
        delay(100);
        setLED(0, 0, 0);
        delay(100);
      }
      break;
  }
}