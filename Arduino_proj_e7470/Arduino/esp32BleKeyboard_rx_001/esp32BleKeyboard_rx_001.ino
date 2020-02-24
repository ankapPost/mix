/**
 * This example turns the ESP32 into a Bluetooth LE keyboard that writes the words, presses Enter, presses a media key and then Ctrl+Alt+Delete
 */
#include <BleKeyboard.h>

BleKeyboard bleKeyboard;

// pin assignment for volume control
const int buttonPin = 34;
const int encA =  36;
const int encB =  39;
int d = 15;
int i = 10;
int s = 0;
bool encB_value;

void volume_up() {
    s = s + i;
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    Serial.println("in volume_up function");

//    volume_up_animation();
}

void volume_down() {
    s = s - i;
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    Serial.println("in volume_down function");
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
void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encA), process_encder, RISING );
//  attachInterrupt(digitalPinToInterrupt(buttonPin), process_button, FALLING );
  digitalWrite(encB, LOW);
  
  Serial.begin(115200);
  Serial.println("Starting esp32 BLE keyboard!");
  bleKeyboard.begin();
}

void loop() {
  delay(d);
}
