#include <HTTPClient.h>
#include <SD.h>
#include <WiFiClient.h>
#include "HTTPResponse.h"

#include <map>

class HTTPHandler {
 public:
  explicit HTTPHandler(const String& baseUrl);

  HTTPResponse post(const String& endpoint, const String& payload,
              const String& contentType = "application/json");
  HTTPResponse get(const String& endpoint,
             const std::map<String, String>& params = {});
  HTTPResponse patch(const String& endpoint, const String& payload,
               const String& contentType = "application/json");
  HTTPResponse del(const String& endpoint);

 private:
  String baseUrl;
  HTTPClient httpClient;
  enum Method { GET, POST, PATCH, PUT, DELETE };
  HTTPResponse request(Method method, const String& endpoint,
                 const String& payload = "", const String& contentType = "");
  String buildUrl(const String& endpoint) const;
  String encodeUrl(const String& value) const;
  String buildQueryParams(const String& endpoint,
                          const std::map<String, String>& params) const;
};
