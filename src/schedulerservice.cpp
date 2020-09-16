#include "schedulerservice.h"

SchedulerService::SchedulerService(QObject* parent) : QObject(parent) {
  // TODO implement
}

bool SchedulerService::startScheduling(QJsonValue plan) {
  // TODO implement
  return false;
}

double SchedulerService::getProgress() {
  // TODO implement
  return 0.0;
}

QJsonValue SchedulerService::getResult() {
  // TODO implement
  return QJsonValue(QJsonValue::Undefined);
}
