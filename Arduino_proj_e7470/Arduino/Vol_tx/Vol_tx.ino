#include "HID-Project.h"
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <NRFLite.h>

// pin assignment for volume control
const int buttonPin = 7;
const int ledPin =  13;
const int encA =  2;
const int encB =  3;
const int encVDD = 4;

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        6 // On Trinket or Gemma, suggest changing this to 1
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 24 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);


// pin assignment for nrf24
const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;
struct RadioPacket {
    uint8_t FromRadioId;
    uint32_t OnTimeMillis;
    uint8_t consumerSignal;
};
NRFLite _radio;
RadioPacket _radioData;


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

  Serial.begin (115200);
  Serial.println("Basic Encoder Test:");

  Consumer.begin();
  pixels.begin();

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
    {
        Serial.println("Cannot communicate with radio");
        while (1); // Wait here forever.
    }
    
    _radioData.FromRadioId = RADIO_ID;

}

int brightness = 150;
int pi;
void running_pixel(boolean direction){
  pixels.clear();
   for(int i=0; i<NUMPIXELS+1; i++) {
     if (direction) {
       pi = i;
     } else {
       pi = NUMPIXELS - i -1;
     }
     pixels.setPixelColor(pi, pixels.Color(0, brightness, 0));
     pixels.show();
     delay(3);
     pixels.clear();
   }
   pixels.clear();
}

void volume_up_animation(){
  running_pixel(1);
}
void volume_down_animation(){
  running_pixel(0);  
}

int d = 5;
int i = 10;
int s = 0;
bool encB_value;

void volume_up() {
    s = s + i;
    _radioData.consumerSignal = 4;
    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))){
        Serial.println("Sending MEDIA_VOLUME_UP");
    } else {
        Serial.println("Failed Sending MEDIA_VOLUME_UP");
 //       _radioData.FailedTxCount++;
    }
//    Consumer.write(MEDIA_VOLUME_UP);
//    volume_up_animation();
}

void volume_down() {
    s = s - i;
    _radioData.consumerSignal = 5;
    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))){
        Serial.println("Sending MEDIA_VOLUME_DOWN");
    } else {
        Serial.println("Failed Sending MEDIA_VOLUME_DOWN");
 //       _radioData.FailedTxCount++;
    }
//    Consumer.write(MEDIA_VOLUME_DOWN);
//    volume_down_animation();
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
int current_button_value, last_button_value;
int fixed_button_press_start = 0, in_bounce = 0;
unsigned long bounce_tolerance = 20;
unsigned long button_press_time, time_passed;
unsigned long last_button_press_time;
unsigned long bounce_start_time;
void process_button() {
    button_press_time = millis();
    current_button_value = digitalRead(buttonPin);
    Serial.println("\n--Start--   " + String(button_press_time));
    
    if (in_bounce || current_button_value) {
      time_passed = button_press_time - bounce_start_time;
      if (time_passed > bounce_tolerance) {
        in_bounce = 0;
      }
      Serial.print("Found bouncing at: " + String(button_press_time));
      Serial.print(", bounce_start_time: " + String(bounce_start_time));
      Serial.print(", Diff: " + String(time_passed));
      Serial.println(", Button Value: " + String(current_button_value));
      return;
    }
        
    Serial.print  ("Pressed button at: " + String(button_press_time));
    Serial.println(", Button Value: " + String(current_button_value));
    if (play_state) {
      Consumer.write(MEDIA_PLAY_PAUSE);
      Serial.println("Sent MEDIA_PLAY_PAUSE");
      play_state = 0;
    } else {
      play_state = 1;
      Consumer.write(MEDIA_PAUSE);
      Serial.println("Sent MEDIA_PAUSE");
    }
    in_bounce = 1;
    bounce_start_time = button_press_time;
}

void loop() {
//  Serial.println(s);
//    Serial.println(digitalRead(buttonPin));
  
  analogWrite(ledPin,s);

  delay(d);
}
