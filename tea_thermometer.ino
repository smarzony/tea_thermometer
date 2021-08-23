#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define ONE_WIRE_BUS 2
#define BUZZER_PIN 7
#define LED_PIN 13
#define POTENTIOMETER_PIN A0
#define BUTTON_PIN 3

#define TEMP_EDGE_RISING 0
#define TEMP_EDGE_NOT_SET 1
#define TEMP_EDGE_FALLING 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define OLED_RESET     -4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float temp_last = 0.0;
uint8_t temp_set;
volatile uint8_t edge_type = TEMP_EDGE_NOT_SET;

unsigned long long now,
    display_refresh_timer;



void setup()
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    delay(500);
    display.clearDisplay();
    sensors.begin();
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), change_edge, FALLING);
    Serial.begin(9600);
    Serial.println("Oj tak byczq");
    edge_type = TEMP_EDGE_NOT_SET;

}

void loop()
{
    now = millis();
    // read_button_inc_switch(BUTTON_PIN, TEMP_EDGE_NOT_SET, TEMP_EDGE_FALLING, edge_type);
    if (edge_type == TEMP_EDGE_NOT_SET)
    {
        temp_set = map(analogRead(POTENTIOMETER_PIN), 0, 1023, 20, 80);
    }
    sensors.requestTemperatures(); 
    float temp_celsius = sensors.getTempCByIndex(0);

    if (now - display_refresh_timer > 500)
    {
        display_refresh_timer = now;
        if(temp_celsius != DEVICE_DISCONNECTED_C) 
        {
            Serial.print("Temperature for the device 1 (index 0) is: ");
            Serial.println(temp_celsius);
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
            display.println(String(temp_celsius, 1));
            display.setCursor(0, 27);
            display.println(temp_set);
            display.setCursor(64, 27);
            display.println(edge_type);
            display.display(); 
            temp_last = temp_celsius;
        } 
        else
        {
            Serial.println("Error: Could not read temperature data");
        }
    }
}

void change_edge() {
    unsigned long interrupt_time = millis();
    static unsigned long last_interrupt_time = 0;
    if (interrupt_time - last_interrupt_time > 200) 
    {
        edge_type += 1;
        if (edge_type > TEMP_EDGE_FALLING)
        {
            edge_type = TEMP_EDGE_RISING;
        }
    }
    last_interrupt_time = interrupt_time;
}
