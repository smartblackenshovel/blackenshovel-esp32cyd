#pragma once
#include <ArduinoJson.h>
#include <vector>

std::vector<String> extractValues(JsonDocument json, const String& key);

void debug(const JsonDocument& doc);
