#include <Arduino.h>
#include <LiquidCrystal.h>

// RS, E, D4, D5, D6, D7
LiquidCrystal lcd(23, 22, 19, 18, 17, 16);

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Starting LCD...");

  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Hello, world!");

  lcd.setCursor(0, 1);
  lcd.print("NodeMCU-32S LCD");
  Serial.println("LCD text written.");
}

void loop() {}
