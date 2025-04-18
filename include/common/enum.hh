#ifndef COMMON_ENUM_HH
#define COMMON_ENUM_HH

enum class HTTPMethod {
  GET,
  POST,
  PUT,
  DELETE,
  HEAD,
  OPTIONS,
  TRACE,
  CONNECT,
};

enum class ContentType {
  FORM,
  JSON,
  PLAIN,
  XML,
  MULTIPART,
};

enum class StatusCode {
  CONTINUED             = 100,
  OK                    = 200,
  FOUND                 = 302,
  NOT_MODIFIED          = 304,
  BAD_REQUEST           = 400,
  UNAUTHORIZED          = 401,
  NOT_FOUND             = 404,
  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED       = 501,
};

#endif // COMMON_ENUM_HH