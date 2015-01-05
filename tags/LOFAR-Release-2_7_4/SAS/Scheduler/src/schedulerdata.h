/*
 * schedulerdata.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 26, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulerdata.h $
 *
 */

#ifndef SCHEDULERDATA_H_
#define SCHEDULERDATA_H_

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include "schedulerdatablock.h"

// TODO: Why not include the headerfiles?
//        OR create a master include that includes all the pipelines in one go?
class Pipeline;
class CalibrationPipeline;
class ImagingPipeline;
class PulsarPipeline;
class LongBaselinePipeline;

class SchedulerData
{
public:
	SchedulerData();
	SchedulerData(unsigned short undo_levels);
	virtual ~SchedulerData();

	friend QDataStream& operator<< (QDataStream &out, const SchedulerData &data); // used for writing the data to binary file
	friend QDataStream& operator>> (QDataStream &in, SchedulerData &data); // used for reading binary scheduling project from file

	void cleanup(void); // clean up data objects
	void addUsedTaskID(unsigned int taskID) {itsData.addUsedTaskID(taskID);}
	void addUsedTaskIDs(const std::vector<unsigned> &taskIDs) {itsData.addUsedTaskIDs(taskIDs);}
	void setSASUsedTaskIDs(const std::vector<unsigned int> &task_ids) {itsData.setSASUsedTaskIDs(task_ids);}
	void updateUsedTaskIDs(void) {itsData.updateUsedTaskIDs();} // adds the task IDs currently used in the current schedulerDataBlock
    bool addTask(Task *task, bool check_ID_unused = true) {return itsData.addTask(task, check_ID_unused);} // adds a new task to the list of unscheduled tasks
//    bool addPipeline(Pipeline *pPipe, bool dont_check_free_id = false) {return itsData.addPipeline(pPipe, dont_check_free_id);}
    StationTask *newReservation(unsigned task_id, bool override_SAS_taskIDs = false) {return itsData.newReservation(task_id, override_SAS_taskIDs);} // create a new reservation with task ID equal to task_id, return pointer to task object or 0 when task_id already exists
    Pipeline *newPipeline(unsigned task_id, pipelineType type, bool override_SAS_taskIDs = false) {return itsData.newPipeline(task_id, type, override_SAS_taskIDs);} // create a new pipeline with task ID equal to task_id, return pointer to task object or 0 when task_id already exists
    Observation *newObservation(unsigned int task_id, bool override_SAS_taskIDs = false) {return itsData.newObservation(task_id, override_SAS_taskIDs);} // create a new task with task ID equal to task_id, return pointer to task or 0 when task_id already exists
    Task *newTask(unsigned int task_id, bool override_SAS_taskIDs = false) {return itsData.newTask(task_id, override_SAS_taskIDs);} // create a new task with task ID equal to task_id, return pointer to task or 0 when task_id already exists
    Task *newTask(unsigned int task_id, const OTDBtree &SAS_tree, bool override_SAS_taskIDs = false) {return itsData.newTask(task_id, SAS_tree, override_SAS_taskIDs);}
    // removeTask: remove a task from the scheduler and return it. CAUTION The task is not actually deleted from memory.
    // If you want to free the memory you should call delete on the returned task object
    Task *deleteTask(unsigned id, id_type IDtype = ID_SCHEDULER, bool erase = true) {return itsData.deleteTask(id, IDtype, erase);}
	bool getSaveRequired(void) {return itsData.getSaveRequired();} // controls if data changes need to be saved
	bool getUploadRequired(void) {return itsData.getUploadRequired();}
	bool getNextTask(Task &task) {return itsData.getNextTask(task);} // returns true if the next unscheduled task was found. parameter task contains the task
	bool scheduleNextTask(AstroDateTime const &schedule_time) {return itsData.scheduleNextTask(schedule_time);} // schedules the next unscheduled task (which can be obtained with getNextTask
	bool scheduleTask(Task * pTask) { return itsData.scheduleTask(pTask); }
	bool moveTaskToInactive(unsigned taskID) {return itsData.moveTaskToInactive(taskID);}
	Task * moveTaskFromInactive(unsigned task_id) {return itsData.moveTaskFromInactive(task_id);}
    std::vector<unsigned> moveTask(Task *pTask, const AstroDateTime &new_start, bool unschedule_conflicting_tasks) {return itsData.moveTask(pTask, new_start, unschedule_conflicting_tasks); } // moves the task to the new start time (always succeeds) throwing out conflicting tasks
    bool shiftTask(unsigned int task_id, bool unschedule_conflicting_tasks) {return itsData.shiftTask(task_id, unschedule_conflicting_tasks);} // shifts the task over its duration in its current preferred direction if possible
	bool rescheduleTask(unsigned int task_id, const AstroDateTime &new_start_time) {return itsData.rescheduleTask(task_id, new_start_time);} // tries to reschedule the task at the given new start time
	bool rescheduleAbortedTask(unsigned task_id, const AstroDateTime & new_start) {return itsData.rescheduleAbortedTask(task_id, new_start);} // reschedules an aborted task at the first opportunity in the future
	void holdNextTask(const std::string & reason) {itsData.holdNextTask(reason);} // puts the next unscheduled task at the end of the unscheduled task list
	void tryScheduleUnscheduledTasks(void) {itsData.tryScheduleUnscheduledTasks();} // iterates over all unscheduled tasks to try and schedule them
	// the following change.. functions change a task's schedule times without checking conflicts with other tasks
	bool changeTaskStartTime(unsigned task_id, const AstroDateTime &new_start) {return itsData.changeTaskStartTime(task_id, new_start);}
	bool changeTaskEndTime(unsigned task_id, const AstroDateTime &new_end) {return itsData.changeTaskEndTime(task_id, new_end);}
	bool changeTaskDuration(unsigned task_id, const AstroTime &new_duration) {return itsData.changeTaskDuration(task_id, new_duration);}
	bool changeTaskSchedule(unsigned task_id, const AstroDateTime &new_start, const AstroDateTime &new_end) {return itsData.changeTaskSchedule(task_id, new_start, new_end);}
	void scheduleFixedTasks(void) {itsData.scheduleFixedTasks();} // schedules tasks marked with fixed day or fixed time
	void updateTasksStationIDs(void) {itsData.updateTasksStationIDs();}
	bool undo(bool store_redo = true); // undo the last operation on the scheduler data
	bool redo(void); // redo the last undo operation
	void create_undo_level(void) {itsUndoStack.push_back(itsData);}
	void deleteLastStoredUndo(void) {itsUndoStack.pop_back();}
    const Storage &getStorage(void) const {return itsData.getStorage();}

