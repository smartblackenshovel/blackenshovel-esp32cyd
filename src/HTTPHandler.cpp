#include "HTTPHandler.h"

HttpHandler::HttpHandler(const String& baseUrl) : baseUrl(baseUrl) {}

String HttpHandler::buildUrl(const String& endpoint) const {
  return baseUrl + endpoint;
}

String HttpHandler::urlencode(const String& value) const {
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

String HttpHandler::buildQueryParams(const String& endpoint,
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

String HttpHandler::request(const String& endpoint, const String& method,
                            const String& payload, const String& contentType) {
  String url = buildUrl(endpoint);
  httpClient.begin(url);

  int code = -1;

  if (method == "POST" || method == "PATCH" || method == "PUT" ||
      (method == "GET" && payload.length() > 0)) {
    httpClient.addHeader("Content-Type", contentType);
  }

  if (method == "GET") {
    if (payload.length() > 0) {
      code = httpClient.sendRequest("GET", payload);
    } else {
      code = httpClient.GET();
    }
  } else if (method == "POST") {
    code = httpClient.POST(payload);
  } else if (method == "PATCH") {
    code = httpClient.sendRequest("PATCH", payload);
  } else if (method == "PUT") {
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

String HttpHandler::post(const String& endpoint, const String& payload,
                         const String& contentType) {
  return request(endpoint, "POST", payload, contentType);
}

String HttpHandler::get(const String& endpoint,
                        const std::map<String, String>& params,
                        const String& payload, const String& contentType) {
  String fullEndpoint = buildQueryParams(endpoint, params);
  return request(fullEndpoint, "GET", payload, contentType);
}

String HttpHandler::patch(const String& endpoint, const String& payload,
                          const String& contentType) {
  return request(endpoint, "PATCH", payload, contentType);
}

String HttpHandler::del(const String& endpoint) {
  return request(endpoint, "DELETE");
}
