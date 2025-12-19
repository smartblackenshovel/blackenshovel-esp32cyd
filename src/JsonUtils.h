#include <ArduinoJson.h>
#include <vector>

std::vector<String> extractValues(JsonDocument json, const String& key) {
  std::vector<String> result;

  JsonArray arr = json.as<JsonArray>();
  for (JsonObject obj : arr) {
      result.push_back(String(obj[key].as<const char*>()));
  }

  return result;
}

void debug(const JsonDocument& doc) {
  String output;
  serializeJsonPretty(doc, output);
  Serial.println(output);
}
