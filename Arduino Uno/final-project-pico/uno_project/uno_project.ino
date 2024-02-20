//include needed libraries
#include <EventAnalog.h>
#include <EventJoystick.h>
#include <TouchScreen.h>

#include <SPI.h>

#include <Adafruit_ILI9341.h>

#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <Fonts/Picopixel.h>
#include <Fonts/Dosis_VariableFont_wght20pt7b.h>


#include <Adafruit_FT6206.h>

// melody for alarm
#include "pitches.h"
// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

//interrupt stuff
const int LED = 13;
bool    led_status =                        LOW;
#define switched                            true // value if the button switch has been pressed
#define triggered                           true // controls interrupt handler
#define interrupt_trigger_type            RISING // interrupt triggered on a RISING input
#define debounce                              10 // time to wait in milli secs

volatile  bool interrupt_process_status = {
  !triggered                                     // start with no switch press pending, ie false (!triggered)
};

bool initialisation_complete =            false; // inhibit any interrupts until initialisation is complete

// Define TFT pins
#define DC_pin 9
#define CS_pin 10

// Color definitions
#define JUNGLE_GREEN     0x1944
#define SILVER   0xC637
#define STEEL    0x6493
#define BLUE     0x0351
#define POMODORO 0xEA4A

EventJoystick ej1(A0, A1);

// global vars
Adafruit_ILI9341 tft = Adafruit_ILI9341(CS_pin, DC_pin);


const int buzzerPin = 3;
const int selButton = 2;
int selPos = 1;
const int count = 4;
int x;
int y;
bool b;
int progress = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(buzzerPin, OUTPUT);
  pinMode(selButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(selButton),
                  button_interrupt_handler,
                  interrupt_trigger_type);
  initialisation_complete = true; // open interrupt processing for business
  setupGui();
  Serial.begin(9600); // Start serial
  //x = map(0, -10, 10, 100, 200); // returns 150 in x

  //ej1.setCentreBoundary(870);
  ej1.setOuterBoundary(630);
  //Link the event(s) you require to your function
  ej1.setChangedHandler(moveCursor);
  ej1.setNumIncrements(10);
}

void loop() {
  // put your main code here, to run repeatedly:
  //countdown();
  //some joystick code here?
  // always be readin joystick input
  //x = analogRead(A0);
  //y = analogRead(A1);
  b = digitalRead(2);

  ej1.update();

  if (buttonPress() == switched) {
    // button on/off cycle now complete, so flip LED between HIGH and LOW
    countdown();
  }
}

// my functions

void countdown() {
  //does the countdown, keeps track of time
  //also does progress bar

  static unsigned long timer = 0;
  unsigned long interval = 100;

  while (millis() - timer >= interval)
  {
    if (selPos == 2) {
      //tft.drawRoundRect(20, 205, 280, 25, 4, SILVER);
      timer = millis();
      tft.fillRoundRect(20, 205, progress, 25, 4, SILVER); //fill progress bar
      tft.fillRoundRect(20, 40, 120, 90, 8, STEEL); // cover old data
      tft.setCursor(35, 80);
      tft.setTextColor(SILVER);
      tft.setFont(&Picopixel);
      tft.setTextSize(5);
      tft.println(progress);
      progress += 10;
      if (progress > 280) //this is the length of the rectangle
      {
        //time's up! use electropiezo buzzer
        timesUp();
        //at this point, call submenu??
        //look for interrupt?
        while (1);  // finished just wait.  Take this out for your use.
        // or reset the numbers for another go
      }
    }
    else if (selPos == 4) {
      //tft.drawRoundRect(20, 205, 280, 25, 4, SILVER);
      timer = millis();
      tft.fillRoundRect(20, 205, progress, 25, 4, SILVER); //fill progress bar
      tft.fillRoundRect(180, 40, 120, 90, 8, STEEL); // cover old data
      tft.setCursor(200, 80);
      tft.setTextColor(SILVER);
      tft.setFont(&Picopixel);
      tft.setTextSize(5);
      tft.println(progress);
      progress += 10;
      if (progress > 280) //this is the length of the rectangle
      {
        //time's up! use electropiezo buzzer
        timesUp();
        while (1);  // finished just wait.  Take this out for your use.
      }
    }

  }

}

void timesUp() {
  //sends a signal to the piezo buzzer
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(buzzerPin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(buzzerPin);
  }
}

