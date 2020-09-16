#ifndef SCHEDULERSERVICE_H
#define SCHEDULERSERVICE_H

#include <QJsonValue>
#include <QObject>
//#include "Scheduler.h"

/**
 *  @class SchedulerService
 *  @brief A service for scheduling plans
 *
 *  The SchedulerService provides methods to schedule plans.
 *  Only one plan can be scheduled simultaneously.
 */
class SchedulerService : public QObject {
  Q_OBJECT

 private:
  // Scheduler &scheduler;

 public:
  /**
   *  @brief Creates a new SchedulerService
   *  @param parent is the parent of this QObject
   */
  explicit SchedulerService(QObject* parent = nullptr);

 public slots:

  /**
   *  @brief Start scheduling the plan
   *  @param [in] plan is a QJsonValue representing the plan, that should be
   * scheduled
   *  @param [in] parent is the pare
   *  @return A boolean indicating if scheduling was started
   *
   *  Returns false if a plan is already being scheduled or plan is invalid
   */
  bool startScheduling(QJsonValue plan);

  /**
   *  @brief Get the progress of scheduling
   *  @return A double between 0.0 and 1.0 representing the current planning
   * progress
   *
   *  Get the progress of scheduling
   */
  double getProgress();

  /**
   *  @brief Get the scheduled plan
   *  @return A QJsonValue containing the scheduled plan or nothing
   *
   *  Returns the result as a JsonValue. If no result exists, a QJsonValue with
   * the value undefined is returned.
   */
  QJsonValue getResult();
};

#endif  // SCHEDULERSERVICE_H
