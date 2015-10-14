/*
 * station.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jan 21, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/station.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "station.h"
#include "task.h"
#include "Controller.h"
#include <map>
#include <vector>
#include <cmath>
#include <algorithm>
using std::vector;
using std::map;
#ifdef DEBUG_SCHEDULER
#include <iostream>
#endif

const char *antenna_modes_str[NR_ANTENNA_MODES] = {"UNSPECIFIED", "LBA INNER",
		"LBA OUTER", "LBA SPARSE EVEN", "LBA SPARSE ODD", "LBA X", "LBA Y", "HBA ZERO", "HBA ZERO INNER", "HBA ONE", "HBA ONE INNER", "HBA DUAL", "HBA DUAL INNER", "HBA JOINED", "HBA JOINED INNER"};

const char *SAS_antenna_modes_str[NR_ANTENNA_MODES] = {"","LBA_INNER",
	"LBA_OUTER", "LBA_SPARSE_EVEN", "LBA_SPARSE_ODD", "LBA_X", "LBA_Y", "HBA_ZERO", "HBA_ZERO_INNER", "HBA_ONE", "HBA_ONE_INNER", "HBA_DUAL", "HBA_DUAL_INNER", "HBA_JOINED", "HBA_JOINED_INNER"};

const char *clock_frequencies_str[NR_CLOCK_FREQUENCIES] = {"UNSPECIFIED","160MHz","200MHz"};

const char *filter_types_str[NR_FILTER_TYPES] = {"UNSPECIFIED", "LBA 10-70MHz", "LBA 30-70MHz",
		"LBA 10-90MHz", "LBA 30-90MHz", "HBA 110-190MHz", "HBA 170-230MHz", "HBA 210-250MHz"};

const char *SAS_filter_types_str[NR_FILTER_TYPES] = {"", "LBA_10_70", "LBA_30_70",
		"LBA_10_90", "LBA_30_90", "HBA_110_190", "HBA_170_230", "HBA_210_250"};

station_clock stringToStationClockType(const std::string &str) {
	if (!str.empty()) {
		for (short i = 0; i < END_STATION_CLOCK; ++i) {
			if (str.compare(clock_frequencies_str[i]) == 0) {
				return static_cast<station_clock>(i);
			}
		}
	}
	return static_cast<station_clock>(0);
}

Station::Station() :
	station_id(0)
{

}

Station::Station(const std::string &name, unsigned id) :
	station_id(id), itsName(name)
{

}

Station::Station(const Station &other) :
	station_id(other.getStationID()), itsName(other.getName()), itsTasks(other.getTasks())
{
}

Station::~Station()
{
}

Station & Station::operator=(const Station &rhs) {
	station_id = rhs.getStationID();
	itsName = rhs.getName();
	itsTasks = rhs.getTasks();
	return *this;
}

QDataStream& operator<< (QDataStream &out, const Station &station) { // used for writing to binary file
	out << (quint16) station.station_id
	    << station.itsName
	    << (quint32) station.itsTasks.size(); // number of tasks
	for (stationTasksVector::const_iterator it = station.itsTasks.begin(); it != station.itsTasks.end(); ++it) {
		out << (quint32) it->first << it->second.first << it->second.second;
	}
	return out;
}

QDataStream& operator>> (QDataStream &in, Station &station) {
	station.itsTasks.clear();
	quint32 nrOfTasks;
	quint32 taskID;
	AstroDateTime dateTime;
	std::pair<AstroDateTime, AstroDateTime > timePair;
	in >> station.station_id // station ID
	   >> station.itsName
	//read the numer of tasks assigned to this station
	   >> nrOfTasks;
	for (quint32 i = 0; i < nrOfTasks; ++i) {
		in >> taskID;
		in >> dateTime;
		timePair.first = dateTime;
		in >> dateTime;
		timePair.second = dateTime;
		station.itsTasks.push_back(
				std::pair<unsigned, std::pair<AstroDateTime, AstroDateTime > >(taskID, timePair)
		);
	}
	return in;
}

/*
bool Station::getConflicts(const AstroDateTime &request_start, const AstroDateTime &request_end, vector<unsigned> &conflict_ids)
{
	bool bConflicts = false;
	map<AstroDateTime, unsigned>::iterator it;
	if (it = startTimes.lower_bound(request_start) != startTimes.end()) {
		AstroDateTime end_confl_task = endTimes.find(it.second);
		if (end_confl_task < request_end + MIN_TIME_BETWEEN_TASKS) {
			conflict_ids.push_back(it.second);
			bConflicts = true;
		}
	}
	if (it = endTimes.lower_bound(request_start) != startTimes.end()) {
		AstroDateTime start_confl_task = startTimes.find(it.second);
		if (start_confl_task < request_end + MIN_TIME_BETWEEN_TASKS) {
			conflict_ids.push_back(it.second);
			bConflicts = true;
		}
	}
	return bConflicts;
}
*/

