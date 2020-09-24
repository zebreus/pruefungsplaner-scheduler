#ifndef TESTDATA_TEST_CPP
#define TESTDATA_TEST_CPP

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <QFile>
#include <QJsonDocument>
#include <QSharedPointer>
#include <QString>
#include <QTemporaryDir>
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

TEST(testDataTests, prepareScheduledDirectoryWorks) {
  QTemporaryDir directory;
  ASSERT_TRUE(prepareScheduledDirectory(directory.path()));
  ASSERT_TRUE(
      QFile(directory.path() + "/pruef-intervalle.csv").open(QFile::ReadWrite));
  ASSERT_TRUE(
      QFile(directory.path() + "/pruefungen.csv").open(QFile::ReadWrite));
  ASSERT_TRUE(
      QFile(directory.path() + "/zuege-pruef.csv").open(QFile::ReadWrite));
  ASSERT_TRUE(QFile(directory.path() + "/zuege-pruef-pref2.csv")
                  .open(QFile::ReadWrite));
  ASSERT_TRUE(QFile(directory.path() + "/SPA-ERGEBNIS-PP/SPA-planung-pruef.csv")
                  .open(QFile::ReadWrite));
  ASSERT_TRUE(QFile(directory.path() + "/SPA-ERGEBNIS-PP/SPA-zuege-pruef.csv")
                  .open(QFile::ReadWrite));
}

#endif
