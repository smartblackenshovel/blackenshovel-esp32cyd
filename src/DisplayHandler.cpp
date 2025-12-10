#include "DisplayHandler.h"

DisplayHandler* DisplayHandler::instance = nullptr;

DisplayHandler::DisplayHandler(SessionManager& SessionManager)
    : touchscreenSPI(VSPI),
      touchscreen(XPT2046_CS, XPT2046_IRQ),
      tft(),
      sessionManager(sessionManager),
      LVGLVersion(String("LVGL Library Version: ") + lv_version_major() + "." +
                   lv_version_minor() + "." + lv_version_patch())
{
  instance = this;
}

void DisplayHandler::begin() {
  lvglInit();
  touchscreenBegin();
  lvglConfig();
}

void DisplayHandler::handle() {
  lv_task_handler();
  lv_tick_inc(TICK_DELAY);
  delay(TICK_DELAY);
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
    nextScreen(String(lv_list_get_btn_text(usersList, obj)));
  }

}

void DisplayHandler::createMainGUI(void) {
  usersList = lv_list_create(lv_screen_active());
  lv_obj_set_size(usersList, SCREEN_WIDTH - 20, SCREEN_HEIGHT / 3);
  lv_obj_center(usersList);

  lv_obj_t* btn;

  lv_list_add_text(usersList, "User Selection");

  std::vector<User> users = sessionManager.getUsers();

  
  btn = lv_list_add_btn(usersList, LV_SYMBOL_FILE, "User 1");
  lv_obj_add_event_cb(btn, eventHandlerStatic, LV_EVENT_ALL, NULL);
  btn = lv_list_add_btn(usersList, LV_SYMBOL_FILE, "User 2");
  lv_obj_add_event_cb(btn, eventHandlerStatic, LV_EVENT_ALL, NULL);
  btn = lv_list_add_btn(usersList, LV_SYMBOL_FILE, "User 3");
  lv_obj_add_event_cb(btn, eventHandlerStatic, LV_EVENT_ALL, NULL);
}

void DisplayHandler::lvObjDelAnim(lv_anim_t* a) {
  lv_obj_t* obj = (lv_obj_t*)a->var;
  lv_obj_del(obj);
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

void DisplayHandler::nextScreen(String userName) {
  lvAnimAllOut(lv_screen_active(), 0);
  // lv_obj_t* textLabel = lv_label_create(lv_screen_active());
  // lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
  // lv_label_set_text(textLabel, test.c_str());
  // lv_obj_set_width(textLabel, 150);
  // lv_obj_set_style_text_align(textLabel, LV_TEXT_ALIGN_CENTER, 0);
  // lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 0);

  LV_IMAGE_DECLARE(rapperswil_map);
  lv_obj_t * img1 = lv_image_create(lv_screen_active());
  lv_image_set_src(img1, &rapperswil_map);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
}