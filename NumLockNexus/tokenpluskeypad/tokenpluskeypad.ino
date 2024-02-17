/*
By:- Unmesh Subedi
github:- https://github.com/Celestial071
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>


const int en = 2, rw = 1, rs = 0, d4 = 4, d5 = 5, d6 = 6, d7 = 7, bl = 3;
LiquidCrystal_I2C lcd1(0x27, en, rw, rs, d4, d5, d6, d7, bl, POSITIVE);
Servo mymotor;

const byte ROWS = 4;
const byte COLS = 4;
bool timerRunning = false;
unsigned long startTime = 0;
int IR_test = 6;
String phoneNumber = "";
String enteredToken = "";
bool carDetected = false;
//make a array that stores hh,mm,ss data
int timeData[3] = {0};
float totalCharge = 0;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
int availableAttempt = 3;
char arr[16] = "1234567890ABC*";

byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {8, 9, 10, 11};

String token = "";
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  mymotor.attach(12);
  lcd1.begin(16, 2);
  randomSeed(analogRead(0));
  moveServo(90);
  pinMode(IR_test, INPUT);
  lcd1.setCursor(0, 0);
  lcd1.print("Waiting for car");
  delay(3000);
}


void moveServo(int theta){
  for(int i=0; i<=theta;i++){
    mymotor.write(i);
    delay(15);
  }
}

void moveServoZero(){
  for(int i=90; i>=0;i--){
    mymotor.write(i);
    delay(15);
  }
}

void Start_timer() {
  startTime = millis();
  timerRunning = true;
}

void Stop_timer() {
  timerRunning = false;
}

void time_val() {
  if (timerRunning) {
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - startTime;
    displayTime(elapsed);
  }
}

void displayTime(unsigned long total_time) {
  total_time = total_time / 1000; // Convert to seconds
  int seconds = total_time % 60;
  int minutes = (total_time / 60) % 60;
  int hours = (total_time / 3600) % 24;

  timeData[0] = hours;
  timeData[1] = minutes;
  timeData[2] = seconds;
  lcd1.setCursor(0, 0);
  lcd1.print("Time: ");
  if (hours < 10) lcd1.print("0");
  lcd1.print(hours);
  lcd1.print(":");
  if (minutes < 10) lcd1.print("0");
  lcd1.print(minutes);
  lcd1.print(":");
  if (seconds < 10) lcd1.print("0");
  lcd1.print(seconds);
}

void generate_token() {
  token = "";
  for (int i = 0; i < 5; i++) {
    token += arr[random(0, 14)];
  }
}

String getUserInput() {
  String input = "";
  char key;
  do {
    key = keypad.getKey();
    if (key != NO_KEY && key != '#') {
      input += key;
      lcd1.setCursor(input.length() - 1, 1); // Adjust cursor position
      lcd1.print(key);
    }
  } while (key != '#');
  return input;
}

void sendPhoneNumberAndToken(String phoneNumber, String token){
  Serial.print(phoneNumber);
  Serial.print(",");
  Serial.println(token);
}

void sendWarning(){
  Serial.println(-1);
}

void askToken(){
  lcd1.setCursor(0,0);
  lcd1.print("Token:");
  lcd1.setCursor(0,1);
  enteredToken = getUserInput();
}

bool checkPhoneNum(String number){
  if(number.length() != 10) return false;
  for(int i=0; i < number.length(); i++){
    if(!isDigit(number[i])){
      return false;
    }
  }
  return true;
}

void displayCharge(){
  lcd1.clear();
  lcd1.setCursor(0,0);
  lcd1.print("Total Charge:");
  lcd1.setCursor(0,1);
  lcd1.print("Nrs ");
  lcd1.print(totalCharge);
}

void resetSystem(){
  carDetected = false;
  timerRunning = false;
  availableAttempt = 3;
  moveServo(90);
}

void calculateCharge(){
  totalCharge = (timeData[0] * 3600 + timeData[1] *60 + timeData[2]) * 3.7;
}

void loop() {
  // Check for car presence
  if (digitalRead(IR_test) == LOW) {
    if (!carDetected) {
      // First detection of the car
      carDetected = true;
      moveServoZero();
      lcd1.clear();
      lcd1.print("Enter Phone No:");
      phoneNumber = "";
      enteredToken = "";
    } else if (!timerRunning) {
      // If phone number is empty or invalid, ask for input again
      if (phoneNumber == "" || !checkPhoneNum(phoneNumber)) {
        phoneNumber = getUserInput();
        if (checkPhoneNum(phoneNumber)) {
          Start_timer();
          generate_token();
          sendPhoneNumberAndToken(phoneNumber, token);
          lcd1.clear();
          lcd1.print("Token Sent");
          lcd1.setCursor(0, 1);
        } else {
          lcd1.clear();
          lcd1.print("Invalid Number");
          delay(2000);
          lcd1.clear();
          lcd1.print("Enter Phone No:");
        }
      }
    }

    // Continuously display time when car is detected and timer is running
    if (carDetected && timerRunning) {
      time_val();
      delay(500); // Short delay to allow time to be read
    }
  } else if (carDetected && timerRunning) {
    // Car is no longer detected, stop the timer and ask for token
    Stop_timer();
    lcd1.clear();
    lcd1.print("Enter Token:");
    while (availableAttempt > 0 && enteredToken != token) {
      enteredToken = getUserInput();
      if (enteredToken == token) {
        moveServo(90); // Open the gate
        calculateCharge();
        displayCharge();
        delay(5000); // Display charge for 5 seconds
        resetSystem();
        break;
      } else {
        availableAttempt--;
        lcd1.clear();
        lcd1.print("Wrong Token");
        lcd1.setCursor(0, 1);
        lcd1.print("Attempts left:");
        lcd1.print(availableAttempt);
        delay(2000);
        lcd1.clear();
        lcd1.print("Enter Token:");
      }
    }
    if (availableAttempt <= 0) {
      sendWarning();
      lcd1.clear();
      lcd1.print("Max Attempts");
      delay(2000);
      resetSystem();
    }
  }
}

//test for IR that was malfunctioning

/*void loop(){
  if(digitalRead(IR_test)==LOW){
    lcd1.print("Detected IR");
    delay(500);
  }else{
    lcd1.print("haven't detected");
    delay(500);
  }
  lcd1.clear();
  lcd1.setCursor(0, 0);
}*/
