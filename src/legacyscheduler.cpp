#include "legacyscheduler.h"

LegacyScheduler::LegacyScheduler(QSharedPointer<Plan> plan,
                                 const QString& algorithmBinary,
                                 const bool printLog,
                                 const SchedulingMode mode,
                                 QObject* parent)
    : QObject(parent),
      workingDirectory(),
      csvHelper(workingDirectory.path()),
      originalPlan(plan),
      algorithmBinary(algorithmBinary),
      printLog(printLog),
      mode(mode) {
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
  if (!csvHelper.writePlan(originalPlan.get())) {
    emit failedScheduling("Failed to prepare environment");
    return false;
  } else {
    return true;
  }
}

bool LegacyScheduler::executeScheduler() {
  std::clog << "Starting scheduling";

  QString input;
  switch (mode) {
    case Fast:
      input = "jn";
      break;
    case Good:
      input = "jjn";
      break;
  }

  QFile logfile(workingDirectory.path() + "/scheduler.log");
  QFile errorLogfile(workingDirectory.path() + "/schedulerErrors.log");
  QString command("echo -ne " + input + " | " + algorithmBinary + " -p " +
                  workingDirectory.path() + " -PP 2>&1 >> " +
                  logfile.fileName() + " | tee " + errorLogfile.fileName() +
                  " >> " + logfile.fileName());
  int exitCode = system(command.toUtf8().constData());

  if (printLog) {
    if (logfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream fileStream(&logfile);
      qDebug() << fileStream.readAll();
      logfile.close();
    }
  }
  // Add -t 16 for 16 threads
  // Use this for planning with soft constraints (Execution takes ~1 hour)
  // int exitCode = system(QString("echo -ne jjn | ./SPA-algorithmus -p " +
  // workingDirectory.path() + " -PP").toUtf8().constData());
  if (logfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream fileStream(&logfile);
    QString errormessage;
    while (!fileStream.atEnd()) {
      QString line = fileStream.readLine();
      if(line.contains("FEHLER") || ( line.contains("WARNUNG") && !line.contains("WARNUNGEN")) || line.contains("kann nicht zugeteilt werden")){
        errormessage += line + '\n';
        qDebug() << line;
      }
    }
    logfile.close();
    if (errormessage != "") {
      emit failedScheduling(errormessage);
      return false;
    }
  }
  if (exitCode == 0) {
    return true;
  } else {
    // Try to detect a errormessage in the logfile
    emit failedScheduling("Failed to execute SPA-algorithmus");
    return false;
  }
}

bool LegacyScheduler::readResults() {
  QSharedPointer<Plan> plan = originalPlan;
  if (plan == nullptr) {
    emit failedScheduling("Failed to read plan");
    return false;
  }

  if (csvHelper.readSchedule(plan.get())) {
    emit finishedScheduling(plan);
    return true;
  } else {
    emit failedScheduling(
        "Failed to read scheduling results. Maybe the algorithm was not able "
        "to schedule, but did not error.");
    return false;
  }
}
