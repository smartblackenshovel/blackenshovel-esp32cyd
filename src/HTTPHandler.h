#include <HTTPClient.h>
#include <SD.h>
#include <WiFiClient.h>

#include <map>

class HTTPHandler {
 public:
  explicit HTTPHandler(const String& baseUrl);

  // bool getToFile(const String& endpoint, const String& outputStream);
  String post(const String& endpoint, const String& payload,
              const String& contentType = "application/json");
  String get(const String& endpoint,
             const std::map<String, String>& params = {});
  String patch(const String& endpoint, const String& payload,
               const String& contentType = "application/json");
  String del(const String& endpoint);

 private:
  String baseUrl;
  HTTPClient httpClient;
  enum Method { GET, POST, PATCH, PUT, DELETE };
  String request(Method method, const String& endpoint,
                 const String& payload = "", const String& contentType = "");
  String buildUrl(const String& endpoint) const;
  String urlencode(const String& value) const;
  String buildQueryParams(const String& endpoint,
                          const std::map<String, String>& params) const;
};
