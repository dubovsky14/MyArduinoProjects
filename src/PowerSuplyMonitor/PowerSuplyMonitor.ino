
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <string>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);

#define VOLTAGE_PIN 26
#define VOLTAGE_CONST 0.0244
#define CURRENT_PIN 27
#define CURRENT_CONST 0.01135

double total_charge = 0;
int32_t last_time = 0;

void setup() {
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();

    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Hello");

    display.display();

    delay(2000);

    pinMode(VOLTAGE_PIN, INPUT);
    pinMode(CURRENT_PIN, INPUT);

}

std::string get_message(float value, std::string unit) {
    if (value >= 0.8) {
        return std::to_string(value).substr(0,5) + " " + unit;
    }
    else  {
        return std::to_string(value*1000).substr(0,5) + " m" + unit;
    }
}

void loop() {
    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);


    float voltage = VOLTAGE_CONST*analogRead(VOLTAGE_PIN);
    float current = CURRENT_CONST*analogRead(CURRENT_PIN);

    total_charge += current*(millis() - last_time)/3600000.0;
    last_time = millis();

    const std::string voltage_message = get_message(voltage, "V");
    const std::string current_message = get_message(current, "A");
    const std::string charge_message = get_message(total_charge, "Ah");

    display.println(voltage_message.c_str());
    display.println(current_message.c_str());
    display.println(charge_message.c_str());

    display.display();

    int delay_step = 500;
    delay(delay_step);
}