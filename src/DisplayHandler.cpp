#include "DisplayHandler.h"

DisplayHandler* DisplayHandler::instance = nullptr;

DisplayHandler::DisplayHandler(SessionManager& sessionManager)
    : touchscreenSPI(VSPI),
      touchscreen(XPT2046_CS, XPT2046_IRQ),
      tft(),
      sessionManager(sessionManager),
      LVGLVersion(String("LVGL Library Version: ") + lv_version_major() + "." +
                   lv_version_minor() + "." + lv_version_patch()),
      currentScreen(INITIALIZE)
{
  instance = this;
}

void DisplayHandler::begin() {
  lvglInit();
  touchscreenBegin();
  lvglConfig();
}

void DisplayHandler::refreshGUI() {
  lv_task_handler();
  lv_tick_inc(TICK_DELAY);
  delay(TICK_DELAY);
}

void DisplayHandler::updateGUI() {
  switch (currentScreen) {
    case LOAD:
      userSelectionScreen();
      break;
    case USER_SELECTION:
      break;
    case USER_SELECTED:
      loadScreen(currentScreen);
      break;
    case LOAD_MAP:
      openMapScreen();
      break;
    case MAP:
      updateMapScreen();
      break;
    default:
      break;
  }
}

void DisplayHandler::touchscreenBegin() {
  Serial.println("Initializing touchscreen and TFT...");
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);
}

void DisplayHandler::logPrint(lv_log_level_t level, const char* buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

void DisplayHandler::lvglInit() {
  Serial.println("Initializing LVGL Library...");
  lv_init();
  lv_log_register_print_cb(logPrint);
}

void DisplayHandler::lvglConfig() {
  Serial.println("Configuring LVGL Library...");
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf,
                            sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreenReadStatic);
}

void DisplayHandler::touchscreenReadStatic(lv_indev_t* indev,
                                      lv_indev_data_t* data) {
  if (instance) {
    instance->touchscreenRead(indev, data);
  }
}

void DisplayHandler::touchscreenRead(lv_indev_t* indev,
                                      lv_indev_data_t* data) {
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    data->point.x = x;
    data->point.y = y;
    Serial.printf("Touch at x: %d, y: %d, z: %d\n", x, y, z);
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

void DisplayHandler::eventHandlerStatic(lv_event_t* e) {
  if (instance) {
    instance->eventHandler(e);
  }
}

void DisplayHandler::eventHandler(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = (lv_obj_t*)lv_event_get_target(e);
  if (code == LV_EVENT_CLICKED) {
    LV_LOG_USER("List item clicked: %s", lv_list_get_btn_text(usersList, obj));
    String selectedUserName = String(lv_list_get_btn_text(usersList, obj));
    sessionManager.selectUser(selectedUserName);
    currentScreen = USER_SELECTED;
  }

}

void DisplayHandler::showTextOnCenter(String text) {
  lv_obj_t* textLabel = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
  lv_label_set_text(textLabel, text.c_str());
  lv_obj_set_width(textLabel, 150);
  lv_obj_set_style_text_align(textLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 0);
}

void DisplayHandler::initializeScreen() {
  loadScreen(INITIALIZE);
}

void DisplayHandler::loadScreen(Screens screen) {
  if (screen == INITIALIZE) {
    cleanScreen();
    showTextOnCenter(String("Loading users for your organization..."));
    currentScreen = LOAD;
  } else if (screen == USER_SELECTED) {
    cleanScreen();
    showTextOnCenter(String("Loading map and calculating current location and next nearest spot..."));
    currentScreen = LOAD_MAP;
  }
}

void DisplayHandler::userSelectionScreen() {
  std::vector<User> users = sessionManager.getUsers();
  if (users.empty()) {
    return;
  }

  cleanScreen();

  usersList = lv_list_create(lv_screen_active());
  lv_obj_set_size(usersList, SCREEN_WIDTH - 20, SCREEN_HEIGHT / 3);
  lv_obj_center(usersList);

  lv_obj_t* btn;

  lv_list_add_text(usersList, "User Selection");

  for (const User& user : users) {
    btn = lv_list_add_btn(usersList, LV_SYMBOL_FILE, user.getName().c_str());
    lv_obj_add_event_cb(btn, eventHandlerStatic, LV_EVENT_ALL, NULL);
  }
  
  currentScreen = USER_SELECTION;
}

void DisplayHandler::openMapScreen() {
  if (!sessionManager.getSessionUser() || !sessionManager.getNextSpot() || sessionManager.getSessionUser()->getLocation().getLatitude() == 0.0) {
    return;
  }

  cleanScreen();

  LV_IMAGE_DECLARE(rapperswil_map);
  lv_obj_t * img1 = lv_image_create(lv_screen_active());
  lv_image_set_src(img1, &rapperswil_map);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);

  drawSpot(sessionManager.getNextSpot()->getLocation().getX(), sessionManager.getNextSpot()->getLocation().getY());
  drawUserLoc(sessionManager.getSessionUser()->getLocation().getX(), sessionManager.getSessionUser()->getLocation().getY());
  drawShovelIcon();

  currentScreen = MAP;
}

