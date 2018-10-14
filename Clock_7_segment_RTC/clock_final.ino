
// DS1302:  RST pin    -> Arduino Digital 2
//          DATA pin   -> Arduino Digital 3
//          CLK pin  -> Arduino Digital 4

#include <DS1302.h>
#include <Wire.h>
#include <hardwareSerial.h>
#include "TM1637.h"

#define CLK 5 //pins definitions for TM1637 and can be changed to other ports
#define DIO 6

TM1637 tm1637(CLK, DIO);
DS1302 rtc(2, 3, 4);

void setup()
{
  // Serial.begin(9600);

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL); //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  // Set the clock to run-mode, and disable the write protection

  rtc.writeProtect(true);
  rtc.halt(false);

  // The following lines can be commented out to use the values already stored in the DS1302
  
  // rtc.writeProtect(false);
  // rtc.setDOW(SUNDAY);        // Set Day-of-Week to FRIDAY
  // rtc.setTime(15, 38, 00);   // Set the time to 12:00:00 (24hr format)
  // rtc.setDate(14, 10, 2018); // Set the date to August 6th, 2010

}

uint8_t u8arry[] = {0, 1, 2, 3};
boolean onoff = true;

void loop()
{

  //   Serial.print(rtc.getDOWStr(FORMAT_SHORT));
  //   Serial.print(rtc.getDateStr());
  //  Serial.println(rtc.getTimeStr(1));
  // get time is uint8_t array to feed the 4 segment display.

  rtc.getu8time(u8arry);
  onoff = !onoff;

  tm1637.display(0, u8arry[0]);
  tm1637.display(1, u8arry[1]);
  tm1637.point(onoff);
  tm1637.display(2, u8arry[2]);
  tm1637.display(3, u8arry[3]);

  delay(1000);
}
