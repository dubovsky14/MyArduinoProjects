
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>

#include <stdlib.h>
#include <cmath>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_ADDR   0x3C


#define VOLTAGE_PIN 26
#define VOLTAGE_CONST 0.0244
#define CURRENT_PIN 27
#define CURRENT_CONST 0.01135

using namespace std;

const int32_t button_output_switch_index     = 26;
const int32_t button_voltage_increase_index  = 15;
const int32_t button_voltage_decrease_index  = 14;
const int32_t output_leds_indices[]   = {6,7,8};
const int32_t output_leds_indices_size = sizeof(output_leds_indices)/sizeof(output_leds_indices[0]);
const int32_t output_voltage_pins[]   = {0,1,2};
const int32_t output_voltage_pins_size = sizeof(output_voltage_pins)/sizeof(output_voltage_pins[0]);
const int32_t max_voltage             = 100;
int32_t output_voltages[]             = {0,0,0};

int32_t current_output_pin = 0;
int32_t last_change_time = 0;

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

void switch_output_pin() {
    current_output_pin = (current_output_pin + 1) % output_voltage_pins_size;

    Serial.print("New output pin: ");
    Serial.println(current_output_pin);
}

void decrease_output_voltage() {
    output_voltages[current_output_pin] = (output_voltages[current_output_pin] + max_voltage) % (max_voltage+1);

    Serial.print("Current output: ");
    Serial.print(current_output_pin);
    Serial.print("Setting new voltage: ");
    Serial.println(output_voltages[current_output_pin]);
}


void increase_output_voltage() {
    output_voltages[current_output_pin] = (output_voltages[current_output_pin] + 1) % (max_voltage+1);

    Serial.print("Current output: ");
    Serial.print(current_output_pin);
    Serial.print("Setting new voltage: ");
    Serial.println(output_voltages[current_output_pin]);
}

void turn_off_all_leds() {
    for (const int32_t i_led : output_leds_indices) {
        digitalWrite(i_led, LOW);
    }
    display.clearDisplay();
    display.display();
}

void update_leds() {
    for (const int32_t i_led : output_leds_indices) {
        digitalWrite(i_led, (i_led == output_leds_indices[current_output_pin]) ? HIGH : LOW);
    }
    display.clearDisplay();

    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    const std::string message = std::to_string(output_voltages[current_output_pin]) + " %";
    display.println(message.c_str());

    display.display();
}

void set_pwm_pins() {
    const int32_t time = millis();
    const int32_t min_pulse_width = 50;
    const int32_t max_pulse_width = max_voltage*min_pulse_width;

    for (uint32_t i = 0; i < output_voltage_pins_size; i++) {
        const float value_fraction = float(output_voltages[i]) / max_voltage;
        if (time % max_pulse_width < output_voltages[i] * min_pulse_width) {
            digitalWrite(output_voltage_pins[i], HIGH);
        } else {
            digitalWrite(output_voltage_pins[i], LOW);
        }
    }
}

void setup() {
    Serial.begin(115200);

    // put your setup code here, to run once:
    pinMode(button_output_switch_index, INPUT);
    pinMode(button_voltage_increase_index, INPUT);
    pinMode(button_voltage_decrease_index, INPUT);

    for (const int32_t i_led : output_leds_indices) {
        pinMode(i_led, OUTPUT);
    }

    for (const int32_t i_led : output_leds_indices) {
        digitalWrite(i_led, HIGH);
    }

    for (const int32_t i_pin : output_voltage_pins) {
        pinMode(i_pin, OUTPUT);
        digitalWrite(i_pin, LOW);
    }

    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();

    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Hello");

    display.display();

    delay(5000);
    Serial.print("Setup done");
}

void loop() {
    set_pwm_pins();
    if (millis() - last_change_time > 5000) {
        turn_off_all_leds();
    }

    // put your main code here, to run repeatedly:
    bool output_switch_button_pressed = false;
    while (digitalRead(button_output_switch_index) == HIGH) {
        output_switch_button_pressed = true;
        last_change_time = millis();
        delay(100);

        Serial.println("button_output_switch_index pressed");
    }
    if (output_switch_button_pressed) {
        switch_output_pin();
        update_leds();
    }

    bool voltage_increase_button_pressed = false;
    while (digitalRead(button_voltage_increase_index) == HIGH) {
        voltage_increase_button_pressed = true;
        last_change_time = millis();
        delay(100);
        Serial.println("button_voltage_increase_index pressed");
    }
    if (voltage_increase_button_pressed) {
        increase_output_voltage();
        update_leds();
    }

    bool voltage_decrease_button_pressed = false;
    while (digitalRead(button_voltage_decrease_index) == HIGH) {
        voltage_decrease_button_pressed = true;
        last_change_time = millis();
        delay(100);
        Serial.println("button_voltage_decrease_index pressed");
    }
    if (voltage_decrease_button_pressed) {
        decrease_output_voltage();
        update_leds();
    }
}