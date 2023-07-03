#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>

#define SDA 4
#define SCL 5
#define BUTTON 0
#define GRN_LED 13
#define YLW_LED 14
#define RED_LED 12

void setup() {
  Wire.begin(SDA, SCL);
  oled.init();
  oled.clearDisplay();
  oled.setTextXY(0, 0);
  oled.putString("Hello World!");
  oled.setTextXY(1,0);          // row 1 col 0
  oled.putString("Button Pushed?");
  oled.setTextXY(2,0);
  oled.putString("No");
  
  pinMode(BUTTON, INPUT);
  pinMode(GRN_LED, OUTPUT);
  pinMode(YLW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  digitalWrite(GRN_LED, HIGH);
  digitalWrite(YLW_LED, HIGH);
  digitalWrite(RED_LED, HIGH);
  delay(1000);
  digitalWrite(GRN_LED, LOW);
  digitalWrite(YLW_LED, LOW);
  digitalWrite(RED_LED, LOW);
  delay(1000);  
}

void loop() {
  if(digitalRead(BUTTON) == LOW) {
    // If the button is pressed, turn on the green LED
    digitalWrite(GRN_LED, HIGH);
    oled.setTextXY(2,0);
    oled.putString("Yes");
  } else {
    digitalWrite(GRN_LED, LOW);
    oled.setTextXY(2,0);
    oled.putString("No ");
  }
}
