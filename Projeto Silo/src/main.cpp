#include <Arduino.h>
 
#define LED_VERDE 2
#define LED_AMARELO 4
#define LED_VERMELHO 5

const int potPin = 34;
int potValue = 0;
 
void setup() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  Serial.begin(9600);
}
 
void loop() {
  potValue = analogRead(potPin);
  Serial.println(potValue);
  delay(500);
  if (potValue >= 0 && potValue <= 1000)
  {
    digitalWrite(LED_VERDE, HIGH);
    Serial.println("Led Verde acesso!");
  }
  else if (potValue >= 1001 && potValue <= 2000)
  {
    digitalWrite(LED_AMARELO, HIGH);
    Serial.println("Led Amarelo acesso!");
  }
  else 
  {
    digitalWrite(LED_VERMELHO, HIGH);
    Serial.println("Led Vermelho acesso!");
  }
}