/*
  Snapback @ Maker Faire Rome 2014
 Bacterium
 
 This sketch is written for the Maker Faire 2014, to be used together with chemicals sketch.
 
 Parts required:
 1 Red LED 
 1 10 kilohm resistors
 3 220 ohm resistors
 3 photoresistors
 
 Created 01 September 2014
 Modified 02 September 2014
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
// in the next version, this pattern will be sent over serial by the other bacterium
const int penicillinPattern[20] = {0,0,0,1,1,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1};

const int maxLife = 5000;  // whatever you want
int myLife = maxLife;  // we start with an healty bacterium

bool isPenicillinResistance = false;

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
      
    chemRingBuffer[chemRingBufferWrIdx] = chem;
    chemRingBufferWrIdx = (chemRingBufferWrIdx + 1) % nSamples;
    
    // Check if chemicals are dangerous
    // use cross-correlation to identify chemicals
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
    // decrease if penicillin is found
    int acceptanceThr =  9; 
    if ((isPenicillinResistance == false) && (crossValue >= acceptanceThr)) {
      Serial.println("penicillin!");
      lifeDown();
    }
    //always increase after a cycle
    lifeUp();
    // update life led
    int lifeLedValue = map(myLife, 0, maxLife, 0, 255);
    lifeLedValue = constrain(lifeLedValue,2,255);
    analogWrite(lifeLedPin,lifeLedValue);
    Serial.print("life: ");
    Serial.print(myLife);
    Serial.print("(");
    Serial.print(lifeLedValue);
    Serial.println(")");
      
    // Check plasmid
    int plasmidState = digitalRead(plasmidPin);
    if ( (plasmidState == true) && (isPenicillinResistance == false) ) {
      isPenicillinResistance = true;
      myLife = maxLife*0.3;
      // turn on led to visualize that plasmid is present
      digitalWrite(ledPin, HIGH);
      Serial.println("Plasmid received!");
    }
  delay(100);  // delay has to be the same of chemicals sketch
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
