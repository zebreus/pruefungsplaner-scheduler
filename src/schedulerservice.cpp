#include "schedulerservice.h"

SchedulerService::SchedulerService(QObject* parent)
    : QObject(parent),
      scheduler(nullptr),
      progress(0.0),
      result(QJsonValue::Undefined) {}

bool SchedulerService::startScheduling(QJsonObject plan) {
  if (scheduler != nullptr) {
    return false;
  }

  QSharedPointer<Plan> planPointer(new Plan());
  planPointer->fromJsonObject(plan);

  scheduler.reset(new LegacyScheduler(planPointer));
  QObject::connect(
      (LegacyScheduler*)scheduler.data(), &LegacyScheduler::updateProgress,
      [this](double updatedProgress) { progress = updatedProgress; });
  QObject::connect((LegacyScheduler*)scheduler.data(),
                   &LegacyScheduler::failedScheduling,
                   [this](QString errorMessage) {
                     result = errorMessage;
                     progress = 1.0;
                   });
  QObject::connect((LegacyScheduler*)scheduler.data(),
                   &LegacyScheduler::finishedScheduling,
                   [this](QSharedPointer<Plan> scheduledPlan) {
                     result = scheduledPlan->toJsonObject();
                     progress = 1.0;
                   });

  scheduler->startScheduling();

  return true;
}

double SchedulerService::getProgress() {
  return progress;
}

QJsonValue SchedulerService::getResult() {
  return result;
}
