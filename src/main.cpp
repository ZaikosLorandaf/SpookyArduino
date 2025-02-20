#include <Keypad.h>


const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'C', 'D', 'E', 'F'},
  {'B', '9', '6', '3'},
  {'0', '8', '5', '2'},
  {'A', '7', '4','1'}
};

byte rowPins[ROWS] = {25, 24, 23, 22}; 
byte colPins[COLS] = {29, 28, 27, 26}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.println(customKey);
  }
}
