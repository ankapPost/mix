
const int buttonPin = 4;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
const int encA =  2;      // the number of the LED pin
const int encB =  3;     
int buttonState = 0;         // variable for reading the pushbutton status

void key() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    digitalWrite(ledPin, LOW);
  } else {
  }
  analogWrite(ledPin, HIGH);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encA), process_encder, RISING );
  digitalWrite(encB, LOW);

  Serial.begin (9600);
  Serial.println("Basic Encoder Test:");
}

int d = 15;
int i = 10;
int s = 0;
bool encB_value;

void process_encder() {
  encB_value = digitalRead(encB);
  if (encB_value == HIGH && s < 256) {
    s = s + i;
  } else if (encB_value == LOW && s > 0) {
    s = s - i;
  } else {Serial.print("Error: unknown encB value: ");Serial.println(encB);}
  Serial.println(s);
}
void loop() {

//  Serial.println(s);
  
  analogWrite(ledPin,s);

  delay(d);
}
void loop2() {
  if (s > 256) {  i = -1;  }
  if ( s < 0 ) {  i = 1;  }
  
  s = s + i;
  
  Serial.println(s);
  
  analogWrite(ledPin,s);

  delay(d);
}
