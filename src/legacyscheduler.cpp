#include "legacyscheduler.h"

LegacyScheduler::LegacyScheduler(QSharedPointer<Plan> plan,
                                 const QString& algorithmBinary,
                                 const bool printLog,
                                 const SchedulingMode mode,
                                 QObject* parent)
    : Scheduler(parent),
      workingDirectory(),
      csvHelper(workingDirectory.path()),
      originalPlan(plan),
      printLog(printLog),
      mode(mode),
      schedulerProcess(this),
      emitedFailedOrFinished(false) {
  prepareEnvironment();

  QList<QString> arguments;
  arguments += "-p";
  arguments += workingDirectory.path();
  arguments += "-PP";

  schedulerProcess.setArguments(arguments);
  schedulerProcess.setProgram(algorithmBinary);

  connect(&schedulerProcess, &QProcess::readyReadStandardOutput, this, [this]() {
    schedulerProcess.setReadChannel(QProcess::StandardOutput);
    while(schedulerProcess.canReadLine()) {
      processLine(schedulerProcess.readLine(), QProcess::StandardOutput);
    }
  });
  connect(&schedulerProcess, &QProcess::readyReadStandardError, this, [this]() {
    schedulerProcess.setReadChannel(QProcess::StandardError);
    while(schedulerProcess.canReadLine()) {
      processLine(schedulerProcess.readLine(), QProcess::StandardError);
    }
  });
  connect(&schedulerProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
    qDebug() << "The error is: " << error;
    emit updateProgress(1.0);
    switch(error) {
      case QProcess::FailedToStart:
        failScheduling("Failed to start legacy scheduler");
        return;
      case QProcess::Crashed:
        failScheduling("Legacy scheduler crashed");
        return;
      case QProcess::Timedout:
        failScheduling("Legacy scheduler took to long");
        return;
      case QProcess::ReadError:
        failScheduling("An internal read error occurred");
        return;
      case QProcess::WriteError:
        failScheduling("An internal write error occurred");
        return;
      default:
      case QProcess::UnknownError:
        failScheduling("An unknown error occurred");
        return;
    }
  });
  connect(&schedulerProcess,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          this,
          [this](int exitCode, QProcess::ExitStatus exitStatus) {
            std::clog << "finished with code " << exitCode << "!\n";
            emit updateProgress(1.0);
            if(exitStatus == QProcess::CrashExit) {
              failScheduling("LegacyScheduler crashed");
              return;
            }
            if(exitCode != 0) {
              failScheduling("LegacyScheduler did not exit with code 0");
              return;
            }
            if(!readResults()) {
              failScheduling("Failed to read plan");
              return;
            }
          });
}

LegacyScheduler::~LegacyScheduler() {
  if(schedulerProcess.state() != QProcess::NotRunning) {
    schedulerProcess.terminate();
    schedulerProcess.waitForFinished(500);
    if(schedulerProcess.state() != QProcess::NotRunning) {
      schedulerProcess.kill();
    }
  }
}

bool LegacyScheduler::startScheduling() {
  emitedFailedOrFinished = false;
  failReason = "";
  prepareCommand = "";
  emit updateProgress(0.0);
  if(!prepareEnvironment()) {
    return false;
  }
  if(!executeScheduler()) {
    return false;
  }
  return true;
}

void LegacyScheduler::stopScheduling() {
  schedulerProcess.terminate();
}

bool LegacyScheduler::prepareEnvironment() {
  if(!csvHelper.writePlan(originalPlan.get())) {
    return false;
  } else {
    return true;
  }
}

bool LegacyScheduler::executeScheduler() {
  std::clog << "Starting scheduling";

  schedulerProcess.open();

  schedulerProcess.waitForStarted();

  if(schedulerProcess.state() != QProcess::Running) {
    return false;
  }

  switch(mode) {
    case Fast:
      schedulerProcess.write("jn");
      break;
    case Good:
      schedulerProcess.write("jjn");
      break;
  }
  schedulerProcess.closeWriteChannel();
  // schedulerProcess.waitForFinished(200000);

  return true;
}

void LegacyScheduler::processLine(const QString& line, QProcess::ProcessChannel) {
  if(printLog) {
    qDebug() << "Read line: " << line;
  }
  if(line.contains("FEHLER") || (line.contains("WARNUNG") && !line.contains("WARNUNGEN")) || line.contains("kann nicht zugeteilt werden")) {
    emit emitWarning(line);
    return;
  }

  // Detect if the scheduler is stuck and send sigint
  QRegularExpression schedulerStuckExpression("hängt \\([0-9][0-9]+\\)");
  if(schedulerStuckExpression.match(line).hasMatch()) {
    schedulerProcess.terminate();
  }

  if(mode == Good) {
    // Detect the real location of the output directory and move it to the expected location
    // Good mode names the output directory with a timestamp
    QRegularExpression matchResultFolderExpression("Details in: (.*)/[^/]*");
    auto resultFolderMatch = matchResultFolderExpression.match(line);
    if(resultFolderMatch.hasMatch()) {
      QString path = resultFolderMatch.captured(1);
      QString targetPath = workingDirectory.path() + "/SPA-ERGEBNIS-PP";
      prepareCommand = "rm -r '" + targetPath + "' ; mv '" + path + "' '" + targetPath + "'";
    }

    QRegularExpression currentBestExpression("ESoftBest: ([0-9]+)");
    auto currentBestMatch = currentBestExpression.match(line);
    if(currentBestMatch.hasMatch()) {
      QString currentBestString = currentBestMatch.captured(1);
      bool ok;
      int currentBest = currentBestString.toInt(&ok);
      if(ok) {
        // Progress is at least 0.05, to indicate, that it started
        float progress = ((1.0 - (std::clamp(currentBest, 0, 150) / 150.0)) * 0.95) + 0.05;
        emit updateProgress(progress);
      }
    }
  }
}

bool LegacyScheduler::readResults() {
  if(!prepareCommand.isEmpty()) {
    system(prepareCommand.toStdString().data());
  }
  QSharedPointer<Plan> plan = originalPlan;
  if(plan == nullptr) {
    failReason = "Failed to read plan";
    return false;
  }

  if(csvHelper.readSchedule(plan.get())) {
    emit finishedScheduling(plan);
    emitedFailedOrFinished = true;
    return true;
  } else {
    failReason = "Failed to read scheduling results. Maybe the algorithm was not able "
                 "to schedule, but did not error.";
    return false;
  }
}

void LegacyScheduler::failScheduling(QString alternativeReason) {
  if(emitedFailedOrFinished == false) {
    emitedFailedOrFinished = true;
    if(failReason != "") {
      emit failedScheduling(failReason);
      return;
    }
    emit failedScheduling(alternativeReason);
  }
}
