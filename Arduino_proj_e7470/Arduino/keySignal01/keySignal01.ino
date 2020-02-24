#include "Keyboard.h"

const int buttonPin = 4;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
const int encA =  2;      // the number of the LED pin
const int encB =  3;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encA), process_encder, RISING );
  digitalWrite(encB, LOW);

  Serial.begin (9600);
  Serial.println("Basic Encoder Test:");

  Keyboard.begin();
}

int d = 10;
int i = 10;
int s = 0;
bool encB_value;

const int AS_RAW_SCANCODE = 136;
void volume_up() {
    s = s + i;
    Keyboard.press(174);
    Keyboard.releaseAll();
//    Keyboard.write(174);
//    Keyboard.write(0xAE);
}
void volume_down() {
    s = s - i;
    Keyboard.press(175);
    Keyboard.releaseAll();
//    Keyboard.write(175);
//    Keyboard.write(0xAF);  
}

void process_encder() {
  encB_value = digitalRead(encB);
  if (encB_value == HIGH) {
    volume_up();
  } else if (encB_value == LOW) {
    volume_down();
  } else {Serial.print("Error: unknown encB value: ");Serial.println(encB);}
  Serial.println(s);
  delay(d);

}
void loop() {

//  Serial.println(s);
  
  analogWrite(ledPin,s);

  delay(d);
}
