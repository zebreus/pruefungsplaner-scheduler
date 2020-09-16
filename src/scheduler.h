#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <plan.h>
#include <QSharedPointer>
#include <QString>

/**
 *  @interface Scheduler
 *  @brief Schedules a plan
 *
 *  A class implementing Scheduler can schedule plans
 */
class Scheduler {
 public:
  /**
   *  @brief Start scheduling the plan passed in the constructor
   *  @return A boolean indicating if scheduling was started
   */
  virtual bool startScheduling() = 0;

  // virtual destructor for interface
  virtual ~Scheduler() {}

 signals:

  /**
   *  @brief This signal will be emitted, when progress is made
   *  @param progress is the current progress
   */
  virtual void updateProgress(double progress) = 0;

  /**
   *  @brief This signal will be emitted if the scheduling finished successfully
   *  @param plan is a pointer to the scheduled plan
   */
  virtual void finishedScheduling(QSharedPointer<Plan> plan) = 0;

  /**
   *  @brief This signal will be emitted, if scheduling failed
   *  @param message contains a message with information about the failure
   */
  virtual void failedScheduling(QString message) = 0;
};

#endif  // SCHEDULER_H
