
#include <Keypad.h>

#include <LiquidCrystal.h>

#define max_joystick 800
#define min_joystick 200

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Accéléromètre
int x, y, z;
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Joystick
const int VRX_PIN = A4; // Arduino pin connected to VRX pin
const int VRY_PIN = A3;// Arduino pin connected to VRY pin

int xValue = 0; // To store value of the X axis
int yValue = 0; // To store value of the Y axis
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/ 
const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;


LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Keypad
const byte ROWS = 4;
const byte COLS = 4;

int cursorplace = 0;
int cursorrangee =0;
int cursorcolonne =0;

char hexaKeys[ROWS][COLS] = {
  {'C', 'D', 'E', 'F'},
  {'B', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'A', '7', '4','1'}
};

byte rowPins[ROWS] = {25, 24, 23, 22};
byte colPins[COLS] = {29, 28, 27, 26};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//Boutons
const int boutonhaut=31, boutondroit=33, boutonbas=32, boutongauche=30;
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
// 5 LED de couleurs
const int LED2=42, LED3=43, LED4=40, LED5=38, LED6=36;
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
const int BARGRAPHE_SIZE = 10;
const int POT_INPUT = A0;

int Bar_Pins[] = {52, 50, 48, 46, 44, 53, 51, 49, 47, 45};

int pot_Value = 0;
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/




void setup(){
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  //Boutons
  pinMode(boutonhaut,INPUT_PULLUP);
  pinMode(boutongauche,INPUT_PULLUP);
  pinMode(boutondroit,INPUT_PULLUP);
  pinMode(boutonbas,INPUT_PULLUP);
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  //5 LED de couleur
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  pinMode(LED4,OUTPUT);
  pinMode(LED5,OUTPUT);
  pinMode(LED6,OUTPUT);
  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  pinMode(VRX_PIN,INPUT);
  pinMode(VRY_PIN,INPUT);

  /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
  for(int barSegment = 0; barSegment < BARGRAPHE_SIZE; barSegment++)
  {
    pinMode(Bar_Pins[barSegment],OUTPUT);
    digitalWrite(Bar_Pins[barSegment],LOW);
  }

  Serial.begin(9600);
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

    //Serial.println(customKey);
    //lcd.print(customKey);
    lcd.setCursor(cursorcolonne,cursorrangee);
    lcd.write(customKey);
    Serial.println(cursorplace);
    cursorplace++;

  }


  if (digitalRead(boutonhaut) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton du haut appuyé");
    digitalWrite(LED5,HIGH);
    delay(1000);
    digitalWrite(LED5,LOW);
  }

  if (digitalRead(boutondroit) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton de droite appuyé");
    digitalWrite(LED3,HIGH);
    delay(1000);
    digitalWrite(LED3,LOW);
  }


  if (digitalRead(boutonbas) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton du bas appuyé");
    digitalWrite(LED4,HIGH);
    delay(1000);
    digitalWrite(LED4,LOW);
  }

  if (digitalRead(boutongauche) == LOW) { // Bouton pressé (car pull-up utilisé)
    Serial.println("Bouton de gauche appuyé");
    digitalWrite(LED6,HIGH);
    delay(1000);
    digitalWrite(LED6,LOW);
  }
  // read analog X and Y analog values
  xValue = analogRead(VRX_PIN);
  yValue = analogRead(VRY_PIN);

  if (xValue>max_joystick){
    lcd.write("Gauche");
    delay(500);
    lcd.clear();
    lcd.setCursor(1,0);
  }

  if (yValue>max_joystick){
    lcd.write("Haut");
    delay(500);
    lcd.clear();
    lcd.setCursor(1,0);
  }

  if (xValue<min_joystick){
    lcd.write("Droite");
    delay(500);
    lcd.clear();
    lcd.setCursor(1,0);
  }

  if (yValue<min_joystick){
    lcd.write("Bas");
    delay(500);
    lcd.clear();
    lcd.setCursor(1,0);
  }

  x = analogRead(A8);       // read analog input pin A4
  y = analogRead(A9);       // read analog input pin A5
  z = analogRead(A10);       // read analog input pin A6
  Serial.print(x, DEC);    // print the acceleration in the X axis
  Serial.print(" ");       // prints a space between the numbers
  Serial.print(y, DEC);    // print the acceleration in the Y axis
  Serial.print(" ");       // prints a space between the numbers
  Serial.println(z, DEC);  // print the acceleration in the Z axis
  //delay(250);

  pot_Value = analogRead(POT_INPUT);
  int barLevel = map(pot_Value, 0, 1023, 0, BARGRAPHE_SIZE);

  for(int barSegment = 0; barSegment < BARGRAPHE_SIZE; barSegment++) {
    if(barSegment < barLevel)
      digitalWrite(Bar_Pins[barSegment],HIGH);
    else if (barSegment > barLevel)
      digitalWrite(Bar_Pins[barSegment],LOW);
    if(pot_Value==0)
      digitalWrite(52, LOW);
  }
//Serial.println(analogRead(A0));



// print data to Serial Monitor on Arduino IDE
/* Serial.print("x = ");
   Serial.print(xValue);
   Serial.print(", y = ");
   Serial.println(yValue);
   delay(1000);*/
//Serial.println("TEST");
}

