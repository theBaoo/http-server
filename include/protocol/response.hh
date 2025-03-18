#ifndef PROTOCOL_RESPONSE_H
#define PROTOCOL_RESPONSE_H

#include <map>
#include <string>

const std::string HTTPV = "HTTP/1.1";
const std::string CRLF = "\r\n";

enum class StatusCode {
  OK = 200,
  BAD_REQUEST = 400,
  NOT_FOUND = 404,
  INTERNAL_SERVER_ERROR = 500,
};

class ResponseBuilder {
 private:
  StatusCode status_code_;
  std::string status_message_;
  std::string body_;
  std::map<std::string, std::string> headers_;

 public:
  auto build() -> std::string;
  void setStatusCode(StatusCode status_code);
  void setStatusMessage(const std::string& status_message);
  void setBody(const std::string& body);
  void addHeader(const std::string& field, const std::string& value);
};

#endif // PROTOCOL_RESPONSE_H