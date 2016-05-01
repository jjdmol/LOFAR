/*
 * Scheduler.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 4-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Scheduler.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "Scheduler.h"
#include "schedulerdata.h"
#include "station.h"
#include "task.h"
#include "Controller.h"
#include <cmath>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <QMessageBox>
using std::vector;
using std::max;
using std::min;


Scheduler::Scheduler() :
	pData(0), data_loaded(false), userDefinedStopCriteria(false), maxNrOfOptimizeIterations(MAX_OPTIMIZE_ITERATIONS),
	userAcceptedPenalty(0), minTimeBetweenTasks(MIN_TIME_BETWEEN_TASKS_JULIAN)
{
}

Scheduler::~Scheduler() {
}

void Scheduler::setData(SchedulerData &data) {
	pData = &data;
	data_loaded = true;
}

void Scheduler::updateSettings(void) { // updates the scheduler settings according to the settings in Controller::theSchedulerSettings
	userAcceptedPenalty = Controller::theSchedulerSettings.getUserAcceptedPenalty();
	userAcceptedPenaltyEnabled = Controller::theSchedulerSettings.getUserAcceptedPenaltyEnabled();
	maxNrOptimizationsEnabled = Controller::theSchedulerSettings.getMaxNrOptimizationsEnabled();
	maxNrOfOptimizeIterations = Controller::theSchedulerSettings.getMaxNrOptimizations();
	minTimeBetweenTasks = Controller::theSchedulerSettings.getMinimumTimeBetweenTasks();
}

/*
void Scheduler::calculateSunSetsAndSunDowns(void) {
}

void Scheduler::calculatePenalties(void) {
	// For every task (scheduled and unscheduled) do the following:
	// step 0: determine if the penalty needs to be recalculated ( Task::penaltyCalculationNeeded() ), if so:
	// for every station used by the task calculate:
	// x = [0:1:100] = percentage of task scheduled at daytime for that station
	// y = night time weight factor (for penalty calculation [0.0, 1.0]
	// curve 1: y = 1  ( in other words: task may not be scheduled during daytime)
	// curve 2: y = 1-c^-x  // c = 1.1
	// curve 3: y = 1-c^-x  // c = 1.04
	// curve 4: y = x
	// curve 5: y = c^(x-100) - 0.019800040113920 // c = 1.04
	// curve 6: y = c^(x-100) - 0.000072565715901  // c = 1.1
	// curve 7: y = 0 ( in other words: task may be scheduled at daytime without penalty)

	// penalty contribution of (partially) day time scheduling
	// step 1: calculate sun set and sun down for the currently scheduled day
	// step 2: calculate percentage overlap of schedule period with day time (factor x)
	// step 3: calculate night time weight factor according to user chosen curve (1 to 7) (this is factor y)
	// step 4: calculate penalty contribution for partially day time scheduling with:
	// penalty contribution = d * y,  where d is the maximum penalty for full day time scheduling
	// step 5: sum all penalties of all tasks to determine the total penalty for the schedule being examined

}
*/

