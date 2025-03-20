#include <Arduino.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <ezButton.h>
#include "ArduinoJson/Json/JsonSerializer.hpp"
#include "HardwareSerial.h"
#include "Key.h"
#include "display.hpp"

/*~~ Json Init ~~~*/
JsonDocument controller;

/*~~~ Vibrator ~~~*/
#define VIB_PIN d7

/*~~~ Joystick ~~~*/
#define JOY_MIN_TRESH 200
#define JOY_MAX_TRESH 800
int joyState;

#define VRX A4
#define VRY A3

#define J_CENTER  0
#define UP      1
#define DOWN    2
#define RIGHT   3
#define LEFT    4

/*~~~~ Buttons ~~~*/
#define B_UP_PIN    31
#define B_DOWN_PIN  32
#define B_RIGHT_PIN 33
#define B_LEFT_PIN  30

/*~~~~~ LEDs ~~~~~*/
#define LED1 42
#define LED2 43
#define LED3 40
#define LED4 38
#define LED5 36

#define ROWS 4
#define COLS 4

/* Potentiometer */
#define POT_INPUT A0

/*~~ BarGraph ~~~*/
#define BARGRAPHE_SIZE 10
int barPin[] = {52, 50, 48, 46, 44, 53, 51, 49, 47, 45};

/* Accelerometer */
#define ACCEL_X A8
#define ACCEL_Y A9
#define ACCEL_Z A10
bool accelNeeded = false;
struct Accel{
  int x;
  int y;
  int z;
};
Accel accel{};


/*~~~~~ LCD ~~~~*/
const int rs = 12;
const int en = 11;
const int d4 = 2;
const int d5 = 3;
const int d6 = 4;
const int d7 = 5;
Display lcd(rs, en, d4, d5, d6, d7);

float lcdTime;


/*~~~~ Timers ~~~*/
int displayTime = 500;
int ledTime = 500;
struct Timers {
  unsigned long time;
  int status;
};
Timers timerLED1;
Timers timerLED2;
Timers timerLED3;
Timers timerLED4;
Timers timerLED5;
Timers timerVib;


/*~~~~ Keypad ~~~*/
int cursorplace = 0;
int cursorrangee = 0;
int cursorcolonne = 0;

char keypadMessage[LCD_COL] = {};
int MessageIndex = 0;
bool currentMessage = false;

byte rowPins[ROWS] = {25, 24, 23, 22};
byte colPins[COLS] = {29, 28, 27, 26};

char hexaKeys[ROWS][COLS] = {
  {'C', 'D', 'E', 'F'},
  {'B', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'A', '7', '4','1'}
};
Keypad keys = Keypad(
    makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);



/*~~~ Joystick ~~~*/
int xVal = 0;
int yVal = 0;

int getPosition() {
  xVal = analogRead(VRX);
  yVal = analogRead(VRY);

  if (yVal > JOY_MAX_TRESH)
    return UP;
  else if (yVal < JOY_MIN_TRESH)
    return DOWN;
  else if (xVal > JOY_MAX_TRESH)
    return RIGHT;
  else if (xVal < JOY_MIN_TRESH)
    return LEFT;
  else
    return J_CENTER;
}


/*~~~ BarGraph ~~~*/
void writeBarGraph(float value) {
  int level = map(value, 0, 1010, 0, BARGRAPHE_SIZE);
  for(int segment = 0; segment < BARGRAPHE_SIZE; segment++) {
    if(segment < level)
      digitalWrite(barPin[segment],HIGH);
    else if (segment > level)
      digitalWrite(barPin[segment],LOW);
    if(value==0)
      digitalWrite(52, LOW);
  }
}

/* Potentiometer */
int potValue, prevPotValue;
float getPot() {
  potValue = 0.9 * potValue + 0.1 * analogRead(POT_INPUT);
  if (prevPotValue != potValue){
    prevPotValue = potValue;
    /*controller["pot"] = map(potValue, 0, 1015, 0, 20);*/
  }
  return potValue;
}


/* Accelerometer */
struct Accel getAccel() {
  accel.x = analogRead(A8);
  accel.y = analogRead(A9);
  accel.z = analogRead(A10);
  return accel;
}

/*~~~~ Buttons ~~~*/
ezButton bUp(B_UP_PIN);
ezButton bDown(B_DOWN_PIN);
ezButton bRight(B_RIGHT_PIN);
ezButton bLeft(B_LEFT_PIN);