void moveCursor(EventJoystick& ej) {
  //joystick moves cursor
  //this is an interrupt

  Serial.println(ej.x.position());


  //if(ej.x.position() == 10 != ej.x.hasChanged()){
  if (ej.x.position() == 10 != ej.x.previousPosition()) {
    selPos++;
  }
  else {}

  if (ej.x.position() == 0 != ej.x.previousPosition()) {
    selPos--;
  }
  else {}

  if (selPos > count) {
    selPos = 1; //wraps around from end
  }
  if (selPos < 1) {
    selPos = count; //wraps around from beginning
  }

  //depending on what the value of selPos is, highlight the box on the screen
  switch (selPos) {
    case 1: {
        tft.drawRoundRect(20, 40, 120, 90, 8, POMODORO); //'work' box
        //redraw other boxes
        tft.fillRoundRect(180, 40, 120, 90, 8, STEEL); //right box
        tft.drawRoundRect(20, 140, 120, 50, 8, STEEL);//left button
        tft.drawRoundRect(180, 140, 120, 50, 8, STEEL);//right button
        break;
      }
    case 2: {
        tft.drawRoundRect(20, 140, 120, 50, 8, POMODORO); //work start/restart button
        //draw other boxes
        tft.fillRoundRect(20, 40, 120, 90, 8, STEEL); //left box
        tft.fillRoundRect(180, 40, 120, 90, 8, STEEL); //right box
        tft.drawRoundRect(180, 140, 120, 50, 8, STEEL); //right button
        break;
      }
    case 3: {
        tft.drawRoundRect(180, 40, 120, 90, 8, POMODORO); //'break' box
        tft.fillRoundRect(20, 40, 120, 90, 8, STEEL);
        tft.drawRoundRect(20, 140, 120, 50, 8, STEEL);
        tft.drawRoundRect(180, 140, 120, 50, 8, STEEL);
        break;

      }
    case 4: {
        tft.drawRoundRect(180, 140, 120, 50, 8, POMODORO); //break start/restart button
        tft.fillRoundRect(20, 40, 120, 90, 8, STEEL); //left box
        tft.fillRoundRect(180, 40, 120, 90, 8, STEEL); //right box
        tft.drawRoundRect(20, 140, 120, 50, 8, STEEL);
        break;
      }
  }
}

bool buttonPress() {
  //button press selects time boxes or starts timer
  //this is also an interrupt
  int button_reading;
  // static variables because we need to retain old values between function calls
  static bool     switching_pending = false;
  static long int elapse_timer;
  if (interrupt_process_status == triggered) {
    // interrupt has been raised on this button so now need to complete
    // the button read process, ie wait until it has been released
    // and debounce time elapsed
    button_reading = digitalRead(selButton);
    if (button_reading == HIGH) {
      // switch is pressed, so start/restart wait for button relealse, plus end of debounce process
      switching_pending = true;
      elapse_timer = millis(); // start elapse timing for debounce checking
    }
    if (switching_pending && button_reading == LOW) {
      // switch was pressed, now released, so check if debounce time elapsed
      if (millis() - elapse_timer >= debounce) {
        // dounce time elapsed, so switch press cycle complete
        switching_pending = false;             // reset for next button press interrupt cycle
        interrupt_process_status = !triggered; // reopen ISR for business now button on/off/debounce cycle complete
        return switched;                       // advise that switch has been pressed
      }
    }
  }
  return !switched; // either no press request or debounce period not elapsed
}

void button_interrupt_handler()
{
  if (initialisation_complete == true)
  { //  all variables are initialised so we are okay to continue to process this interrupt
    if (interrupt_process_status == !triggered) {
      // new interrupt so okay start a new button read process -
      // now need to wait for button release plus debounce period to elapse
      // this will be done in the button_read function
      if (digitalRead(selButton) == HIGH) {
        // button pressed, so we can start the read on/off + debounce cycle wich will
        // be completed by the button_read() function.
        interrupt_process_status = triggered;  // keep this ISR 'quiet' until button read fully completed
      }
    }
  }
} // end of button_interrupt_handler

// ADAGFX library
void setupGui() {
  //lots of tft setup
  tft.begin();

  tft.setRotation(1);
  tft.fillScreen(JUNGLE_GREEN);

  tft.setFont(&Dosis_VariableFont_wght20pt7b);
  tft.setTextColor(SILVER);
  tft.setCursor(20, 30);
  tft.print("Work");
  tft.setCursor(180, 30);
  tft.print("Break");

  //draw two rectangles for countdown time
  //drawRoundRect(x0, y0, w, h, radius, color);
  tft.fillRoundRect(20, 40, 120, 90, 8, STEEL); //left box
  tft.fillRoundRect(180, 40, 120, 90, 8, STEEL); //right box

  //draw play/start buttons for countdown
  tft.setFont(&Picopixel);
  tft.setTextSize(2);
  tft.fillRoundRect(20, 140, 120, 50, 8, BLUE); //left button
  tft.drawRoundRect(20, 140, 120, 50, 8, STEEL);
  tft.setCursor(45, 170);
  tft.print("Start/Reset");


  tft.fillRoundRect(180, 140, 120, 50, 8, BLUE); //right button
  tft.drawRoundRect(180, 140, 120, 50, 8, STEEL);
  tft.setCursor(205, 170);
  tft.print("Start/Reset");

  //draw progress bar
  tft.fillRoundRect(20, 205, 280, 25, 4, STEEL);
  tft.drawRoundRect(20, 205, 280, 25, 4, SILVER);
}
