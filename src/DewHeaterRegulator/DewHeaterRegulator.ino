#include <Arduino.h>

using namespace std;

const int button_output_switch_index     = 8;
const int button_voltage_increase_index  = 7;
const int button_voltage_decrease_index  = 6;
const int range_leds_indices[]    = {14,15,26,27,28,29};
const int range_leds_indices_size = sizeof(range_leds_indices)/sizeof(range_leds_indices[0]);
const int output_leds_indices[]   = {0,1,2};
const int output_leds_indices_size = sizeof(output_leds_indices)/sizeof(output_leds_indices[0]);
const int output_voltage_pins[]   = {3,4,5};
const int output_voltage_pins_size = sizeof(output_voltage_pins)/sizeof(output_voltage_pins[0]);
int output_voltages[]             = {0,0,0};

int current_output_pin = 0;
int last_change_time = 0;


void switch_output_pin() {
    current_output_pin = (current_output_pin + 1) % output_voltage_pins_size;

    Serial.print("New output pin: ");
    Serial.println(current_output_pin);
}

void decrease_output_voltage() {
    const unsigned int max_value = pow(2, range_leds_indices_size) - 1;
    output_voltages[current_output_pin] = (output_voltages[current_output_pin] - 1) % (max_value + 1);


    Serial.print("Current output: ");
    Serial.print(current_output_pin);
    Serial.print("Setting new voltage: ");
    Serial.println(output_voltages[current_output_pin]);
}


void increase_output_voltage() {
    const unsigned int max_value = pow(2, range_leds_indices_size) - 1;
    output_voltages[current_output_pin] = (output_voltages[current_output_pin] + 1) % (max_value + 1);


    Serial.print("Current output: ");
    Serial.print(current_output_pin);
    Serial.print("Setting new voltage: ");
    Serial.println(output_voltages[current_output_pin]);
}

void turn_off_all_leds() {
    for (const int i_led : range_leds_indices) {
        digitalWrite(i_led, LOW);
    }
    for (const int i_led : output_leds_indices) {
        digitalWrite(i_led, LOW);
    }
}

void update_leds() {
    for (const int i_led : output_leds_indices) {
        digitalWrite(i_led, (i_led == output_leds_indices[current_output_pin]) ? HIGH : LOW);
    }

    for (unsigned int i = 0; i < range_leds_indices_size; i++) {
        digitalWrite(range_leds_indices[i], output_voltages[current_output_pin] & (1 << i) ? HIGH : LOW);
    }
}

void set_pwm_pins() {
    const int time = millis();
    const int min_pulse_width = 50;
    const int max_value = pow(2, range_leds_indices_size) - 1;
    const int max_pulse_width = max_value*min_pulse_width;

    for (unsigned int i = 0; i < output_voltage_pins_size; i++) {
        const float value_fraction = float(output_voltages[i]) / max_value;
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
    for (const int i_led : range_leds_indices) {
        pinMode(i_led, OUTPUT);
    }
    for (const int i_led : output_leds_indices) {
        pinMode(i_led, OUTPUT);
    }

    for (const int i_led : range_leds_indices) {
        digitalWrite(i_led, HIGH);
    }
    for (const int i_led : output_leds_indices) {
        digitalWrite(i_led, HIGH);
    }

    for (const int i_pin : output_voltage_pins) {
        pinMode(i_pin, OUTPUT);
        digitalWrite(i_pin, LOW);
    }

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