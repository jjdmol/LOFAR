/*
 * schedulerdatablock.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 12, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulerdatablock.h $
 *
 */

#ifndef SCHEDULERDATABLOCK_H_
#define SCHEDULERDATABLOCK_H_

#include <fstream>
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <algorithm>
#include "lofar_scheduler.h"
#include "station.h"
#include "Storage.h"

// TODO: Why not include the headerfiles?
class Observation;
class Pipeline;
class CalibrationPipeline;
class ImagingPipeline;
class PulsarPipeline;
class LongBaselinePipeline;
class StationTask;

#define FORCE_UNSCHEDULE true
#define OVERRIDE_SAS_TASKIDS true
#define DONT_CHECK_FREE_ID true

typedef std::map <unsigned, Station> stationsMap;
typedef std::deque <Task *> unscheduledTasksDeque;
typedef std::map <unsigned, StationTask *> scheduledTasksMap;
typedef std::map <unsigned, StationTask *> reservationsMap;
typedef std::map <unsigned, Pipeline *> pipelinesMap;
typedef std::map <unsigned, Task *> inActiveTasksMap; // tasks that have status equal or above finished

#include "task.h"
#include "schedulergui.h"

class SchedulerDataBlock
{
public:
	SchedulerDataBlock();
	SchedulerDataBlock(const SchedulerDataBlock &);
	virtual ~SchedulerDataBlock();

	SchedulerDataBlock & operator=(const SchedulerDataBlock &rhs);
	friend QDataStream& operator<< (QDataStream &out, const SchedulerDataBlock &data); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, SchedulerDataBlock &data); // used for reading data from binary file

	void cleanup(void); // clean up data objects
	void addUsedTaskID(unsigned taskID);
	void addUsedTaskIDs(const std::vector<unsigned> &taskIDs);
	const std::vector<unsigned> &getSASUsedTaskIDs(void) const {return itsExistingSASTaskIDs;}
	void setSASUsedTaskIDs(const std::vector<unsigned> &task_ids) {itsExistingSASTaskIDs = task_ids;}
	void updateUsedTaskIDs(void);
	void removeUsedTaskID(unsigned task_id);
	inline bool isTaskIDFree(unsigned taskID) const {return (find(itsUsedTaskIDs.begin(), itsUsedTaskIDs.end(), taskID) == itsUsedTaskIDs.end());}
	inline bool isTaskIDFreeInSAS(unsigned taskID) const {return (find(itsExistingSASTaskIDs.begin(), itsExistingSASTaskIDs.end(), taskID) == itsExistingSASTaskIDs.end());}
    bool addTask(Task *task, bool check_ID_free = true);// adds the (already created) task to scheduler
	unsigned getNewTaskID(bool override_SAS_taskIDs = false) const;
    StationTask *newReservation(unsigned reservation_id, bool override_SAS_taskIDs = false); // create a new reseravation with task ID equal to task_id, return pointer to task object or 0 when reservation_id already exists
    Observation *newObservation(unsigned task_id, bool override_SAS_taskIDs = false); // create a new task with task ID equal to task_id, return pointer to task or 0 when task_id already exists
    Pipeline *newPipeline(unsigned task_id, pipelineType type, bool override_SAS_taskIDs = false); // create a new task with task ID equal to task_id, return pointer to task or 0 when task_id already exists
    Task *newTask(unsigned task_id, bool override_SAS_taskIDs = false);
    Task *newTask(unsigned task_id, const OTDBtree &SAS_tree, bool override_SAS_taskIDs = false);
