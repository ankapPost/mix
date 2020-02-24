#include "HID-Project.h"

const int buttonPin = 7;
const int ledPin =  13;
const int encA =  2;
const int encB =  3;
const int encVDD = 4;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  pinMode(encVDD, OUTPUT);
  digitalWrite(encVDD, HIGH);
  attachInterrupt(digitalPinToInterrupt(encA), process_encder, RISING );
  attachInterrupt(digitalPinToInterrupt(buttonPin), process_button, FALLING );
  digitalWrite(encB, LOW);

  Serial.begin (9600);
  Serial.println("Basic Encoder Test:");

  Consumer.begin();
}

int d = 5;
int i = 10;
int s = 0;
bool encB_value;

// E9, EA, 128,129, 80, 81

void volume_up() {
    s = s + i;
    Consumer.write(MEDIA_VOLUME_UP);
//    Keyboard.press(0x27);
//    Keyboard.releaseAll();
//    Keyboard.write(174);
//    Keyboard.write(0xAE);
}
void volume_down() {
    s = s - i;
    Consumer.write(MEDIA_VOLUME_DOWN);
//    Keyboard.press(0x1E);
//    Keyboard.releaseAll();
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
boolean play_state = 1;
void process_button() {
//  button_value = digitalRead(buttonPin);
    if (play_state) {
      Consumer.write(MEDIA_PLAY_PAUSE);
      play_state = 0;
    } else {
      play_state = 1;
      Consumer.write(MEDIA_PAUSE);
    }
    delay(300);
}

void loop() {

//  Serial.println(s);
  
  analogWrite(ledPin,s);

  delay(d);
}
