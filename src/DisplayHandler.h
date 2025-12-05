#pragma once

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include <Arduino.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

uint32_t draw_buf[DRAW_BUF_SIZE / 4];

extern SPIClass touchscreenSPI;
extern XPT2046_Touchscreen touchscreen;

lv_display_t * disp;
lv_indev_t * indev = lv_indev_create();

int x, y, z;
int btn1_count = 0;
static lv_obj_t * slider_label;
static lv_obj_t * user_list;

void touchscreen_begin();
void lvgl_config();
static void nextScreen(String userName);
void log_print(lv_log_level_t level, const char * buf);
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data);
static void event_handler_btn1(lv_event_t * e);
static void event_handler_btn2(lv_event_t * e);
static void slider_event_callback(lv_event_t * e);
static void event_handler(lv_event_t * e);
void lv_create_main_gui(void);