void DisplayHandler::updateMapScreen() {
  lv_obj_set_pos(spotPlaceholder, sessionManager.getNextSpot()->getLocation().getX(), sessionManager.getNextSpot()->getLocation().getY());
  lv_obj_set_pos(userLoc, sessionManager.getSessionUser()->getLocation().getX(), sessionManager.getSessionUser()->getLocation().getY());
}

void DisplayHandler::drawSpot(int32_t x, int32_t y) {

    spotPlaceholder = lv_obj_create(lv_scr_act());
    lv_obj_set_size(spotPlaceholder, 14, 14);
    lv_obj_clear_flag(spotPlaceholder, LV_OBJ_FLAG_SCROLLABLE);  // Not scrollable
    lv_obj_set_style_bg_color(spotPlaceholder, LV_COLOR_MAKE(255, 0, 0), 0);
    lv_obj_set_style_radius(spotPlaceholder, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(spotPlaceholder, 0, 0);

    // Position the spotPlaceholder so bottom center is at (x, y)
    lv_obj_set_pos(spotPlaceholder, x - 7, y - 14);

    // Optional: add small white center circle
    lv_obj_t* inner = lv_obj_create(spotPlaceholder);
    lv_obj_set_size(inner, 4, 4);
    lv_obj_center(inner);
    lv_obj_set_style_bg_color(inner, lv_color_white(), 0);
    lv_obj_set_style_radius(inner, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(inner, 0, 0);
}

void DisplayHandler::drawUserLoc(int32_t x, int32_t y) {
  userLoc = lv_obj_create(lv_scr_act());
  lv_obj_set_size(userLoc, 14, 14);
  lv_obj_set_pos(userLoc, x, y);
  lv_obj_set_style_bg_color(userLoc, LV_COLOR_MAKE(0, 0, 255), 0);

  lv_obj_set_style_border_width(userLoc, 0, 0);

  dot = lv_obj_create(userLoc);
  lv_obj_set_size(dot, 1, 1);
  lv_obj_center(dot);

  lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dot, lv_color_hex(0x0000FF), 0);
  lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);

  lv_obj_set_style_border_width(dot, 3, 0);
  lv_obj_set_style_border_color(dot, lv_color_hex(0xFFFFFF), 0);
}

void DisplayHandler::spotFinishedEventHandler(lv_event_t * e) {
  sessionManager.completeSpot();

}

void DisplayHandler::spotFinishedEventHandlerStatic(lv_event_t * e) {
  if (instance) {
    instance->spotFinishedEventHandler(e);
  }
}

void DisplayHandler::drawShovelIcon()
{
    shovelBox = lv_obj_create(lv_screen_active());
    lv_obj_set_size(shovelBox, 40, 40);
    lv_obj_add_event_cb(shovelBox, spotFinishedEventHandlerStatic, LV_EVENT_CLICKED, NULL);

    lv_obj_align(shovelBox, LV_ALIGN_TOP_RIGHT, -8, 8);

    lv_obj_set_style_radius(shovelBox, 5, 0);
    lv_obj_set_style_bg_color(shovelBox, lv_color_hex(0xFFFFFF), 0); // white example
    lv_obj_set_style_bg_opa(shovelBox, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(shovelBox, 0, 0);
    lv_obj_set_style_pad_all(shovelBox, 0, 0);

    LV_IMAGE_DECLARE(shovel);
    shovelIcon = lv_image_create(shovelBox);
    lv_image_set_src(shovelIcon, &shovel);

    lv_obj_center(shovelIcon);
}

void DisplayHandler::lvObjDelAnim(lv_anim_t* a) {
  lv_obj_t* obj = (lv_obj_t*)a->var;
  lv_obj_del(obj);
}

void DisplayHandler::cleanScreen() {
  lvAnimAllOut(lv_screen_active(), 100);
}

void DisplayHandler::lvAnimAllOut(lv_obj_t* obj, uint32_t delay) {
  uint32_t child_count = lv_obj_get_child_count(obj);

  for (uint32_t i = 0; i < child_count; i++) {
    lv_obj_t* child = lv_obj_get_child(obj, i);

    lv_anim_t a;
    lv_anim_init(&a);

    lv_anim_set_var(&a, child);
    lv_anim_set_time(&a, 300);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);

    lv_coord_t y = lv_obj_get_y(child);
    lv_coord_t y2 = y + 100;

    lv_anim_set_values(&a, y, y2);

    lv_anim_set_ready_cb(&a, lvObjDelAnim);

    lv_anim_start(&a);
  }
}