int Scheduler::optimize(void)
{
	bestSchedule = pData->getCurrentSchedule(); // needs operator = for class NeighbourSolution
	unsigned int bestPenalty = pData->getCurrentSchedule().getPenalty();
	userAcceptedPenaltyEnabled = Controller::theSchedulerSettings.getUserAcceptedPenaltyEnabled();
	userAcceptedPenalty = Controller::theSchedulerSettings.getUserAcceptedPenalty();
	maxNrOfOptimizeIterations = Controller::theSchedulerSettings.getMaxNrOptimizations();
	maxNrOptimizationsEnabled  = Controller::theSchedulerSettings.getMaxNrOptimizationsEnabled();
	// c is the control parameter that determines the acceptance rate of a test schedule with a higher penalty
	// We gradually force c to zero during subsequent iterations
	unsigned int c = 500; //maxNrOfOptimizeIterations + 1; //(pData->getNrScheduled() + pData->getNrUnscheduled()) * MAX_TASK_PENALTY;
	//unsigned int c_step = 2 * c / maxNrOfOptimizeIterations;
	//	unsigned int currentScheduleMaxChangeCount = pData->getNrOfConflicts();
	//	bool allowUnscheduleFixedTasks = Controller::theSchedulerSettings.getAllowUnscheduleFixedTasks();
	unsigned iteration(0), taskID; // , predTaskID, successorTaskID;
	bool scheduleChanged(false);
	Task * pTask(0);
	while (true) {
		scheduleChanged = false;
		// check stopping criteria
		if (userAcceptedPenaltyEnabled && (bestPenalty <= userAcceptedPenalty)) {
			pData->setCurrentSchedule(bestSchedule);
			return 1; // user accepted penalty reached
		}
		if (maxNrOptimizationsEnabled && (iteration == maxNrOfOptimizeIterations)) {
			pData->setCurrentSchedule(bestSchedule);
			return 2; // max number of optimizations reached
		}
		testSchedule = pData->getCurrentSchedule(); // make a copy of the current schedule
		// calculate a neighbour schedule
		// SIMULATED ANNEALING ALGORITHM
		// choose a random task to reschedule from the conflicting tasks
		taskID = testSchedule.getRandomScheduledTaskID();
		if (taskID) {
			pTask = testSchedule.getTaskForChange(taskID);
		}
		else return 3; // there are no more scheduled tasks that may be changed

		// now try an alteration of the current schedule
		if (pTask->getFixedDay()) {
			if (!pTask->getFixedTime()) { // if task only fixed on day not on time
                if (testSchedule.tryShiftTaskWithinDay(pTask->getID(), true)) {
					scheduleChanged = true;
				}
			}
		}
		else if (pTask->getFixedTime()) { // task only fixed on time not on day
            if (testSchedule.tryMoveTaskToAdjacentDay(pTask->getID(), true)) {
				scheduleChanged = true;
			}
		}
		else {
            if (testSchedule.shiftTask(pTask->getID(), true)) {
				scheduleChanged = true;
			}
		}

		if (!scheduleChanged) { // if the schedule didn't change then we try to unschedule the random task
			if (testSchedule.unscheduleTask(pTask->getID())) {
				scheduleChanged = true;
			}
			else {
				++iteration; // increase the iteration counter to prevent the possibility of an endless loop
			}
		}

		if (scheduleChanged) {
			// now check if other unscheduled tasks can be fit in to the changed schedule
			testSchedule.tryScheduleUnscheduledTasks();
			//testSchedule.addChangedTasks(changedTask);
			// keep track of the best schedule so far.
			testSchedule.calculatePenalty();
			if (testSchedule.getPenalty() < bestSchedule.getPenalty()) {
				bestSchedule = testSchedule;
				bestPenalty = bestSchedule.getPenalty();
				//pData->setCurrentSchedule(bestSchedule); // move to best schedule
			}

			double acceptanceValue = static_cast<double>(rand()-1) / RAND_MAX;
			double currentValue = exp((static_cast<double>(pData->getPenalty()) - static_cast<double>(testSchedule.getPenalty())) / c);

			debugInfo("sisisisi", "iteration: ",iteration+1, ", last penalty: ", testSchedule.getPenalty(), ", current penalty: ", pData->getPenalty(),
					", best schedule penalty: ", bestSchedule.getPenalty());

			if (testSchedule.getPenalty() <= pData->getPenalty()) {
				pData->setCurrentSchedule(testSchedule);// accept schedule if it has a lower penalty
			}
			else if (currentValue >	acceptanceValue) { // move to new schedule with a certain probability
				pData->setCurrentSchedule(testSchedule);
				//currentScheduleMaxChangeCount = pData->getNrOfConflicts();
			}
			// force the control parameter stepwise towards zero during iterations
			if (c > 50) {c -= 2;}

			emit optimizeIterationFinished(++iteration);
		}
	} // while (true)
    return false;
}

