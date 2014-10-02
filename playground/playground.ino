/*
  Snapback @ Maker Faire Rome 2014
 Playground
 
 This sketch is written for the Maker Faire 2014, to be used together with bacterium sketch.
 
 Parts required:
 x Green LED 
 x Yellow LED 
 x 220 ohm resistors
 
 Created 01 September 2014
 Modified 01 September 2014
 by Claudio Capobianco
 
 http://snapback.io
 
 This example code is part of the public domain 
 */

const int pushButton = 2;
const int foodALedPin = 8;  // pin with food A 
const int foodBLedPin = 9;   // pin with food B 

// food patterns: twenty values, ten has to be 0 and ten has to be 1.
const int foodAPattern[20] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};
const int foodBPattern[20] = {0,0,0,1,1,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1};

int foodRingBufferRdIdx = 0;

bool foodAIsEnabled = true;
bool foodBIsEnabled = true;

int lastButtonState = false;

void setup() {
    // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
    // set the food pins as outputs
  pinMode(foodALedPin,OUTPUT);
  pinMode(foodBLedPin,OUTPUT);
  
  // make the pushbutton's pin an input:
  pinMode(pushButton, INPUT);

}

void loop() {
    // read the input pin:
  int buttonState = digitalRead(pushButton);
  if (buttonState != lastButtonState) {
    // toggle food B
    foodBIsEnabled = !(foodBIsEnabled);
  }
  lastButtonState = buttonState;
  // print out the state of the button:
  Serial.println(buttonState);
  
  int idx = 0;
  int foodAValue;
  int foodBValue;
  foodAValue = foodAPattern[foodRingBufferRdIdx];
  foodBValue = foodBPattern[foodRingBufferRdIdx];
  if ( (foodAIsEnabled == true) && (foodAValue == true) ) {
    digitalWrite(foodALedPin,HIGH);
  } else {
    digitalWrite(foodALedPin,LOW);
  }
  if ( (foodBIsEnabled == true) && (foodBValue == true)) {
    digitalWrite(foodBLedPin,HIGH);
  } else {
    digitalWrite(foodBLedPin,LOW);
  }

  foodRingBufferRdIdx = (foodRingBufferRdIdx+1)%20;
  
  delay(100);        // delay in between reads for stability
}
