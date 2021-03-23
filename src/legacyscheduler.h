#ifndef LEGACYSCHEDULER_H
#define LEGACYSCHEDULER_H

#include <signal.h>
#include <unistd.h>

#include <QObject>
#include <QProcess>
#include <QSharedPointer>
#include <QString>

#include "plancsvhelper.h"
#include "scheduler.h"

/**
 *  @class LegacyScheduler
 *  @brief A wrapper for the legacy scheduling algorithm
 *
 *  This class provides a scheduler implementation, which uses the legacy
 * sp-automatisch scheduler. It needs a sp-automatisch binary.
 */
class LegacyScheduler: public Scheduler {
  Q_OBJECT

 public:
  /**
   * @brief The SchedulingModeFlag enum contains the different modes, this
   * scheduler can use
   */
  enum SchedulingModeFlag { Fast = 1, Good = 2 };
  Q_DECLARE_FLAGS(SchedulingMode, SchedulingModeFlag)

 private:
  QTemporaryDir workingDirectory;
  PlanCsvHelper csvHelper;
  QSharedPointer<Plan> originalPlan;
  bool printLog;
  SchedulingMode mode;
  QProcess schedulerProcess;

  QString failReason;
  bool emitedFailedOrFinished;
  QString prepareCommand;

 public:
  /**
   *  @brief Creates a new LegacyScheduler, that will schedule a plan
   *  @param [in] plan will be scheduled
   *  @param [in] configuration is the configuration for this scheduler
   *  @param [in] parent is the parent of this QObject
   */
  explicit LegacyScheduler(QSharedPointer<Plan> plan,
                           const QString& algorithmBinary = "./SPA-algorithmus",
                           const bool printLog = false,
                           const SchedulingMode mode = Fast,
                           QObject* parent = nullptr);

  ~LegacyScheduler();

  /**
   *  @brief Start scheduling the plan passed in the constructor
   *  @return A boolean indicating if scheduling was started
   */
  bool startScheduling() override;

  /**
   * @brief Stop the running scheduling and emit result, if possible
   */
  void stopScheduling() override;

 private:
  bool prepareEnvironment();

  bool executeScheduler();

  void processLine(const QString& line, QProcess::ProcessChannel channel);

  bool readResults();

  /**
   *  @brief Emits failedScheduling with the message reason. If reason is not set, alternativeReason is used
   */
  void failScheduling(QString alternativeReason);
};

#endif  // LEGACYSCHEDULER_H
