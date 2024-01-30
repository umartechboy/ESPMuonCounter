
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
bool hasCamera = false;

int pictureNumber = 0;
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector 
  Serial.begin(115200);
  
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  if (!CameraSetup()){
    Serial.println("Camera init failed");
    hasCamera = false;
  }
  else{
    hasCamera = true;  
    // initialize EEPROM with predefined size
    EEPROM.begin(EEPROM_SIZE);
    pictureNumber = EEPROM.read(64) + 1;    
  
    //Serial.println("Starting SD Card");
    if(!SD_MMC.begin("/sdcard", true)){
      Serial.println("SD Card Mount Failed");
    }
    else{
      uint8_t cardType = SD_MMC.cardType();
      if(cardType == CARD_NONE){
        Serial.println("No SD Card attached");
      }
    }
  }
}

void loop() {
  if (hasCamera){      
    String path = "/picture" + String(pictureNumber) +".jpg";
    if(!SavePhoto(path))
      Serial.println("Photo not saved");
    else{
      EEPROM.write(0, pictureNumber);
      EEPROM.commit();
    }    
    delay(2000);
  }  
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
