#include <Arduino.h>
#include <ArduinoJson.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

JsonDocument controller;

/*~~~ Joystick ~~~*/
#define JOY_MIN_TRESH 800
#define JOY_MAX_TRESH 200

#define VRX A4
#define VRY A3

#define J_CENTER  0
#define J_UP      1
#define J_DOWN    2
#define J_RIGHT   3
#define J_LEFT    4

/*~~~~ Buttons ~~~*/
#define B_UP    31
#define B_DOWN  32
#define B_RIGHT 33
#define B_LEFT  30

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
int x, y, z;

/*~~~~~ LCD ~~~~*/
const int rs = 12;
const int en = 11;
const int d4 = 2;
const int d5 = 3;
const int d6 = 4;
const int d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

float lcdTime;

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
    return J_UP;
  else if (yVal < JOY_MIN_TRESH)
    return J_DOWN;
  else if (xVal > JOY_MAX_TRESH)
    return J_RIGHT;
  else if (xVal < JOY_MIN_TRESH)
    return J_LEFT;
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
  lcd.write(stream);
  lcd.setCursor(1,0);
}


void setup(){

  lcd.begin(16, 2); //Sets the LCD's amount of columns and rows.

  /*~~~~ Buttons ~~~*/
  pinMode(B_UP,INPUT_PULLUP);
  pinMode(B_DOWN,INPUT_PULLUP);
  pinMode(B_RIGHT,INPUT_PULLUP);
  pinMode(B_LEFT,INPUT_PULLUP);

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
  lcd.setCursor(1, 0);
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
  if (digitalRead(B_UP) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton du haut appuyé");
    digitalWrite(LED4,HIGH);
    delay(1000);
    digitalWrite(LED4,LOW);
  }

  if (digitalRead(B_RIGHT) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton de droite appuyé");
    digitalWrite(LED2,HIGH);
    delay(1000);
    digitalWrite(LED2,LOW);
  }


  if (digitalRead(B_DOWN) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton du bas appuyé");
    digitalWrite(LED3,HIGH);
    delay(1000);
    digitalWrite(LED3,LOW);
  }

  if (digitalRead(B_LEFT) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton de gauche appuyé");
    digitalWrite(LED5,HIGH);
    delay(1000);
    digitalWrite(LED5,LOW);
  }

  /*~~~~~~~~~~~~~~~~ Joystick ~~~~~~~~~~~~~~~~~~~~*/
  switch (getPosition()) {
    case J_UP:
      display("Up");
      controller["xJoy"] = 0;
      controller["yJoy"] = 1;
      break;
    case J_DOWN:
      display("Down");
      controller["xJoy"] = 0;
      controller["yJoy"] = -1;
      break;
    case J_RIGHT:
      display("Right");
      controller["xJoy"] = -1;
      controller["yJoy"] = 0;
      break;
    case J_LEFT:
      display("Left");
      controller["xJoy"] = 1;
      controller["yJoy"] = 0;
      break;
    default:
      lcd.write("Centre");
      delay(500);
      lcd.clear();
      lcd.setCursor(1,0);
      controller["xJoy"] = 0;
      controller["yJoy"] = 0;
  }


  /*~~~~~~~~~~~~~~~~ Accelerometer ~~~~~~~~~~~~~~~*/
  x = analogRead(A8);       // read analog input pin A4
  y = analogRead(A9);       // read analog input pin A5
  z = analogRead(A10);       // read analog input pin A6

  /*~~~~~~~~~~~~~~~~~~ BarGraph ~~~~~~~~~~~~~~~~*/
  writeBarGraph(getPot());

  if (!controller.isNull()) {
    serializeJson(controller, Serial);
    controller.clear();
  }
}

