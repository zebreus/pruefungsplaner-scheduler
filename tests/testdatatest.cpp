#ifndef TESTDATA_TEST_CPP
#define TESTDATA_TEST_CPP

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <QFile>
#include <QJsonDocument>
#include <QSharedPointer>
#include <QString>
#include "plan.h"
#include "testdatahelper.h"

using namespace testing;

TEST(testDataTests, getValidPlanWorks) {
  QSharedPointer<Plan> plan = getValidPlan();
  ASSERT_NE(plan, nullptr);
}

TEST(testDataTests, getInvalidPlanWorks) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  ASSERT_NE(plan, nullptr);
}

TEST(testDataTests, getValidJsonPlanWorks) {
  QJsonValue plan = getValidJsonPlan();
  ASSERT_TRUE(plan.isObject());
}

TEST(testDataTests, getInvalidJsonPlanWorks) {
  QJsonValue plan = getInvalidJsonPlan();
  ASSERT_TRUE(plan.isObject());
}

#endif
