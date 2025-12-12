#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include <vector>
#include <time.h>
#include <ArduinoJson.h>
#include "JsonUtils.h"
#include "DisplayHandler.h"
#include "HTTPHandler.h"
#include "SessionManager.h"
#include "DataAccess/SerialPortReader.h"
#include "DataAccess/SerialPortWriter.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define FONT_SIZE 2

#define SD_CS 4

#define HTTP_ACTIVE 0

#define ESP32S3_RX_PIN 35
#define ESP32S3_TX_PIN 22

WiFiClientSecure securedClient;

SerialPortWriter serialPortWriter(Serial1);
SessionManager sessionManager(serialPortWriter);
DisplayHandler displayHandler(sessionManager);

SerialPortReader serialPortReader(Serial1, sessionManager);



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

String shovelSerialNumber = "BS-#13823429-02";
String shovelId;
String sessionId;

HTTPHandler httpHandler(url);

bool selectedUser = false;

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
  // Serial.println("Starting...");

  Serial1.begin(9600, SERIAL_8N1, ESP32S3_RX_PIN, ESP32S3_TX_PIN);

  if (!Serial) {
    // Wait for Serial to initialize
    delay(1000);
  }

  Serial.println("Serial initialized.");

  


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

  //-------------------------------------------------------------------------------------
  #if HTTP_ACTIVE
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
  #endif
  //-------------------------------------------------------------------------------------
  
  #if HTTP_ACTIVE
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
  #endif
}

void loop() {
  serialPortReader.read();
  displayHandler.updateGUI();
  
  #if HTTP_ACTIVE
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
  #endif

  displayHandler.refreshGUI();
}
