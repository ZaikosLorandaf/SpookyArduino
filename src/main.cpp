#include <Arduino.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <ezButton.h>
#include "display.hpp"

/*~~ Json Init ~~~*/
JsonDocument toPC;
/*JsonDocument pc;*/
/*char input[] = "{\"accelNeeded\",\"lcdMessage\"}";*/

/* Json document legend:
~~ Sent from Arduino ~~
bUp:  int (0,1)
bDo:  int (0,1)
bLe:  int (0,1)
bRi:  int (0,1)

joy:  int (0-4)

pot:  int (0-20)

accX: int (0-10)
accY: int (0-10)
accZ: int (0-10)

scream: bool

kpd:  char[LCD_COL] (LCD_COL = 16)

~~ Needed from PC ~~
accelNeeded: byte (000)
lcdMessage: char[16]
*/



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
#define MAX_POT_VAL 1010

/*~~ BarGraph ~~~*/
#define BARGRAPHE_SIZE 10
int barPin[] = {52, 50, 48, 46, 44, 53, 51, 49, 47, 45};

/* Accelerometer */
#define ACCEL_X A8
#define ACCEL_Y A9
#define ACCEL_Z A10
#define ACCEL_MIN 250
#define ACCEL_MAX 400
#define ACCEL_MIN_Z 100
#define ACCEL_MAX_Z 900
#define ACCEL_RANGE 10
byte accelNeeded = 0; //byte value from 0(none) to 7(all)
struct Accel{
  int x;
  int y;
  int z;
};
Accel prevAccel{};
Accel mappedAccel{};

//Remaps acceleromete values in each axis betwen 0 and "ACCEL_RANGE"
void getMappedAccel() {
  mappedAccel.x = map(analogRead(A8), ACCEL_MIN, ACCEL_MAX, 0, ACCEL_RANGE);
  mappedAccel.y = map(analogRead(A9), ACCEL_MIN, ACCEL_MAX, 0, ACCEL_RANGE);
  mappedAccel.z = map(analogRead(A10), ACCEL_MIN_Z, ACCEL_MAX_Z, 0, ACCEL_RANGE);
}

//Checks and sends accelerometer valuer in desird values
void checkAccel(byte axis) {
  if (axis & (1<<0) && prevAccel.x != mappedAccel.x) {
    toPC["accX"] = mappedAccel.x;
    prevAccel.x = mappedAccel.x;
  }
  if (axis & (1<<1) && prevAccel.y != mappedAccel.y) {
    toPC["accY"] = mappedAccel.y;
    prevAccel.y = mappedAccel.y;
  }
  if (axis & (1<<2) && prevAccel.z != mappedAccel.z) {
    toPC["accZ"] = mappedAccel.z;
    prevAccel.z = mappedAccel.z;
  }
}

/*~~~~~ LCD ~~~~*/
const int rs = 12;
const int en = 11;
const int d4 = 2;
const int d5 = 3;
const int d6 = 4;
const int d7 = 5;
Display lcd(rs, en, d4, d5, d6, d7);

float lcdTime;

char* lcdMessage;

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
char pressedKey = false;
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
  int level = map(value, 0, MAX_POT_VAL, 0, BARGRAPHE_SIZE);
  for(int segment = 0; segment < BARGRAPHE_SIZE; segment++) {
    if(segment < level)
      digitalWrite(barPin[segment],HIGH);
    else if (segment > level)
      digitalWrite(barPin[segment],LOW);
    if(value==0)
      digitalWrite(52, LOW);
  }
}

/*~~~ Vibrator ~~~*/
const int motorPin = 7; // Digital pin to which the motor is connected

// Vibrate for 'time' milliseconds
void vibrate(int time) {
  digitalWrite(motorPin, HIGH);
  timerVib.time = millis() + time;
  timerVib.status = 1;
}

/* Potentiometer */
int potValue, prevPotValue, prevMappedVal;

// Change the base of the Potentiometer
// from 0-'MAX_POT_VAL' to 0-'val'
void remapSendValue(int val, int max) {
  int mappedVal;
  mappedVal = map(val, 0, MAX_POT_VAL, 0, max);
  if (prevMappedVal != mappedVal) {
    prevMappedVal = mappedVal;
    vibrate(20);
    toPC["pot"] = mappedVal;
  }
}

