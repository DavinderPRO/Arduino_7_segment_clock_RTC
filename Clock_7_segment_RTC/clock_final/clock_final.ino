
// DS1302:  RST pin    -> Arduino Digital 2
//          DATA pin   -> Arduino Digital 3
//          CLK pin  -> Arduino Digital 4

#include <DS1302.h>
#include <Wire.h>
#include <hardwareSerial.h>
#include "TM1637.h"
#include <ReceiveOnlySoftwareSerial.h>
#include <Arduino_FreeRTOS.h>

// define two tasks for Blink & AnalogRead
void TaskUpdateClock(void *pvParameters);
void TaskReadSerial(void *pvParameters);

ReceiveOnlySoftwareSerial mySerial(10); // RX
#define CLK 5                           //pins definitions for TM1637 and can be changed to other ports
#define DIO 6
TM1637 tm1637(CLK, DIO);
DS1302 rtc(2, 3, 4);

const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

void setup()
{

  Serial.begin(9600); //try  115200 faster communication
  while (!Serial)
  {
  } // wait for Serial to become available

  // set the data rate for the ReceiveOnlySoftwareSerial port

  // Now set up two tasks to run independently.
  xTaskCreate(
      TaskUpdateClock, (const portCHAR *)"Blink" // A name just for humans
      ,
      128 // This stack size can be checked & adjusted by reading the Stack Highwater
      ,
      NULL, 2 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      ,
      NULL);

  xTaskCreate(
      TaskReadSerial, (const portCHAR *)"AnalogRead", 128 // Stack size
      ,
      NULL, 1 // Priority
      ,
      NULL);
}

void loop()
{
}

void TaskUpdateClock(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL);
  rtc.halt(false);
  rtc.writeProtect(false);

  rtc.setDOW(SUNDAY);        // Set Day-of-Week to FRIDAY
  rtc.setTime(22, 24, 00);   // Set the time to 12:00:00 (24hr format)
  rtc.setDate(21, 04, 2019); // Set the date to August 6th, 2010

  uint8_t u8arry[] = {0, 1, 2, 3};
  boolean onoff = true;
  for (;;)
  {

    rtc.getu8time(u8arry);
    tm1637.display(0, u8arry[0]);
    tm1637.display(1, u8arry[1]);
    tm1637.point(onoff);
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
  char startMarker = '<';
  char endMarker = '>';
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
    updatetime(receivedChars);
    newData = false;
  }
}

void updatetime(char input[])
{

  //     char input[] = "13:00";
  char separator[] = ":";
  char *token;

  uint8_t times[] ={10, 10};

  uint8_t i = 0;

  token = strtok(input, separator);

  while (token != NULL)
  {
    Serial.println("token: ");
    Serial.println(token);
    times[i] = (uint8_t)atoi(token); i++;
    token = strtok(NULL, separator);
  }

  Serial.print("final time: ");
  Serial.print(times[0]);
  Serial.print(times[1]);

  rtc.setTime(times[0], times[1], 00);

}