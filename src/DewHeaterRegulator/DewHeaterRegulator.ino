
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>

#include <stdlib.h>
#include <cmath>
#include <thread>

#include "VoltageCurrentMeasurement.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_ADDR   0x3C


#define VOLTAGE_PIN 26
#define VOLTAGE_CONST 0.0244
#define CURRENT_PIN 27
#define CURRENT_CONST 0.01135

using namespace std;

const int32_t g_button_output_switch_index     = 26;
const int32_t g_button_voltage_increase_index  = 15;
const int32_t g_button_voltage_decrease_index  = 14;
const int32_t g_output_leds_indices[]   = {6,7,8};
const int32_t g_output_leds_indices_size = sizeof(g_output_leds_indices)/sizeof(g_output_leds_indices[0]);
const int32_t g_output_voltage_pins[]   = {0,1,2};
const int32_t g_output_voltage_pins_size = sizeof(g_output_voltage_pins)/sizeof(g_output_voltage_pins[0]);
const int32_t g_max_voltage             = 100;

const int32_t g_voltage_pin = 27;
const int32_t g_current_pin = 28;
const float g_voltage_const = 0.0244;
const float g_current_const = 0.01135;

int32_t g_output_voltages[]             = {0,0,0};

int32_t g_current_output_pin = 0;   // 3 = show current, voltage and total charge
int32_t g_last_change_time = 0;

Adafruit_SSD1306 g_display(OLED_WIDTH, OLED_HEIGHT);

VoltageCurrentMeasurement g_voltage_current_measurement(g_voltage_pin, g_current_pin, g_voltage_const, g_current_const);

void switch_output_pin() {
    g_current_output_pin = (g_current_output_pin + 1) % (g_output_voltage_pins_size+1);

    Serial.print("New output pin: ");
    Serial.println(g_current_output_pin);
}

void decrease_output_voltage() {
    if (g_current_output_pin == g_output_voltage_pins_size) {
        return;
    }
    g_output_voltages[g_current_output_pin] = (g_output_voltages[g_current_output_pin] + g_max_voltage) % (g_max_voltage+1);
}


void increase_output_voltage() {
    if (g_current_output_pin == g_output_voltage_pins_size) {
        return;
    }
    g_output_voltages[g_current_output_pin] = (g_output_voltages[g_current_output_pin] + 1) % (g_max_voltage+1);
}

void turn_off_all_leds() {
    for (const int32_t i_led : g_output_leds_indices) {
        digitalWrite(i_led, LOW);
    }
    g_display.clearDisplay();
    g_display.display();
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


void update_leds() {
    for (const int32_t i_led : g_output_leds_indices) {
        digitalWrite(i_led, (i_led == g_output_leds_indices[g_current_output_pin]) ? HIGH : LOW);
    }
    if (g_current_output_pin < g_output_voltage_pins_size) {
        g_display.clearDisplay();

        g_display.setTextSize(3);
        g_display.setTextColor(WHITE);
        g_display.setCursor(0, 0);
        const std::string message = std::to_string(g_output_voltages[g_current_output_pin]) + " %";
        g_display.println(message.c_str());

        g_display.display();
    }
    else {
        g_display.clearDisplay();

        g_display.setTextSize(2);
        g_display.setTextColor(WHITE);
        g_display.setCursor(0, 0);

        g_display.println(VoltageCurrentMeasurement::get_message(g_voltage_current_measurement.get_voltage(), "V").c_str());
        g_display.println(VoltageCurrentMeasurement::get_message(g_voltage_current_measurement.get_current(), "A").c_str());
        g_display.println(VoltageCurrentMeasurement::get_message(g_voltage_current_measurement.get_total_charge(), "Ah").c_str());
        g_display.println(get_time_string(millis()).c_str());
        g_display.display();
    }

}

void set_pwm_pins() {
    const int32_t time = millis();
    const int32_t min_pulse_width = 50;
    const int32_t max_pulse_width = g_max_voltage*min_pulse_width;

    for (uint32_t i = 0; i < g_output_voltage_pins_size; i++) {
        if ((time % max_pulse_width)/min_pulse_width < g_output_voltages[i] ) {
            digitalWrite(g_output_voltage_pins[i], HIGH);
        } else {
            digitalWrite(g_output_voltage_pins[i], LOW);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // put your setup code here, to run once:
    pinMode(g_button_output_switch_index, INPUT);
    pinMode(g_button_voltage_increase_index, INPUT);
    pinMode(g_button_voltage_decrease_index, INPUT);

    pinMode(g_voltage_pin, INPUT);
    pinMode(g_current_pin, INPUT);

    for (const int32_t i_led : g_output_leds_indices) {
        pinMode(i_led, OUTPUT);
    }

    for (const int32_t i_led : g_output_leds_indices) {
        digitalWrite(i_led, HIGH);
    }

    for (const int32_t i_pin : g_output_voltage_pins) {
        pinMode(i_pin, OUTPUT);
        digitalWrite(i_pin, LOW);
    }

    g_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    g_display.clearDisplay();

    g_display.setTextSize(3);
    g_display.setTextColor(WHITE);
    g_display.setCursor(0, 0);
    g_display.println("Hello");

    g_display.display();

    delay(5000);
    Serial.print("Setup done");

}

void loop() {
    set_pwm_pins();
    g_voltage_current_measurement.measure();

    if (millis() - g_last_change_time > 15000 && g_current_output_pin < g_output_voltage_pins_size) {
        turn_off_all_leds();
    }
    else {
        update_leds();
    }

    bool output_switch_button_pressed = false;
    while (digitalRead(g_button_output_switch_index) == HIGH) {
        output_switch_button_pressed = true;
        g_last_change_time = millis();
        delay(100);

        Serial.println("g_button_output_switch_index pressed");
    }
    if (output_switch_button_pressed) {
        switch_output_pin();
        update_leds();
    }

    bool voltage_increase_button_pressed = false;
    while (digitalRead(g_button_voltage_increase_index) == HIGH) {
        voltage_increase_button_pressed = true;
        g_last_change_time = millis();
        delay(100);
        Serial.println("g_button_voltage_increase_index pressed");
    }
    if (voltage_increase_button_pressed) {
        increase_output_voltage();
        update_leds();
    }

    bool voltage_decrease_button_pressed = false;
    while (digitalRead(g_button_voltage_decrease_index) == HIGH) {
        voltage_decrease_button_pressed = true;
        g_last_change_time = millis();
        delay(100);
        Serial.println("g_button_voltage_decrease_index pressed");
    }
    if (voltage_decrease_button_pressed) {
        decrease_output_voltage();
        update_leds();
    }

    delay(2);
}