float getPot() {
  potValue = 0.9 * potValue + 0.1 * analogRead(POT_INPUT);
  if (prevPotValue != potValue){
    prevPotValue = potValue;
    remapSendValue(potValue, 15);
  }
  return potValue;
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
    toPC["bUp"] = 1;
  } else if (bUp.isReleased() && bUpIsPressed) {
    bUpIsPressed = false;
    toPC["bUp"] = 0;
  }

  if (bDown.isPressed() && !bDownIsPressed) {
    bDownIsPressed = true;
    toPC["bDo"] = 1;
  } else if (bDown.isReleased() && bDownIsPressed) {
    bDownIsPressed = false;
    toPC["bDo"] = 0;
  }

  if (bRight.isPressed() && !bRightIsPressed) {
    bRightIsPressed = true;
    toPC["bRi"] = 1;
  } else if (bRight.isReleased() && bRightIsPressed) {
    bRightIsPressed = false;
    toPC["bRi"] = 0;
  }

  if (bLeft.isPressed() && !bLeftIsPressed) {
    bLeftIsPressed = true;
    toPC["bLe"] = 1;
  } else if (bLeft.isReleased() && bLeftIsPressed) {
    bLeftIsPressed = false;
    toPC["bLe"] = 0;
  }
}

/*~~~~ Keypad ~~~~*/
void messageInit() {
  lcd.LiquidCrystal::clear();
  lcd.write("Press A to send.", 0);
  currentMessage = true;
}

void getKeypad() {
  if (pressedKey != 0 && keys.getKey() != 0)
    return;

  if (!currentMessage && keys.keyStateChanged())
    messageInit();

  pressedKey = keys.getKey();

  switch (pressedKey) {
    case 'A':
      toPC["kpd"] = keypadMessage;
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
      pressedKey = false;
      break;
    default:
      keypadMessage[MessageIndex] = pressedKey;
      keypadMessage[MessageIndex + 1] = '\0';
      ++MessageIndex;
      lcd.write(keypadMessage, 1);
      break;
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
    case 5:
      digitalWrite(LED5, LOW);
      break;
  }
}

void turnOffAllLED() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
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
    case 5:
      digitalWrite(LED5, HIGH);
      break;
  }
}

void turnOnLED(int led, unsigned long time) {
  switch (led) {
    case 1:
      digitalWrite(LED1, HIGH);
      timerLED1.time = millis() + time;
      timerLED1.status = 1;
      break;
    case 2:
      digitalWrite(LED2, HIGH);
      timerLED2.time = millis() + time;
      timerLED2.status = 1;
      break;
    case 3:
      digitalWrite(LED3, HIGH);
      timerLED3.time = millis() + time;
      timerLED3.status = 1;
      break;
    case 4:
      digitalWrite(LED4, HIGH);
      timerLED4.time = millis() + time;
      timerLED4.status = 1;
      break;
    case 5:
      digitalWrite(LED5, HIGH);
      timerLED5.time = millis() + time;
      timerLED5.status = 1;
      break;
  }
}

void turnOnLED(int led, int time) {
  switch (led) {
    case 1:
      digitalWrite(LED1, HIGH);
      if (timerLED1.status != 0) break;
      timerLED1.time = millis() + time;
      timerLED1.status = 1;
      break;
    case 2:
      digitalWrite(LED2, HIGH);
      if (timerLED2.status != 0) break;
      timerLED2.time = millis() + time;
      timerLED2.status = 1;
      break;
    case 3:
      digitalWrite(LED3, HIGH);
      if (timerLED3.status != 0) break;
      timerLED3.time = millis() + time;
      timerLED3.status = 1;
      break;
    case 4:
      digitalWrite(LED4, HIGH);
      if (timerLED4.status != 0) break;
      timerLED4.time = millis() + time;
      timerLED4.status = 1;
      break;
    case 5:
      digitalWrite(LED5, HIGH);
      if (timerLED5.status != 0) break;
      timerLED5.time = millis() + time;
      timerLED5.status = 1;
      break;
  }
}

/*~~~~ Timers ~~~*/

