/*
 * station.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jan 21, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/station.h $
 *
 */

#ifndef STATION_H_
#define STATION_H_

#include <map>
#include <vector>
#include <utility>
#include <fstream>
#include <QDataStream>
#include "lofar_utils.h"
#include "astrodatetime.h"

class Task;

// station antenna modes
enum station_antenna_mode {
	UNSPECIFIED_ANTENNA_MODE = 0,
	LBA_INNER,
	LBA_OUTER,
	LBA_SPARSE_EVEN,
	LBA_SPARSE_ODD,
	LBA_X,
	LBA_Y,
	HBA_ZERO,
	HBA_ZERO_INNER,
	HBA_ONE,
	HBA_ONE_INNER,
	HBA_DUAL,
	HBA_DUAL_INNER,
	HBA_JOINED,
	HBA_JOINED_INNER,
	ANTENNA_MODE_END
};

#define NR_ANTENNA_MODES	ANTENNA_MODE_END

// station clock frequencies
enum station_clock {
	UNSPECIFIED_CLOCK,
	clock_160Mhz,
	clock_200Mhz,
	END_STATION_CLOCK
};
#define NR_CLOCK_FREQUENCIES	END_STATION_CLOCK

// station filter types
enum station_filter_type {
	UNSPECIFIED_FILTER,
	LBA_10_70,
	LBA_30_70,
	LBA_10_90,
	LBA_30_90,
	HBA_110_190,
	HBA_170_230,
	HBA_210_250,
	FILTER_TYPE_END
};
#define NR_FILTER_TYPES	FILTER_TYPE_END

extern const char *antenna_modes_str[NR_ANTENNA_MODES];
extern const char *SAS_antenna_modes_str[NR_ANTENNA_MODES];
extern const char *clock_frequencies_str[NR_CLOCK_FREQUENCIES];
extern const char *filter_types_str[NR_FILTER_TYPES];
extern const char *SAS_filter_types_str[NR_FILTER_TYPES];


typedef std::pair<unsigned, std::pair<AstroDateTime, AstroDateTime > > stationTask;
typedef std::vector<stationTask> stationTasksVector;

class Station {
public:
	Station();
	Station(const Station &);
	Station(const std::string &name, unsigned id);
	virtual ~Station();

	Station &operator=(const Station &rhs);
	// returns the first available slot within the requested periods specified in the task.
	// returns false if no slot available. If true is returned then start_time and stop_time hold the
	//bool GetFirstAvailableSlot(Task *ptask, AstroDateTime &start_time, AstroDateTime &stop_time);

	friend QDataStream& operator<< (QDataStream &out, const Station &station);
	friend QDataStream& operator>> (QDataStream &in, Station &station); // used for reading data from binary file

	// Schedules the task with the given times.
    unsigned getStationID(void) const { return station_id; }
	void setName(const std::string & name) { itsName = name; }
	const std::string & getName(void) const { return itsName; }
	const stationTasksVector &getTasks(void) const {return itsTasks;}
	const char * getStationClockStr(station_clock clock);
	unsigned getStationClock(station_clock clock);
	bool addTasktoStation(unsigned task_id, const AstroDateTime &start, const AstroDateTime &end);
	bool moveTask(unsigned task_id, const AstroDateTime &start, const AstroDateTime &end);
	stationTask removeTask(unsigned taskID); // removes the task with the specified task ID and returns it when successful
    void removeAllTasks(void) {itsTasks.clear();}
	std::vector<unsigned> getTaskswithinTimeSpan(const AstroDateTime &start, const AstroDateTime &end) const;
	unsigned getTask(const AstroDateTime &start) const;

	bool findFirstOpportunity(const Task &task, AstroDateTime &first_possible, const AstroTime &min_time_between_tasks, unsigned reservation_id = 0);
	// find the first task that has a date later than date
	void sortTasks2EndTime(void); // sort the tasks vector according to the end times of the tasks
	void printAllTasks(void) const;

private:
	stationTasksVector::const_iterator lower_bound(const AstroDateTime &date) const;

	quint16 station_id;
	std::string itsName;
	stationTasksVector itsTasks;
};


// the following class is needed to be able to sort a vector of pointers to Task objects
// according to the priority attribute of the Task objects

class cmp_TaskEndTime
{
public:
	bool operator() (const stationTask &t1, const stationTask &t2) const
	{
		return t1.second.second < t2.second.second;
	}
};

extern station_clock stringToStationClockType(const std::string &str);

#endif /* STATION_H_ */
