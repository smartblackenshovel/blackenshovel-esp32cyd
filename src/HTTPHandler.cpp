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

String HTTPHandler::request(const String& endpoint, Method method,
                            const String& payload, const String& contentType) {
  String url = buildUrl(endpoint);
  httpClient.begin(url);

  int code = -1;

  switch (method) {
    case GET:
      code = httpClient.GET();
      break;
    case POST:
      httpClient.addHeader("Content-Type", contentType);
      code = httpClient.POST(payload);
      break;
    case PATCH:
      httpClient.addHeader("Content-Type", contentType);
      code = httpClient.sendRequest("PATCH", payload);
      break;
    case PUT:
      httpClient.addHeader("Content-Type", contentType);
      code = httpClient.PUT(payload);
      break;
    case DELETE:
      code = httpClient.sendRequest("DELETE");
      break;
  }

  String response;
  if (code > 0) {
    response = httpClient.getString();
  }

  httpClient.end();
  return response;
}

String HTTPHandler::post(const String& endpoint, const String& payload,
                         const String& contentType) {
  return request(Method::POST, endpoint, payload, contentType);
}

String HTTPHandler::get(const String& endpoint,
                        const std::map<String, String>& params) {
  String fullEndpoint = buildQueryParams(endpoint, params);
  return request(Method::GET, fullEndpoint);
}

String HTTPHandler::patch(const String& endpoint, const String& payload,
                          const String& contentType) {
  return request(Method::PATCH, endpoint, payload, contentType);
}

String HTTPHandler::del(const String& endpoint) {
  return request(Method::DELETE, endpoint);
}
