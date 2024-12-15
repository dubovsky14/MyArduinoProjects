
#include "VoltageCurrentMeasurement.h"

#include <Wire.h>
#include "ADS1X15.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>

#include <vector>
#include <stdlib.h>
#include <cmath>


#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_ADDR   0x3C


const int32_t g_button_output_switch_index     = 26;
const int32_t g_button_voltage_increase_index  = 15;
const int32_t g_button_voltage_decrease_index  = 14;
const int32_t g_output_leds_indices[]   = {2,1,0};
const int32_t g_output_leds_indices_size = sizeof(g_output_leds_indices)/sizeof(g_output_leds_indices[0]);
const int32_t g_output_voltage_pins[]   = {6,7,8};
const int32_t g_output_voltage_pins_size = sizeof(g_output_voltage_pins)/sizeof(g_output_voltage_pins[0]);
const int32_t g_max_voltage             = 100;
const int32_t g_min_pulse_width = 50;
const int32_t g_max_pulse_width = g_max_voltage*g_min_pulse_width;

const int32_t g_voltage_pin = 3;    // on ADS1115
const int32_t g_current_pin = 1;    // on ADS1115
const float g_voltage_const = 7.6;
const float g_current_const = -43;

int32_t g_output_voltages[]             = {0,0,0};

int32_t g_current_output_pin = 0;   // 3 = show current, voltage and total charge
int32_t g_last_change_time = 0;

ADS1115 g_ads1115 = ADS1115(0x48);

Adafruit_SSD1306 g_display(OLED_WIDTH, OLED_HEIGHT);

VoltageCurrentMeasurement g_voltage_current_measurement(&g_ads1115, g_voltage_pin, g_current_pin, g_voltage_const, g_current_const);


using namespace std;

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

void print_messages_to_display(const std::vector<std::string> &message, int text_size) {
    g_display.clearDisplay();

    g_display.setTextSize(text_size);
    g_display.setTextColor(WHITE);
    g_display.setCursor(0, 0);
    for (const std::string &msg : message) {
        g_display.println(msg.c_str());
    }
    g_display.display();
}

void update_leds() {
    for (int32_t i_led = 0; i_led < g_output_leds_indices_size; i_led++) {
        if (i_led == g_current_output_pin)  {
            analogWrite(g_output_leds_indices[i_led], 5);
        }
        else {
            digitalWrite(g_output_leds_indices[i_led], LOW);
        }
    }
    if (g_current_output_pin < g_output_voltage_pins_size) {
        const std::string message = std::to_string(g_output_voltages[g_current_output_pin]) + " %";
        print_messages_to_display({message}, 3);
    }
    else {
        std::vector<std::string> messages   =   {
                VoltageCurrentMeasurement::get_message(g_voltage_current_measurement.get_voltage(), "V"),
                VoltageCurrentMeasurement::get_message(g_voltage_current_measurement.get_current(), "A"),
                VoltageCurrentMeasurement::get_message(g_voltage_current_measurement.get_total_charge(), "Ah"),
                get_time_string(millis())
        };
        print_messages_to_display(messages, 2);
    }
}

void set_pwm_pins() {
    const int32_t time = millis();
    for (uint32_t i = 0; i < g_output_voltage_pins_size; i++) {
        if ((time % g_max_pulse_width)/g_min_pulse_width < g_output_voltages[i] ) {
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

    for (const int32_t i_led : g_output_leds_indices) {
        pinMode(i_led, OUTPUT);
        digitalWrite(i_led, HIGH);
    }

    for (const int32_t i_pin : g_output_voltage_pins) {
        pinMode(i_pin, OUTPUT);
        digitalWrite(i_pin, LOW);
    }

    g_ads1115.begin();
    g_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);

    print_messages_to_display({"Hello"}, 3);

    g_voltage_current_measurement.set_noise_level(0.005, 0.005);
    g_voltage_current_measurement.set_average_time(g_max_pulse_width);
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

    auto process_button_signal = [](int32_t button_index, void (*action)()) {
        bool button_pressed = false;
        if (digitalRead(button_index) == HIGH) {
            button_pressed = true;
            delay(50);
        }
        if (button_pressed && digitalRead(button_index) == HIGH) {  // breadboard is too small, I can't fit a capacitor there :D
            action();
            update_leds();
            g_last_change_time = millis();
        }
    };

    process_button_signal(g_button_output_switch_index, switch_output_pin);
    process_button_signal(g_button_voltage_increase_index, increase_output_voltage);
    process_button_signal(g_button_voltage_decrease_index, decrease_output_voltage);

    delay(2);
}