//	void deleteTask(unsigned taskID);
    Task *deleteTask(unsigned id, id_type IDtype = ID_SCHEDULER, bool erase = true);
	bool scheduleNextTask(const AstroDateTime &schedule_time); // schedules the next unscheduled task (which can be obtained with getNextTask
	bool scheduleTask(Task * pTask); // schedule the unscheduled task pointed to by pTask
	bool moveTaskToInactive(unsigned taskID); // puts the task in the inactive map if its state is one of the inactive states
	bool rescheduleTask(unsigned task_id, const AstroDateTime & new_start); // tries to reschedule the task at the first opportunity from the given start time onwards (does not always succeed)
	bool rescheduleAbortedTask(unsigned task_id, const AstroDateTime & new_start);
	void tryScheduleUnscheduledTasks(void); // iterates over all unscheduled tasks to try and schedule them
	// the following change.. functions change a task's schedule times without checking conflicts with other tasks
	bool changeTaskStartTime(unsigned task_id, const AstroDateTime &new_start);
	bool changeTaskEndTime(unsigned task_id, const AstroDateTime &new_end);
	bool changeTaskDuration(unsigned task_id, const AstroTime &new_duration);
	bool changeTaskSchedule(unsigned task_id, const AstroDateTime &new_start, const AstroDateTime &new_end);
	void scheduleFixedTasks(void); // schedules tasks marked with fixed day or fixed time
	void holdNextTask(const std::string &reason); // puts the next unscheduled task at the end of the unscheduled task list
	void holdNextTask(unscheduled_reasons reason = NO_ERROR); // puts the next unscheduled task at the end of the unscheduled task list
	bool unscheduleTask(unsigned task_id); // un-schedule the task with ID equal to task_id, returns true if un-schedule was successful
	bool unscheduleReservation(unsigned reservation_id); // unschedule a reservation (putting it ON HOLD and removing it from the stations
	bool addStation(unsigned station_id, const std::string &name); // add a new station (group) to the list of stations
	void setChangesMadeFlag(void) {itsSaveRequired = true; itsUploadRequired = true;}
	void clearChangesMadeFlag(void) {itsSaveRequired = false;}
	void clearUploadRequiredFlag(void) {itsUploadRequired = false;}
	unsigned calculatePenalty(void); // calculates the current total penalty (and stores it in totalPenalty)
	void moveUnscheduledTask(unsigned from, unsigned to);
//	void commitChanges(const tableChanges &); // used to commit the changes from the GUI's table view into the data structure of this schedulerDataBlock
	// unscheduleAll: unschedules all tasks and put them in the unscheduled list.
	void unscheduleAll(void);
	void checkStatusChanges(void);
//	void setConflictsTask(unsigned taskID, std::vector<unsigned> & conflicting_tasks) {itsConflicts[taskID] =  conflicting_tasks;}
//	void clearConflictsTask(unsigned taskID) {itsConflicts.erase(taskID);}
//	unsigned getRandomScheduledTaskID(bool includeFixedTasks);
	unsigned getRandomScheduledTaskID(void);
	bool taskExists(unsigned task_id) const;
	// updates all tasks to use the new station IDs from schedulerSettings
	void updateTasksStationIDs(void);
	// calculate the dataslots for all scheduled tasks
//	bool calculateDataSlots(void);

    std::vector<unsigned> moveTask(Task *pTask, const AstroDateTime &new_start, bool unschedule_conflicting_tasks); // returns a vector of conflicting tasks when not successful
    bool shiftTask(unsigned task_id, bool unschedule_conflicting_tasks); // shift the task to the left or right, returns false if the task was not yet scheduled
    bool tryMoveTaskToAdjacentDay(unsigned task_id, bool unschedule_conflicting_tasks); // returns conflicting tasks
    bool tryShiftTaskWithinDay(unsigned task_id, bool unschedule_conflicting_tasks);
//	bool tryShiftTask(unsigned task_id, bool unschedule_conflicting_tasks);

//	void alignLeft(void); // aligns all task as much as possible to the left

	//sorting functions
