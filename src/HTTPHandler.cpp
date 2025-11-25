#include "HTTPHandler.h"

HTTPHandler::HTTPHandler(const String& baseUrl) : baseUrl(baseUrl) {}

String HTTPHandler::buildUrl(const String& endpoint) const {
  return baseUrl + endpoint;
}

String HTTPHandler::urlencode(const String& value) const {
  String encoded = "";
  char c;
  char buf[4];
  for (size_t i = 0; i < value.length(); i++) {
    c = value.charAt(i);
    if (isalnum(c)) {
      encoded += c;
    } else {
      sprintf(buf, "%%%02X", c);
      encoded += buf;
    }
  }
  return encoded;
}

String HTTPHandler::buildQueryParams(const String& endpoint,
                                     const std::map<String, String>& params) const {
  String query = endpoint;
  if (!params.empty()) {
    query += "?";
    bool first = true;
    for (const auto& pair : params) {
      if (!first) query += "&";
      query += pair.first + "=" + urlencode(pair.second);
      first = false;
    }
  }
  return query;
}

String HTTPHandler::request(const String& endpoint, const String& method,
                            const String& payload, const String& contentType) {
  String url = buildUrl(endpoint);
  httpClient.begin(url);

  int code = -1;

  if (method == "GET") {
    if (payload.length() > 0) {
      code = httpClient.sendRequest("GET", payload);
    } else {
      code = httpClient.GET();
    }
  } else if (method == "POST") {
    httpClient.addHeader("Content-Type", contentType);
    code = httpClient.POST(payload);
  } else if (method == "PATCH") {
    httpClient.addHeader("Content-Type", contentType);
    code = httpClient.sendRequest("PATCH", payload);
  } else if (method == "PUT") {
    httpClient.addHeader("Content-Type", contentType);
    code = httpClient.PUT(payload);
  } else if (method == "DELETE") {
    code = httpClient.sendRequest("DELETE");
  } else {
    Serial.printf("Unsupported HTTP method: %s\n", method.c_str());
    httpClient.end();
    return "";
  }

  String response;
  if (code > 0) {
    Serial.printf("%s %s -> Code: %d\n", method.c_str(), url.c_str(), code);
    response = httpClient.getString();
  } else {
    Serial.printf("HTTP %s failed: %s\n", method.c_str(),
                  httpClient.errorToString(code).c_str());
  }

  httpClient.end();
  return response;
}

String HTTPHandler::post(const String& endpoint, const String& payload,
                         const String& contentType) {
  return request(endpoint, "POST", payload, contentType);
}

String HTTPHandler::get(const String& endpoint,
                        const std::map<String, String>& params,
                        const String& payload, const String& contentType) {
  String fullEndpoint = buildQueryParams(endpoint, params);
  return request(fullEndpoint, "GET", payload, contentType);
}

String HTTPHandler::patch(const String& endpoint, const String& payload,
                          const String& contentType) {
  return request(endpoint, "PATCH", payload, contentType);
}

String HTTPHandler::del(const String& endpoint) {
  return request(endpoint, "DELETE");
}