stationTasksVector::const_iterator Station::lower_bound(const AstroDateTime &date) const
{
	// binary search for first task that has an end time later than the time specified in date
	stationTasksVector::const_iterator it = itsTasks.begin();
	if (itsTasks.size() > 1) { // more than one element?
		unsigned iu = itsTasks.size()-1, il = 0, i = 0;
		it = it + (iu / 2);
		while (true) {
			if (it->second.second > date) {
				iu = (il + iu) / 2;
			}
			else {
				il = (il + iu) / 2;
			}
			i = (il + iu) / 2;
			it = itsTasks.begin() + i;
			if ((iu - il) <= 1) { // check stop conditions
				if (itsTasks.at(il).second.second >= date) {
					it = itsTasks.begin() + il;
					return it;
				}
				else if (itsTasks.at(iu).second.second >= date) {
					it = itsTasks.begin() + iu;
					return it;
				}
				else {
					return itsTasks.end();
				}
			}
		}
	}
	else if (itsTasks.size() == 1) { // only one element in itsTasks vector
		if (itsTasks.at(0).second.second >= date) {
			return itsTasks.begin();
		}
		else {
			return itsTasks.end();
		}
	}

	return itsTasks.end();
}


bool Station::findFirstOpportunity(const Task &task, AstroDateTime &first_possible, const AstroTime &min_time_between_tasks, unsigned reservation_id)
{
	bool result(true), stop(false);
	unsigned sday = first_possible.getDate().toJulian();
	unsigned last_day = task.getWindowLastDay().toJulian();
	unsigned first_day = task.getWindowFirstDay().toJulian();
	double stime = first_possible.toJulian() - sday;
	double mintime = task.getWindowMinTime().toJulian();
	double maxtime = task.getWindowMaxTime().toJulian();

	// do some checks on the start time and date
	if (sday > last_day) {
		first_possible = 0.0; // AstroDateTime();
		result = false;
	}
	else if (sday < first_day) {
		sday = first_day;
		stime = mintime;
	}
	if ((stime < mintime) & (sday <= first_day)) {
		stime = mintime;
		sday = first_day;
	}
	if ((stime > maxtime) & (sday >= last_day)) {
		first_possible = 0.0; //AstroDateTime();
		result = false;
	}

	//	printAllTasks();
	// if the task is part of a reservation the ignore that reservation while finding a time slot
	// we do this by temporarily removing the reservation from the task list
	stationTask reservation;
	if (reservation_id) {
		reservation = removeTask(reservation_id);
	}
    else {
        if (task.isObservation()) {
            const Observation *pTask = static_cast<const Observation *>(&task);
            if ((reservation_id = pTask->getReservation())) {
                reservation = removeTask(reservation_id);
            }
        }
    }

	if (result & !itsTasks.empty()) {
		for (unsigned day = sday; day <= last_day; ++day)
		{
			AstroDateTime first_time(day, stime); // first possible time at this day
			AstroTime max_time = task.getWindowMaxTime();
			AstroTime duration = task.getDuration();
			stationTasksVector::const_iterator first_later_task = lower_bound(first_time);
			AstroDateTime projected_start, projected_end;

			if (first_later_task != itsTasks.end()) { // found the first task that has an end time later than the first possible time on this day
				// check if the task we found still ends on this day
				if (first_later_task->second.second.getDate().toJulian() == static_cast<int>(day)) {
					// check if the task can be scheduled at first_time
					if (first_time + duration + min_time_between_tasks < first_later_task->second.first) {
						first_possible = first_time;
						result = true;
						break;
					}
					// possibly the start time of the task found is also later than the first possible time. Check if the new task fits before this task
					projected_start = first_later_task->second.first - static_cast<AstroDateTime>(min_time_between_tasks) - static_cast<AstroDateTime>(duration);
					if (projected_start >= first_time) {
						// still enough distance between the projected start and the previous task?
						if (projected_start - static_cast<AstroDateTime>(min_time_between_tasks) >= (first_later_task-1)->second.second) {
							first_possible = projected_start;
							result = true;
							break;
						}
					}
					while (true) { // here we start checking for a scheduling space after the task found by lower_bound
						projected_start = first_later_task->second.second + static_cast<AstroDateTime>(min_time_between_tasks);
						projected_end = projected_start + static_cast<AstroDateTime>(duration);
						if (projected_end.getDate() <= task.getWindowLastDay()) {
							if (task.getWindowMinTime() + task.getDuration() < task.getWindowMaxTime()) { // if the task can fit within one day time window
								// then also check if the projected end time is smaller than the specified end time of the time window
								if (projected_end > max_time) { // on this day the task doesn't fit the end time requirement of the time window
									break; // goto next day in while loop
								}
							}
							if (first_later_task+1 != itsTasks.end()) { // if there is a next task:
								// check if gap between the two tasks is big enough to fit this task in between
								AstroDateTime tmp = projected_end + static_cast<AstroDateTime>(min_time_between_tasks);
								if ((first_later_task+1)->second.first >=  tmp)
								{ // yes, gap is big enough! schedule the task after the task found.
									first_possible = projected_start;
									result = true;
									stop = true;
									break;
								}
								++first_later_task; // check gaps between next tasks until we run of this day
							}
							else { // the task found was the last task. try to schedule the requested task after this last task.
								// check if the requested task can still be scheduled on this day or has to be scheduled on the next
								first_possible = projected_start;
								result = true;
								stop = true;
								break;
							}
						}
						else { // end time or date is beyond time window for this task
							first_possible = 0.0;
							result = false;
							stop = true;
							break;
						}
					}
					if (stop) break;
				}
				else { // the first task we found with an end time later than our requested first day has an end time on a later day, we can check if the current task fits before the task found.
					if (first_time + static_cast<AstroDateTime>(min_time_between_tasks) + static_cast<AstroDateTime>(duration) <= first_later_task->second.first) { // fits before?
						first_possible = first_time;
						result = true;
						stop = true;
						break;
					}
				}
			}
			else { // no tasks found with an end time later than the time requested, but there could be tasks
				// so make sure we meet the minimum distance requirements from the last task in the vector
				first_possible = std::max(itsTasks.back().second.second + static_cast<AstroDateTime>(min_time_between_tasks), first_time);
				result = true;
				stop = true;
				break;
			}
			if (stop) break;
		}
	}
	else { // no tasks found, the task list is still empty for this station
		first_possible = sday + stime;
		result = true;
	}

	// put back the temporarily removed reservation task
	if (reservation_id) {
		addTasktoStation(reservation_id, reservation.second.first, reservation.second.second);
	}

	return result;
}

