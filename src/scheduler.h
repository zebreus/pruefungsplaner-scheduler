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
class Scheduler: public QObject {
  Q_OBJECT
 public:
  explicit Scheduler(QObject* parent = nullptr): QObject(parent) {}
  /**
   *  @brief Start scheduling the plan passed in the constructor
   *  @return A boolean indicating if scheduling was started
   */
  virtual bool startScheduling() = 0;

  /**
   * @brief Stop the running scheduling and emit result, if possible
   */
  virtual void stopScheduling() = 0;

  // virtual destructor for interface
  virtual ~Scheduler() {}

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
  void emitWarning(const QString& warning);

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

#endif  // SCHEDULER_H
