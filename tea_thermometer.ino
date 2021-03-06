/*
Project author: Smarzony
In order to work, install https://github.com/oshlab/Breadboard-Arduino board.
Program this with programmer AVR ISP on serial port using board ATmega328p (8mhz internal).
During programming push RESET button while output says: "Uploading"
*/

#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define ONE_WIRE_BUS 7
#define BUZZER_PIN 5
#define LED_PIN 13
#define POTENTIOMETER_PIN A0
#define BUTTON_PIN 9

#define TEMP_EDGE_RISING 0
#define TEMP_EDGE_NOT_SET 1
#define TEMP_EDGE_FALLING 2
#define TEMP_EDGE_CHANGE 3

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float temp_celsius = 0.0;
float temp_last = 0.0;
float temp_set;
volatile uint8_t edge_type = TEMP_EDGE_NOT_SET;

unsigned long long now,
    display_refresh_timer,
    alarm_start_timer;

bool alarm = false;

void setup()
{
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    delay(500);
    display.clearDisplay();
    sensors.begin();
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(BUTTON_PIN), change_edge, FALLING);
    Serial.begin(9600);
    Serial.println("Oj tak byczq");
    edge_type = TEMP_EDGE_NOT_SET;
}

void loop()
{
    now = millis();
    digitalWrite(BUZZER_PIN, !alarm);
    digitalWrite(LED_PIN, alarm);

    switch(edge_type)
    {
        case TEMP_EDGE_NOT_SET:
            temp_set = set_temp();
            break;        

        case TEMP_EDGE_RISING:
            temp_set = set_temp(); 
            if ((temp_last <= temp_set) && (temp_celsius > temp_set))
            {
                alarm = true;
                alarm_start_timer = now;
                Serial.println("Set alarm rising TRUE");
            }
            break;        

        case TEMP_EDGE_FALLING:
            temp_set = set_temp();
            if ((temp_last >= temp_set) && (temp_celsius < temp_set))
            {
                alarm = true;
                alarm_start_timer = now;
                Serial.println("Set alarm falling TRUE");
            }
            break; 

        case TEMP_EDGE_CHANGE:  
            temp_set = set_temp();
            if (((temp_last <= temp_set) && (temp_celsius > temp_set)) || ((temp_last >= temp_set) && (temp_celsius < temp_set)))
            {
                alarm = true;
                alarm_start_timer = now;
                Serial.println("Set alarm change TRUE");
            }
            break; 
    } 

    if ( now - alarm_start_timer > 100 && alarm == true)
    {
        alarm = false;
        Serial.println("Reset alarm");
    }

    if (now - display_refresh_timer > 250)
    {
        display.clearDisplay();  
        sensors.requestTemperatures(); 
        if (temp_celsius != DEVICE_DISCONNECTED_C)
            temp_last = temp_celsius;
        temp_celsius = sensors.getTempCByIndex(0);
        display_refresh_timer = now;
        if(temp_celsius != DEVICE_DISCONNECTED_C) 
        {
            Serial.print("Temp read: ");
            Serial.print(temp_celsius);
            Serial.print("\tLast: ");
            Serial.println(temp_last);
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
            display.print("READ: ");
            display.print(temp_celsius, 1);
            display.setCursor(0, 27); 
            display.print("SET:  ");          
            display.print(temp_set, 1);
            display.setTextSize(1);
            display.setCursor(0, 45);
            switch(edge_type)
            {
                case TEMP_EDGE_RISING:
                    display.println("Rising");
                    break;
                case TEMP_EDGE_NOT_SET:
                    display.println("Not set");
                    break;
                case TEMP_EDGE_FALLING:
                    display.println("Falling");
                    break;
                case TEMP_EDGE_CHANGE:
                    display.println("Change");
                    break;
            }          
            display.display();             
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
        if (edge_type > TEMP_EDGE_CHANGE)
        {
            edge_type = TEMP_EDGE_RISING;
        }
        Serial.print("Edge: ");
        Serial.println(edge_type);
    }
    last_interrupt_time = interrupt_time;
}

float set_temp()
{
    int analog_input;
    float output_value;
    analog_input = map(analogRead(0), 0, 1023, 0, 600);
    output_value = (float(analog_input) / 10) + 20.0;
    return output_value;
}