bool Scheduler::createStartSchedule(void)
{
	if (data_loaded) {
		//		if (!(pData->checkTasksForErrors())) {
		pData->unscheduleAll();
		if (pData->getNrUnscheduled()) {
			pData->sortUnscheduledTasks2Priority();
			pData->scheduleFixedTasks();
			pData->tryScheduleUnscheduledTasks();
			unsigned int penalty = pData->calcCurrentPenalty();
			debugInfo("sisisi", "start schedule created. # scheduled: ",
					pData->getNrScheduled(), ", # unscheduled: ", pData->getNrUnscheduled(), ", total penalty: ", penalty);
			return true;
		}
		else {
			QMessageBox::warning(0, tr("No tasks could be unscheduled"),
					tr("There are no tasks that may be unscheduled. Cannot create a start schedule."),tr("Close"));
			return true;
		}
	}
	else return false;
}

bool Scheduler::tryRescheduleTask(unsigned task_id, const AstroDateTime &new_start) {
	return pData->rescheduleTask(task_id, new_start);
}

bool Scheduler::rescheduleAbortedTask(unsigned task_id, const AstroDateTime &new_start) {
	return pData->rescheduleAbortedTask(task_id, new_start);
}

bool Scheduler::checkFilterConflict(unsigned taskToCheck, station_filter_type filter) {
	//std::vector<unsigned int>::const_iterator task_it = task_ids.begin();
    const Observation *pTask = pData->getScheduledObservation(taskToCheck);
	if (pTask) {
        if (pTask->getFilterType() == filter) return false; // no conflict if filters are the same
	}
	else {
		// task was not found in scheduled tasks -> no conflicts possible
		debugWarn("sis","Task with ID: ",taskToCheck, "is not scheduled. Therefore, there are no filter conflicts!");
        return false;
	}
    return true;
}

bool Scheduler::checkClockFrequencyConflict(unsigned task_id, station_clock clock) {
    const Observation *pTask = pData->getScheduledObservation(task_id);
	if (pTask) {
        if (pTask->getStationClock() == clock) {
            return false; // no conflict if filters are the same
		}
		else {
            return true; // conflicting clock frequency settings
		}
	}
    else {
        // task was not found in scheduled tasks -> no conflicts possible
        debugWarn("sis","Task with ID: ",task_id, "is not scheduled. Therefore, there are no filter conflicts!");
        return false;
    }
}

bool Scheduler::checkTaskTypeConflict(unsigned taskToCheck, Task::task_type type) {
    const Task *pTask = pData->getScheduledTask(taskToCheck);
	if (pTask) {
		Task::task_type ttype = pTask->getType();
		switch (type) {
		case Task::OBSERVATION:
            if (ttype == Task::MAINTENANCE) return true; // no other tasks possible when Maintenance is being performed
            return false;
			break;
		case Task::RESERVATION:
            if (ttype == Task::MAINTENANCE) return true; // no other tasks possible when Maintenance is being performed
            return false;
			break;
		case Task::MAINTENANCE:
			return 1; // maintenance cannot be scheduled when other task type is scheduled
			break;
		case Task::PIPELINE:
            if (ttype == Task::MAINTENANCE) return true; // no other tasks possible when Maintenance is being performed
            return false;
			break;
		case Task::SYSTEM:
            if (ttype == Task::MAINTENANCE) return true; // no other tasks possible when Maintenance is being performed
            return false;
			break;
		case Task::UNKNOWN:
            if (ttype == Task::MAINTENANCE) return true; // no other tasks possible when Maintenance is being performed
            return false;
			break;
		default:
			debugWarn("sis", "Task with ID: ",taskToCheck, "has an unknown task type specification");
			break;
		}
	}
	else {
		// task was not found in scheduled tasks -> no conflicts possible
		debugWarn("sis","Task with ID: ",taskToCheck, "is not scheduled. Therefore, there are no task type conflicts!");
	}
    return false;
}

