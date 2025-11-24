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
#include "HTTPHandler.h"
#include <ArduinoJson.h>


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

PNG png;

WiFiClientSecure securedClient;
FileFetcher fileFetcher(securedClient);

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

int x, y, z;

const char* ssid = "iPhone de Lauro";
const char* password = "lolo1234";

void drawMenu(
    const String items[],   // list of Strings
    int size,               // number of items
    int itemHeight = 40,    // height of each row
    uint16_t bgColor = TFT_DARKGREY,  // background color
    uint16_t textColor = TFT_WHITE,   // text color
    uint16_t borderColor = TFT_WHITE  // border color
) {
    tft.fillScreen(TFT_WHITE);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);

    for (int i = 0; i < size; i++) {
        int y = i * itemHeight;

        // Background of each item
        tft.fillRect(0, y, tft.width(), itemHeight - 2, bgColor);

        // Border
        tft.drawRect(0, y, tft.width(), itemHeight - 2, borderColor);

        // Text
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(textColor);
        tft.drawString(items[i], tft.width() / 2, y + itemHeight / 2);
    }
}



void extractSerials(const String& jsonStr, String serials[], int &count, int maxCount) {
    StaticJsonDocument<2048> doc;
    if (deserializeJson(doc, jsonStr) != DeserializationError::Ok) return;

    JsonArray arr = doc.as<JsonArray>();
    count = 0;
    for (JsonObject obj : arr) {
        if (count >= maxCount) break;
        serials[count++] = String(obj["serial_number"].as<const char*>());
    }
}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Starting...");

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  tft.init();
  tft.setRotation(1);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  securedClient.setInsecure();

  char url[] = "https://cute-flowers-boil.loca.lt";

  char organizationId[] = "39eb05fa-f039-4404-a3dc-0ca5a1a47a6e";

  char endpointOrganizations[] = "/organizations";
  char endpointShovels[] = "/shovels";
  char endpointUsers[] = "/users";
  char endpointSpots[] = "/spots";
  char endpointSessions[] = "/sessions";
  char endpointSpotLogs[] = "/spot_logs";
  char endpointSessionLogs[] = "/session_logs";

  char mapEndpoint[] = "/map?lat=47.2229&lon=8.8169";
  char imageFileUri[] = IMAGE_NAME;

  if (!SD.begin()) {
    Serial.println("SD card failed");
    return;
  }

  HTTPHandler httpHandler(url);

  String mapResponse = httpHandler.get("/organizations");

  Serial.println("Map Response:");
  Serial.println(mapResponse);

  // Select Shovel
  String shovelsResponse = httpHandler.get(endpointShovels, {
    {"organization_id", organizationId}
  });

  Serial.println("Shovels Response:");
  Serial.println(shovelsResponse);

  String serials[20];
  int shovelCount = 0;
  extractSerials(shovelsResponse, serials, shovelCount, 20);
  Serial.printf("Extracted %d serials:\n", shovelCount);
  for (int i = 0; i < shovelCount; i++) {
    Serial.println(serials[i]);
  }
  drawMenu(serials, shovelCount);

  // Select User

  // Start Session


  
}

void loop() {

}
