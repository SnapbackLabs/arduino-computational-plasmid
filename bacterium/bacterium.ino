/*
  Snapback @ Maker Faire Rome 2014
 Bacterium
 
 This sketch is written for the Maker Faire 2014, to be used together with chemical sketch.
 
 Parts required:
 1 Red LED 
 1 10 kilohm resistors
 3 220 ohm resistors
 3 photoresistors
 
 Created 01 September 2014
 Modified 01 September 2014
 by Claudio Capobianco
 
 http://snapback.io
 
 This example code is part of the public domain 
 */
 
// PINs constants 
const int ledPin = 13;

const int lifeLedPin = 10;    // LED connected to digital pin 10

const int chemSensorPin = A0;  // pin with the photoresistor for chemicals 

const int plasmidPin = 2; // pin that will receive the plasmid

// Global vars
int chemValue = 0; // value to write to the red LED

int chemSensorValue = 0; // variable to hold the value from the red sensor 

// variable to calibrate low value
int chemLow = 1023;
// variable to calibrate low value
int chemHigh = 0;

const int nSamples = 20;
int chemRingBuffer[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int chemRingBufferWrIdx = 0;

// penicillin pattern: twenty values, ten has to be 0 and ten has to be 1.
const int penicillinPattern[20] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1};

const int maxLife = 5000;  // whatever you want
const int minLife = (maxLife/80); // we don't want that bacterium dies!
int myLife = maxLife;  // we start with an healty bacterium

bool isPenicillinResistence = false;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600); 

  // set the digital pins as outputs
  pinMode(lifeLedPin,OUTPUT);
  pinMode(ledPin,OUTPUT);
  
  calibPhotoresistors();
  
  // make the plasmid pin an input:
  pinMode(plasmidPin, INPUT);
  
}

void loop() {
    chemSensorValue = analogRead(chemSensorPin);
    
    // print out the values to the serial monitor  
//    Serial.print("raw sensor Values \t chem: ");
//    Serial.println(chemSensorValue);
  
    int chem = map(chemSensorValue, chemLow, chemHigh, 0, 100);
    chem = constrain(chem,0,100);
//    Serial.print("calib sensor Values \t chem: ");
//    Serial.println(chem);
      
    chemRingBuffer[chemRingBufferWrIdx] = chem;
    chemRingBufferWrIdx = (chemRingBufferWrIdx + 1) % nSamples;
    
    // Detect food
    // use cross-correlation
    int idx;
    int bufRdIdx = chemRingBufferWrIdx;
    int crossValue = 0;
    for (idx = 0; idx < nSamples; idx++) {
      bufRdIdx = (bufRdIdx+1)%nSamples;
      crossValue += (penicillinPattern[idx]*chemRingBuffer[bufRdIdx]);///(1000*1000);
    }
//    Serial.print("cross penicillin Values: ");
//    Serial.println(crossValue);
      
    // Update life value
    // increase if any food is found
    int acceptanceThr =  700; 
    if ((isPenicillinResistence == false) && (crossValue > acceptanceThr)) {
      Serial.println("penicillin!");
      lifeDown();
    }
    //always increase after a cycle
    lifeUp();
    // update life led
    int lifeLedValue = map(myLife, 0, maxLife, 0, 255);
    lifeLedValue = constrain(lifeLedValue,0,255);
    analogWrite(lifeLedPin,lifeLedValue);
//    Serial.print("life: ");
//    Serial.print(myLife);
//    Serial.print("(");
//    Serial.print(lifeLedValue);
//    Serial.println(")");
      
    // Check plasmid
    int plasmidState = digitalRead(plasmidPin);
    Serial.print("plasmidState: ");
    Serial.println(plasmidState);
    if ( (plasmidState == true) && (isPenicillinResistence == false) ) {
      Serial.println("Plasmid received!");
    }
    if (plasmidState == true) {
      isPenicillinResistence = true;
    }
  delay(100);
}

void lifeUp() {
  int stepUp = 1;
  int newLife = myLife+stepUp;
  myLife = min(newLife,maxLife);
}

void lifeDown() {
    int stepDown = maxLife / 5; // will almost die in 3 rounds
    int newLife = myLife-stepDown;
    myLife = max(minLife,newLife);
}

void calibPhotoresistors() {
  // Make the LED pin an output and turn it on
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  // calibrate for the first five seconds after program runs
  while (millis() < 5000) {
    delay(5);
    chemSensorValue = analogRead(chemSensorPin);
    
    // record the maximum sensor value
    if (chemSensorValue > chemHigh) {
      chemHigh = chemSensorValue;
    }
    // record the minimum sensor value
    if (chemSensorValue < chemLow) {
      chemLow = chemSensorValue;
    }
  }
  Serial.print("calib: chem [");
    Serial.print(chemLow);
    Serial.print(",");
    Serial.print(chemHigh);
    Serial.println("]");
    // turn the LED off, signaling the end of the calibration period
  digitalWrite(ledPin, LOW);
}
