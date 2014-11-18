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

	void setMinimumTimeBetweenTasks(const AstroTime &min_time) {minTimeBetweenTasks = min_time;}
	void updateSettings(void); // updates the scheduler settings according to the settings in Controller::theSchedulerSettings
	bool rescheduleAbortedTask(unsigned task_id, const AstroDateTime &new_start);

private:
    bool rescheduleAbortedTask(Task *task);
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

