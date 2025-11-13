#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <PNGdec.h>
#include <TFT_eFEX.h>
#include <SD.h>
#include "DisplayHandler.h"

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define FONT_SIZE 2

#define SD_CS 4

TFT_eSPI tft = TFT_eSPI();
TFT_eFEX fex = TFT_eFEX(&tft);

PNG png;

WiFiClientSecure securedClient;
FileFetcher fileFetcher(securedClient);

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

int x, y, z;

const char* ssid = "iPhone de Lauro";
const char* password = "lolo1234";

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Starting...");

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  securedClient.setInsecure();

  char url[] = "https://famous-insects-sniff.loca.lt/map?lat=47.2229&lon=8.8169";
  char imageFileUri[] = IMAGE_NAME;

  if (!SD.begin()) {
    Serial.println("SD card failed");
    return;
  }

  HTTPClient http;
  
  http.begin(url);
  int code = http.GET();

  if (code > 0) {
    Serial.printf("Final HTTP code: %d\n", code);
    if (code == HTTP_CODE_OK) {
      WiFiClient* stream = http.getStreamPtr();
      File file = SD.open("/test.jpg", FILE_WRITE);
      if (!file) {
        Serial.println("Failed to open file for writing");
        http.end();
        return;
      }

      uint8_t buffer[512];
      Serial.println("Downloading image...");

      while (http.connected() || stream->available()) {
        size_t size = stream->available();
        if (size) {
          size_t c = stream->readBytes(buffer, (size > sizeof(buffer) ? sizeof(buffer) : size));
          file.write(buffer, c);
        }
        delay(1);
      }

      file.close();
      Serial.println("Download complete!");
    }
  } else {
    Serial.printf("HTTP GET failed: %s\n", http.errorToString(code).c_str());
  }
  http.end();

  fex.drawJpgFile(SD, "/test.jpg", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  // File testFile = SD.open("/image.jpg", FILE_WRITE);
  // if (testFile) {
  //   testFile.println("Hello SD!");
  //   testFile.close();
  //   Serial.println("Write successful");
  // } else {
  //   Serial.println("Error opening file for writing");
  // }
  // Test reading the file
  // File testFile = SD.open("/image.jpg");
  // if (testFile) {
  //   Serial.println("Reading file contents:");
  //   while (testFile.available()) {
  //     Serial.write(testFile.read());
  //   }
  //   testFile.close();
  // } else {
  //   Serial.println("Error opening file for reading");
  // }
}
  //fex.drawJpeg("/image.jpg", 0, 0);

  // DisplayImageHandler::init(tft, securedClient, fileFetcher);
  // DisplayImageHandler::getImage(url);
  // DisplayImageHandler::displayImage(imageFileUri);

  

void loop() {

}
