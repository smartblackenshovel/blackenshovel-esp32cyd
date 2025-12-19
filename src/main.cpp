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
#include <vector>
#include <time.h>
#include <ArduinoJson.h>
#include "JsonUtils.h"


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

char url[] = "https://tame-otters-sniff.loca.lt";

String organizationId;

char endpointOrganizations[] = "/organizations";
char endpointShovels[] = "/shovels";
char endpointUsers[] = "/users";
char endpointSpots[] = "/spots";
char endpointSessions[] = "/sessions";
char endpointSpotLogs[] = "/spot_logs";
char endpointSessionLogs[] = "/session_logs";
char mapEndpoint[] = "/map?lat=47.2229&lon=8.8169";
char imageFileUri[] = IMAGE_NAME;
String shovelSerialNumber = "BS-#13823429-02";
String shovelId;
String sessionId;

HTTPHandler httpHandler(url);

bool selectedUser = false;

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

String currentTime() {
  time_t now = time(NULL);
  struct tm* timeinfo = localtime(&now);
  if (timeinfo == nullptr) return "Time not set";
  char buffer[40];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
  return String(buffer);
}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("Starting...");

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing touchscreen and TFT...");

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(1);
  tft.init();
  tft.setRotation(1);

  //-------------------------------------------------------------------------------------

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  securedClient.setInsecure();

  //-------------------------------------------------------------------------------------

  Serial.println("Syncing time...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); 
  Serial.println("Waiting for time...");
  time_t now;
  while ((now = time(NULL)) < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Time initialized.");

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing SD card...");

  if (!SD.begin()) {
    Serial.println("SD card failed");
    return;
  }

  Serial.println("SD card initialized.");

  //-------------------------------------------------------------------------------------

  Serial.println("Fetching Shovel...");
  HTTPResponse shovelsResponse = httpHandler.get(endpointShovels, {
    {"serial_number", shovelSerialNumber}
  });

  if (shovelsResponse.isSuccess()) {
    Serial.println("Shovel fetched successfully.");
    Serial.println(shovelsResponse.getContent());
    std::vector<String> shovelsSerialNumber = extractValues(shovelsResponse.getContentAsJson(), "serial_number");
    Serial.printf("Shovel Serial Number: %s\n", shovelsSerialNumber[0].c_str());
    shovelId = extractValues(shovelsResponse.getContentAsJson(), "id")[0];
  } else {
    Serial.printf("Failed to fetch shovels. Status code: %d\n", shovelsResponse.getStatusCode());
    return;
  }

  //-------------------------------------------------------------------------------------
  
  Serial.println("Fetching Organization...");

  organizationId = extractValues(shovelsResponse.getContentAsJson(), "organization_id")[0];

  HTTPResponse organizationsResponse = httpHandler.get(
    "/organizations/" + organizationId
  );

  if (organizationsResponse.isSuccess()) {
    Serial.println("Organization fetched successfully.");
    Serial.println(organizationsResponse.getContent());
  } else {
    Serial.printf("Failed to fetch organization. Status code: %d\n", organizationsResponse.getStatusCode());
    return;
  }
}

void loop() {

  if (!selectedUser) {

  // SELECT USER -------------------------------------------------------------------------

    Serial.println("Fetching Users...");
    HTTPResponse usersResponse = httpHandler.get(endpointUsers, {
      {"organization_id", organizationId}
    });

    if (usersResponse.isSuccess()) {
      Serial.println("Users fetched successfully.");
      Serial.println(usersResponse.getContent());
      std::vector<String> userNames = extractValues(usersResponse.getContentAsJson(), "name");
      int userCount = userNames.size();
      drawMenu(userNames, userCount);
    } else {
      Serial.printf("Failed to fetch users. Status code: %d\n", usersResponse.getStatusCode());
      return;
    }

    delay(5000);

    String selectedUserId = "9c1055f3-c3e5-4bdf-9ccb-66dd2fe57767";

    // START SESSION ----------------------------------------------------------------------

    Serial.println("Starting session...");
    if (shovelId == "" || selectedUserId == "") {
      return;
    }

    JsonDocument sessionPayload;
    sessionPayload["user_id"] = selectedUserId;
    sessionPayload["shovel_id"] = shovelId;

    HTTPResponse startSessionResponse = httpHandler.post(
      endpointSessions,
      sessionPayload
    );

    if (startSessionResponse.isSuccess()) {
      Serial.println("Session started successfully.");
      Serial.println(startSessionResponse.getContent());
      sessionId = startSessionResponse.getContentAsJson()["id"].as<const char*>();
      Serial.printf("Session ID: %s\n", sessionId);
    } else {
      Serial.printf("Failed to start session. Status code: %d\n", startSessionResponse.getStatusCode());
      return;
    }
  }

  selectedUser = true;

  // GET COORDINATES OF CURRENT LOCATION -------------------------------------------------

  Serial.println("Getting current location...");
  float latitude = 47.223117;
  float longitude = 8.817105;

  // START SESSION LOG -------------------------------------------------------------------

  Serial.println("Creating session log...");
  JsonDocument sessionLogPayload;
  sessionLogPayload["session_id"] = sessionId;
  sessionLogPayload["latitude"] = latitude;
  sessionLogPayload["longitude"] = longitude;
  sessionLogPayload["timestamp"] = currentTime();

  debug(sessionLogPayload);

  HTTPResponse sessionLogResponse = httpHandler.post(
    endpointSessionLogs,
    sessionLogPayload
  );

  if (sessionLogResponse.isSuccess()) {
    Serial.println("Session log posted successfully.");
    Serial.println(sessionLogResponse.getContent());
  } else {
    Serial.printf("Failed to post session log. Status code: %d\n", sessionLogResponse.getStatusCode());
  }

  delay(10000);
  return;

  // Get Coordinates of destination spot

  // Get Accel Data and Gyro data

  // Run ML Model

  // Display Routing

  // POST Session Log

  // Check if arrived at spot

  // Show confirmation dialog

  // POST Spot Log

  // Check if finished spot

  // Show confirmation dialog
  
  // POST Spot Log

  // Check for touch to end session

  // End Session
}
