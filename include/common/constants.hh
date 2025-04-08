#ifndef COMMON_CONSTANTS_H
#define COMMON_CONSTANTS_H

#include <cstddef>
#include <string>

const size_t MAX_BUFFER_SIZE = 1024;
const size_t MAX_CONNECTIONS = 10;

const std::string ADDR     = "0.0.0.0";
const std::string PORT     = "80";
const std::string SSL_PORT = "4433";

// 或在CMake中传递
// add_defination or target_compile_definitions
#ifdef __APPLE__
const std::string root = "/Users/thebao/codes/cpp/http-server";
#elif defined(__linux__)
const std::string root = "/root/net-xjtu/http-server";
#endif

const std::string stt  = root + "/static";
const std::string cert = root + "/certificate/cert.pem";
const std::string key  = root + "/certificate/key.pem";

#endif // COMMON_CONSTANTS_H