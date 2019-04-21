
// DS1302:  RST pin    -> Arduino Digital 2
//          DATA pin   -> Arduino Digital 3
//          CLK pin  -> Arduino Digital 4

#include <DS1302.h>
#include <Wire.h>
#include <hardwareSerial.h>
#include "TM1637.h"
#include <ReceiveOnlySoftwareSerial.h>
#include <Arduino_FreeRTOS.h>

// define two tasks for UpdateClock & ReadSerial
void TaskUpdateClock(void *pvParameters);
void TaskReadSerial(void *pvParameters);

#define CLK 5                           //pins definitions for TM1637 and can be changed to other ports
#define DIO 6

TM1637 tm1637(CLK, DIO);
DS1302 rtc(2, 3, 4);
ReceiveOnlySoftwareSerial mySerial(10); //  Software receive RX PIN

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
  }

  xTaskCreate(TaskUpdateClock, (const portCHAR *)"UpdateClock", 128, NULL, 2, NULL);
  xTaskCreate(TaskReadSerial, (const portCHAR *)"ReadSerial", 128, NULL, 1, NULL);
}
void loop()
{
}

void TaskUpdateClock(void *pvParameters)
{
  (void)pvParameters;

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
  rtc.halt(false);
  rtc.writeProtect(false);

  // rtc.setDOW(SUNDAY);        // Set Day-of-Week to FRIDAY
  // rtc.setTime(22, 24, 00);   // Set the time to 12:00:00 (24hr format)
  // rtc.setDate(21, 04, 2019); // Set the date to August 6th, 2010
  uint8_t u8arry[] = {0, 1, 2, 3};
  boolean middlepoint = true;

  for (;;)
  {
    rtc.getu8time(u8arry);
    tm1637.display(0, u8arry[0]);
    tm1637.display(1, u8arry[1]);
    tm1637.point(middlepoint);
    tm1637.display(2, u8arry[2]);
    tm1637.display(3, u8arry[3]);
    vTaskDelay(1000 / portTICK_PERIOD_MS); // wait for one second
  }
}

void TaskReadSerial(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  mySerial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  for (;;)
  {
    recvWithStartEndMarkers();
    showNewData();
    vTaskDelay(1);
  }
}

void recvWithStartEndMarkers()
{

  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '(';
  char endMarker = ')';
  char rc;

  while (mySerial.available() > 0 && newData == false)
  {
    rc = mySerial.read();

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars)
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

void showNewData()
{
  if (newData == true)
  {
    Serial.print("This just in ... ");
    Serial.println(receivedChars);
    bluetooth_updatetime(receivedChars);
    newData = false;
  }
}

// updatetime("13:00")
void bluetooth_updatetime(char input[])
{
  char separator[] = ":";
  char *token;
  uint8_t times[] = {10, 10};
  uint8_t i = 0;
  token = strtok(input, separator);
  while (token != NULL)
  {
    times[i] = (uint8_t)atoi(token);
    i++;
    token = strtok(NULL, separator);
  }
  rtc.setTime(times[0], times[1], 00);
}
