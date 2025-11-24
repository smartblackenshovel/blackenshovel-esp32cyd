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

  // Select User

  // Start Session

  
  
}

void loop() {

}
