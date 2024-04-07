#include <Arduino.h>


using namespace std;

const int button_output_switch_index      = 0;
const int button_voltage_regulator_index  = 1;
const int range_leds_indices[]    = {2,3,4,5};
const int range_leds_indices_size = sizeof(range_leds_indices)/sizeof(range_leds_indices[0]);
const int output_leds_indices[]   = {6,7};
const int output_leds_indices_size = sizeof(output_leds_indices)/sizeof(output_leds_indices[0]);
const int output_voltage_pins[]   = {8,9};
const int output_voltage_pins_size = sizeof(output_voltage_pins)/sizeof(output_voltage_pins[0]);
int output_voltages[]             = {0,0};

int current_output_pin = 0;
int last_change_time = 0;


void switch_output_pin() {
    current_output_pin = (current_output_pin + 1) % output_voltage_pins_size;

    Serial.print("New output pin: ");
    Serial.println(current_output_pin);
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

void setup() {
    Serial.begin(115200);

    // put your setup code here, to run once:
    pinMode(button_output_switch_index, INPUT);
    pinMode(button_voltage_regulator_index, INPUT);
    for (const int i_led : range_leds_indices) {
        pinMode(i_led, OUTPUT);
    }
    for (const int i_led : output_leds_indices) {
        pinMode(i_led, OUTPUT);
    }
    
    //debug
    pinMode(14,OUTPUT);
    digitalWrite(14,HIGH);

    for (const int i_led : range_leds_indices) {
        digitalWrite(i_led, HIGH);
    }
    for (const int i_led : output_leds_indices) {
        digitalWrite(i_led, HIGH);
    }

    delay(5000);
    Serial.print("Setup done");
    digitalWrite(14,LOW);
}

void loop() {
    if (millis() - last_change_time > 5000) {
        turn_off_all_leds();
    }

    // put your main code here, to run repeatedly:
    bool output_switch_button_pressed = false;
    while (digitalRead(button_output_switch_index) == HIGH) {
        output_switch_button_pressed = true;
        last_change_time = millis();        
        digitalWrite(14,HIGH);
        delay(100);
        digitalWrite(14,LOW);
        delay(100);

        Serial.println("button_output_switch_index pressed");
    }
    if (output_switch_button_pressed) {
        switch_output_pin();
        update_leds();
    }

    bool voltage_regulator_button_pressed = false;
    while (digitalRead(button_voltage_regulator_index) == HIGH) {
        voltage_regulator_button_pressed = true;
        last_change_time = millis();
        digitalWrite(14,HIGH);
        delay(100);
        digitalWrite(14,LOW);
        delay(1000);
        Serial.println("button_voltage_regulator_index pressed");
    }
    if (voltage_regulator_button_pressed) {
        increase_output_voltage();
        update_leds();
    }
}