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
#include "Camera.h"
#include <SFE_BMP180.h>
#include "FreeSans14pt7b.h"
#include "FreeSans8pt7b.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include <EEPROM.h>            // read and write from flash memory

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
//        else Serial.println("error retrieving pressure measurement\n");
      }
      //else Serial.println("error starting pressure measurement\n");
    }
    //else Serial.println("error retrieving temperature measurement\n");
  }
  //else Serial.println("error starting temperature measurement\n");
  return 0;
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

int countA = 0, countB = 0;
uint16_t logNumber = 1;
String logFileName;
String snapShotsDir;
int snapShotsCount = 0;
bool hasCamera = false;
bool hasSD = false;
void ISR_A(){
  countA++;
}
void ISR_B(){
  countB++;
}
void setup() {
  //Serial.begin(115200);
  delay(100);
  Wire.begin(13, 3);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
  }
  // else 
  //   Serial.println(F("SSD1306 allocation done"));

  
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.println("booting...");
  display.print("bmp 180 init... ");
  display.display();

  if (pressure.begin()) {
    //Serial.println("BMP180 init success");    
    display.println("done");
    display.display();

    baseline = getPressure();
    
    // Serial.print("baseline pressure: ");
    // Serial.print(baseline);
    // Serial.println(" mb");  
  }
  else
  {
    // Serial.println("BMP180 init fail (disconnected?)\n\n");
    display.println("failed");
    display.display();
  }

  display.print("SD MMC...");
  display.display();
  
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin("/sdcard", true)){
    // Serial.println("SD Card Mount Failed");
    display.println("mount failed");
    display.display();
    delay(1000);
    hasSD = false;
  }
  else{
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
      //Serial.println("No SD Card attached");
      display.println("No SD Card");
      display.display();
      delay(1000);
      hasSD = false;
    }
    else {
      hasSD = true;
      // Serial.println("SD Card present");
      display.println("SD OK");
      display.display();
    }
  }
  display.print("OV2640... ");
  display.display();
  hasCamera = CameraSetup();
  if (hasCamera) {
    // Serial.println("Camera present");
    display.println("OK");
    display.display();
  } else {    
    display.println("failed");
    display.display();
  }
  if (hasSD) {
    delay(1000);
    
    for (int i =0 ; i< 0xFFFF && hasSD;i++){
      String fName = String("/M Log ") + String(i) + String(".csv");
      // Serial.print("Checking ");
      // Serial.print(fName);
      if (SD_MMC.exists(fName)){
        // Serial.println(", exists");
        continue;
      }
        // Serial.println(", doesn't exist");
      // we have found an available slot. Create the file.
      File f = SD_MMC.open(fName, FILE_WRITE);
      f.println("FlightTime_s,CountA_N,CountB_N,Altitude_ft");
      f.close();
      logFileName = fName;
      // Serial.print("Log file name: ");
      // Serial.println(logFileName);
      if (hasCamera) {
        snapShotsDir = String("/snapshots_") + String(i);
        if (!SD_MMC.exists(snapShotsDir)){
          SD_MMC.mkdir(snapShotsDir);
        }
        // Serial.print("Snap shots directory: ");
        // Serial.println(snapShotsDir);
      }
      break;
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("Saving at:");
    display.setFont(&FreeSans8pt7b);
    display.setCursor(0,25);
    display.print(logFileName);
    display.display();
    
    for(int i = 5; i >= 0 ; i--){
      display.setFont();
      display.fillRect(100, 24, 28, 8, BLACK);
      display.setCursor(122, 24);
      display.print(i);
      display.display();
      delay(1000);
    }
  }

  pinMode(4, INPUT);
  pinMode(1, INPUT);
  attachInterrupt(4, ISR_A, RISING); // flash pin
  attachInterrupt(1, ISR_B, RISING); // TX pin
  delay(100);
  countA = 0;
  countB = 0;
}
long lastCountA = 0;
long lastCountB = 0;
double lastAltitude = 0;
long lastLoggedAt = 0;
int photoNumber = 1;
void loop() {
  
  double a,P;
  // Get a new pressure reading:

  P = getPressure();

  // Show the relative altitude difference between
  // the new reading and the baseline reading:

  a = pressure.altitude(P, baseline);
  
  // Serial.print("Relative altitude: ");
  // if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  // Serial.print(a,1);
  // Serial.print(" meters, ");
  // if (a >= 0.0) Serial.print(" "); // add a space for positive numbers
  // Serial.print(a*3.28084,0);
  // Serial.print(" feet, Count A: ");
  // Serial.print(countA);
  // Serial.print(", Count B: ");
  // Serial.println(countB);
  
  
  // display.setCursor(0, 0);
  // display.setTextColor(SSD1306_WHITE);
  // display.clearDisplay();
  // display.println("relative altitude: ");
  // if (a >= 0.0) display.print(" "); // add a space for positive numbers
  // display.print(a,1);
  // display.print(" meters, ");
  // if (a >= 0.0) display.print(" "); // add a space for positive numbers
  // display.print(a*3.28084,0);
  // display.println(" feet");

  // display.print("A: ");
  // display.print(countA);
  // display.print(", B: ");
  // display.println(countB);
  // display.display();

  display.clearDisplay();

  display.setFont(&FreeSans8pt7b);

  display.fillRect(0, 0, 12, 32, WHITE);
  display.setTextColor(BLACK);
  display.setCursor(1, 11);
  display.print("A");
  display.setCursor(1, 30);
  display.print("B");

  display.setTextColor(WHITE);
  display.setCursor(16, 11);
  display.print(countA);
  display.setCursor(16, 30);
  display.println(countB);
  
  display.setCursor(90, 11);
  display.print(a*3.28084,0);
  display.print("ft");
  
  int x = 128;
  int y = 24;
  if (hasCamera){
    x -= 16;
    display.fillRoundRect(x + 8 - 3, y - 7, 6, 6, 1, WHITE);
    display.fillRoundRect(x, y - 5, 16, 12, 3, WHITE);
    display.fillCircle(x + 8, y, 3, BLACK);
    display.drawLine(x + 8 - 2, y - 6 + 1, x + 8 + 1, y - 6 + 1, BLACK);
    x -= 3; // margin
  }
  if (hasSD){
    x -= 16; // size
    display.fillRoundRect(x, y - 3, 16, 10, 1, WHITE);
    display.fillRoundRect(x + 4, y - 5, 12, 10, 1, WHITE);
    display.setFont();
    display.setCursor(x + 2 , y - 2);
    display.setTextColor(BLACK);
    display.print("SD");
  }
  

  display.display();
  
  delay(250);
  if ((lastCountA != countA || lastCountB != countB || abs(a - lastAltitude) > 1 || (millis() - lastLoggedAt) > 60000)
      &&
      (millis() - lastLoggedAt > 1000 && hasSD)){
    File file = SD_MMC.open(logFileName, FILE_APPEND);
    file.print(millis() / 1000.0F, 0); file.print(",");
    file.print(countA); file.print(",");
    file.print(countB); file.print(",");
    file.println(a);
    file.close();
    //Serial.print("#");
    if(hasCamera){
      String photoName = snapShotsDir + "/snap " + String(photoNumber) + ".jpg";
      SavePhoto(photoName);
      //Serial.print("@");
      photoNumber++;
    }
    //Serial.println();
    lastCountA = countA;
    lastCountB = countB;
    lastAltitude = a;
    lastLoggedAt = millis();
  }
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
