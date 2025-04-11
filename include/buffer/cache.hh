#ifndef BUFFER_CACHE_HH
#define BUFFER_CACHE_HH

#include <string>

auto getEtag(const std::string& uri) -> std::string;
auto getLastModify(const std::string& uri) -> std::string;

#endif // BUFFER_CACHE_HH