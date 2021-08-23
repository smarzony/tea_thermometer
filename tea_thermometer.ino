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
    digitalWrite(BUZZER_PIN, alarm);

    switch(edge_type)
    {
        case TEMP_EDGE_NOT_SET:        
            temp_set = float(map(analogRead(POTENTIOMETER_PIN), 0, 1023, 20, 80));
            break;        

        case TEMP_EDGE_RISING:        
            if ((temp_last < temp_set) && (temp_celsius > temp_set))
            {
                alarm = true;
                alarm_start_timer = now;
                Serial.println("Set alarm rising TRUE");
            }
            break;        

        case TEMP_EDGE_FALLING:    // TODO FALLING alarm not working
            if ((temp_last > temp_set) && (temp_celsius < temp_set))
            {
                alarm = true;
                alarm_start_timer = now;
                Serial.println("Set alarm falling TRUE");
            }
            break;    
    }


    if ( now - alarm_start_timer > 500 && alarm == true)
    {
        alarm = false;
        Serial.println("Reset alarm");
    }

    if (now - display_refresh_timer > 1000)
    {
        sensors.requestTemperatures(); 
        temp_last = temp_celsius;
        temp_celsius = sensors.getTempCByIndex(0);
        display_refresh_timer = now;
        if(temp_celsius != DEVICE_DISCONNECTED_C) 
        {
            Serial.print("Temp read: ");
            Serial.println(temp_celsius);
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(WHITE);
            display.setCursor(0, 10);
            display.print("READ: ");
            display.println(String(temp_celsius, 1));
            // display.setCursor(64, 10);
            // display.println(String(temp_last, 1));
            display.setCursor(0, 27); 
            display.print("SET:  ");          
            display.println(String(temp_set, 0));
            display.setTextSize(1);
            display.setCursor(0, 45);
            switch(edge_type)
            {
                case TEMP_EDGE_NOT_SET:
                    display.println("Not set");
                    break;
                case TEMP_EDGE_RISING:
                    display.println("Rising");
                    break;
                case TEMP_EDGE_FALLING:
                    display.println("Falling");
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
        if (edge_type > TEMP_EDGE_FALLING)
        {
            edge_type = TEMP_EDGE_RISING;
        }
    }
    last_interrupt_time = interrupt_time;
}
