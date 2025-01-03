
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>

#include <stdlib.h>
#include <cmath>

#include "ADS1X15.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

ADS1115 g_ads1115 = ADS1115(0x48);
int32_t g_start_time_measurement = -1;

#define BATTERY_VOLTAGE_PIN    0
#define HALF_INPUT_VOLTAGE_PIN 1
#define HALL_SENSOR_OUTPUT_PIN 2

double total_charge = 0;
int32_t last_time = 0;

float get_voltage(const int pin) {
    return g_ads1115.toVoltage(g_ads1115.readADC(pin));
}

float get_battery_voltage() {
    return 1.024*(78./10.)*get_voltage(BATTERY_VOLTAGE_PIN);
}

float get_current() {
    const float half_input_voltage = 0.996*get_voltage(HALF_INPUT_VOLTAGE_PIN);
    const float hall_sensor_output = 2*get_voltage(HALL_SENSOR_OUTPUT_PIN);

    return 13*(hall_sensor_output - half_input_voltage);
}

double RandomUniform()  {
    return (double(rand())+1) / (double(RAND_MAX)+2);
}

double RandomUniform(float a, float b)  {
    if (a > b) {
        return RandomUniform(b, a);
    }
    // Be carefull about the brackts, otherwise RAND_MAX will overflow
    return (b-a)*RandomUniform() + a;
}


std::string get_message(float value, std::string unit) {
    if (value >= 0.8) {
        return std::to_string(value).substr(0,5) + " " + unit;
    }
    else  {
        return std::to_string(value*1000).substr(0,5) + " m" + unit;
    }
}

std::string get_time_string(int time_milis) {
    int time_seconds = time_milis/1000;
    int time_minutes = time_seconds/60;
    int time_hours = time_minutes/60;

    time_seconds %= 60;
    time_minutes %= 60;

    return  ((time_hours >= 10) ? "" : "0") + std::to_string(time_hours) + ":" +
            ((time_minutes >= 10) ? "" : "0") + std::to_string(time_minutes) + ":" +
            ((time_seconds >= 10) ? "" : "0") + std::to_string(time_seconds);
}

void setup() {
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();

    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Hello");

    display.display();

    delay(2000);

}

void loop() {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    const int next_time = millis() + 1000;

    float avg_voltage = 0;
    float avg_current = 0;
    int n_samples = 0;

    while (millis() < next_time) {
        const float voltage = get_battery_voltage();
        const float current = get_current();

        n_samples++;

        avg_voltage += voltage;
        avg_current += current;

        delay(RandomUniform(5,20));
    }

    if (n_samples == 0) {
        return;
    }

    const int time_now = millis();
    const int time_step = time_now - last_time;
    last_time = time_now;

    avg_voltage /= n_samples;
    avg_current /= n_samples;


    total_charge += avg_current*time_step/3600000.0;

    const std::string voltage_message = get_message(avg_voltage, "V");
    const std::string current_message = get_message(avg_current, "A");
    const std::string charge_message = get_message(total_charge, "Ah");

    display.println(voltage_message.c_str());
    display.println(current_message.c_str());
    display.println(charge_message.c_str());

    const std::string time_message = get_time_string(last_time);
    display.println(time_message.c_str());

    display.display();

}