#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include "HTTPHandler.h"
#include <vector>
#include <time.h>
#include <ArduinoJson.h>
#include "JsonUtils.h"
#include <lvgl.h>


#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define FONT_SIZE 2

#define SD_CS 4

#define HTTP_ACTIVE 0

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

WiFiClientSecure securedClient;

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

String shovelSerialNumber = "BS-#13823429-02";
String shovelId;
String sessionId;

HTTPHandler httpHandler(url);

bool selectedUser = false;

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
    /* Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();*/
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

int btn1_count = 0;
// Callback that is triggered when btn1 is clicked
static void event_handler_btn1(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  if(code == LV_EVENT_CLICKED) {
    btn1_count++;
    LV_LOG_USER("Button clicked %d", (int)btn1_count);
  }
}

// Callback that is triggered when btn2 is clicked/toggled
static void event_handler_btn2(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
  if(code == LV_EVENT_VALUE_CHANGED) {
    LV_UNUSED(obj);
    LV_LOG_USER("Toggled %s", lv_obj_has_state(obj, LV_STATE_CHECKED) ? "on" : "off");
  }
}

static lv_obj_t * slider_label;
// Callback that prints the current slider value on the TFT display and Serial Monitor for debugging purposes
static void slider_event_callback(lv_event_t * e) {
  lv_obj_t * slider = (lv_obj_t*) lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Slider changed to %d%%", (int)lv_slider_get_value(slider));
}

void lv_create_main_gui(void) {
  // Create a text label aligned center on top ("Hello, world!")
  lv_obj_t * text_label = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(text_label, LV_LABEL_LONG_WRAP);    // Breaks the long lines
  lv_label_set_text(text_label, "Hello, world!");
  lv_obj_set_width(text_label, 150);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(text_label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(text_label, LV_ALIGN_CENTER, 0, -90);

  lv_obj_t * btn_label;
  // Create a Button (btn1)
  lv_obj_t * btn1 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, event_handler_btn1, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -50);
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  btn_label = lv_label_create(btn1);
  lv_label_set_text(btn_label, "Button");
  lv_obj_center(btn_label);

  // Create a Toggle button (btn2)
  lv_obj_t * btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler_btn2, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 10);
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  btn_label = lv_label_create(btn2);
  lv_label_set_text(btn_label, "Toggle");
  lv_obj_center(btn_label);
  
  // Create a slider aligned in the center bottom of the TFT display
  lv_obj_t * slider = lv_slider_create(lv_screen_active());
  lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
  lv_obj_add_event_cb(slider, slider_event_callback, LV_EVENT_VALUE_CHANGED, NULL);
  lv_slider_set_range(slider, 0, 100);
  lv_obj_set_style_anim_duration(slider, 2000, 0);

  // Create a label below the slider to display the current slider value
  slider_label = lv_label_create(lv_screen_active());
  lv_label_set_text(slider_label, "0%");
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
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

  Serial.println("Starting...");

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing SD card...");

  if (!SD.begin()) {
    Serial.println("SD card failed");
    return;
  }

  Serial.println("SD card initialized."); // Important to have SD card initialized before TFT_eSPI

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing LVGL Library...");
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.println(LVGL_Arduino);
  lv_init();
  lv_log_register_print_cb(log_print);
  Serial.println("LVGL initialized.");

  //-------------------------------------------------------------------------------------

  Serial.println("Initializing touchscreen and TFT...");
  
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);

  //-------------------------------------------------------------------------------------

  Serial.println("Configuring LVGL Library...");

  lv_display_t * disp;
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  lv_create_main_gui();

  Serial.println("LVGL configured.");

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

  lv_task_handler();
  lv_tick_inc(5);
  delay(5);
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
