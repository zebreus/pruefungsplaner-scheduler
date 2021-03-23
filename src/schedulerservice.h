#ifndef SCHEDULERSERVICE_H
#define SCHEDULERSERVICE_H

#include <QJsonValue>
#include <QObject>

#include "configuration.h"
#include "legacyscheduler.h"
#include "plan.h"
#include "scheduler.h"

/**
 *  @class SchedulerService
 *  @brief A service for scheduling plans
 *
 *  The SchedulerService provides methods to schedule plans.
 *  Only one plan can be scheduled simultaneously.
 *
 *  This class will be rewritten, when the jsonrpc module supports signals.
 */
class SchedulerService: public QObject {
  Q_OBJECT

 private:
  QSharedPointer<Configuration> configuration;
  QScopedPointer<Scheduler> scheduler;
  double progress;
  QJsonValue result;
  QString customAlgorithm;

 public:
  /**
   *  @brief Creates a new SchedulerService
   *  @param [in] configuration is the Configuration for this service
   *  @param parent is the parent of this QObject
   */
  explicit SchedulerService(const QSharedPointer<Configuration> configuration, QObject* parent = nullptr);

 public slots:

  /**
   *  @brief Start scheduling the plan
   *  @param [in] plan is a QJsonValue representing the plan, that should be
   * scheduled
   *  @param [in] parent is the pare
   *  @return A boolean indicating if scheduling was started
   *
   *  Returns false if a plan is already being scheduled
   */
  bool startScheduling(QJsonObject plan);

  /**
   *  @brief Set the mode for the next and all subsequent schedules
   *  @param [in] mode is the scheduling mode
   *  @return A boolean indicating, if setting the mode was successfull
   *
   *  mode has to be "legacy-good" or "legacy-fast"
   */
  bool setSchedulingAlgorithm(QString mode);

  /**
   *  @brief Try to stop the current scheduling
   *  @return A boolean indicating if scheduler was asked to stop
   *
   *  Try to stop the current scheduling. Will emit finishedScheduling or failedScheduling, when it stopped
   */
  bool stopScheduling();

  /**
   *  @brief Get the progress of scheduling
   *  @return A double between 0.0 and 1.0 representing the current planning
   * progress
   *
   *  Get the progress of scheduling. If scheduling was successful or failed it
   * returns 1.0
   */
  double getProgress();

  /**
   *  @brief Get the scheduled plan
   *  @return A QJsonValue containing the scheduled plan, an errormessage or
   * nothing
   *
   *  Returns the result as a JsonValue. If no result exists, a QJsonValue with
   * type QJsonValue::Undefined is returned. If no result exists, a QJsonValue
   * with the error message as a string is returned.
   */
  QJsonValue getResult();

 signals:
  /**
   *  @brief This signal will be emitted, when progress is made
   *  @param progress is the current progress
   */
  void updateProgress(double progress);

  /**
   *  @brief This signal will be emitted, when something may have went wrong
   *  @param progress is the current progress
   */
  void emitWarning(QString warning);

  /**
   *  @brief This signal will be emitted if the scheduling finished successfully
   *  @param plan is a pointer to the scheduled plan
   */
  void finishedScheduling(QJsonObject plan);

  /**
   *  @brief This signal will be emitted, if scheduling failed
   *  @param message contains a message with information about the failure
   */
  void failedScheduling(QString message);
};

#endif  // SCHEDULERSERVICE_H
