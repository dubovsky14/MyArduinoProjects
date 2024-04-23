
#include <Arduino.h>
#include <string>

#include <Wire.h>
#include "ADS1X15.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

ADS1115 g_ads1115 = ADS1115(0x48);
int32_t g_start_time_measurement = -1;

void setup() {
    Serial.begin(9600);

    delay(8000);
    Serial.println("Going to set ADS1115!");
    Wire.begin();
    const bool a = g_ads1115.begin();
    Serial.print("ADS1115 set: ");
    Serial.println(a);
}

float get_voltage(const int pin) {
    return g_ads1115.toVoltage(g_ads1115.readADC(pin));
}

void loop() {
    if (!g_ads1115.isConnected()) {
        Serial.println("ADS1115 not connected!");
        delay(1000);
        return;
    }

    const float voltage_source = get_voltage(0);
    const float voltage_drain = 11.013*get_voltage(1);
    const float voltage_gate = (1329./329.)*get_voltage(3);

    if (g_start_time_measurement == -1 && voltage_source > 0.2) {
        g_start_time_measurement = millis();
    }

    if (g_start_time_measurement == -1) {
        return;
    }

    Serial.print("Vs = ");
    Serial.print(voltage_source);
    Serial.print("   Vg = ");
    Serial.print(voltage_gate);
    Serial.print("   Vd = ");
    Serial.println(voltage_drain-voltage_source);
    delay(1);

    if (voltage_source < 0.01) {
        Serial.println("End of measurement!");
        g_start_time_measurement = -1;
    }
}