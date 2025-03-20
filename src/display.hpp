#ifndef DISPLAY_HPP
#define DISPLAY_HPP

#include "LiquidCrystal.h"
#include <Arduino.h>

#define LCD_COL 16
#define LCD_ROW 2

class Display: public LiquidCrystal {
  public:
    struct Timer {
      unsigned long time1;
      unsigned long time2;
      int status1;
      int status2;
    };

    Display(
        uint8_t rs, uint8_t en, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
      :LiquidCrystal(rs, en, d4, d5, d6, d7) {
        for (int i = 0; i < LCD_COL; i++)
          emptyStr[i] = ' ';
        emptyStr[LCD_COL] = '\0';
      }

    template<typename T>
      void write(T stream) {
        home();
        LiquidCrystal::write(stream);

      }
    template<typename T>
      void write(T stream, int row) {
        clear(row);
        setCursor(0, row);
        LiquidCrystal::write(stream);
      }

    template<typename T>
      void write(T stream, int row, unsigned long delay) {
        switch (row) {
          case 0:
            timer.time1 = millis() + delay;
            timer.status1 = 1;
          case 1:
            timer.time2 = millis() + delay;
            timer.status2 = 1;
        }
        clear(row);
        setCursor(0, row);
        LiquidCrystal::write(stream);
      }

    void clear(int row) {
      setCursor(0, row);
      LiquidCrystal::write(emptyStr);
    }


    void checkTimers() {
      if (timer.time1 < millis() && timer.status1) {
        clear(0);
        timer.status1 = 0;
      }

      if (timer.time2 < millis() && timer.status2) {
        clear(1);
        timer.status2 = 0;
      }
    }



  private:
    Timer timer;      //Timer for upper row of lcd
    char emptyStr[LCD_COL + 1]; //Null terminated string
    char message1[LCD_COL];
    char message2[LCD_COL];


};



#endif
