#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Hardware SPI
#define OLED_DC     6
#define OLED_CS     10
#define OLED_RESET  8
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

//OLED inits
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

//UI inits
int screenNumber = 0;
int redLED = 5;
bool celsius = true;

//Accelerometer 
#include "SparkFunLIS3DH.h"
#include "Wire.h"
#include "SPI.h"
LIS3DH myIMU; //Default constructor is I2C, addr 0x19.
unsigned long steps = 0;

//Time
#include <RTCZero.h>
RTCZero rtc;

//Initialize time
const byte seconds = 0;
const byte minutes = 29;
const byte hours = 17;

//Initialize date
const byte day = 13;
const byte month = 4;
const byte year = 20;

//heartrate graph
int xaxis = 0;

unsigned long counter = 0;
unsigned long startTime = 0;

void setup()   {
  //set up pinout                
  Serial.begin(9600);
  pinMode(7, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, HIGH);

  //display inits
  display.begin(SSD1306_SWITCHCAPVCC);
  display.display();
  delay(2000);
  display.clearDisplay();


  //time inits
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);


  //accelerometer
  myIMU.begin();
}

//TODO: Add images/icons
//TODO: Remove double input (or make easier to use)
//TODO: Create button click(), hold(length), etc.
//TODO: Try double click input
void loop()
{
  //count steps
  if(myIMU.readFloatAccelX() > 1.2)
  {
    steps++;
    delay(500);
  }

  //navigate screen forward
  if(!digitalRead(12) && digitalRead(7))
  {
    screenNumber++;
    delay(250);
    if(screenNumber > 4)
    {
      screenNumber = 0;
    }
  }

  //TODO: Change navigation, remove backwards traversal
  //navigate screen backwards
  if(!digitalRead(7) && digitalRead(12))
  {
    screenNumber--;
    delay(250);
    if(screenNumber < 0)
    {
      screenNumber = 4;
    }
  }

  //TODO: Add more function (date, month, etc.)
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

  //TODO: Fix spacing
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

  //Get BPM data
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

  //TODO: Remove auto-start
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

  //thermometer screen
  if(screenNumber == 4)
  {
    //toggle between celsius and fahrenheit
    if(!digitalRead(12) && !digitalRead(7)){
      celsius = !celsius;
      delay(250);
    }
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Temp:");

    //convert analog signal to temperature
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
