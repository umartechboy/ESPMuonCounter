
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include "Camera.h"

// define the number of bytes you want to access
#define EEPROM_SIZE 1

int pictureNumber = 0;
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector 
  Serial.begin(115200);
  
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  CameraSetup();
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;
  
  String path = "/picture" + String(pictureNumber) +".jpg";
  SavePhoto(path, false);
  EEPROM.write(0, pictureNumber);
  EEPROM.commit();
  
  delay(2000);
  Serial.println("Going to sleep now");
  delay(2000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  
}
// #include <Arduino.h>
// #include "Camera.h"
// #include "soc/soc.h"           // Disable brownout problems
// #include "soc/rtc_cntl_reg.h"  // Disable brownout problems
// #include <EEPROM.h>            // read and write from flash memory

// int pictureNumber = 0;

// void setup() {
//   WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector 
//   Serial.begin(115200);
//   int err = CameraSetup();
//   if (err == 2)
//     Serial.println("SD Card Mount Failed");
//   else if (err == 3)
//     Serial.println("No SD Card attached");
//   // initialize EEPROM with predefined size
//   EEPROM.begin(EEPROM_SIZE);
//   if (err){    
//     Serial.printf("Camera init failed with error 0x%x", err);
//     while(1)
//       yield();
//   }
// }

// void loop() {
  
//   pictureNumber = EEPROM.read(0) + 1;
  
//   // Path where new picture will be saved in SD Card
//   String path = "/picture" + String(pictureNumber) +".jpg";

//   TakePhoto(path);  
//   delay(2000);
// }
