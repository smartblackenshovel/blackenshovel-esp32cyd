#pragma once

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include <Arduino.h>
#include "models/User.h"
#include "SessionManager.h"

// Screen / Touch constants
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

#define TICK_DELAY 5

class DisplayHandler {
public:
    DisplayHandler(SessionManager& sessionManager);         // Constructor
    void begin();             // Initialize display, touchscreen, LVGL
    void refreshGUI();            // Call periodically (e.g., in loop)
    void updateGUI();
    void initializeScreen();
    String getLVGLVersion() const { return LVGLVersion; }
    SessionManager& sessionManager;

private:
    static DisplayHandler* instance;

    enum Screens {
      INITIALIZE,
      LOAD,
      USER_SELECTION,
      MAP
    };

    Screens currentScreen;

    SPIClass touchscreenSPI;
    XPT2046_Touchscreen touchscreen;
    TFT_eSPI tft;

    uint32_t draw_buf[DRAW_BUF_SIZE / 4];

    lv_display_t * disp;
    lv_indev_t * indev;

    lv_obj_t * usersList;

    lv_obj_t * spotPlaceholder = nullptr;
    lv_obj_t * userLoc = nullptr;
    lv_obj_t * accuracy;
    lv_obj_t * dot;

    String LVGLVersion;

    int x, y, z;

    void touchscreenBegin();
    void lvglInit();
    void lvglConfig();
    void nextScreen(String userName);
    static void logPrint(lv_log_level_t level, const char * buf);

    static void touchscreenReadStatic(lv_indev_t * indev, lv_indev_data_t * data);
    void touchscreenRead(lv_indev_t * indev, lv_indev_data_t * data);

    static void lvObjDelAnim(lv_anim_t * a);
    static void lvAnimAllOut(lv_obj_t * obj, uint32_t delay);
    void cleanScreen();

    static void eventHandlerStatic(lv_event_t * e);
    void eventHandler(lv_event_t * e);

    void showTextOnCenter(String text);
    void drawSpot(int32_t x, int32_t y);
    void drawUserLoc(int32_t x, int32_t y);

    void loadScreen();
    void userSelectionScreen();
    void openMapScreen();
    void updateMapScreen();
};