// To track the current state of the buttons in order
// to only send state changes to computer
bool bUpIsPressed = false;
bool bDownIsPressed = false;
bool bRightIsPressed = false;
bool bLeftIsPressed = false;



void getButton() {
  if (bUp.isPressed() && !bUpIsPressed) {
    bUpIsPressed = true;
    controller["bUp"] = 1;
  } else if (bUp.isReleased() && bUpIsPressed) {
    bUpIsPressed = false;
    controller["bUp"] = 0;
  }

  if (bDown.isPressed() && !bDownIsPressed) {
    bDownIsPressed = true;
    controller["bDown"] = 1;
  } else if (bDown.isReleased() && bDownIsPressed) {
    bDownIsPressed = false;
    controller["bDown"] = 0;
  }

  if (bRight.isPressed() && !bRightIsPressed) {
    bRightIsPressed = true;
    controller["bRight"] = 1;
  } else if (bRight.isReleased() && bRightIsPressed) {
    bRightIsPressed = false;
    controller["bRight"] = 0;
  }

  if (bLeft.isPressed() && !bLeftIsPressed) {
    bLeftIsPressed = true;
    controller["bLeft"] = 1;
  } else if (bLeft.isReleased() && bLeftIsPressed) {
    bLeftIsPressed = false;
    controller["bLeft"] = 0;
  }
}

/*~~~~ Keypad ~~~~*/
void messageInit() {
  lcd.LiquidCrystal::clear();
  lcd.write("Press A to send.", 0);
  currentMessage = true;
}

void getKeypad() {
  if (!currentMessage && keys.keyStateChanged())
    messageInit();

  switch (keys.getKey()) {
    case 'A':
      keypadMessage[MessageIndex] = '\0';
      controller["keypad"] = keypadMessage;
      lcd.write("Sent!", 0, 1000);
      lcd.write(keypadMessage, 1, 1000);
      currentMessage = false;
      MessageIndex = 0;
      keypadMessage[0] = '\0';
      break;
    case 'C':
      lcd.LiquidCrystal::clear();
      lcd.write("Cleared!", 0, 500);
      currentMessage = false;
      MessageIndex = 0;
      keypadMessage[0] = '\0';
      break;
    case NO_KEY:
      break;
    default:
      keypadMessage[MessageIndex] = keys.getKey();
      MessageIndex++;
      lcd.write(keypadMessage, 1);
  }
}

/*~~~~~ LEDs ~~~~~*/
void turnOffLED(int led) {
  switch (led) {
    case 1:
      digitalWrite(LED1, LOW);
      break;
    case 2:
      digitalWrite(LED2, LOW);
      break;
    case 3:
      digitalWrite(LED3, LOW);
      break;
    case 4:
      digitalWrite(LED4, LOW);
      break;
  }
}

void turnOffAllLED() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

void turnOnLED(int led) {
  switch (led) {
    case 1:
      digitalWrite(LED1, HIGH);
      break;
    case 2:
      digitalWrite(LED2, HIGH);
      break;
    case 3:
      digitalWrite(LED3, HIGH);
      break;
    case 4:
      digitalWrite(LED4, HIGH);
      break;
  }
}

void turnOnLED(int led, int time) {
  switch (led) {
    case 1:
      digitalWrite(LED1, HIGH);
      if (timerLED1.status == 0) {
        timerLED1.time = millis();
        timerLED1.status = 1;
      } else if (timerLED1.time + time < millis()) {
        turnOffLED(1);
        timerLED1.status = 0;
      }
      break;
    case 2:
      digitalWrite(LED2, HIGH);
      if (timerLED2.status == 0) {
        timerLED2.time = millis();
        timerLED2.status = 1;
      } else if (timerLED2.time + time < millis()) {
        turnOffLED(2);
        timerLED2.status = 0;
      }
      break;
    case 3:
      digitalWrite(LED3, HIGH);
      if (timerLED3.status == 0) {
        timerLED3.time = millis();
        timerLED3.status = 1;
      } else if (timerLED3.time + time < millis()) {
        turnOffLED(3);
        timerLED3.status = 0;
      }
      break;
    case 4:
      digitalWrite(LED4, HIGH);
      if (timerLED4.status == 0) {
        timerLED4.time = millis();
        timerLED4.status = 1;
      } else if (timerLED4.time + time < millis()) {
        turnOffLED(4);
        timerLED4.status = 0;
      }
      break;
  }
}


