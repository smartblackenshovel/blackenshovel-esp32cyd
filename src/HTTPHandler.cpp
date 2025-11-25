#include "HTTPHandler.h"

HTTPHandler::HTTPHandler(const String& baseUrl) : baseUrl(baseUrl) {}

String HTTPHandler::buildUrl(const String& endpoint) const {
  if (baseUrl.endsWith("/") && endpoint.startsWith("/")) {
        return baseUrl + endpoint.substring(1);
    } else if (!baseUrl.endsWith("/") && !endpoint.startsWith("/")) {
        return baseUrl + "/" + endpoint;
    } else {
        return baseUrl + endpoint;
    }
}

String HTTPHandler::encodeUrl(const String& value) const {
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
      query += pair.first + "=" + encodeUrl(pair.second);
      first = false;
    }
  }
  return query;
}

HTTPResponse HTTPHandler::request(Method method, const String& endpoint,
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

  HTTPResponse httpResponse(code, response);
  return httpResponse;
}

HTTPResponse HTTPHandler::post(const String& endpoint, const String& payload,
                         const String& contentType) {
  return request(Method::POST, endpoint, payload, contentType);
}

HTTPResponse HTTPHandler::get(const String& endpoint,
                        const std::map<String, String>& params) {
  String fullEndpoint = buildQueryParams(endpoint, params);
  return request(Method::GET, fullEndpoint);
}

HTTPResponse HTTPHandler::patch(const String& endpoint, const String& payload,
                          const String& contentType) {
  return request(Method::PATCH, endpoint, payload, contentType);
}

HTTPResponse HTTPHandler::del(const String& endpoint) {
  return request(Method::DELETE, endpoint);
}
