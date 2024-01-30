/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x32 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SFE_BMP180.h>

SFE_BMP180 pressure;
double baseline; // baseline pressure

double getPressure()
{
  char status;
  double T,P,p0,a;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define SSD1306_WHITE WHITE
#define SSD1306_BLACK BLACK
#define SSD1306_INVERSE INVERSE

// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

void setup() {
  Serial.begin(115200);
  delay(1000);
  Wire.begin(13, 15);
  if (pressure.begin()) {
    Serial.println("BMP180 init success");    
    baseline = getPressure();
    
    Serial.print("baseline pressure: ");
    Serial.print(baseline);
    Serial.println(" mb");  
  }
  else
  {
    Serial.println("BMP180 init fail (disconnected?)\n\n");
  }

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
  }
  else 
    Serial.println(F("SSD1306 allocation done"));

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  display.display();
}

void loop() {
  
  double a,P;
  // Get a new pressure reading:

  P = getPressure();

  // Show the relative altitude difference between
  // the new reading and the baseline reading:

  a = pressure.altitude(P, baseline);
  
  Serial.println("Relative altitude: ");
  if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  Serial.print(a,1);
  Serial.print(" meters, ");
  if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  Serial.print(a*3.28084,0);
  Serial.println(" feet");
  
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  display.print("relative altitude: ");
  if (a >= 0.0) display.print(" "); // add a space for positive numbers
  display.print(a,1);
  display.print(" meters, ");
  if (a >= 0.0) display.print(" "); // add a space for positive numbers
  display.print(a*3.28084,0);
  display.println(" feet");
  display.display();
  
  delay(500);
}
// // #include <Arduino.h>
// // #include "Camera.h"
// // #include "soc/soc.h"           // Disable brownout problems
// // #include "soc/rtc_cntl_reg.h"  // Disable brownout problems
// // #include <EEPROM.h>            // read and write from flash memory

// // int pictureNumber = 0;

// // void setup() {
// //   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector 
// //   Serial.begin(115200);
// //   int err = CameraSetup();
// //   if (err == 2)
// //     Serial.println("SD Card Mount Failed");
// //   else if (err == 3)
// //     Serial.println("No SD Card attached");
// //   // initialize EEPROM with predefined size
// //   EEPROM.begin(EEPROM_SIZE);
// //   if (err){    
// //     Serial.printf("Camera init failed with error 0x%x", err);
// //     while(1)
// //       yield();
// //   }
// // }

// // void loop() {
  
// //   pictureNumber = EEPROM.read(0) + 1;
  
// //   // Path where new picture will be saved in SD Card
// //   String path = "/picture" + String(pictureNumber) +".jpg";

// //   TakePhoto(path);  
// //   delay(2000);
// // }
