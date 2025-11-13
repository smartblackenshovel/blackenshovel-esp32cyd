#include <FS.h>
#include "SPIFFS.h"
#include <WiFiClientSecure.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include <FileFetcher.h>

#define IMAGE_NAME "/map.png"

namespace DisplayImageHandler {

    void init(TFT_eSPI& display, WiFiClientSecure& client, FileFetcher& fetcher);
    int displayImage(char* imageFileUri);
    int getImage(char* imageUrl);

}
