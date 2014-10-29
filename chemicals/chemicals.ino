/*
  Snapback @ Maker Faire Rome 2014
 Chemicals
 
 This sketch is written for the Maker Faire 2014, to be used together with bacterium sketch.
 
 Parts required:
 x Green LED 
 x Yellow LED 
 x 220 ohm resistors
 
 Created 01 September 2014
 Modified 02 September 2014
 by Claudio Capobianco
 
 http://snapback.io
 
 This example code is part of the public domain 
 */

const int ledPin = 13;
const int toggleButton = 2;
const int penicillinLedPin = 8;  // pin with food A 

// penicillin pattern: half has to be 0 and half has to be 1
const int patternLen = 24;
const int penicillinPattern[patternLen] = {0,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1};

int penicillinRingBufferRdIdx = 0;

bool isTherePenicillin = true;

void setup() {
    // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
    // set the food pins as outputs
  pinMode(penicillinLedPin,OUTPUT);
  pinMode(ledPin,OUTPUT);

  // make the pushbutton's pin an input:
  pinMode(toggleButton, INPUT);

}

void loop() {
 
  // Check penicillin is enabled or not
  // read the input pin
  int buttonState = digitalRead(toggleButton);
  // print out the state of the button:
  Serial.print("buttonState: ");
  Serial.println(buttonState);
  isTherePenicillin = buttonState;
 
 // Manage penicillin LED
  if (isTherePenicillin == true) {
    digitalWrite(ledPin,HIGH);
    int idx = 0;
    int penicillinValue;
    penicillinValue = penicillinPattern[penicillinRingBufferRdIdx];
    if (penicillinValue == true) {
      digitalWrite(penicillinLedPin,HIGH);
    } else {
      digitalWrite(penicillinLedPin,LOW);
    }
    penicillinRingBufferRdIdx = (penicillinRingBufferRdIdx+1)%patternLen;
  } else {
    digitalWrite(ledPin,LOW);
    digitalWrite(penicillinLedPin,LOW);
  }
   
  delay(100);        // delay has to be the same of bacterium sketch
}
