#include <Servo.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo myServo;

int motorPin = 9;
int buttonPin = 13;
int const tempPin = A1;
int const potPin = A0;

int motorOn = 0; //keep track dc motor
 
const int tempLen = 50; //number of stored temperatures
float temps[tempLen];   //buffer of temps to get average from
int tempIndex = 0;      //pointer to read and write to temps 
int tempSetup = 0;      //bool - flag to fill temps with first temp since run

int fanOnTemp = 22;     //initial fan activation temp

void setup() {
  lcd.begin(16, 2);
  myServo.attach(8);
  pinMode(motorPin, OUTPUT);
  pinMode(buttonPin, OUTPUT);
  analogReference(EXTERNAL);
}

void loop() {
  checkSettingsButton(); //check if user is trying to set activation temperature
  changeFanAngle();      //check if user has tried to change the angle of servo
  
  float displayTemp = tempAvg(tempIndex, getTemp()); //get the current temp average
  delay(250);
  decideMotor(displayTemp); //determine if the motor should be on or not
  displayLCD(displayTemp);  //update the lcd with all current information
}

void changeFanAngle() {
  //potentiometer is used to change servo motor angle
  int potVal = analogRead(potPin);
  int angle = map(potVal, 0, 1023, 0, 179);
  myServo.write(angle);
}

int getNextIndex() {
  //increment the pointer for temps safely
  if (tempIndex + 1 == tempLen) {
    return 0;
  } else {
    return tempIndex + 1;
  }
}

float getTemp() {
  //convert reading to voltage using reference (3.3v)
  int reading = analogRead(tempPin);
  float voltage = reading * 3.3;
  voltage /= 1024.0;
  //and convert it into a meaningful celsius value
  float temp = (voltage - 0.5) * 100;
  return temp;
}

void setupTemps(float temp) {
  //fill temps with the initial reading from temp sensor
  if (tempSetup == 0) { //only ever happens once
    for(int i = 0; i < tempLen; i++) {
      temps[i] = temp;
    }
    tempSetup = 1;
  }
}

float tempAvg(int index, float temp) {
  setupTemps(temp);
  
  temps[index] = temp;
  //point to next value in temps
  tempIndex = getNextIndex();
  //get the average  
  float total = 0.0;
  for(int i = 0; i < tempLen; i++){
    total = total + temps[i];
  }
  float tempAvg = total/tempLen;
  return tempAvg;
}

int decideMotor(float temp) {
  //decide if on based by temp
  if (temp > fanOnTemp) {
    digitalWrite(motorPin, HIGH);
    motorOn = 1;
  } else {
    digitalWrite(motorPin, LOW);
    motorOn = 0;
  }
}

void displayLCD(float displayTemp) {
  //top left message: current average temperature
  lcd.setCursor(0,0);
  lcd.print("Temp: " + String(displayTemp) + "C");
  //top right message: fan ON/OFF
  lcd.setCursor(13,0);
  if (motorOn == 1) {
    lcd.print(" ON");
  } else {
    lcd.print("OFF");
  }
  //bottom left: fan activation temperature
  lcd.setCursor(0,1);
  lcd.print("Turns on at " + String(fanOnTemp) + "C");
}

void checkSettingsButton() {
  //cycle through the fan activation temperatures
  //10C to 30C
  if (digitalRead(buttonPin) == HIGH) {
    fanOnTemp++;
    if (fanOnTemp == 31){
      fanOnTemp = 10;
    }
  }
}