void Station::printAllTasks(void) const {
	std::cout << "All tasks for station " << station_id << ":";
	if (!itsTasks.empty()) {
		std::cout << std::endl;
		for (stationTasksVector::const_iterator it=itsTasks.begin(); it != itsTasks.end(); ++it) {
			std::cout << it->first << ": start: " << it->second.first.toString() << ", end: " << it->second.second.toString() << std::endl;
		}
	}
	else {
		std::cout << "no tasks." << std::endl;
	}
}


void Station::sortTasks2EndTime()
{
	sort(itsTasks.begin(), itsTasks.end(), cmp_TaskEndTime());
}


bool Station::addTasktoStation(unsigned task_id, const AstroDateTime &start, const AstroDateTime &end)
{
	// Insert task in the vector tasks and sort the vector on the end times
	for (stationTasksVector::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		if (it->first == task_id) {
//#ifdef DEBUG_SCHEDULER
//			std::cerr << "Station::addTasktoStation: tried to schedule a task which is already scheduled on this station." << std::endl;
//#endif
//			return false;
			// already scheduled on this station.
			// this happens e.g. when the task goes from PRESCHEDULED to SCHEDULED state
			// only update the start and end time
			it->second.first = start;
			it->second.second = end;
			sortTasks2EndTime();
			return true;
		}
	}
	std::pair<AstroDateTime, AstroDateTime> times = std::pair<AstroDateTime, AstroDateTime>(start,end);
	stationTask task =
		(
			stationTask(task_id, times)
		);
	itsTasks.push_back(task);
	sortTasks2EndTime();
	return true;
}

