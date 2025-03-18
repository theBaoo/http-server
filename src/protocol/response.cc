#include "protocol/response.hh"

#include <string>

#include "protocol/parser.hh"

auto ResponseBuilder::build() -> std::string {
  std::string response;
  response +=
      HTTPV + " " + std::to_string(static_cast<int>(status_code_)) + " " + status_message_ + CRLF;
  for (const auto& [field, value] : headers_) {
    response += field;
    response += ": ";
    response += value;
    response += CRLF;
  }
  response += CRLF;
  response += body_;
  return response;
}
void ResponseBuilder::setStatusCode(StatusCode status_code) {
  status_code_ = status_code;
}
void ResponseBuilder::setStatusMessage(const std::string& status_message) {
  status_message_ = status_message;
}
void ResponseBuilder::setBody(const std::string& body) {
  body_ = body;
}
void ResponseBuilder::addHeader(const std::string& field, const std::string& value) {
  headers_[field] = value;
}