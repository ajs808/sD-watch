#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Uncomment this block to use hardware SPI
#define OLED_DC     6
#define OLED_CS     10
#define OLED_RESET  8
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//this is for the screen
int screenNumber = 0;
int redLED = 5;
bool celsius = true;

//This is for the pedometer
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"
LIS3DH myIMU; //Default constructor is I2C, addr 0x19.
unsigned long steps = 0;


//This is for the time
#include <RTCZero.h>
RTCZero rtc;
/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 29;
const byte hours = 17;

/* Change these values to set the current initial date */
const byte day = 13;
const byte month = 4;
const byte year = 20;


//this is for the graph
int xaxis = 0;


//this is for the timer
unsigned long counter = 0;
unsigned long startTime = 0;

void setup()   {                
  Serial.begin(9600);
  pinMode(7, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, HIGH);

  //this is for the display
  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();
  delay(2000);
  display.clearDisplay();


  //this is for the time
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);


  //this is for the step counter / pedometer
  myIMU.begin();
}

void loop()
{
  //this is for the pedometer
  if(myIMU.readFloatAccelX() > 1.2)
  {
    steps++;
    delay(500);
  }

  //this is for reading the button to change the screen
  if(!digitalRead(12) && digitalRead(7))
  {
    screenNumber++;
    delay(250);
    if(screenNumber > 4)
    {
      screenNumber = 0;
    }
  }
  if(!digitalRead(7) && digitalRead(12))
  {
    screenNumber--;
    delay(250);
    if(screenNumber < 0)
    {
      screenNumber = 4;
    }
  }

  //clock screen
  if(screenNumber == 0)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("");
    display.print(rtc.getHours());
    display.print(":");
    if(rtc.getMinutes() < 10){
      display.print("0");
    }
    display.print(rtc.getMinutes());
    display.print(":");
    if(rtc.getSeconds() < 10){
      display.print("0");
    }
    display.println(rtc.getSeconds());
    display.display();
  }

  //pedometer screen
  if(screenNumber == 1)
  {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("STEPS:");
    display.println(steps);
    display.display();
  }

  //graph screen
  if(screenNumber == 2)
  {
    digitalWrite(redLED, LOW);
    if(xaxis == 0)
    {
      display.clearDisplay();
    }
    int photo = analogRead(A1);
    Serial.println(photo);
    photo = map(photo, 0, 1023, 64, 0);
    display.drawPixel(xaxis,photo, WHITE); 
    display.display();
    
    xaxis++;
    if(xaxis > 128)
    {
      xaxis = 0;
    }
  }

  //stopwatch screen
  if(screenNumber == 3)
  {
    if(counter == 0)
    {
      startTime = millis();
    }
    
    unsigned long currentTime = (millis()-startTime)/1000;
    unsigned long stopwatch_minutes = currentTime/60;
    unsigned long stopwatch_seconds = currentTime%60;
    unsigned long stopwatch_millis = (millis()-startTime)/100%10;
    counter++;
    
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(stopwatch_minutes);
    display.print(":");
    if(stopwatch_seconds < 10)
    {
      display.print("0");
      display.print(stopwatch_seconds);
    }
    else
    {
      display.print(stopwatch_seconds);
    }
    display.print(":");
    display.println(stopwatch_millis);
    display.display();
  }
  if(screenNumber == 4)
  {
    if(!digitalRead(12) && !digitalRead(7)){
      celsius = !celsius;
      delay(250);
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Temp:");
    double volts = 3.3*(analogRead(A5) / 1023.0);
    double Rtherm = (10000.0*volts)/(3.3-volts);
    double temp = (1.0  /((1/298.15)+(1/4250.0)*log(Rtherm/22000.0)))-273.15;
    if(celsius){
      display.print(temp);
      display.println(" C*");
    }
    else{
      temp = temp*9/5+32;
      display.print(temp);
      display.println(" F*");
    }
    delay(250);
    display.display();
  }

  if(screenNumber != 3) {
    counter = 0;
    digitalWrite(redLED, HIGH);
  }
}
