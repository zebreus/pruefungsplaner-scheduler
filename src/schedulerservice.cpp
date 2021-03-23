#include "schedulerservice.h"

SchedulerService::SchedulerService(const QSharedPointer<Configuration> configuration, QObject* parent)
    : QObject(parent), configuration(configuration), scheduler(nullptr), progress(0.0), result(QJsonValue::Undefined) {}

bool SchedulerService::startScheduling(QJsonObject plan) {
  if(scheduler != nullptr) {
    return false;
  }

  QSharedPointer<Plan> planPointer(new Plan());
  planPointer->fromJsonObject(plan);

  QString schedulingAlgorithm = configuration->getDefaultSchedulingAlgorithm();
  if(!customAlgorithm.isEmpty()) {
    schedulingAlgorithm = customAlgorithm;
  }

  if(schedulingAlgorithm.startsWith("legacy")) {
    LegacyScheduler::SchedulingMode legacySchedulerMode;
    if(schedulingAlgorithm == "legacy-fast") {
      legacySchedulerMode = LegacyScheduler::Fast;
    } else {
      legacySchedulerMode = LegacyScheduler::Good;
    }
    scheduler.reset(new LegacyScheduler(planPointer,
                                        configuration->getLegacySchedulerAlgorithmBinary(),
                                        configuration->getLegacySchedulerPrintLog(),
                                        legacySchedulerMode));
  } else {
    return false;
  }

  QObject::connect(scheduler.data(), &Scheduler::updateProgress, [this](double updatedProgress) {
    progress = updatedProgress;
  });
  QObject::connect(scheduler.data(), &Scheduler::updateProgress, this, &SchedulerService::updateProgress);
  QObject::connect(scheduler.data(), &Scheduler::failedScheduling, this, &SchedulerService::failedScheduling);
  QObject::connect(scheduler.data(), &Scheduler::emitWarning, this, &SchedulerService::emitWarning);
  QObject::connect(scheduler.data(), &Scheduler::failedScheduling, [this](QString errorMessage) {
    result = errorMessage;
    progress = 1.0;
  });
  QObject::connect(scheduler.data(), &Scheduler::finishedScheduling, [this](QSharedPointer<Plan> scheduledPlan) {
    result = scheduledPlan->toJsonObject();
    emit finishedScheduling(result.toObject());
    progress = 1.0;
  });

  scheduler->startScheduling();

  return true;
}

bool SchedulerService::setSchedulingAlgorithm(QString mode) {
  if(mode == "legacy-fast" || mode == "legacy-good") {
    customAlgorithm = mode;
    return true;
  }

  return false;
}

bool SchedulerService::stopScheduling() {
  if(scheduler.isNull()) {
    return false;
  }
  scheduler->stopScheduling();
  return true;
}

double SchedulerService::getProgress() {
  return progress;
}

QJsonValue SchedulerService::getResult() {
  return result;
}
