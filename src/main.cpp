#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include <vector>
#include <time.h>
#include <ArduinoJson.h>
#include "utils/JsonUtils.h"
#include "DisplayHandler.h"
#include "SessionManager.h"
#include "dataAccess/SerialPortReader.h"
#include "dataAccess/SerialPortWriter.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define FONT_SIZE 2

#define SD_CS 4

#define ESP32S3_RX_PIN 35
#define ESP32S3_TX_PIN 22

SerialPortWriter serialPortWriter(Serial1);
SessionManager sessionManager(serialPortWriter);
DisplayHandler displayHandler(sessionManager);

SerialPortReader serialPortReader(Serial1, sessionManager);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  Serial1.begin(9600, SERIAL_8N1, ESP32S3_RX_PIN, ESP32S3_TX_PIN);

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing SD card...");

  if (!SD.begin()) {
    Serial.println("SD card failed");
    return;
  }

  Serial.println("SD card initialized."); // Important to have SD card initialized before TFT_eSPI
  if (SD.exists("/lake.jpg")) {
    Serial.println("Image exists!");
  }

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing LVGL Library...");
  String LVGL_Arduino = displayHandler.getLVGLVersion();
  Serial.println(LVGL_Arduino);
  
  Serial.println("LVGL initialized.");

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing Display...");
  
  displayHandler.begin();

  //-------------------------------------------------------------------------------------
  Serial.println("Creating main GUI...");
  displayHandler.initializeScreen();
}

void loop() {
  serialPortReader.read();
  displayHandler.updateGUI();
  displayHandler.refreshGUI();
}
