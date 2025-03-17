#include <Arduino.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

/*~~ Json Init ~~~*/
JsonDocument controller;

/*~~~ Joystick ~~~*/
#define JOY_MIN_TRESH 800
#define JOY_MAX_TRESH 200

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
#define LCD_COL 16
#define LCD_ROW 2
const int rs = 12;
const int en = 11;
const int d4 = 2;
const int d5 = 3;
const int d6 = 4;
const int d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

float lcdTime;


/*~~~~ Timers ~~~*/
int displayTime = 500;
int ledTime = 500;
struct Timers {
  unsigned long time;
  int status;
};
Timers timerLCD;
Timers timerLED1;
Timers timerLED2;
Timers timerLED3;
Timers timerLED4;


/*~~~~ Keypad ~~~*/
int cursorplace = 0;
int cursorrangee = 0;
int cursorcolonne = 0;

byte rowPins[ROWS] = {25, 24, 23, 22};
byte colPins[COLS] = {29, 28, 27, 26};

char hexaKeys[ROWS][COLS] = {
  {'C', 'D', 'E', 'F'},
  {'B', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'A', '7', '4','1'}
};
Keypad customKeypad = Keypad(
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
  int level = map(value, 0, 1023, 0, BARGRAPHE_SIZE);
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
  }
  return potValue;
}

template <typename T>
void display(T stream){

  if (timerLCD.status)
    for (int i = 0; i < LCD_COL; i++)
      lcd.scrollDisplayRight();

  timerLCD.time = millis();
  timerLCD.status = 1;
  lcd.write(stream);
  lcd.setCursor(1,0);
}

void clearDisplay() {
  lcd.clear();
  lcd.setCursor(1,0);
  timerLCD.time = 0;
  timerLCD.status = 0;
}


/* Accelerometer */
struct Accel getAccel() {
  accel.x = analogRead(A8);
  accel.y = analogRead(A9);
  accel.z = analogRead(A10);
  return accel;
}

/*~~~~ Buttons ~~~*/
int getButton() {
  if (digitalRead(B_UP_PIN) == LOW)
    return UP;
  if (digitalRead(B_DOWN_PIN) == LOW)
    return DOWN;
  if (digitalRead(B_LEFT_PIN) == LOW)
    return LEFT;
  if (digitalRead(B_RIGHT_PIN) == LOW)
    return RIGHT;

  return 0;
}

/*~~~~~ LEDs ~~~~~*/
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


/*~~~~ Timers ~~~*/
void checktimer() {
  if (millis() - timerLCD.time > displayTime) {
    timerLCD.status = 0;
    clearDisplay();
  }

  if (millis() - timerLED1.time > ledTime)
    turnOffLED(1);
  if (millis() - timerLED2.time > ledTime)
    turnOffLED(2);
  if (millis() - timerLED3.time > ledTime)
    turnOffLED(3);
  if (millis() - timerLED4.time > ledTime)
    turnOffLED(4);
}





void setup(){

  lcd.begin(LCD_COL,LCD_ROW); //Sets the LCD's amount of columns and rows.

  /*~~~~ Buttons ~~~*/
  pinMode(B_UP_PIN,INPUT_PULLUP);
  pinMode(B_DOWN_PIN,INPUT_PULLUP);
  pinMode(B_RIGHT_PIN,INPUT_PULLUP);
  pinMode(B_LEFT_PIN,INPUT_PULLUP);

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
  for(int segment = 0; segment < BARGRAPHE_SIZE; segment++)
  {
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
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  char customKey = customKeypad.getKey();

  if (customKey){
    if (cursorplace >= 16)
    {
      cursorrangee = 1;
      cursorcolonne = cursorplace -16;
    }
    else{
      cursorcolonne = cursorplace;
    }

    //lcd.print(customKey);
    lcd.setCursor(cursorcolonne,cursorrangee);
    lcd.write(customKey);
    /*Serial.println(cursorplace);*/
    cursorplace++;
  }


  /*~~~~~~~~~~~~~~~~ Bouttons ~~~~~~~~~~~~~~~~*/
  switch (getButton()) {
    case UP:
      display("Button Up");
      timerLCD.time = millis();
      controller["bUp"] = 1;
      turnOnLED(1);
      break;
    case DOWN:
      display("Button Down");
      timerLCD.time = millis();
      controller["bDown"] = 1;
      turnOnLED(2);
      break;
    case RIGHT:
      display("Button Right");
      timerLCD.time = millis();
      controller["bRight"] = 1;
      turnOnLED(3);
      break;
    case LEFT:
      display("Button Left");
      timerLCD.time = millis();
      controller["bLeft"] = 1;
      turnOnLED(4);
      break;
  }


  /*~~~~~~~~~~~~~~~~ Joystick ~~~~~~~~~~~~~~~~~~~~*/
  switch (getPosition()) {
    case UP:
      display("Up");
      timerLCD.time = millis();
      controller["xJoy"] = 0;
      controller["yJoy"] = 1;
      break;
    case DOWN:
      display("Down");
      timerLCD.time = millis();
      controller["xJoy"] = 0;
      controller["yJoy"] = -1;
      break;
    case RIGHT:
      display("Right");
      timerLCD.time = millis();
      controller["xJoy"] = -1;
      controller["yJoy"] = 0;
      break;
    case LEFT:
      display("Left");
      timerLCD.time = millis();
      controller["xJoy"] = 1;
      controller["yJoy"] = 0;
      break;
    default:
      lcd.write("Centre");
      timerLCD.time = millis();
      delay(500);
      lcd.clear();
      lcd.setCursor(1,0);
      controller["xJoy"] = 0;
      controller["yJoy"] = 0;
  }



  /*~~~~~~~~~~~~~~~~ Accelerometer ~~~~~~~~~~~~~~~*/
  if (accelNeeded) {
    accel = getAccel();
    controller["accelX"] = accel.x;
    controller["accelY"] = accel.y;
    controller["accelZ"] = accel.z;
  }

  /*~~~~~~~~~~~~~~~~~~ BarGraph ~~~~~~~~~~~~~~~~*/
  writeBarGraph(getPot());


  /*~~~~~~~~~~~~~~~~~~ Timers ~~~~~~~~~~~~~~~~~~*/
  checktimer();

  if (!controller.isNull()) {
    serializeJson(controller, Serial);
    controller.clear();
  }
}

