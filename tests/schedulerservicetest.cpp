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

#include "configuration.h"
#include "plan.h"
#include "schedulerservice.h"
#include "testdatahelper.h"

using namespace testing;

QSharedPointer<Configuration> getDefaultConfiguration() {
  QList<QString> arguments{"pruefungsplaner-scheduler-tests", "--storage", "/tmp", "--legacy-scheduler-binary", "./SPA-algorithmus"};
  QSharedPointer<Configuration> configuration(new Configuration(arguments));
  return configuration;
}

TEST(schedulerServiceTests, getResultAfterSchedulingReturnsPlan) {
  QJsonObject jsonPlan = getValidJsonPlan();
  SchedulerService schedulerService(getDefaultConfiguration());
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
  ASSERT_TRUE(schedulerService.getResult().isObject());
}

TEST(schedulerServiceTests, getResultWithoutSchedulingReturnsUndefined) {
  SchedulerService schedulerService(getDefaultConfiguration());
  ASSERT_TRUE(schedulerService.getResult().isUndefined());
}

TEST(schedulerServiceTests, getResultWithUnschedulablePlanReturnsErrormessage) {
  QJsonObject jsonPlan = getInvalidJsonPlan();
  SchedulerService schedulerService(getDefaultConfiguration());
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
  ASSERT_TRUE(schedulerService.getResult().isString());
}

TEST(schedulerServiceTests, progessAfterConstructionIsZero) {
  SchedulerService schedulerService(getDefaultConfiguration());
  ASSERT_EQ(schedulerService.getProgress(), 0.0);
}

TEST(schedulerServiceTests, progessAfterSchedulingIsOne) {
  QJsonObject jsonPlan = getValidJsonPlan();
  SchedulerService schedulerService(getDefaultConfiguration());
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
}

TEST(schedulerServiceTests, progessAfterFailedSchedulingIsOne) {
  QJsonObject jsonPlan = getInvalidJsonPlan();
  SchedulerService schedulerService(getDefaultConfiguration());
  schedulerService.startScheduling(jsonPlan);

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && schedulerService.getProgress() != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(schedulerService.getProgress(), 1.0);
}

TEST(schedulerServiceTests, secondSchedulingAttemptFails) {
  QJsonObject jsonPlan = getInvalidJsonPlan();
  SchedulerService schedulerService(getDefaultConfiguration());
  ASSERT_TRUE(schedulerService.startScheduling(jsonPlan));
  ASSERT_FALSE(schedulerService.startScheduling(jsonPlan));
}

#endif
