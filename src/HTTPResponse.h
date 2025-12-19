#include <Arduino.h>
#include <ArduinoJson.h>

class HTTPResponse {
public:
    HTTPResponse(int statusCode, const String& content)
        : statusCode(statusCode), content(content) { }

    int getStatusCode() const { return statusCode; }

    bool isSuccess() const { return statusCode >= 200 && statusCode <= 299; }

    bool isValid() const { return statusCode > 0; }

    const String& getContent() const { return content; }

    JsonDocument getContentAsJson() const {
        JsonDocument doc;
        deserializeJson(doc, content);
        return doc;
    }

    bool hasContent() const { return content.length() > 0; }

private:
    int statusCode;
    String content;
};