/* Checks LED timers and turns them off accordingly */
void checktimer() {
  if (millis() > timerLED1.time && timerLED1.status) {
    turnOffLED(1);
    timerLED1.status = 0;
  }
  if (millis() > timerLED2.time && timerLED2.status) {
    turnOffLED(2);
    timerLED2.status = 0;
  }
  if (millis() > timerLED3.time && timerLED3.status) {
    turnOffLED(3);
    timerLED3.status = 0;
  }
  if (millis() > timerLED4.time && timerLED4.status) {
    turnOffLED(4);
    timerLED4.status = 0;
  }
  if (millis() > timerLED5.time && timerLED5.status) {
    turnOffLED(5);
    timerLED5.status = 0;
  }
  if (millis() > timerVib.time && timerVib.status) {
    digitalWrite(motorPin, LOW);
    timerVib.status = 0;
  }
}

void checkJoyState() {
  switch (getPosition()) {
    case UP:
      if (joyState == UP) break;
      lcd.write("Up", 1, 500);
      toPC["joy"] = UP;
      joyState = UP;
      turnOnLED(5);
      break;
    case DOWN:
      if (joyState == DOWN) break;
      lcd.write("Down", 1, 500);
      toPC["joy"] = DOWN;
      joyState = DOWN;
      turnOnLED(5);
      break;
    case RIGHT:
      if (joyState == RIGHT) break;
      lcd.write("Right", 1, 500);
      toPC["joy"] = RIGHT;
      joyState = RIGHT;
      turnOnLED(5);
      break;
    case LEFT:
      if (joyState == LEFT) break;
      lcd.write("Left", 1, 500);
      toPC["joy"] = LEFT;
      joyState = LEFT;
      turnOnLED(5);
      break;
    case J_CENTER:
      if (joyState == J_CENTER) break;
      lcd.write("Centre", 1, 500);
      toPC["joy"] = J_CENTER;
      joyState = J_CENTER;
      turnOnLED(5, ledTime);
      break;
  }
}

void getButtonState() {
  if (bUpIsPressed) {
    lcd.write("Button Up", 1, 500);
    turnOnLED(1, ledTime);
  }
  if (bDownIsPressed) {
    lcd.write("Button Down", 1, 500);
    turnOnLED(2, ledTime);
  }
  if (bRightIsPressed) {
    lcd.write("Button Right", 0, 500);
    turnOnLED(3, ledTime);
  }
  if (bLeftIsPressed) {
    lcd.write("Button Left", 0, 500);
    turnOnLED(4, ledTime);
  }
}


void readMsg() {
  JsonDocument doc;
  JsonVariant parseMsg;

  DeserializationError error = deserializeJson(doc, Serial);

  if (error) {
    Serial.print("deserialize() failed: ");
    Serial.println(error.c_str());
  }

  if (!doc["accelNeeded"].isNull())
    accelNeeded = doc["accelNeeded"].as<byte>();
  else
    accelNeeded = 0;

  if (!doc["lcdMessage"].isNull())
    lcd.write(doc["lcdMessage"].as<unsigned char>());
}


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
  Serial.begin(115200);
  toPC["init"] = 1;
  serializeJson(toPC, Serial);
  toPC.clear();
}

void loop(){
  /*~~~~~~ Needed for Buttons... ~~~~~~~*/
  bUp.loop();
  bDown.loop();
  bRight.loop();
  bLeft.loop();

  /*~~~~~~~~~~~~ Functions ~~~~~~~~~~~~~*/
  getKeypad();
  getButton();
  checktimer();
  lcd.checkTimers();

  /*~~~~~~~~~~~ Accelerometer ~~~~~~~~~~*/
  getMappedAccel();
  checkAccel(accelNeeded);

  /*~~~~~~~~~~~~~ BarGraph ~~~~~~~~~~~~~*/
  writeBarGraph(getPot());

  /*~~~~~~~~~~~~~ Buttons ~~~~~~~~~~~~~~*/
  getButtonState();

  /*~~~~~~~~~~~~~ Joystick ~~~~~~~~~~~~~*/
  checkJoyState();

  /*~~~~~~~~~~~~~~~ Json ~~~~~~~~~~~~~~~*/

  if (Serial.available())
    readMsg();

  if (!toPC.isNull()) {
    serializeJson(toPC, Serial);
    toPC.clear();
  }
}
