#include "DisplayHandler.h"

void touchscreen_begin() {
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);
}

void lvgl_config() {
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);
}
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
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

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

// Callback that prints the current slider value on the TFT display and Serial Monitor for debugging purposes
static void slider_event_callback(lv_event_t * e) {
  lv_obj_t * slider = (lv_obj_t*) lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  LV_LOG_USER("Slider changed to %d%%", (int)lv_slider_get_value(slider));
}

static void event_handler(lv_event_t * e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = (lv_obj_t*) lv_event_get_target(e);
  if(code == LV_EVENT_CLICKED) {
    LV_LOG_USER("List item clicked: %s", lv_list_get_btn_text(user_list, obj));
    nextScreen(String(lv_list_get_btn_text(user_list, obj)));
  }
}


void lv_create_main_gui(void) {
  // Create a user selector list
  user_list = lv_list_create(lv_screen_active());
  lv_obj_set_size(user_list, SCREEN_WIDTH - 20, SCREEN_HEIGHT / 3);
  lv_obj_center(user_list);

  lv_obj_t * btn;
  
  lv_list_add_text(user_list, "User Selection");
  btn = lv_list_add_btn(user_list, LV_SYMBOL_FILE, "User 1");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
  btn = lv_list_add_btn(user_list, LV_SYMBOL_FILE, "User 2");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
  btn = lv_list_add_btn(user_list, LV_SYMBOL_FILE, "User 3");
  lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
}

static void lv_obj_del_anim_ready_cb(lv_anim_t * a) {
    lv_obj_t * obj = (lv_obj_t *)a->var;
    lv_obj_del(obj);
}

static void lv_demo_printer_anim_out_all(lv_obj_t * obj, uint32_t delay) {
    uint32_t child_count = lv_obj_get_child_count(obj);

    for (uint32_t i = 0; i < child_count; i++) {
        lv_obj_t * child = lv_obj_get_child(obj, i);

        lv_anim_t a;
        lv_anim_init(&a);

        lv_anim_set_var(&a, child);
        lv_anim_set_time(&a, 300);
        lv_anim_set_delay(&a, delay);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);

        lv_coord_t y  = lv_obj_get_y(child);
        lv_coord_t y2 = y + 100;

        lv_anim_set_values(&a, y, y2);

        // Delete object when animation completes
        lv_anim_set_ready_cb(&a, lv_obj_del_anim_ready_cb);

        lv_anim_start(&a);
    }
}

static void nextScreen(String userName) {
  lv_demo_printer_anim_out_all(lv_screen_active(), 0);
  lv_obj_t * textLabel = lv_label_create(lv_screen_active());
  lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);    // Breaks the
  lv_label_set_text(textLabel, userName.c_str());
  lv_obj_set_width(textLabel, 150);    // Set smaller width to make the lines wrap
  lv_obj_set_style_text_align(textLabel, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(textLabel, LV_ALIGN_CENTER, 0, 0);
}