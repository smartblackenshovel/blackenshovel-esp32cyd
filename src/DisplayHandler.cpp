#include "DisplayHandler.h"
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include <FileFetcher.h>
#include <WiFiClientSecure.h>

namespace DisplayImageHandler {

    static PNG png;
    static fs::File file;
    static TFT_eSPI* tft = nullptr;
    static WiFiClientSecure* securedClient = nullptr;
    static FileFetcher* fileFetcher = nullptr;

    int PNGDraw(PNGDRAW *pDraw)
    {
        uint16_t usPixels[320];

        png.getLineAsRGB565(pDraw, usPixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
        tft->pushImage(0, pDraw->y, pDraw->iWidth, 1, usPixels);

        return 1;
    }

    void* myOpen(const char* filename, int32_t* size) {
        file = SPIFFS.open(filename);
        *size = file.size();
        return &file;
    }

    void myClose(void* handle) {
        if (file) file.close();
    }

    int32_t myRead(PNGFILE* handle, uint8_t* buffer, int32_t length) {
        if (!file) return 0;
        return file.read(buffer, length);
    }

    int32_t mySeek(PNGFILE* handle, int32_t position) {
        if (!file) return 0;
        return file.seek(position);
    }

    void init(TFT_eSPI& display, WiFiClientSecure& client, FileFetcher& fetcher) {
        bool spiffsInitSuccess = SPIFFS.begin(false) || SPIFFS.begin(true);
        if (!spiffsInitSuccess)
        {
            Serial.println("SPIFFS initialisation failed!");
            while (1)
                yield(); // Stay here twiddling thumbs waiting
        }
        tft = &display;
        securedClient = &client;
        fileFetcher = &fetcher;

        tft->init();
        tft->setRotation(1);
        tft->fillScreen(TFT_BLACK);
    }

    int displayImage(char* imageFileUri) {
        if (!tft) return -1;

        tft->fillScreen(TFT_BLACK);

        int rc = png.open(
            imageFileUri,
            myOpen,
            myClose,
            myRead,
            mySeek,
            PNGDraw
        );

        if (rc == PNG_SUCCESS) {
            rc = png.decode(nullptr, 0);
            png.close();
        } else {
            Serial.print("PNG error: "); Serial.println(rc);
        }

        return rc;
    }

    int getImage(char* imageUrl) {
        if (!fileFetcher) return -1;

        if (SPIFFS.exists(IMAGE_NAME)) {
            SPIFFS.remove(IMAGE_NAME);
        }

        fs::File f = SPIFFS.open(IMAGE_NAME, "w+");
        if (!f) return -1;

        bool gotImage = fileFetcher->getFile(imageUrl, &f);
        f.close();
        return gotImage;
    }

}
