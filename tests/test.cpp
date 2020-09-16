#ifndef TEST_H
#define TEST_H

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

using namespace testing;

TEST(mytests, test) {
  EXPECT_EQ(1, 1);
  ASSERT_THAT(0, Eq(0));
}

#endif  // TST_TEST1_H
