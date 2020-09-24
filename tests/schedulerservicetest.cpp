#ifndef LEGACYSCHEDULER_TEST_CPP
#define LEGACYSCHEDULER_TEST_CPP

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QString>
#include "plan.h"
#include "schedulerservice.h"

/**
 *  @brief Load a schedulable plan as a QJsonValue
 *  @return A QJsonValue containing the plan
 *
 * If there are problems loading the file or the file is incorrect the
 * getValidJsonPlanWorks test will fail
 */
QJsonObject getValidJsonPlan() {
  QFile file("./tests/data/plan.json");
  EXPECT_TRUE(file.exists())
      << "Example plan json file (" << file.fileName().constData()
      << ") does not exist.";
  EXPECT_TRUE(file.open(QFile::ReadOnly | QFile::Text))
      << "Cannot open example plan json file (" << file.fileName().constData()
      << ").";
  QString jsonString = QTextStream(&file).readAll();
  QJsonDocument document = QJsonDocument::fromJson(jsonString.toUtf8());
  EXPECT_TRUE(document.isObject()) << "Json file does not contain an object.";
  return document.object();
}

/**
 *  @brief Load an unschedulable Plan as a QJsonValue
 *  @return A QJsonValue containing the plan
 *
 * The returned Plan can not be scheduled without conflicts
 * If there are problems loading the file or the file is incorrect the
 * getInvalidJsonPlanWorks test will fail
 */
QJsonObject getInvalidJsonPlan() {
  // TODO create invalid plan json
  QFile file("./tests/data/unschedulableplan.json");
  EXPECT_TRUE(file.exists())
      << "Unschedulable plan json file (" << file.fileName().constData()
      << ") does not exist.";
  EXPECT_TRUE(file.open(QFile::ReadOnly | QFile::Text))
      << "Cannot open unschedulable plan json file ("
      << file.fileName().constData() << ").";
  QString jsonString = QTextStream(&file).readAll();
  QJsonDocument document = QJsonDocument::fromJson(jsonString.toUtf8());
  EXPECT_TRUE(document.isObject())
      << "Unschedulable Plan json file does not contain an object.";
  return document.object();
}

using namespace testing;

TEST(testDataTests, getValidJsonPlanWorks) {
  QJsonValue plan = getValidJsonPlan();
  ASSERT_TRUE(plan.isObject());
}

TEST(testDataTests, getInvalidJsonPlanWorks) {
  QJsonValue plan = getInvalidJsonPlan();
  ASSERT_TRUE(plan.isObject());
}

TEST(schedulerServiceTests, getResultAfterSchedulingReturnsPlan) {
  QJsonObject jsonPlan = getValidJsonPlan();
  SchedulerService schedulerService;
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit &&
         schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
  ASSERT_TRUE(schedulerService.getResult().isObject());
}

TEST(schedulerServiceTests, getResultWithoutSchedulingReturnsUndefined) {
  SchedulerService schedulerService;
  ASSERT_TRUE(schedulerService.getResult().isUndefined());
}

TEST(schedulerServiceTests, getResultWithUnschedulablePlanReturnsErrormessage) {
  QJsonObject jsonPlan = getInvalidJsonPlan();
  SchedulerService schedulerService;
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit &&
         schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
  ASSERT_TRUE(schedulerService.getResult().isString());
}

TEST(schedulerServiceTests, progessAfterConstructionIsZero) {
  SchedulerService schedulerService;
  ASSERT_EQ(schedulerService.getProgress(), 0.0);
}

TEST(schedulerServiceTests, progessAfterSchedulingIsOne) {
  QJsonObject jsonPlan = getValidJsonPlan();
  SchedulerService schedulerService;
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit &&
         schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
}

TEST(schedulerServiceTests, progessAfterFailedSchedulingIsOne) {
  QJsonObject jsonPlan = getInvalidJsonPlan();
  SchedulerService schedulerService;
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit &&
         schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
}

TEST(schedulerServiceTests, secondSchedulingAttemptFails) {
  QJsonObject jsonPlan = getInvalidJsonPlan();
  SchedulerService schedulerService;
  ASSERT_TRUE(schedulerService.startScheduling(jsonPlan));
  ASSERT_FALSE(schedulerService.startScheduling(jsonPlan));
}

#endif