//	void sortScheduledTask2StartTime(void);
//	void sortUnscheduledTasks2StartTime(void);
	void sortUnscheduledTasks2Priority(void);

	// get methods
	size_t getNrTasks() const {return unscheduledTasks.size() + scheduledTasks.size() + inactiveTasks.size() + itsReservations.size() + itsPipelines.size();}
	bool getNextTask(Task &task) const; // returns true if the next unscheduled task was found. parameter task contains the task
	bool SaveRequired(void) const { return itsSaveRequired; } // controls if data changes need to be saved
	size_t getNrUnscheduled(void) const {return unscheduledTasks.size();}
	size_t getNrScheduled(void) const { return scheduledTasks.size(); }
	size_t getNrReservations(void) const { return itsReservations.size(); }
	size_t getNrInactive(void) const { return inactiveTasks.size(); }
	size_t getNrPipelines(void) const { return itsPipelines.size(); }
	const unscheduledTasksDeque &getUnscheduledTasks(void) const { return unscheduledTasks; }
    const scheduledTasksMap &getScheduledTasks(void) const {return scheduledTasks;}
    const reservationsMap &getReservations(void) const {return itsReservations;}
	const pipelinesMap &getPipelineTasks(void) const {return itsPipelines;}
	const inActiveTasksMap &getInactiveTasks(void) const {return inactiveTasks;}
    std::vector<Task *> getReservationsVector(void) const;
    std::vector<Task *> getMaintenanceVector(void) const;
	std::vector<Task *> getScheduledTasksVector(void) const; // no pipelines
	std::vector<Task *> getInactiveTaskVector(void) const;
	std::vector<Task *> getUnScheduledTasksVector(void) const;
    std::vector<Task *> getPipelinesVector(void) const;
    std::vector<Pipeline *> getScheduledPipelinesVector(void) const;
	Task *getTaskForChange(unsigned task_id, id_type = ID_SCHEDULER); // returns the task with id equal to task_id so the caller can edit its properties
    Pipeline *getPipelineForChange(unsigned task_id); // returns the task with id equal to task_id so the caller can edit its properties
    StationTask *getReservationForChange(unsigned task_id); // returns the task with id equal to task_id so the caller can edit its properties
    std::vector<unsigned> tasksInGroup(unsigned groupID, selector_types type = SEL_ALL_TASKS) const;
	std::map<unsigned, std::vector<Task *> > getGroupedTasks(Task::task_status state) const; // key = groupID
	std::map<unsigned, std::vector<Task *> > getGroupedObservations(Task::task_status state = Task::TASK_STATUS_END) const; // key = groupID
	std::vector<Task *> getTasksInScheduleSortStartTime(void) const;
	std::vector<Task *> getScheduledTasksSortStartTime(void) const;
	std::vector<Task *> getScheduledObservationsSortStartTime(void) const;
	std::vector<Task *> getPreScheduledObservationsSortStartTime(void) const;
	std::vector<Task *> getPreScheduledTasksSortStartTime(void) const;
    std::vector<Observation *> getFutureObservationsSortStartTime(void) const;	// get all observations (no maintenance and reservations) with status SCHEDULED and PRESCHEDULED in the future
    std::vector<Task *> getUnScheduledTasksSortFirstDate(void) const;
    std::map<std::string, std::vector<Task *> > getPublishTasks(const AstroDateTime &start, const AstroDateTime &end) const;
	const Task *getTask(unsigned task_id, id_type IDtype = ID_SCHEDULER) const;
    const StationTask *getStationTask(unsigned task_id, id_type IDtype = ID_SCHEDULER) const;
    const Observation *getObservation(unsigned taskID, id_type IDtype = ID_SCHEDULER) const;
    const Pipeline *getPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const;
    const CalibrationPipeline *getCalibrationPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const;
    const ImagingPipeline *getImagingPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const;
    const PulsarPipeline *getPulsarPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const;
    const LongBaselinePipeline *getLongBaselinePipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const;
    const StationTask *getScheduledTask(unsigned task_id) const;
    StationTask *getScheduledTask(unsigned task_id);
    const Observation *getScheduledObservation(unsigned task_id) const;
    const Pipeline *getPipelineTask(unsigned task_id) const;
    const StationTask *getReservation(unsigned reservation_id) const;
	const Task *getUnscheduledTask(unsigned task_id) const;
	const Task *getInactiveTask(unsigned task_id) const;
	Task *getUnscheduledTaskForChange(unsigned task_id) const;
	Task *getInactiveTaskForChange(unsigned task_id) const;
	const Station *getStation(unsigned station_id) const;
	Station *getStationForChange(unsigned station_id);
	void updateStations(void); // creates the stations which are defined in theSchedulerSettings
	unsigned getPenalty(void) const {return itsPenalty;}
	const stationsMap &getStations(void) const {return itsStations;}
	const AstroDateTime & getTaskStartTime(unsigned taskID) const;
	const AstroDateTime & getTaskEndTime(unsigned taskID) const;
	AstroDateTime getTaskFirstPossibleDate(unsigned taskID) const;
	const AstroTime & getTaskPredecessorMaxTimeDif(unsigned taskID) const;
	Task::task_status getTaskStatus(unsigned taskID) const;
	bool getSaveRequired(void) const {return itsSaveRequired;}
	bool getUploadRequired(void) const {return itsUploadRequired;}
	const errorTasksMap &getErrorTasks(void) const {return errorTasks;}
	bool newErrorTasks(void);
	void clearError(unsigned taskID, data_headers header);
	void markErrorTasksStatus(void); // sets the status of task with errors to state ERROR
	unsigned nrOfErrorTasks(void) const {return errorTasks.size();}
	const Storage &getStorage(void) const {return itsStorage;}

	// checking tasks
    bool checkStationConflicts(StationTask *pTask);
	bool checkTasksForErrors(void); // check for all errors, returns 1 if no-errors or errors were fixed
	bool predecessorExists(unsigned predecessorID) const {return taskExists(predecessorID);}
    unscheduled_reasons checkTaskStations(StationTask *pTask); // check stations of the given task
	// do a 'manual' check for errors on a single task
	bool checkTask(Task *pTask);
	// checks if the specified task time window is within the schedule
	bool checkTaskBoundaries(const Task &task) const;
	const std::vector<unsigned> &getUsedTaskIDs(void) const {return itsUsedTaskIDs;}
	void setUsedTaskIDs(const std::vector<unsigned> &usedIDs) {itsUsedTaskIDs = usedIDs;}
	bool findFirstOpportunity(const Task *pTask, AstroDateTime &start_time, unsigned reservation_id = 0);
	void initStorage(void) {itsStorage.initStorage();}

	void clearStorageClaims(void) {itsStorage.clearStorageClaims();}
	std::vector<storageResult> addStorageToTask(Task *pTask, const storageMap &storageLocations) {return itsStorage.addStorageToTask(pTask, storageLocations);}
	std::vector<storageResult> addStorageToTask(Task *pTask, dataProductTypes dataProduct, const storageVector &storageLocations, bool noCheck = false) {
		return itsStorage.addStorageToTask(pTask, dataProduct, storageLocations, noCheck);
	}
	void removeStorageForTask(unsigned taskID) {itsStorage.removeTaskStorage(taskID);}
	std::vector<storageResult> checkAssignedTaskStorage(Task *pTask, dataProductTypes dataProduct) {return itsStorage.checkAssignedTaskStorage(pTask, dataProduct);}
	storageLocationOptions getStorageLocationOptions(dataProductTypes dataProduct, const AstroDateTime &startTime, const AstroDateTime &endTime, const double &fileSize, const double &bandWidth, unsigned minNrFiles, sortMode sort_mode = SORT_NONE, const std::vector<int> &nodes = std::vector<int>()) {
		return itsStorage.getStorageLocationOptions(dataProduct, startTime, endTime, fileSize, bandWidth, minNrFiles, sort_mode, nodes);
	}
	const std::vector<storageResult> &getLastStorageCheckResult(void) const {return itsStorage.getLastStorageCheckResult();}
	void setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts) {itsStorage.setAllowedStorageHosts(allowedStorageHosts);}
	Task * moveTaskFromInactive(unsigned task_id);

