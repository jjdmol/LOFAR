/*
 * Scheduler.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 4-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Scheduler.h $
 *
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <QObject>
#include <vector>
#include "station.h"
#include "task.h"
#include "schedulerdatablock.h"
#include "neighboursolution.h"
//#include "schedulersettings.h"


class SchedulerData;

class Scheduler : public QObject {

	Q_OBJECT

public:
	Scheduler();
	virtual ~Scheduler();

	bool createStartSchedule(void); // creates a start schedule
	int optimize(void); // starts optimizing the schedule
	void setData(SchedulerData &data); // loads the data in the scheduler

	// sorts the unscheduled tasks according to priority (MOVED TO schedulerDataBlock)
	//void sortUnscheduledTasks2Priority();

	// check if the task with ID taskToCheck conflicts with the specified filter setting
	bool checkFilterConflict(unsigned taskToCheck, station_filter_type filter);
	// check if the task with ID taskToCheck conflicts with the specified station clock setting
    bool checkClockFrequencyConflict(unsigned task_id, station_clock clock);
	// check if the task with ID taskToCheck conflicts with the specified task type
	bool checkTaskTypeConflict(unsigned taskToCheck, Task::task_type type);
//	void logConflicts(unsigned int taskID, std::vector<unsigned int> &possible_conflicts);
	//void scheduleFixedTasks(void); // schedules tasks marked with fixed day or fixed time
//	void calculatePenalties(void);
//	void calculateSunSetsAndSunDowns(void);
	void setMinimumTimeBetweenTasks(const AstroTime &min_time) {minTimeBetweenTasks = min_time;}
	void setMaxOptimizationIterations(unsigned max_optimizations) {maxNrOfOptimizeIterations = max_optimizations;}
	void updateSettings(void); // updates the scheduler settings according to the settings in Controller::theSchedulerSettings
	bool tryRescheduleTask(unsigned task_id, const AstroDateTime &new_start);
	bool rescheduleAbortedTask(unsigned task_id, const AstroDateTime &new_start);

private:
	bool tryMoveTaskToAdjacentDay(Task *task);
	bool tryShiftTask(Task *task, SchedulerDataBlock &testSchedule);
	bool tryShiftTaskWithinDay(Task *task);


signals:
	void dataContainsErrors(void);
	void optimizeIterationFinished(unsigned);

private:
	SchedulerData *pData;
	NeighbourSolution testSchedule, bestSchedule;
	std::vector<SchedulerDataBlock> possibleSolutions;
	bool data_loaded;
	bool userDefinedStopCriteria;
	bool userAcceptedPenaltyEnabled;
	bool maxNrOptimizationsEnabled;
	unsigned maxNrOfOptimizeIterations;
	unsigned userAcceptedPenalty;
	AstroTime minTimeBetweenTasks;
};

#endif /* SCHEDULER_H_ */

