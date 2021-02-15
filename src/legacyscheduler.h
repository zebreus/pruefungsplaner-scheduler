#ifndef LEGACYSCHEDULER_H
#define LEGACYSCHEDULER_H

#include <QObject>
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
class LegacyScheduler : public QObject, public Scheduler {
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
  QString algorithmBinary;
  bool printLog;
  SchedulingMode mode;

 public:
  /**
   *  @brief Creates a new LegacyScheduler, that will schedule a plan
   *  @param [in] plan will be scheduled
   *  @param [in] configuration is the configuration for this scheduler
   *  @param [in] parent is the parent of this QObject
   */
  explicit LegacyScheduler(QSharedPointer<Plan> plan,
                           const QString& algorithmBinary,
                           const bool printLog = false,
                           const SchedulingMode mode = Fast,
                           QObject* parent = nullptr);

  /**
   *  @brief Start scheduling the plan passed in the constructor
   *  @return A boolean indicating if scheduling was started
   */
  bool startScheduling();

 private:
  bool prepareEnvironment();

  bool executeScheduler();

  bool readResults();

 signals:

  /**
   *  @brief This signal will be emitted, when progress is made
   *  @param progress is the current progress
   */
  void updateProgress(double progress);

  /**
   *  @brief This signal will be emitted if the scheduling finished successfully
   *  @param plan is a pointer to the scheduled plan
   */
  void finishedScheduling(QSharedPointer<Plan> plan);

  /**
   *  @brief This signal will be emitted, if scheduling failed
   *  @param message contains a message with information about the failure
   */
  void failedScheduling(QString message);
};

#endif  // LEGACYSCHEDULER_H
