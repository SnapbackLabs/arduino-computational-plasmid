/*
  Snapback @ Maker Faire Rome 2014
 Bacterium
 
 This sketch is written for the Maker Faire 2014, to be used together with chemicals sketch.
 
 Parts required:
 1 Red LED 
 1 10 kilohm resistors
 3 220 ohm resistors
 3 photoresistors
 
 Created 01 October 2014
 Modified 22 October 2014
 by Claudio Capobianco
 
 http://snapback.io
 
 This example code is part of the public domain 
 */


#include <SoftwareSerial.h>
 
// PINs constants 
const int ledPin = 13;
const int calibLedPin = ledPin;
const int penicillinResistenceLedPin = ledPin;
const int lifeLedPin = 10;    // LED connected to digital pin 10
const int chemSensorPin = A0;  // pin with the photoresistor for chemicals 
const int pilumPin = 4; // pin that will recognize the presence of the pilum (not used)
const int rxPin = 2;  // software serial
const int txPin = 3;  // software serial

// Chemicals
int chemValue = 0; // value to write to the red LED
int chemSensorValue = 0; // variable to hold the value from the red sensor 
int chemLow = 1023; // variable to calibrate low value
int chemHigh = 0; // variable to calibrate low value
const int chemBufferLen = 24;
int chemRingBuffer[chemBufferLen] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int chemRingBufferWrIdx = 0;

// Plasmid
// pattern: twentyfour values, half has to be 0 and half has to be 1.
const int startCode = 0x0A;
const int plasmidLen = 24;
int plasmid[plasmidLen] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
// pattern of the plasmid that mediate the resistance to penicillin
const int penicillinResistencePlasmid[plasmidLen] = {0,0,0,1,1,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1};


// Life
const int maxLife = 5000;  // whatever you want
int myLife = maxLife;  // we start with an healty bacterium
int lifeCounter = 0;

// Penicillin
bool isPenicillinResistance = false;

// Inter-bacteria communication
SoftwareSerial plasmidSerial =  SoftwareSerial(rxPin, txPin);
int sendPauseCounter = 0;

// Select bacteria
enum bacteriumName {
  Colin,  // resistent to penicillin
  Kalin   // not resistent to penicillin
};
bacteriumName myName = Kalin;



void setup() {
  // initialize serial communications at 9600 bps
  Serial.begin(9600); 
  plasmidSerial.begin(9600);

  // set the digital pins as outputs
  pinMode(lifeLedPin,OUTPUT);
  pinMode(ledPin,OUTPUT);

  // calibrate sensors  
  calibPhotoresistors();
  
  // make the plasmid pin an input
  pinMode(pilumPin, INPUT);
  
  
  // initialize bacterium
  if (myName == Colin) {
    isPenicillinResistance = true;
    for (int i = 0; i < plasmidLen; i++) {
      plasmid[i] = penicillinResistencePlasmid[i];
    }
  }
  
}

void loop() {
  // Update chemical ring buffer
  updateChemical();
    
  // Check if chemicals are dangerous
  int dangerFactor = computeDangerFactor();
        
  updateLifeValue(dangerFactor);
  
  updatePlasmid(dangerFactor);

  updateLifeLed();
  updatePlasmidResistenceLed();
  
  delay(100);  // delay has to be the same of chemicals sketch
}

void updateChemical() {
  chemSensorValue = analogRead(chemSensorPin);

  //Serial.print("raw sensor Values \t chem: ");
  //Serial.println(chemSensorValue);

  // Create the pattern of the chemicals around
  // map the value to the range [0,1000]
  int chem = map(chemSensorValue, chemLow, chemHigh, 0, 1000);
  // map again manually in [0,1] (to avoid numerics oddities)
  if (chem > 500) {
    chem = 1;
  } else {
    chem = 0;
  }

//    Serial.print("calib sensor Values \t chem: ");
//    Serial.println(chem);

  // fill the buffer
  chemRingBuffer[chemRingBufferWrIdx] = chem;
  chemRingBufferWrIdx = (chemRingBufferWrIdx + 1) % chemBufferLen;
  
}

