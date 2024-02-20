#define FAST 100
#define SLOW 2000
const int LED = 13;
const int SWITCHA = 2;
const int SWITCHB = 3;
// our flash rate
// "volatile" is needed as a hint to the compiler to not
//  pull any tricks to make the code faster
volatile int rate;
// When button A is pressed, flash quickly
void myISRA () {
  rate = FAST;
}
// When button B is pressed, flash slowly
void myISRB () {
  rate = SLOW;
}
void setup() {
  // start slow
  rate = SLOW;
  pinMode(LED, OUTPUT);
  pinMode(SWITCHA, INPUT_PULLUP);
  pinMode(SWITCHB, INPUT_PULLUP);
  // Attach the interrupts to the buttons
  attachInterrupt(digitalPinToInterrupt(SWITCHA), myISRA, FALLING);
  attachInterrupt(digitalPinToInterrupt(SWITCHB), myISRB, FALLING);
}
void loop() {
  // flash the LED at the current rate
  digitalWrite(LED, HIGH);
  delay(rate);
  digitalWrite(LED, LOW);
  delay(rate);
}