	// get methods
    std::vector<unsigned> tasksInGroup(unsigned groupID, selector_types type = SEL_ALL_TASKS) const {return itsData.tasksInGroup(groupID, type);}
	const SchedulerDataBlock & getCurrentSchedule(void) const { return itsData;}
	size_t getNrScheduled(void) const { return itsData.getNrScheduled();}
	size_t getNrUnscheduled(void) const {return itsData.getNrUnscheduled();}
	size_t getNrReservations(void) const {return itsData.getNrReservations();}
	size_t getNrInactive(void) const {return itsData.getNrInactive();}
	size_t getNrPipelines(void) const {return itsData.getNrPipelines();}
	const unscheduledTasksDeque &getUnscheduledTasks(void) const { return itsData.getUnscheduledTasks(); }
	const scheduledTasksMap &getScheduledTasks(void) const {return itsData.getScheduledTasks();}
	const reservationsMap &getReservations(void) const {return itsData.getReservations();}
	const pipelinesMap &getPipelineTasks(void) const {return itsData.getPipelineTasks();}
    StationTask *getScheduledTask(unsigned task_id) {return itsData.getScheduledTask(task_id);}
    const StationTask *getScheduledTask(unsigned task_id) const { return itsData.getScheduledTask(task_id); }
    const inActiveTasksMap &getInactiveTasks(void) const {return itsData.getInactiveTasks();}
    std::vector<Task *> getReservationsVector(void) const {return itsData.getReservationsVector();}
    std::vector<Task *> getMaintenanceVector(void) const {return itsData.getMaintenanceVector();}
	std::vector<Task *> getScheduledTasksVector(void) const {return itsData.getScheduledTasksVector();} // no pipelines
	std::vector<Task *> getInactiveTaskVector(void) const {return itsData.getInactiveTaskVector();}
	std::vector<Task *> getUnScheduledTasksVector(void) const {return itsData.getUnScheduledTasksVector();}
    std::vector<Task *> getPipelinesVector(void) const {return itsData.getPipelinesVector();}
    std::vector<Pipeline *> getScheduledPipelinesVector(void) const {return itsData.getScheduledPipelinesVector();}
    const Task *getTask(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getTask(taskID, IDtype);}
    const StationTask *getReservation(unsigned reservation_id) const { return itsData.getReservation(reservation_id); }
    const Task *getInactiveTask(unsigned task_id) const { return itsData.getInactiveTask(task_id); }
    const Task *getUnscheduledTask(unsigned task_id) const { return itsData.getUnscheduledTask(task_id); }
    const StationTask *getStationTask(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getStationTask(taskID, IDtype);}
    const Observation *getObservation(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getObservation(taskID, IDtype);}
    const Pipeline *getPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getPipeline(taskID, IDtype);}
    const CalibrationPipeline *getCalibrationPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getCalibrationPipeline(taskID, IDtype);}
    const ImagingPipeline *getImagingPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getImagingPipeline(taskID, IDtype);}
    const PulsarPipeline *getPulsarPipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getPulsarPipeline(taskID, IDtype);}
    const LongBaselinePipeline *getLongBaselinePipeline(unsigned taskID, id_type IDtype = ID_SCHEDULER) const {return itsData.getLongBaselinePipeline(taskID, IDtype);}

    const Observation *getScheduledObservation(unsigned task_id) const { return itsData.getScheduledObservation(task_id); }
	Task *getTaskForChange(unsigned task_id, id_type IDtype = ID_SCHEDULER) { return itsData.getTaskForChange(task_id, IDtype); }
    StationTask *getReservationForChange(unsigned task_id) { return itsData.getReservationForChange(task_id); }
    Pipeline *getPipelineForChange(unsigned task_id) { return itsData.getPipelineForChange(task_id); } // returns the task with id equal to task_id so the caller can edit its properties
	std::vector<Task *> getTasksInScheduleSortStartTime(void) const {return itsData.getTasksInScheduleSortStartTime();}
	std::vector<Task *> getScheduledTasksSortStartTime(void) const {return itsData.getScheduledTasksSortStartTime();}
	std::vector<Task *> getScheduledObservationsSortStartTime(void) const {return itsData.getScheduledObservationsSortStartTime();}
	std::vector<Task *> getPreScheduledObservationsSortStartTime(void) const {return itsData.getPreScheduledObservationsSortStartTime();}
	std::vector<Task *> getPreScheduledTasksSortStartTime(void) const {return itsData.getPreScheduledTasksSortStartTime();}
    std::vector<Observation *> getFutureObservationsSortStartTime(void) const {return itsData.getFutureObservationsSortStartTime();}
	std::vector<Task *> getUnScheduledTasksSortFirstDate(void) const {return itsData.getUnScheduledTasksSortFirstDate();}
    std::map<unsigned, std::vector<Task *> > getGroupedTasks(Task::task_status state) const {return itsData.getGroupedTasks(state);} // key = groupID
    std::map<unsigned, std::vector<Task *> > getGroupedObservations(Task::task_status state = Task::TASK_STATUS_END) const {return itsData.getGroupedObservations(state);} // key = groupID
	const std::map<std::string, std::vector<Task *> > getPublishTasks(const AstroDateTime &start, const AstroDateTime &end) const {return itsData.getPublishTasks(start, end);}
	const Station *getStation(unsigned int station_id) const {return itsData.getStation(station_id);}
	const stationsMap &getStations(void) const {return itsData.getStations();}
    size_t getNrTasks() const { return itsData.getNrTasks(); } // number of columns equals the number of data headers
	std::vector<std::string> const & getDataHeaders(void) const { return itsDataHeaders; } // the data headers
	unsigned int calcCurrentPenalty(void) { return itsData.calculatePenalty(); }
	unsigned int getPenalty(void) const {return itsData.getPenalty();}
