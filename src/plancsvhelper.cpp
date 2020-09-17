#include "plancsvhelper.h"

PlanCsvHelper::PlanCsvHelper(QString path) : basePath(path) {
  initializeFilePaths();
}

PlanCsvHelper::PlanCsvHelper() : temporaryDirectory(new QTemporaryDir()) {
  basePath = temporaryDirectory->path();
  initializeFilePaths();
}

QSharedPointer<Plan> PlanCsvHelper::readPlan() {
  // TODO implement
  return nullptr;
}

bool PlanCsvHelper::writePlan(QSharedPointer<Plan> plan) {
  // TODO implement
  return false;
}

bool PlanCsvHelper::isWritten() {
  return examsIntervalsFile.exists() && examsFile.exists() &&
         groupsExamsFile.exists() && groupsExamsPrefFile.exists();
}

bool PlanCsvHelper::isScheduled() {
  return planningExamsResultFile.exists() && groupsExamsResultFile.exists();
}

QString PlanCsvHelper::getPath() {
  return basePath;
}

void PlanCsvHelper::initializeFilePaths() {
  examsIntervalsFile.setFileName(basePath + "/pruef-intervalle.csv");
  examsFile.setFileName(basePath + "/pruefungen.csv");
  groupsExamsFile.setFileName(basePath + "/zuege-pruef.csv");
  groupsExamsPrefFile.setFileName(basePath + "/zuege-pruef-pref2.csv");
  planningExamsResultFile.setFileName(basePath +
                                      "/SPA-ERGEBNIS-PP/SPA-planung-pruef.csv");
  groupsExamsResultFile.setFileName(basePath +
                                    "/SPA-ERGEBNIS-PP/SPA-zuege-pruef.csv");
}