bool Station::moveTask(unsigned task_id, const AstroDateTime &start, const AstroDateTime &end)
{
	for (stationTasksVector::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		if (it->first == task_id) {
			it->second = std::pair<AstroDateTime, AstroDateTime>(start,end);
			sortTasks2EndTime();
			return true;
		}
	}
	return false;
}

/*
std::vector<unsigned> Station::getParallelTasks(const AstroDateTime &req_start, const AstroDateTime &req_end) {
	std::vector<unsigned> taskIDs;
	stationTasksVector::const_iterator first_later_task = lower_bound(req_start);
	// while not yet at end of task vector AND
	// while (start time of first_later_task <= requested start end) AND (end time of first_later_task >= requested start)
	// then the task pointed to by first_later_task iterator overlaps with the period between requested start and end time.
	while (first_later_task++ != itsTasks.end() && (first_later_task->second.first <= req_end && first_later_task->second.second >= req_start)) {
		taskIDs.push_back(first_later_task->first);
	}
}
*/


stationTask Station::removeTask(unsigned task_id) {
	stationTask removed_task;
	for (stationTasksVector::iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
		if (it->first == task_id) {
			removed_task.first = task_id;
			removed_task.second = it->second;
			itsTasks.erase(it);
			return removed_task;
		}
	}
	return removed_task;
}
/*
void Station::removeTask(unsigned taskID) {
	if (!itsTasks.empty()) {
		// find the task with the specified taskID
		for (stationTasksVector::iterator i = itsTasks.begin(); i != itsTasks.end(); ++i) {
			if (i->first == taskID) {
				itsTasks.erase(i);
				return;
			}
		}
	}
}
*/

std::vector<unsigned> Station::getTaskswithinTimeSpan(const AstroDateTime &start, const AstroDateTime &end) const {
	std::vector<unsigned> tasks;
	AstroTime min_time_between_tasks(Controller::theSchedulerSettings.getMinimumTimeBetweenTasks());
//	std::cout << "getting tasks from station:" << itsName << std::endl
//	<< " within timespan " << start.toString() << " - " << end.toString() << std::endl;
	for (stationTasksVector::const_iterator it = itsTasks.begin(); it != itsTasks.end(); ++it) {
//		std::cout << "task:" << it->first << ", start: " << it->second.first.toString() << ", end: " << it->second.second.toString() <<std::endl;
//		std::cout << "test 1:" << static_cast<int>(it->second.first < end + min_time_between_tasks) << ", test 2:" << static_cast<int>(it->second.second > start - min_time_between_tasks) << std::endl;
		if ((it->second.first < end + min_time_between_tasks) & (it->second.second > start - min_time_between_tasks)) {
			tasks.push_back(it->first);
		}
	}
	return tasks;
}

unsigned Station::getTask(const AstroDateTime &start) const {
	stationTasksVector::const_iterator cit = lower_bound(start);
	if (cit != itsTasks.end()) {
		return cit->first;
	}
	else return 0;
}


const char * getStationClockStr(station_clock clock) {
	if (clock >= 0 && clock <= NR_CLOCK_FREQUENCIES-1) {
		return clock_frequencies_str[clock];
	}
	return "UNKOWN";
}

unsigned getStationClock(station_clock clock) {
	if (clock == clock_160Mhz) return 160;
	else if (clock == clock_200Mhz) return 200;
	else {
		debugErr("s","Unsupported clock frequency!");
		return 0;
	}
}

const char * getAntennaModeStr(station_antenna_mode mode) {
	if (mode >= 0 && mode <= NR_ANTENNA_MODES-1) {
		return antenna_modes_str[mode];
	}
	return "UNKNOWN";
}


