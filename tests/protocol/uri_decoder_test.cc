#include "gtest/gtest.h"
#include "protocol/uri_decoder.hh"

TEST(URIDecoderTest, Decode) { // NOLINT
  std::string uri = "http%3A%2F%2Flocalhost%3A8080%2Fpath%3Fquery%3Dvalue";
  std::string replaced = URIDecoder::replacePercent(uri);
  EXPECT_EQ(replaced, "http://localhost:8080/path?query=value");
}

TEST(URIDecoderTest, ParseParam) { // NOLINT
  std::string uri = "http://localhost:8080/path?query=value&field=";
  auto params = URIDecoder::parseParam(uri);
  EXPECT_EQ(params.size(), 2);
  EXPECT_EQ(params["query"], "value");
  EXPECT_EQ(params["field"], "");
}