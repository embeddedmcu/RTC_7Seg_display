#include <Wire.h>  // Handles I2C
#include <EEPROM.h>  // Brightness, Baud rate, and I2C address are stored in EEPROM
// This code uses the SevSeg library, which can be donwnloaded from:
// https://github.com/sparkfun/SevSeg
#include "SevSeg.h" //Library to control generic seven segment displays
#include <RTClib.h>

SevSeg myDisplay; //Create an instance of the object
RTC_DS1307 rtc;
char formatted[] = "00-00-00 00:00:00x";

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Struct for 4-digit, 7-segment display 
// Stores display value (digits),  decimal status (decimals) for each digit, and cursor for overall display
struct display
{
  char digits[4];
  unsigned char decimals;
  unsigned char cursor;
} 
display;  // displays be displays

// This is effectively the UART0 byte received interrupt routine
// But not quite: serialEvent is only called after each loop() interation
void serialEvent()
{
  while (Serial.available()) 
  {
    unsigned char c = Serial.read();  // Read data byte into c, from UART0 data register
    Serial.write(c);
  }
}

//This is the normal mode where we display whatever data is coming in over UART, SPI, and I2C
void displayData()
{
static   unsigned char toggle = 0;

  DateTime now = rtc.now();
  char hr = 0;
  char mins;
/*  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();*/
  hr = now.hour();
  mins = now.minute();
//  Serial.print(hr, DEC);

  display.digits[0] = hr/10;
  display.digits[1] = hr%10;
  display.digits[2] = mins/10;
  display.digits[3] = mins%10;
  
  if((now.second() & 0x01) == 0x01)
     display.decimals = 0;
  else
     display.decimals = 1<<4;
  
  if(!(hr >=6 && hr < 20))
  myDisplay.DisplayString(display.digits, display.decimals); //(numberToDisplay, decimal point location)

  //delay(1000);  
}

//Sets up the hardware pins to control the 7 segments and display type
void setupDisplay()
{
  myDisplay.SetBrightness(100); //Set the display to 100% bright

  // Set the initial state of displays and decimals 'x' =  off
  display.digits[0] = 'x';
  display.digits[1] = 'x';
  display.digits[2] = 'x';
  display.digits[3] = 'x';
  display.decimals = 0;  // Turn all decimals off
  display.cursor = 0;  // Set cursor to first (left-most) digit

  //Declare what pins are connected to the digits

  int digit1 = 16; // DIG1 = A2/16 (PC2)
  int digit2 = 17; // DIG2 = A3/17 (PC3)
  int digit3 = 3;  // DIG3 = D3 (PD3)
  int digit4 = 4;  // DIG4 = D4 (PD4)

  //Declare what pins are connected to the segments
  int segA = 8;  // A = D8 (PB0)
  int segB = 14; // B = A0 (PC0)
  int segC = 6;  // C = D6 (PD6), shares a pin with colon cathode
  int segD = 15; // D = A1 (PC1)
  int segE = 23; // E = PB7 (not a standard Arduino pin: Must add PB7 as digital pin 23 to pins_arduino.h)
  int segF = 7;  // F = D7 (PD6), shares a pin with apostrophe cathode
  int segG = 5;  // G = D5 (PD5)
  int segDP= 22; //DP = PB6 (not a standard Arduino pin: Must add PB6 as digital pin 22 to pins_arduino.h)

  int digitColon = 2; // COL-A = D2 (PD2) (anode of colon)
  int segmentColon = 6; // COL-C = D6 (PD6) (cathode of colon), shares a pin with C
  int digitApostrophe = 9; // APOS-A = D9 (PB1) (anode of apostrophe)
  int segmentApostrophe = 7; // APOS-C = D7 (PD7) (cathode of apostrophe), shares a pin with F

  int numberOfDigits = 4; //Do you have a 2 or 4 digit display?

  int displayType = COMMON_ANODE; //SparkFun 10mm height displays are common anode

  //Initialize the SevSeg library with all the pins needed for this type of display
  myDisplay.Begin(displayType, numberOfDigits, 
  digit1, digit2, digit3, digit4, 
  digitColon, digitApostrophe, 
  segA, segB, segC, segD, segE, segF, segG, 
  segDP,
  segmentColon, segmentApostrophe);
}


void setup()
{  
  Serial.begin(9600);
  setupDisplay(); //Initialize display stuff (common cathode, digits, brightness, etc)
  //Wire.begin();

  //Preload the display buffer with a default
  display.digits[0] = 0;
  display.digits[1] = 0;
  display.digits[2] = 0;
  display.digits[3] = 0;
  display.decimals = 1 << 4;  // Turn on colon
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }  
}

// The display is constantly PWM'd in the loop()
void loop()
{
    displayData();
}