/*~~~~ Timers ~~~*/

/* Checks LED timers and turns them off accordingly */
void checktimer() {
  if (millis() - timerLED1.time > ledTime)
    turnOffLED(1);
  if (millis() - timerLED2.time > ledTime)
    turnOffLED(2);
  if (millis() - timerLED3.time > ledTime)
    turnOffLED(3);
  if (millis() - timerLED4.time > ledTime)
    turnOffLED(4);
}


/*~~~ Vibrator ~~~*/
const int motorPin = 9; // Digital pin to which the motor is connected

/*
   digitalWrite(motorPin, HIGH);
   delay(1000); // Vibration for 1 second

// Turn off the vibration motor
digitalWrite(motorPin, LOW);
delay(2000); // Pause for 2 seconds before the next vibration
}
*/



void setup(){

  lcd.begin(LCD_COL,LCD_ROW); //Sets the LCD's amount of columns and rows.


  /*~~~ Vibrator ~~~*/
  pinMode(motorPin, VIB_PIN); // Set the motor pin as an output


  /*~~~~~ LEDs ~~~~~*/
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(LED4,OUTPUT);
  pinMode(LED5,OUTPUT);

  /*~~~ Joystick ~~~*/
  pinMode(VRX,INPUT);
  pinMode(VRY,INPUT);

  /* Potentiometer */
  potValue = analogRead(POT_INPUT);
  prevPotValue = analogRead(POT_INPUT);

  /*~~~ BarGraph ~~~*/
  for(int segment = 0; segment < BARGRAPHE_SIZE; segment++) {
    pinMode(barPin[segment],OUTPUT);
    digitalWrite(barPin[segment],LOW);
  }

  /*~~~~ Serial ~~~~*/
  Serial.begin(9600);
  controller["init"] = 1;
  serializeJson(controller, Serial);
  controller.clear();
}

void loop(){
  bUp.loop();
  bDown.loop();
  bRight.loop();
  bLeft.loop();

  getKeypad();
  getButton();
  lcd.checkTimers();
  checktimer();

  /*~~~~~~~~~~~~~~~~~~ BarGraph ~~~~~~~~~~~~~~~~*/
  writeBarGraph(getPot());

  /*~~~~~~~~~~~~~~~~ Buttons ~~~~~~~~~~~~~~~~~*/
  if (bUpIsPressed) {
    lcd.write("Button Up", 1, 500);
    turnOnLED(1);
  }

  if (bDownIsPressed) {
    lcd.write("Button Down", 1, 500);
    turnOnLED(2);
  }

  if (bRightIsPressed) {
    lcd.write("Button Right", 0, 500);
    turnOnLED(3);
  }

  if (bLeftIsPressed) {
    lcd.write("Button Left", 0, 500);
    turnOnLED(4);
  }


  /*~~~~~~~~~~~~~~~~ Joystick ~~~~~~~~~~~~~~~~~~~~*/
  switch (getPosition()) {
    case UP:
      if (joyState == UP) break;
      lcd.write("Up", 1, 500);
      controller["joy"] = UP;
      joyState = UP;
      break;
    case DOWN:
      if (joyState == DOWN) break;
      lcd.write("Down", 1, 500);
      controller["joy"] = DOWN;
      joyState = DOWN;
      break;
    case RIGHT:
      if (joyState == RIGHT) break;
      lcd.write("Right", 1, 500);
      controller["joy"] = RIGHT;
      joyState = RIGHT;
      break;
    case LEFT:
      if (joyState == LEFT) break;
      lcd.write("Left", 1, 500);
      controller["joy"] = LEFT;
      joyState = LEFT;
      break;
    case J_CENTER:
      if (joyState == J_CENTER) break;
      lcd.write("Centre", 1, 500);
      controller["joy"] = J_CENTER;
      joyState = J_CENTER;
  }



  /*~~~~~~~~~~~~~~~~ Accelerometer ~~~~~~~~~~~~~~~*/
  if (accelNeeded) {
    accel = getAccel();
    controller["accelX"] = accel.x;
    controller["accelY"] = accel.y;
    controller["accelZ"] = accel.z;
  }

  if (!controller.isNull()) {
    serializeJson(controller, Serial);
    controller.clear();
  }
}
