/*
 Proof of concept sketch for uploading files from RFduino over Bluetooth Low Energy.
 
 Note: This is an abuse of bluetooth low energy. It works OK for small files.
 For larger files, you must add a delay between each send otherwise data is lost. 
*/

#include <SPI.h>
#include <SD.h>
#include <stdlib.h>
#include <RFduinoBLE.h>

const int chipSelect = 6;

boolean sendLogFile = false;

void setup() {
  Serial.begin(9600);
  
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
  RFduinoBLE.advertisementData = "file";
  RFduinoBLE.begin();
}

void loop() {
  
  if (sendLogFile) {
    char *file_name = "sample.log";
    int dataSize = 0;
    int bufferSize = 12; 
    uint8_t buffer[bufferSize];  
    
    Serial.print("Sending log file ");Serial.println(file_name);
    File file = SD.open(file_name, FILE_READ);
  
    if (file) {
      int index = 0;
      
      while (file.available()) {
     
        buffer[index] = file.read();
        index++;
        if (index == bufferSize) {
          dataSize += bufferSize;
          
          // print some output for debugging
          Serial.print("+");
          if (dataSize % (bufferSize * 12 * 6) == 0) { 
            Serial.println(""); // linefeed 
          } 
          
          // send the data
          RFduinoBLE.send((char*)buffer, bufferSize);
          
          // reset buffer
          index = 0;
          memset(buffer, 0, bufferSize);
          
          delay(30); // larger files need a bigger delay
        }
      }
      
      if (index > 0) {
        dataSize += index;
        Serial.println("Sending final chunk");
        RFduinoBLE.send((char*)buffer, bufferSize);
      }
      Serial.println("File data sent.");
      
      // send unique token to denote the end of file
      char *token = "QED";
      Serial.print("Sending end token ");Serial.println(token);
      RFduinoBLE.send(token, sizeof(token));
      file.close();
      Serial.print("Finished. Sent ");
      Serial.print(dataSize);
      Serial.println(" bytes.");
      sendLogFile = false;
    } else {
      Serial.print("error opening ");Serial.println(file_name);
    }
  }
  Serial.print(".");
  delay(2000);
}

void RFduinoBLE_onReceive(char *data, int len) {
  Serial.println("Log file requested.");  
  sendLogFile = true;
}