int computeDangerFactor() {
      // use cross-correlation to identify chemicals
    int idx;
    int bufRdIdx = chemRingBufferWrIdx;
    int crossValue = 0;
    for (idx = 0; idx < chemBufferLen; idx++) {
      bufRdIdx = (bufRdIdx+1)%chemBufferLen;
      crossValue += (plasmid[idx]*chemRingBuffer[bufRdIdx]);///(1000*1000);
    }
//    Serial.print("cross penicillin Values: ");
//    Serial.println(crossValue);
  return crossValue;
}

void updateLifeValue(int dangerFactor) {
  // decrease if penicillin is found
  int acceptanceThr =  9; 
  if ((isPenicillinResistance == false) && (dangerFactor >= acceptanceThr)) {
    Serial.println("penicillin!");
    lifeDown();
  }
  //always increase after a cycle
  lifeUp();
}

void lifeUp() {
  int stepUp = (myLife*0.01); // not linear, since brightness perception is not linear
  stepUp = max(1,stepUp);
  int newLife = myLife+stepUp;
  myLife = min(newLife,maxLife);
}

void lifeDown() {
    int stepDown = maxLife / 3; // will (almost) die in few rounds
    int newLife = myLife-stepDown;
    myLife = max(0,newLife);
}

void updateLifeLed() {
  int lifeLedValue = map(myLife, 0, maxLife, 0, 255);
  lifeLedValue = constrain(lifeLedValue,2,255); // min = 2: we don't want that bacterium dies
  analogWrite(lifeLedPin,lifeLedValue);
  Serial.print("life: ");
  Serial.print(myLife);
  Serial.print("(");
  Serial.print(lifeLedValue);
  Serial.println(")");
}


void updatePlasmid(int dangerFactor) {
  int triggerThr = -1;
  if (dangerFactor > triggerThr) {
    if (isPenicillinResistance == true) {
      sendPauseCounter++;
      if (sendPauseCounter > 30) { // send every 3''
        sendPauseCounter = 0;
        sendPlasmid();
      }
    } else {
      receivePlasmid();
    }
  }
}

void sendPlasmid() {
  Serial.print("Sending plasmid...");
  // write start byte
  plasmidSerial.write(startCode);
  // write plasmid buffer
  for (int i = 0; i<plasmidLen; i+=1){
    plasmidSerial.write(plasmid[i]);
    Serial.print(plasmid[i]);
  }
  Serial.println(" sent");
}

void receivePlasmid() {
  if (plasmidSerial.available() > 0) {
    if (plasmidSerial.read() == startCode) {
      Serial.print("Receiving plasmid...");
      // wait to fill the buffer
      while (plasmidSerial.available() < plasmidLen) {
      }
      // receive buffer
      for (int i = 0; i<plasmidLen; i+=1) {
        plasmid[i] = plasmidSerial.read();
        Serial.print(plasmid[i]);
      }
      isPenicillinResistance = true;
      Serial.println(" received");
    }
  }
}

void updatePlasmidResistenceLed() {
  if (isPenicillinResistance == true) {
    digitalWrite(penicillinResistenceLedPin, HIGH);
  } else {
    digitalWrite(penicillinResistenceLedPin, LOW);
  }
}

void calibPhotoresistors() {
  // Make the LED pin an output and turn it on
  pinMode(calibLedPin, OUTPUT);
  digitalWrite(calibLedPin, HIGH);

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
//  Serial.print("calib: chem [");
//    Serial.print(chemLow);
//    Serial.print(",");
//    Serial.print(chemHigh);
//    Serial.println("]");
  // turn the LED off, signaling the end of the calibration period
  digitalWrite(calibLedPin, LOW);
}




// FIXME TBD REMOVE: just activate plasmid when pin is high
//void updatePlasmid() { 
//  // Check plasmid
//  int plasmidState = digitalRead(pilumPin);
//  if ( (plasmidState == true) && (isPenicillinResistance == false) ) {
//    isPenicillinResistance = true;
//    myLife = maxLife;
//    // turn on led to visualize that plasmid is present
//    digitalWrite(ledPin, HIGH);
////      Serial.println("Plasmid received!");
//  }
//}

