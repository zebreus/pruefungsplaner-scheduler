#include "legacyscheduler.h"

LegacyScheduler::LegacyScheduler(QSharedPointer<Plan> plan, QObject* parent)
    : QObject(parent),
      workingDirectory(),
      csvHelper(workingDirectory.path()),
      originalPlan(plan) {
  prepareEnvironment();
}

bool LegacyScheduler::startScheduling() {
  emit updateProgress(0.0);
  if (!prepareEnvironment()) {
    return false;
  }
  emit updateProgress(0.2);
  if (!executeScheduler()) {
    return false;
  }
  emit updateProgress(0.9);
  if (!readResults()) {
    return false;
  }
  emit updateProgress(1.0);
  return true;
}

bool LegacyScheduler::prepareEnvironment() {
  if (!csvHelper.writePlan(originalPlan)) {
    emit failedScheduling("Failed to prepare environment");
    return false;
  } else {
    return true;
  }
}

bool LegacyScheduler::executeScheduler() {
  int exitCode = system(
      QString("echo -ne jn | ./SPA-algorithmus -p . -PP").toUtf8().constData());
  if (exitCode == 0) {
    return true;
  } else {
    emit failedScheduling("Failed to execute SPA-algorithmus");
    return false;
  }
}

bool LegacyScheduler::readResults() {
  // TODO decide if this should change the originalPlan and emit it, or just
  // emit a copy
  QSharedPointer<Plan> plan = csvHelper.readPlan();
  if (plan == nullptr) {
    emit failedScheduling("Failed to read plan");
    return false;
  }

  if (csvHelper.readSchedule(plan)) {
    emit finishedScheduling(plan);
    return true;
  } else {
    emit failedScheduling("Failed to read scheduling results");
    return false;
  }
}
