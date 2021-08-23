#include <DallasTemperature.h>


void setup()
{
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    Serial.println("Oj tak byczq");
}

void loop()

{
    Serial.println("+1");
    digitalWrite(13, LOW);
    delay(500);
    digitalWrite(13, HIGH);
    delay(500);
}