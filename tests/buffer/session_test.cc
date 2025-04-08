#include <gtest/gtest.h>
#include "buffer/session.hh"

TEST(SessionTest, ToString) { // NOLINT
  Session session;
  session.from_string("session_id:username, 2023-10-01 12:00:00");
  EXPECT_EQ(session.to_string(), "session_id:username, 2023-10-01 12:00:00");
}

TEST(SessionTest, FromStringValidInput) { // NOLINT
  Session session;
  session.from_string("abc123:john_doe, 2023-10-01 12:00:00");
  EXPECT_EQ(session.to_string(), "abc123:john_doe, 2023-10-01 12:00:00");
}

TEST(SessionTest, FromStringInvalidFormat) { // NOLINT
  Session session;
  EXPECT_THROW(session.from_string("invalid_format_string"), std::invalid_argument); // NOLINT
}

TEST(SessionTest, FromStringInvalidTime) { // NOLINT
  Session session;
  EXPECT_THROW(session.from_string("abc123:john_doe, invalid_time"), std::invalid_argument); // NOLINT
}

TEST(SessionTest, RoundTripConversion) { // NOLINT
  Session session;
  session.from_string("session_id:username, 2023-10-01 12:00:00");
  std::string serialized = session.to_string();

  Session new_session;
  new_session.from_string(serialized);

  EXPECT_EQ(new_session.to_string(), "session_id:username, 2023-10-01 12:00:00");
}