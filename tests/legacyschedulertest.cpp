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
#include "legacyscheduler.h"
#include "plan.h"

/**
 *  @brief Load a schedulable Plan object
 *  @return A QSharedPointer to a Plan object
 *
 * If there are problems loading the file or the file is incorrect the
 * getValidPlanWorks test will fail
 */
QSharedPointer<Plan> getValidPlan() {
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
  QSharedPointer<Plan> plan(new Plan());
  plan->fromJsonObject(document.object());
  return plan;
}

/**
 *  @brief Load an unschedulable Plan object
 *  @return A QSharedPointer to a Plan object
 *
 * The returned Plan object can not be scheduled without conflicts
 * If there are problems loading the file or the file is incorrect the
 * getInvalidPlanWorks test will fail
 */
QSharedPointer<Plan> getInvalidPlan() {
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
  QSharedPointer<Plan> plan(new Plan());
  plan->fromJsonObject(document.object());
  return plan;
}

using namespace testing;

TEST(testDataTests, getValidPlanWorks) {
  QSharedPointer<Plan> plan = getValidPlan();
  ASSERT_NE(plan, nullptr);
}

TEST(testDataTests, getInvalidPlanWorks) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  ASSERT_NE(plan, nullptr);
}

TEST(legacySchedulerTests, constructorWorksWithNullptr) {
  LegacyScheduler(nullptr, nullptr);
}

TEST(legacySchedulerTests, startSchedulingReturnsTrueOnSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);
  ASSERT_TRUE(scheduler.startScheduling());
}

TEST(legacySchedulerTests, startSchedulingReturnsFalseOnUnschedulablePlan) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  LegacyScheduler scheduler(plan);
  ASSERT_FALSE(scheduler.startScheduling());
}

TEST(legacySchedulerTests, startSchedulingReturnsFalseOnNullptrPlan) {
  LegacyScheduler scheduler(nullptr);
  ASSERT_FALSE(scheduler.startScheduling());
}

TEST(legacySchedulerTests, startSchedulingAddsScheduleToPlanOnSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);
  ASSERT_TRUE(scheduler.startScheduling());
  bool unscheduledModule = false;
  for (auto module : plan->modules) {
    if (module->getOrigin() != "EIT") {
      for (auto week : plan->weeks) {
        for (auto day : week->getDays()) {
          for (auto timeslot : day->getTimeslots()) {
            for (auto scheduledModule : timeslot->getModules()) {
              if (module->getNumber() == scheduledModule->getNumber()) {
                goto continueModuleLoop;
              }
            }
          }
        }
      }
      // Module not scheduled -> break loop
      unscheduledModule = true;
      break;
    // Module scheduled -> check next module
    continueModuleLoop:
      continue;
    }
  }

  ASSERT_FALSE(unscheduledModule);
}

TEST(legacySchedulerTests,
     startSchedulingEmitsPointerFromConstructorOnSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  QSharedPointer<Plan> emittedPlan = nullptr;
  QObject::connect(
      &scheduler, &LegacyScheduler::finishedScheduling,
      [&emittedPlan](QSharedPointer<Plan> plan) { emittedPlan = plan; });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit && emittedPlan == nullptr) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_NE(emittedPlan, nullptr);
  ASSERT_EQ(emittedPlan, plan);
}

TEST(legacySchedulerTests,
     startSchedulingEmitsFailedSchedulingOnUnschedulablePlan) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  LegacyScheduler scheduler(plan);

  int failedSignalCount = 0;
  QObject::connect(&scheduler, &LegacyScheduler::failedScheduling,
                   [&failedSignalCount](QString) { failedSignalCount++; });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit && failedSignalCount == 0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(failedSignalCount, 1)
      << "Expected 1 failedScheduling, but got " << failedSignalCount;
}

TEST(legacySchedulerTests,
     startSchedulingDoesNotEmitSuccessOnUnschedulablePlan) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  LegacyScheduler scheduler(plan);

  bool emittedFinished = false;
  QObject::connect(&scheduler, &LegacyScheduler::finishedScheduling,
                   [&emittedFinished](auto) { emittedFinished = true; });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit && !emittedFinished) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_FALSE(emittedFinished);
}

TEST(legacySchedulerTests, startSchedulingDoesNotEmitFailedOnSchedulablePlan) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  bool emittedFailed = false;
  QObject::connect(&scheduler, &LegacyScheduler::failedScheduling,
                   [&emittedFailed](auto) { emittedFailed = true; });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit && !emittedFailed) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_FALSE(emittedFailed);
}

TEST(legacySchedulerTests, startSchedulingEmitsFullProgressAfterSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  double lastProgressUpdate = -1.0;
  QObject::connect(&scheduler, &LegacyScheduler::updateProgress,
                   [&lastProgressUpdate](double progress) {
                     lastProgressUpdate = progress;
                   });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while (QTime::currentTime() < limit && lastProgressUpdate != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(lastProgressUpdate, 1.0)
      << "Expected the last updateProgress signal to emit 1.0, but got "
      << lastProgressUpdate;
}
#endif