//	unsigned int getCurrentDataLevel(void) {return current_data_level;}
	const errorTasksMap &getErrorTasks(void) const {return itsData.getErrorTasks();}
	bool newErrorTasks(void) {return itsData.newErrorTasks();}
	void clearError(unsigned int taskID, data_headers header) { itsData.clearError(taskID, header);}
	void markErrorTasksStatus(void) { itsData.markErrorTasksStatus(); }
	size_t nrOfErrorTasks(void) const { return itsData.nrOfErrorTasks(); }
	bool stationExist(const std::string &stationName) const;// { return itsData.stationExist(stationID); }
//	unsigned int getNewTaskID(const std::vector<unsigned> &exclude_IDs) const { return itsData.getNewTaskID(exclude_IDs); };
	unsigned int getNewTaskID(bool override_SAS_taskIDs = false) const { return itsData.getNewTaskID(override_SAS_taskIDs); }
	inline bool isTaskIDFree(unsigned taskID) const {return itsData.isTaskIDFree(taskID);}
	void checkStatusChanges(void) {itsData.checkStatusChanges();}
    bool checkStationConflicts(StationTask *pTask) {return itsData.checkStationConflicts(pTask);}
    unscheduled_reasons checkTaskStations(StationTask *pTask) {return itsData.checkTaskStations(pTask);} // check stations of the given task

	//set methods
	bool addStation(unsigned int station_id, const std::string &name) { return itsData.addStation(station_id, name); }
	void setChangesMadeFlag(void) {itsData.setChangesMadeFlag();}
	void clearChangesMadeFlag(void) {itsData.clearChangesMadeFlag();}
	void clearUploadRequiredFlag(void) {itsData.clearUploadRequiredFlag();}
	bool setUndoLevels(unsigned short nr_levels);
	void setDataHeaders(std::vector<std::string> const &headers);
	void moveUnscheduledTask(unsigned int from, unsigned int to) { itsData.moveUnscheduledTask(from, to); }
	void setCurrentSchedule(SchedulerDataBlock &new_schedule) {itsData = new_schedule; }
    void sortUnscheduledTasks2Priority() { itsData.sortUnscheduledTasks2Priority(); } // sort the unscheduled tasks according to priority
	bool unscheduleTask(unsigned int task_id) {return itsData.unscheduleTask(task_id);}
	bool unscheduleReservation(unsigned reservation_id) {return itsData.unscheduleReservation(reservation_id);} // unschedule a reservation (putting it ON HOLD and removing it from the stations
	void unscheduleAll(void) { itsData.unscheduleAll(); }
	void setTaskStationIDs(Task *task); // sets the correct station IDs in the task
	void updateStations(void) {itsData.updateStations();}
	void initStorage(void) {itsData.initStorage();}
	void clearStorageClaims(void) {itsData.clearStorageClaims();}
	std::vector<storageResult> addStorageToTask(Task *pTask, const storageMap &storageLocations) {return itsData.addStorageToTask(pTask, storageLocations);}
	std::vector<storageResult> addStorageToTask(Task *pTask, dataProductTypes dataProduct, const storageVector &storageLocations, bool noCheck = false) {
		return itsData.addStorageToTask(pTask, dataProduct, storageLocations, noCheck);
	}
	void removeStorageForTask(unsigned taskID) {itsData.removeStorageForTask(taskID);}
	std::vector<storageResult> checkAssignedTaskStorage(Task *pTask, dataProductTypes dataProduct) {return itsData.checkAssignedTaskStorage(pTask, dataProduct);}
	storageLocationOptions getStorageLocationOptions(dataProductTypes dataProduct, const AstroDateTime &startTime, const AstroDateTime &endTime, const double &fileSize, const double &bandWidth, unsigned minNrFiles, sortMode sort_mode = SORT_NONE, const std::vector<int> &nodes = std::vector<int>()) {
		return itsData.getStorageLocationOptions(dataProduct, startTime, endTime, fileSize, bandWidth, minNrFiles, sort_mode, nodes);
	}
	const std::vector<storageResult> &getLastStorageCheckResult(void) const {return itsData.getLastStorageCheckResult();}
	void setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts) {itsData.setAllowedStorageHosts(allowedStorageHosts);}

	// task checking methods
	bool checkTasksForErrors(void) {return itsData.checkTasksForErrors();} // check all tasks for all errors
	bool predecessorExists(unsigned int predID) const {return itsData.predecessorExists(predID);}
	bool checkTaskBoundaries(const Task &task) const {return itsData.checkTaskBoundaries(task);}
	bool checkTask(Task *pTask) {return itsData.checkTask(pTask);}
	//checks and possibly fixes tasks that have non-existing predecessors
	bool findFirstOpportunity(const Task *pTask, AstroDateTime &start_time, unsigned reservation_id = 0) {return itsData.findFirstOpportunity(pTask, start_time, reservation_id);}

private:
	SchedulerDataBlock itsData;
	std::vector<SchedulerDataBlock> itsUndoStack;
	std::vector<SchedulerDataBlock> itsRedoStack;
	std::vector<std::string> itsDataHeaders;
};

inline QDataStream& operator<< (QDataStream &out, const SchedulerData &data)
{
	out << data.itsData;
    return out;
}

inline QDataStream& operator>> (QDataStream &in, SchedulerData &data)
{
	in >> data.itsData;
	data.updateUsedTaskIDs();
    return in;
}

#endif /* SCHEDULERDATA_H_ */

