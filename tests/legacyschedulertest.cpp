#ifndef LEGACYSCHEDULER_TEST_CPP
#define LEGACYSCHEDULER_TEST_CPP

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <testdatahelper.h>

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QSharedPointer>
#include <QSignalSpy>
#include <QString>

#include "legacyscheduler.h"
#include "plan.h"

using namespace testing;

TEST(legacySchedulerTests, constructorWorksWithNullptr) {
  LegacyScheduler(nullptr, nullptr);
}

TEST(legacySchedulerTests, startSchedulingReturnsTrueOnSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);
  ASSERT_TRUE(scheduler.startScheduling());
}

TEST(legacySchedulerTests, startSchedulingReturnsTrueOnUnschedulablePlan) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  LegacyScheduler scheduler(plan);
  ASSERT_TRUE(scheduler.startScheduling());
}

TEST(legacySchedulerTests, startSchedulingReturnsFalseOnNullptrPlan) {
  LegacyScheduler scheduler(nullptr);
  ASSERT_FALSE(scheduler.startScheduling());
}

TEST(legacySchedulerTests, startSchedulingAddsScheduleToPlanOnSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  bool finished = false;
  bool failed = false;
  QCoreApplication::connect(&scheduler, &Scheduler::finishedScheduling, [&finished]() {
    finished = true;
  });
  QCoreApplication::connect(&scheduler, &Scheduler::failedScheduling, [&failed]() {
    failed = true;
  });
  ASSERT_TRUE(scheduler.startScheduling());

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && !finished && !failed) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_TRUE(finished);
  ASSERT_FALSE(failed);

  bool unscheduledModule = false;
  for(auto module : plan->getModules()) {
    if(module->getOrigin() != "EIT" && module->getActive() == true) {
      for(auto week : plan->getWeeks()) {
        for(auto day : week->getDays()) {
          for(auto timeslot : day->getTimeslots()) {
            for(auto scheduledModule : timeslot->getModules()) {
              if(module->getNumber() == scheduledModule->getNumber()) {
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

TEST(legacySchedulerTests, startSchedulingEmitsPointerFromConstructorOnSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  QSharedPointer<Plan> emittedPlan = nullptr;
  QObject::connect(&scheduler, &Scheduler::finishedScheduling, [&emittedPlan](QSharedPointer<Plan> plan) {
    emittedPlan = plan;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && emittedPlan == nullptr) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_NE(emittedPlan, nullptr);
  ASSERT_EQ(emittedPlan, plan);
}

TEST(legacySchedulerTests, startSchedulingEmitsFailedSchedulingOnUnschedulablePlan) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  LegacyScheduler scheduler(plan);

  int failedSignalCount = 0;
  QObject::connect(&scheduler, &Scheduler::failedScheduling, [&failedSignalCount](QString) {
    failedSignalCount++;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && failedSignalCount == 0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(failedSignalCount, 1) << "Expected 1 failedScheduling, but got " << failedSignalCount;
}

TEST(legacySchedulerTests, startSchedulingDoesNotEmitSuccessOnUnschedulablePlan) {
  QSharedPointer<Plan> plan = getInvalidPlan();
  LegacyScheduler scheduler(plan);

  bool emittedFinished = false;
  QObject::connect(&scheduler, &Scheduler::finishedScheduling, [&emittedFinished](auto) {
    emittedFinished = true;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && !emittedFinished) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_FALSE(emittedFinished);
}

TEST(legacySchedulerTests, startSchedulingDoesNotEmitFailedOnSchedulablePlan) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  bool emittedFailed = false;
  QObject::connect(&scheduler, &Scheduler::failedScheduling, [&emittedFailed](auto) {
    emittedFailed = true;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && !emittedFailed) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_FALSE(emittedFailed);
}

TEST(legacySchedulerTests, startSchedulingDoesEmitFailOnPlanWithInvalidModules) {
  QSharedPointer<Plan> plan = getValidPlan();
  plan->getModules()[0]->setGroups(QList<Group*>());
  plan->getModules()[0]->setActive(true);
  LegacyScheduler scheduler(plan);

  bool emittedFailed = false;
  QObject::connect(&scheduler, &Scheduler::failedScheduling, [&emittedFailed](auto) {
    emittedFailed = true;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && !emittedFailed) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_TRUE(emittedFailed);
}

TEST(legacySchedulerTests, startSchedulingDoesNotEmitFailOnPlanWithInactiveInvalidModules) {
  QSharedPointer<Plan> plan = getValidPlan();
  plan->getModules()[0]->setGroups(QList<Group*>());
  plan->getModules()[0]->setActive(false);
  LegacyScheduler scheduler(plan);

  bool emittedFailed = false;
  QObject::connect(&scheduler, &Scheduler::failedScheduling, [&emittedFailed](auto) {
    emittedFailed = true;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && !emittedFailed) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_FALSE(emittedFailed);
}

TEST(legacySchedulerTests, startSchedulingEmitsFullProgressAfterSuccess) {
  QSharedPointer<Plan> plan = getValidPlan();
  LegacyScheduler scheduler(plan);

  double lastProgressUpdate = -1.0;
  QObject::connect(&scheduler, &Scheduler::updateProgress, [&lastProgressUpdate](double progress) {
    lastProgressUpdate = progress;
  });

  scheduler.startScheduling();

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && lastProgressUpdate != 1.0) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  ASSERT_EQ(lastProgressUpdate, 1.0) << "Expected the last updateProgress signal to emit 1.0, but got " << lastProgressUpdate;
}

TEST(legacySchedulerTests, startSchedulingRemovesOldScheduledModulesFromPlan) {
  QSharedPointer<Plan> plan = getValidPlan();
  // Preschedule a module twice, so at least one of them gets removed
  ASSERT_GE(plan->getWeeks().size(), 1);
  ASSERT_GE(plan->getWeeks()[0]->getDays().size(), 1);
  ASSERT_GE(plan->getWeeks()[0]->getDays()[0]->getTimeslots().size(), 2);
  ASSERT_GE(plan->getModules().size(), 1);
  plan->getWeeks()[0]->getDays()[0]->getTimeslots()[0]->addModule(plan->getModules()[0]);
  plan->getWeeks()[0]->getDays()[0]->getTimeslots()[1]->addModule(plan->getModules()[0]);

  LegacyScheduler scheduler(plan);
  bool emittedFinished = false;
  QObject::connect(&scheduler, &Scheduler::finishedScheduling, [&emittedFinished](auto) {
    emittedFinished = true;
  });

  ASSERT_TRUE(scheduler.startScheduling());

  QTime limit = QTime::currentTime().addMSecs(500);
  while(QTime::currentTime() < limit && !emittedFinished) {
    QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
  }

  EXPECT_FALSE(plan->getWeeks()[0]->getDays()[0]->getTimeslots()[0]->getModules().contains(plan->getModules()[0]) &&
               plan->getWeeks()[0]->getDays()[0]->getTimeslots()[1]->getModules().contains(plan->getModules()[0]));
}
#endif