private:
    unsigned getTaskID(unsigned task_id, bool override_SAS_taskIDs);
	void printUsedTaskIDs(void) const;
	// remove a task is inactive (finished, aborted, etc) from the stations
    void removeTaskFromStations(StationTask *task);
	const Task *getTaskByTaskID(unsigned task_id) const;
	const Task *getTaskBySASTreeID(quint32 sas_tree_id) const; // returns the task with the specified SAS id so the caller can edit its properties
	const Task *getTaskByMomID(quint32 momID) const;
	Task *getTaskForChangeByTaskID(unsigned task_id) const;
	Task *getTaskForChangeBySASTreeID(quint32 sas_tree_id) const;
	Task *getTaskForChangeByMomID(quint32 momID) const;
    Task *readTaskFromStream(QDataStream &in);
    void writeTaskToStream(QDataStream &out, Task *pTask) const;
	// withinPredecessorsRange: check if the task is still within predecessor range
	// return value:
	// -1: both the lower limit and upper limit have been reached (no movement possible)
	// 0: clear of both limits can move left (earlier) or right (later)
	// 1: lower limit reached, can move right (later)
	// 2: upper limit reached, can move left (earlier)
	short int withinPredecessorsRange(const Task *task, const AstroDateTime &new_start) const;

protected:
	// scheduler data objects
	// map conflicts will hold for every unscheduled task the possible conflicting (scheduled) task IDs.
	unscheduledTasksDeque unscheduledTasks; // contains pointers to the tasks that have not yet been scheduled
	scheduledTasksMap scheduledTasks; // contains the already scheduled tasks. Key = TaskId, Value = pointer to task
	reservationsMap itsReservations; // contains all reservations
	pipelinesMap itsPipelines; // contains all pipelines
    errorTasksMap errorTasks; // contains the tasks that have erroneous parameters. The second part contains the specific field which has errors
	inActiveTasksMap inactiveTasks; // contains 'unscheduleable' tasks with status: idle, described, prepared, aborted, finished, obsolete
	stationsMap itsStations; // contains the existing stations
	Storage itsStorage; // contains the existing storage nodes
	std::vector<unsigned> itsUsedTaskIDs; // keep a record of the used task IDs
	std::vector<unsigned> itsExistingSASTaskIDs; // keep a record of the task IDs that exist already in SAS
	unsigned itsPenalty; // the total penalty for this schedule
	bool itsSaveRequired, itsUploadRequired;
	Task nullTask;
};

#endif /* SCHEDULERDATABLOCK_H_ */
