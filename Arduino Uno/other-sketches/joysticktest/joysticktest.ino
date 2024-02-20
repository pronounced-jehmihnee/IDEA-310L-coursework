#include <TouchScreen.h>

#include <SPI.h>

#include <Adafruit_ILI9341.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

// Define TFT pins
#define DC_pin 9
#define CS_pin 10

// Color definitions
#define BLACK    0x0000
#define WHITE    0xFFFF
#define JUNGLE_GREEN     0x1944
#define SILVER   0xC637
#define STEEL    0x6493
#define BLUE     0x0351
#define POMODORO 0xEA4A

Adafruit_ILI9341 tft = Adafruit_ILI9341(CS_pin, DC_pin);

const int buttonPin = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP); //joystick button
  tft.begin();
  tft.fillScreen(JUNGLE_GREEN);
}

void loop() {
  // put your main code here, to run repeatedly:
  bool button = digitalRead(buttonPin);
  int x = analogRead(A0);
  int y = analogRead(A1);
  f_drawJoystickInfo(button, x, y);
}

void f_drawJoystickInfo(bool button, int x, int y) {
  tft.setTextColor(WHITE);
  tft.setCursor(0, 0);
  tft.println("Joystick");
  tft.println("    test");
  tft.println();
  tft.print("X: ");
  tft.println(x);
  tft.print("Y: ");
  tft.println(y);

    if(!button) {
      tft.setCursor(0, 54);
      tft.setTextColor(BLACK, WHITE);
      tft.print("Button");
    }
  int x1 = map(x, 0, 1023, tft.width() - 60 - 2, tft.width() - 2);
  int y1 = map(y, 0, 1023, tft.height() - 60 - 2, tft.height() - 2);
  tft.drawRect(tft.width() - 60 - 2, tft.height() - 60 - 2, 60, 60, WHITE);  //draw frame
  tft.drawLine(x1, y1 - 2, x1, y1 + 2, WHITE);  //draw joystick position inside frame
  tft.drawLine(x1 - 2, y1, x1 + 2, y1, WHITE);

}
