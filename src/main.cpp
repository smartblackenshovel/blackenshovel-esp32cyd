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
#include <vector>


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

void drawMenu(const std::vector<String>& items, int itemHeight = 40, uint16_t bgColor = TFT_DARKGREY, uint16_t textColor = TFT_WHITE, uint16_t borderColor = TFT_WHITE) {
    tft.fillScreen(TFT_BLACK);
    int size = items.size();
    for (int i = 0; i < size; i++) {
        int y = i * itemHeight;
        tft.fillRect(0, y, tft.width(), itemHeight - 2, bgColor);
        tft.drawRect(0, y, tft.width(), itemHeight - 2, borderColor);
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(textColor);
        tft.drawString(items[i], tft.width()/2, y + itemHeight/2);
    }
}

std::vector<String> extractValues(const String& jsonStr, const String& key) {
    std::vector<String> result;

    StaticJsonDocument<2048> doc;
    if (deserializeJson(doc, jsonStr) != DeserializationError::Ok) return result;

    JsonArray arr = doc.as<JsonArray>();
    for (JsonObject obj : arr) {
        result.push_back(String(obj[key].as<const char*>()));
    }

    return result;
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

  char url[] = "https://tiny-teeth-thank.loca.lt";

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

  HTTPResponse mapResponse = httpHandler.get("/organizations");

  Serial.println("Map Response:");
  Serial.println(mapResponse.getContent());

  // Select Shovel
  String shovelsResponse = httpHandler.get(endpointShovels, {
    {"organization_id", organizationId}
  });

  Serial.println("Shovels Response:");
  Serial.println(shovelsResponse);

  std::vector<String> shovelsSerialNumber = extractValues(shovelsResponse, "serial_number");
  int shovelCount = shovelsSerialNumber.size();

  Serial.printf("Extracted %d serials:\n", shovelCount);
  for (String shovelSerialNumber : shovelsSerialNumber) {
    Serial.println(shovelSerialNumber);
  }
  drawMenu(shovelsSerialNumber, shovelCount);

  String selectedShovelSerial = "BS-#13823429-01";

  auto selectedShovelIt = std::find(shovelsSerialNumber.begin(), shovelsSerialNumber.end(), selectedShovelSerial);
  if (selectedShovelIt == shovelsSerialNumber.end()) {
    Serial.println("Selected shovel not found!");
    return;
  }
  int index = selectedShovelIt - shovelsSerialNumber.begin();
  Serial.printf("Selected shovel index: %d\n", index);

  // Select User
  String usersResponse = httpHandler.get(endpointUsers, {
    {"organization_id", organizationId}
  });
  Serial.println("Users Response:");
  Serial.println(usersResponse);

  std::vector<String> userNames = extractValues(usersResponse, "name");
  int userCount = userNames.size();
  drawMenu(userNames, userCount);


  // Start Session


  
}

void loop() {

}
