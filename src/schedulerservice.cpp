#include "schedulerservice.h"

SchedulerService::SchedulerService(
    const QSharedPointer<Configuration> configuration,
    QObject* parent)
    : QObject(parent),
      configuration(configuration),
      scheduler(nullptr),
      progress(0.0),
      result(QJsonValue::Undefined) {}

bool SchedulerService::startScheduling(QJsonObject plan) {
  if (scheduler != nullptr) {
    return false;
  }

  QSharedPointer<Plan> planPointer(new Plan());
  planPointer->fromJsonObject(plan);

  QString schedulingAlgorithm = configuration->getDefaultSchedulingAlgorithm();
  if (schedulingAlgorithm.startsWith("legacy")) {
    LegacyScheduler::SchedulingMode legacySchedulerMode;
    if (schedulingAlgorithm == "legacy-fast") {
      legacySchedulerMode = LegacyScheduler::Fast;
    } else {
      legacySchedulerMode = LegacyScheduler::Good;
    }
    scheduler.reset(new LegacyScheduler(
        planPointer, configuration->getLegacySchedulerAlgorithmBinary(),
        configuration->getLegacySchedulerPrintLog(), legacySchedulerMode));
  } else {
    return false;
  }

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
