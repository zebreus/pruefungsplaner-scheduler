#ifndef TEST_CPP
#define TEST_CPP

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(mytests, test) {
  EXPECT_EQ(1, 1);
  ASSERT_THAT(0, Eq(0));
}

#endif  // TEST_CPP
