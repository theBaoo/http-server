#include "gtest/gtest.h"
#include "protocol/uri_decoder.hh"

TEST(URIDecoderTest, Decode) { // NOLINT
  std::string uri = "http%3A%2F%2Flocalhost%3A8080%2Fpath%3Fquery%3Dvalue";
  std::string decoded = URIDecoder::decode(uri);
  EXPECT_EQ(decoded, "http://localhost:8080/path?query=value");
}