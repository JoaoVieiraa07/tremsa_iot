#include <WiFi.h>

void wifi_connect(const char* SSID, const char* PASS) {
    // Conexão Wi-Fi
    WiFi.begin(SSID, PASS);
    Serial.print("Conectando no WiFi");
    
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(200);
    }
    Serial.println("\nConectado com sucesso ao WiFi");

    return;
}