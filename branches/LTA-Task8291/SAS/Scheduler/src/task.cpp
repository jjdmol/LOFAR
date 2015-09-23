/*
 * task.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jan 21, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/task.cpp $
 *
 */


#include "lofar_scheduler.h"
#include "lofar_utils.h"
#include "task.h"
#include "Controller.h"
#include "SASConnection.h"
#include <QSqlRecord>
#include <sstream>
#include <vector>
#include <string>
#include <stdio.h>
#include <cmath>
using std::vector;
using std::string;
#ifdef DEBUG_SCHEDULER
#include <iostream>
#endif

const char *task_types_str[NR_TASK_TYPES] = {"OBSERVATION","RESERVATION","MAINTENANCE","PIPELINE","SYSTEM","UNKNOWN"};

const char *task_states_str[NR_TASK_STATES] = {"IDLE", "ERROR", "DESCRIBED", "PREPARED", "ON_HOLD",
			"UNSCHEDULED", "CONFLICT", "PRESCHEDULED", "SCHEDULED", "STARTING", "ACTIVE", "COMPLETING", "FINISHED", "ABORTED", "OBSOLETE"};

const char *unscheduled_reason_str[NR_REASONS] = {
		"",
        "Warning!",
		"Task does does not fulfill reservation constraints",
		"The digital beam duration differs from the observation duration",
		"Task is scheduled in the past or too close to now",
		"Task has conflicts with other tasks",
		"Task duration is zero",
		"Task start time is not set",
		"Task end time is not set",
		"Task has empty subband list",
		"Task has too many subbands",
		"Unknown bit mode",
		"Task is outside schedule bounds",
		"No predecessor defined",
		"Predecessor(s) not found or does not exist",
		"Maximum distance to predecessor exceeded",
		"Predecessor is not scheduled",
		"Predecessor minimum distance later than maximum distance",
		"Start time is before or too close to the predecessor task",
		"first date too late for predecessor to fit before",
		"Last possible date too early to maintain distance to predecessor",
		"Planned start later than end",
		"Empty station list",
		"Task uses non-existing station(s)",
		"Forbidden station combination for HBA1",
		"No opening found",
		"Task is on hold",
		"Task has errors",
		"Antenna mode unspecified",
		"Station clock unspecified",
		"Station filter unspecified",
		"Incompatible station clock and antenna mode",
		"Incompatible station clock and filter type",
		"Task specification incomplete",
		"No incoherent TABs defined while incoherent data output is enabled",
		"No coherent TABs defined while coherent data output is enabled",
		"No TABs defined and fley's eye is off while in Stokes mode",
		"Channels per subband no integer multiple of coherent/incoherent channels per subband",
		"skyImages = input MS subbands / (subbands_per_image * slices_per_image) (needs to be integer value)",
		"No data output type selected (coherent/incoherent/correlated)",
		"A required input data product could not be found within the predecessor tasks",
        "demix freq.step and time step should be multiples of resp. averaging freq.step and time step",       
        "one of the specified sources to be demixed is unknown",
        "Mismatch between input and output node location, retry assigning resources: LOC 1",
        "Mismatch between input and output node location, retry assigning resources: LOC 2",
};

const char * TASK_CONFLICTS[_END_CONFLICTS_] = {
		"no conflicts", //NO_CONFLICT
		"bit mode cannot be different", // BITMODE
		"no more data slots available", // OUT_OF_DATASLOTS
		"Station is in maintenance", // MAINTENANCE
		"Station is reserved", // RESERVATION
        "Station in use by other observation", // STATIONS
		"Not enough total free storage space", // INSUFFICIENT_STORAGE
		"No data output for this task", // NO_DATA
		"Not enough available storage nodes for required bandwidth", // CONFLICT_STORAGE_EXCEEDS_BANDWIDTH
		"Storage node inactive", // STORAGE_NODE_INACTIVE
		"Storage node does not exist", // CONFLICT_STORAGE_NODE_INEXISTENT
		"No storage nodes available", // CONFLICT_STORAGE_NO_NODES
		"Number of storage nodes available less than minimum required", // CONFLICT_STORAGE_TOO_FEW_NODES
		"Number of assigned storage nodes is less than minimum required", // CONFLICT_STORAGE_MINIMUM_NODES
		"No suitable storage options found", // NO_OPTIONS
		"Storage raid array has insufficient free space", // STORAGE_NODE_SPACE
		"Exceeds maximum write speed for raid array", // STORAGE_WRITE_SPEED
		"Raid array not found", // RAID_ARRRAY_NOT_FOUND
		"Requested start time in the past", // STORAGE_TIME_TOO_EARLY
		"Network bandwidth to storage node too high", // STORAGE_NODE_BANDWIDTH
		"Bandwidth required for single file too high", // CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH
		"Storage auto selection modes are different within group", // CONFLICT_GROUP_STORAGE_MODE_DIFFERENT
		"Grouped tasks have unequal number of data products", // CONFLICT_GROUP_STORAGE_UNEQUAL_DATAPRODUCTS
		"No storage nodes assigned to some data products", // NO_STORAGE_ASSIGNED
		"skyImages = input MS subbands / (subbands_per_image * slices_per_image) (needs to be integer value)" // NON_INTEGER_SKYIMAGES
};

const char *SAS_item_names[NR_TASK_PROPERTIES] = {"task_id", "antenna mode", "clock", "contact e-mail", "contact name", "contact phone",
            "data slots", "filter type", "first possible day", "fixed day", "fixed time", "last possible day",
			"nighttime weightfactor", "pred.max.time dif.", "pred.min.time dif.", "predecessor", "priority",
			"project ID", "group ID", "reservation", "scheduled end", "scheduled start", "stations",
			"duration",	"task name", "status", "type", "window max.time", "window min.time", "analog beam settings",
			"digital beam settings", "tied array beam settings", "bandpass correction", "delay comp.",
			"flys eye", "Coherent dedispersion", "coherent stokes settings", "incoherent stokes settings",
			"correlator int. time", "bits per sample", "channels per subband", "slices_per_image", "subbands_per_image",
            "imaging specify fov", "imaging fov", "imaging npix", "imaging cellsize", "subbands_per_subbandgroup", "subbandgroups_per_MS",
            "demix always", "demix_if_needed", "demix_skyModel", "demix_freqstep", "demix_timestep",
            "avg.freqstep", "avg.timestep", "cal_skyModel", "pulsar pipeline settings", "enabled output data types",
            "output storage settings", "output data products", "TBB piggyback", "Aartfaac piggyback", "input data products", "description"};

const char * DATA_TYPES[_END_DATA_TYPES] = {"I", "IQUV", "XXYY", "UNDEFINED"};

dataTypes stringToStokesType(const std::string &str) {
	if (!str.empty()) {
		for (short i = 0; i < DATA_TYPE_UNDEFINED; ++i) {
			if (str.compare(DATA_TYPES[i]) == 0) {
				return static_cast<dataTypes>(i);
			}
		}
	}
	return static_cast<dataTypes>(DATA_TYPE_UNDEFINED);
}

const char * DATA_PRODUCTS[NR_DATA_PRODUCT_TYPES] = {"Correlated", "Coherent Stokes", "Incoherent Stokes",
        "Instrument Model", "Pulsar", "Sky Image", "Unknown data type"};

dataProductTypes stringToDataProductType(const std::string &str) {
	if (!str.empty()) {
		for (dataProductTypes i = _BEGIN_DATA_PRODUCTS_ENUM_; i < _END_DATA_PRODUCTS_ENUM_; i = dataProductTypes(i + 1)) {
			if (str.compare(DATA_PRODUCTS[i]) == 0) {
				return i;
			}
		}
	}
	return DP_UNKNOWN_TYPE;
}

Task::task_status taskStatusFromString(const std::string &statusStr) {
	Task::task_status status(Task::TASK_STATUS_END);
	for (unsigned int idx = 0; idx != NR_TASK_STATES; ++idx) {
		if (statusStr.compare(task_states_str[idx]) == 0) {
			status = static_cast<Task::task_status>(idx);
			break;
		}
	}
	return status;
}

Task::task_type taskTypeFromString(std::string processType) {
	Task::task_type type(Task::UNKNOWN);
	std::transform(processType.begin(), processType.end(), processType.begin(), ::toupper);
	if (!processType.empty()) {
		for (unsigned int idx = 0; idx != NR_TASK_TYPES; ++idx) {
			if (processType.compare(task_types_str[idx]) == 0) {
				type = static_cast<Task::task_type>(idx);
				break;
			}
		}
	}
	return type;
}

Task *cloneTask(const Task *pTask) {
    const Observation *orgObs(0);
    const StationTask *orgStat(0);
    const Pipeline * orgPipe(0);
    const CalibrationPipeline *orgCalPipe(0);
    const ImagingPipeline *orgImgPipe(0);
    const PulsarPipeline *orgPulsPipe(0);
    const LongBaselinePipeline *orgLBPipe(0);

    orgStat = dynamic_cast<const StationTask *>(pTask);
    if (orgStat) { // is this a StationTask or derived type?
        if ((orgObs = dynamic_cast<const Observation *>(pTask))) {
            return new Observation(*orgObs);
        }
        else return new StationTask(*orgStat);
    }
    else if ((orgPipe = dynamic_cast<const Pipeline *>(pTask))) { // is this a Pipeline or derived type?
        if ((orgCalPipe = dynamic_cast<const CalibrationPipeline *>(pTask))) return new CalibrationPipeline(*orgCalPipe);
        else if ((orgImgPipe = dynamic_cast<const ImagingPipeline *>(pTask))) return new ImagingPipeline(*orgImgPipe);
        else if ((orgPulsPipe = dynamic_cast<const PulsarPipeline *>(pTask))) return new PulsarPipeline(*orgPulsPipe);
        else if ((orgLBPipe = dynamic_cast<const LongBaselinePipeline *>(pTask))) return new LongBaselinePipeline(*orgLBPipe);
    }

    return 0;
}

Task::task_status convertSASstatus(SAS_task_status sas_state) {
	switch (sas_state) {
	case SAS_STATE_IDLE:
		return Task::IDLE;
	case SAS_STATE_DESCRIBED:
		return Task::DESCRIBED;
	case SAS_STATE_PREPARED:
		return Task::PREPARED;
	case SAS_STATE_ON_HOLD:
		return Task::ON_HOLD;
	case SAS_STATE_APPROVED: // the state where the observation has been approved by all parties, the task doesn't have a start and stop time yet.
		return Task::UNSCHEDULED;
	case SAS_STATE_CONFLICT:
		return Task::CONFLICT;
	case SAS_STATE_PRESCHEDULED: // the task has a tentavive start and stop time
		return Task::PRESCHEDULED;
	case SAS_STATE_SCHEDULED: // the state scheduled if the observation has a start and stop time and can be started
		return Task::SCHEDULED;
	case SAS_STATE_QUEUED: // the state the SAS observation is being started
		return Task::STARTING;
	case SAS_STATE_ACTIVE: // when the SAS observation is running
		return Task::ACTIVE;
	case SAS_STATE_COMPLETING:
		return Task::COMPLETING;
	case SAS_STATE_FINISHED:
		return Task::FINISHED;
	case SAS_STATE_ABORTED:
		return Task::ABORTED;
	case SAS_STATE_OBSOLETE:
		return Task::OBSOLETE;
	case SAS_STATE_ERROR:
		return Task::ERROR;
	default:
		return Task::TASK_STATUS_END;
	}
}

Task::Task()
: taskID(0), itsPriority(0.0), itsStatus(DESCRIBED), itsTaskType(UNKNOWN),
  fixed_day(false), fixed_time(false), itsPenalty(0), penaltyCalculationNeeded(true), itsShiftDirection(SHIFT_RIGHT)
{
	clearAllConflicts();
	// set the time window equal to the schedule boundaries
	firstPossibleDay = Controller::theSchedulerSettings.getEarliestSchedulingDay();
	lastPossibleDay = Controller::theSchedulerSettings.getLatestSchedulingDay();
	windowMaxTime = AstroTime(23,59,59);
}

Task::Task(unsigned task_id)
: taskID(task_id), itsPriority(0.0), itsStatus(UNSCHEDULED), itsTaskType(UNKNOWN),
  fixed_day(false), fixed_time(false), itsPenalty(0), penaltyCalculationNeeded(true),
  itsShiftDirection(SHIFT_RIGHT)
{
	clearAllConflicts();
	// set the time window equal to the schedule boundaries
	firstPossibleDay = Controller::theSchedulerSettings.getEarliestSchedulingDay();
	lastPossibleDay = Controller::theSchedulerSettings.getLatestSchedulingDay();
	windowMaxTime = AstroTime(23,59,59);
}

// the following constructor creates a new task which can be used when the SAS tree doesn't contain a 'Scheduler' branch
Task::Task(unsigned task_id, const OTDBtree &SAS_tree)
: itsProjectName(SAS_tree.campaign()), taskID(task_id), itsPriority(0.0),
    fixed_day(false), fixed_time(false), itsPenalty(0), penaltyCalculationNeeded(true),
    itsShiftDirection(SHIFT_RIGHT), itsSASTree(SAS_tree)
{
	setType(SAS_tree.processType(), SAS_tree.processSubType(), SAS_tree.strategy());

	setStatusSAS(SAS_tree.state());

	clearAllConflicts();
	// set the time window equal to the schedule boundaries
	firstPossibleDay = Controller::theSchedulerSettings.getEarliestSchedulingDay();
	lastPossibleDay = Controller::theSchedulerSettings.getLatestSchedulingDay();
	windowMaxTime = AstroTime(23,59,59);
}

Task::Task(const QSqlQuery &query, const OTDBtree &SAS_tree)
: itsProjectName(SAS_tree.campaign()),
  itsPenalty(0), penaltyCalculationNeeded(true), itsShiftDirection(SHIFT_RIGHT), itsSASTree(SAS_tree)
{
	setType(SAS_tree.processType(), SAS_tree.processSubType(),SAS_tree.strategy());

	unsigned secondsDuration = query.value(query.record().indexOf("taskDuration")).toUInt();
	if (secondsDuration) {
		itsDuration = itsDuration.addSeconds(secondsDuration);
	}

	setStatusSAS(SAS_tree.state());

	taskID = query.value(query.record().indexOf("taskID")).toUInt();
	itsContactEmail = query.value(query.record().indexOf("contactEmail")).toString().toStdString();
	itsContactName = query.value(query.record().indexOf("contactName")).toString().toStdString();
	itsContactPhone = query.value(query.record().indexOf("contactPhone")).toString().toStdString();
	int day = query.value(query.record().indexOf("firstPossibleDay")).toInt();
	if (day) {
		firstPossibleDay = day;
	}
	else { // first possible day not set use the schedule start day or the current date whichever is latest
		firstPossibleDay = std::max(QDate::currentDate().toJulianDay() - J2000_EPOCH, (int)Controller::theSchedulerSettings.getEarliestSchedulingDay().toJulian());
	}
	QString time = query.value(query.record().indexOf("windowMaximumTime")).toString();
	if (!time.isEmpty()) {
		windowMaxTime = time;
	}
	else {
		windowMaxTime = AstroTime("23:59:59");
	}

	time = query.value(query.record().indexOf("windowMinimumTime")).toString();
	if (!time.isEmpty()) {
		windowMinTime = time;
	}
	else {
		windowMinTime = AstroTime("00:00:00");
	}
	fixed_day = query.value(query.record().indexOf("fixedDay")).toBool();
	fixed_time = query.value(query.record().indexOf("fixedTime")).toBool();
	day = query.value(query.record().indexOf("lastPossibleDay")).toInt();
	if (day) {
		lastPossibleDay = day;
	}
	else { // first possible day not set use the schedule start day
		lastPossibleDay = Controller::theSchedulerSettings.getLatestSchedulingDay();
	}
	predecessorMaxTimeDif = query.value(query.record().indexOf("predMaxTimeDif")).toString();
	predecessorMinTimeDif = query.value(query.record().indexOf("predMinTimeDif")).toString();
	setPredecessors(query.value(query.record().indexOf("predecessors")).toString());
    itsPriority = query.value(query.record().indexOf("priority")).toDouble();
	itsTaskName = query.value(query.record().indexOf("taskName")).toString().toStdString();
	setReason(query.value(query.record().indexOf("reason")).toString().toStdString());
	clearAllConflicts();
}

Task::~Task()
{
}

QDataStream& operator<< (QDataStream &out, const Task &task) {
	if (out.status() == QDataStream::Ok) {
		out << task.taskID
			<< task.itsProjectID
			<< task.itsProjectName
		    << task.itsProjectPI
		    << task.itsProjectCO_I
		    << task.itsTaskName
		    << task.itsContactName
		    << task.itsContactPhone
		    << task.itsContactEmail
            << (quint8) task.itsTaskType;

		// predecessors
		const IDvector &predecessors(task.getPredecessors());
		out << (quint32) predecessors.size();
		for (IDvector::const_iterator it = predecessors.begin(); it != predecessors.end(); ++it) {
			out << (quint8) it->first << it->second;
		}
		out << task.predecessorMinTimeDif << task.predecessorMaxTimeDif;

		// successors
		const IDvector &sucessors(task.getSuccessors());
		out << (quint32) sucessors.size();
		for (IDvector::const_iterator it = sucessors.begin(); it != sucessors.end(); ++it) {
			out << (quint8) it->first << it->second;
		}

        out << task.itsPriority
            << task.firstPossibleDay
		    << task.lastPossibleDay
		    << task.windowMinTime
		    << task.windowMaxTime
		    << task.itsDuration
			<< task.scheduledStart
			<< task.scheduledEnd
			<< task.fixed_day
			<< task.fixed_time
			<< (quint8) task.itsStatus
            << task.itsPenalty
            << task.itsShiftDirection
            << task.itsSASTree;
	}
	return out;
}

QDataStream& operator>> (QDataStream &in, Task &task) {
	if (in.status() == QDataStream::Ok) {
        quint32 nrOfObjects, taskID;
        quint8 status, type;

		in >> task.taskID
		   >> task.itsProjectID
		   >> task.itsProjectName
		   >> task.itsProjectPI
		   >> task.itsProjectCO_I
		   >> task.itsTaskName
		   >> task.itsContactName
		   >> task.itsContactPhone
		   >> task.itsContactEmail;

		in >> type;
        task.itsTaskType = (Task::task_type) type;

		// predecessors
		task.clearPredecessors();
		in >> nrOfObjects;
		for (quint32 i = 0; i < nrOfObjects; ++i) {
			in >> type >> taskID;
			task.itsPredecessors.push_back(IDvector::value_type(static_cast<id_type>(type), taskID));
		}
		in >> task.predecessorMinTimeDif >> task.predecessorMaxTimeDif;

		// successors
		task.clearSuccessors();
		in >> nrOfObjects;
		for (quint32 i = 0; i < nrOfObjects; ++i) {
			in >> type >> taskID;
			task.itsSuccessors.push_back(IDvector::value_type(static_cast<id_type>(type), taskID));
		}

        in >> task.itsPriority
           >> task.firstPossibleDay
		   >> task.lastPossibleDay
		   >> task.windowMinTime
		   >> task.windowMaxTime
		   >> task.itsDuration
		   >> task.scheduledStart
		   >> task.scheduledEnd
		   >> task.fixed_day
           >> task.fixed_time;

		in >> status;
		task.itsStatus = (Task::task_status) status;

        in >> task.itsPenalty
		   >> task.itsShiftDirection
		   >> task.itsSASTree;
	}
	return in;
}

Task & Task::operator=(const Task &other) {
    if (this != &other) {
        itsProjectID = other.itsProjectID;
        itsProjectName = other.itsProjectName;
        itsProjectPI = other.itsProjectPI;
        itsProjectCO_I = other.itsProjectCO_I;
        itsTaskName = other.itsTaskName;
        itsContactName = other.itsContactName;
        itsContactPhone = other.itsContactPhone;
        itsContactEmail = other.itsContactEmail;
        taskID = other.taskID;
        itsPriority = other.itsPriority;
        itsStatus = other.itsStatus;
        itsTaskType = other.itsTaskType;
        fixed_day = other.fixed_day;
        fixed_time = other.fixed_time;
        itsPenalty = other.itsPenalty;
        penaltyCalculationNeeded = other.penaltyCalculationNeeded;
        itsReason = other.itsReason;
        itsShiftDirection = other.itsShiftDirection;
        itsPredecessors = other.itsPredecessors;
        itsSuccessors = other.itsSuccessors;
        itsConflicts = other.itsConflicts;
        predecessorMinTimeDif = other.predecessorMinTimeDif;
        predecessorMaxTimeDif = other.predecessorMaxTimeDif;
        windowMinTime = other.windowMinTime;
        windowMaxTime = other.windowMaxTime;
        scheduledStart = other.scheduledStart;
        scheduledEnd = other.scheduledEnd;
        itsDuration = other.itsDuration;
        firstPossibleDay = other.firstPossibleDay;
        lastPossibleDay = other.lastPossibleDay;
        itsSASTree = other.itsSASTree;
    }
    return *this;
}

void Task::syncStartStopTimes(void) {
    itsSASTree.setStartTime(scheduledStart);
    itsSASTree.setStopTime(scheduledEnd);
}

void Task::setScheduledPeriod(const AstroDateTime &start, const AstroDateTime &end) {
	scheduledStart = start;
	scheduledEnd = end;
	if (scheduledEnd > scheduledStart) {
		itsDuration = scheduledEnd - scheduledStart;
	}
	else {
        itsDuration.clearTime();
	}
}

void Task::setConflict(task_conflict conflict, bool enable) {
	switch (conflict) {
	case CONFLICT_BITMODE:
		itsConflicts.bitmode = enable;
		break;
	case CONFLICT_OUT_OF_DATASLOTS:
		itsConflicts.out_of_dataslots = enable;
		break;
	case CONFLICT_MAINTENANCE:
		itsConflicts.maintenance = enable;
		break;
	case CONFLICT_RESERVATION:
		itsConflicts.storage_insufficient = enable;
		break;
    case CONFLICT_STATIONS:
        itsConflicts.stations = enable;
        break;
	case CONFLICT_INSUFFICIENT_STORAGE:
		itsConflicts.storage_insufficient = enable;
		break;
	case CONFLICT_STORAGE_NO_DATA:
		itsConflicts.storage_no_data = enable;
		break;
	case CONFLICT_STORAGE_EXCEEDS_BANDWIDTH:
		itsConflicts.storage_exceeds_bandwidth = enable;
		break;
	case CONFLICT_STORAGE_NODE_INACTIVE:
		itsConflicts.storage_node_inactive = enable;
		break;
	case CONFLICT_STORAGE_NODE_INEXISTENT:
		itsConflicts.storage_node_inexistent = enable;
		break;
	case CONFLICT_STORAGE_NO_NODES:
		itsConflicts.storage_no_nodes = enable;
		break;
	case CONFLICT_STORAGE_TOO_FEW_NODES:
		itsConflicts.storage_not_enough_nodes = enable;
		break;
	case CONFLICT_STORAGE_MINIMUM_NODES:
		itsConflicts.storage_minimum_nodes = enable;
		break;
	case CONFLICT_STORAGE_NO_OPTIONS:
		itsConflicts.storage_no_options = enable;
		break;
	case CONFLICT_STORAGE_NODE_SPACE:
		itsConflicts.storage_node_space = enable;
		break;
	case CONFLICT_STORAGE_WRITE_SPEED:
		itsConflicts.storage_write_speed = enable;
		break;
	case CONFLICT_RAID_ARRRAY_NOT_FOUND:
		itsConflicts.storage_raid_not_found = enable;
		break;
	case CONFLICT_STORAGE_TIME_TOO_EARLY:
		itsConflicts.storage_time_to_early = enable;
		break;
	case CONFLICT_STORAGE_NODE_BANDWIDTH:
		itsConflicts.storage_node_bandwidth = enable;
		break;
	case CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH:
		itsConflicts.storage_single_file_bw_too_high = enable;
		break;
	case CONFLICT_GROUP_STORAGE_MODE_DIFFERENT:
		itsConflicts.storage_mode_different = enable;
		break;
	case CONFLICT_GROUP_STORAGE_UNEQUAL_DATAPRODUCTS:
		itsConflicts.storage_unequal_dataproducts = enable;
		break;
	case CONFLICT_NO_STORAGE_ASSIGNED:
		itsConflicts.storage_no_storage_assigned = enable;
		break;
    case CONFLICT_NON_INTEGER_OUTPUT_FILES:
        itsConflicts.non_integer_nr_output_files = enable;
		break;
	default:
		break;
	}
}

void Task::clearAllConflicts(void) {
	itsConflicts.bitmode = false;
	itsConflicts.out_of_dataslots = false;
	itsConflicts.maintenance = false;
	itsConflicts.reservation = false;
    itsConflicts.stations = false;
	itsConflicts.storage_insufficient = false;
	itsConflicts.storage_no_data = false;
	itsConflicts.storage_exceeds_bandwidth = false;
	itsConflicts.storage_node_inactive = false;
	itsConflicts.storage_node_inexistent = false;
	itsConflicts.storage_no_nodes = false;
	itsConflicts.storage_not_enough_nodes = false;
	itsConflicts.storage_minimum_nodes = false;
	itsConflicts.storage_no_options = false;
	itsConflicts.storage_node_space = false;
	itsConflicts.storage_write_speed = false;
	itsConflicts.storage_raid_not_found = false;
	itsConflicts.storage_time_to_early = false;
	itsConflicts.storage_node_bandwidth = false;
	itsConflicts.storage_single_file_bw_too_high = false;
	itsConflicts.storage_mode_different = false;
	itsConflicts.storage_unequal_dataproducts = false;
	itsConflicts.storage_no_storage_assigned = false;
    itsConflicts.non_integer_nr_output_files = false;
}

void Task::clearAllStorageConflicts(void) {
	itsConflicts.storage_insufficient = false;
	itsConflicts.storage_no_data = false;
	itsConflicts.storage_exceeds_bandwidth = false;
	itsConflicts.storage_node_inactive = false;
	itsConflicts.storage_node_inexistent = false;
	itsConflicts.storage_no_nodes = false;
	itsConflicts.storage_not_enough_nodes = false;
	itsConflicts.storage_minimum_nodes = false;
	itsConflicts.storage_no_options = false;
	itsConflicts.storage_node_space = false;
	itsConflicts.storage_write_speed = false;
	itsConflicts.storage_raid_not_found = false;
	itsConflicts.storage_time_to_early = false;
	itsConflicts.storage_node_bandwidth = false;
	itsConflicts.storage_single_file_bw_too_high = false;
	itsConflicts.storage_mode_different = false;
	itsConflicts.storage_unequal_dataproducts = false;
	itsConflicts.storage_no_storage_assigned = false;
}

std::vector<std::string> Task::getTypesStrings(void) const {
	std::vector<std::string> typesVec;
	for (short int i = 0; i < NR_TASK_TYPES; ++i)
		typesVec.push_back(task_types_str[i]);
	return typesVec;
}

void Task::setStatus(const std::string &stat) {
	if (!stat.empty()) {
		setStatus(taskStatusFromString(stat));
	}
}

void Task::setStatusSAS(SAS_task_status sas_status) {
	itsStatus = convertSASstatus(sas_status);
	itsSASTree.itsTreeState = sas_status;
}

void Task::setStatus(const task_status stat) {
	if (stat >= 0 && stat < TASK_STATUS_END) {
		itsStatus = stat;
		switch (stat) {
		case IDLE:
			itsSASTree.itsTreeState = SAS_STATE_IDLE;
			break;
		case DESCRIBED:
			itsSASTree.itsTreeState = SAS_STATE_DESCRIBED;
			break;
		case PREPARED:
			itsSASTree.itsTreeState = SAS_STATE_PREPARED;
			break;
		case ON_HOLD:
			itsSASTree.itsTreeState = SAS_STATE_ON_HOLD;
			break;
		case UNSCHEDULED:
			itsSASTree.itsTreeState = SAS_STATE_APPROVED;
			break;
		case CONFLICT:
			itsSASTree.itsTreeState = SAS_STATE_CONFLICT;
			break;
		case PRESCHEDULED:
			itsSASTree.itsTreeState = SAS_STATE_PRESCHEDULED;
			break;
		case SCHEDULED:
			itsSASTree.itsTreeState = SAS_STATE_SCHEDULED;
			break;
		case STARTING:
			itsSASTree.itsTreeState = SAS_STATE_QUEUED;
			break;
		case ACTIVE:
			itsSASTree.itsTreeState = SAS_STATE_ACTIVE;
			break;
		case COMPLETING:
			itsSASTree.itsTreeState = SAS_STATE_COMPLETING;
			break;
		case FINISHED:
			itsSASTree.itsTreeState = SAS_STATE_FINISHED;
			break;
		case ABORTED:
			itsSASTree.itsTreeState = SAS_STATE_ABORTED;
			break;
		case ERROR:
			itsSASTree.itsTreeState = SAS_STATE_ERROR;
			break;
		case OBSOLETE:
			itsSASTree.itsTreeState = SAS_STATE_OBSOLETE;
			break;
		default:
			break;
		}
	}
	else {
		debugWarn("s", "Unknown task status specified!");
	}
}

void Task::setType(const QString &processType, processSubTypes processSubtype, const QString &strategy) {
    itsTaskType = Task::UNKNOWN;
	if (!processType.isEmpty()) { // processType may not be empty
		for (unsigned int idx = 0; idx != NR_TASK_TYPES; ++idx) {
            if (processType.compare(task_types_str[idx]) == 0) {
                itsTaskType = static_cast<task_type>(idx);
                itsSASTree.itsProcessType = processType;
                break;
            }
		}
	}
	itsSASTree.itsProcessSubtype = processSubtype;
	itsSASTree.itsStrategy = strategy;
}

void Task::setType(const task_type tasktype) {
	if (tasktype >= 0 && tasktype < TASK_TYPE_END) {
        itsTaskType = tasktype;
	}
	else {
        itsTaskType = Task::UNKNOWN;
		debugWarn("s", "Unknown task type specified!");
	}
}

void Task::setReason(unscheduled_reasons reason) {
	if (reason < UNSCHEDULED_REASON_END) {
		itsReason = unscheduled_reason_str[reason];
	}
	else {
		debugWarn("sis", "Task:", taskID, " , Error:Unknown unscheduled reason");
	}
}

/*
bool Task::setReason(unsigned short reason) {
	if (reason < UNSCHEDULED_REASON_END) {
		unscheduledReason = unscheduled_reason_str[reason];
		return true;
	}
	debugWarn("sis", "Task:", taskID, " , Error:Unknown unscheduled reason");
	return false;
}

bool Task::setReason(const std::string &reason) {
	if (!reason.empty()) {
		for (unsigned int idx = 0; idx != NR_REASONS; ++idx) {
			if (reason.compare(unscheduled_reason_str[idx]) == 0) {
				unscheduledReason = static_cast<unscheduled_reasons>(idx);
				return true;
			}
		}
	}
	debugWarn("siss", "Task:", taskID, " , Error:Unknown unscheduled reason: ", reason.c_str());
	return false;
}
*/
/*
void Task::addPredecessor(unsigned int pid, AstroTime min, AstroTime max) {
	std::pair<AstroTime, AstroTime> minMaxTimes (min, max);
	std::pair<unsigned int, std::pair<AstroTime, AstroTime> > predecessor (pid, minMaxTimes);
	predecessors.push_back(predecessor);
}
*/

QString Task::getPredecessorsString(const QChar &separater) const {
    QStringList predlist;
    const QString & ObsIDPrefix(Controller::theSchedulerSettings.getObservationIDprefix());
    QString prefix;
    for (IDvector::const_iterator it = itsPredecessors.begin(); it != itsPredecessors.end(); ++it) {
        switch (it->first) {
        case ID_SCHEDULER:
            prefix = "S";
            break;
        case ID_MOM:
            prefix = "M";
            break;
        case ID_SAS:
            prefix = ObsIDPrefix;
            break;
        }
        predlist.append(prefix + QString::number(it->second));
    }
    return "[" + predlist.join(separater) + "]";
}

void Task::setPredecessors(const QString &predecessors) {
	itsPredecessors.clear();
    const QString & ObsIDPrefix(Controller::theSchedulerSettings.getObservationIDprefix());
	std::vector<QString> preds(string2VectorOfStrings(predecessors));
	for (std::vector<QString>::const_iterator it = preds.begin(); it != preds.end(); ++it) {
		if (it->startsWith('S')) {
			addPredecessor(it->right(it->size()-1).toUInt(),ID_SCHEDULER);
		}
		else if (it->startsWith('M')) {
			addPredecessor(it->right(it->size()-1).toUInt(),ID_MOM);
		}
        else if (it->startsWith(ObsIDPrefix)) {
			addPredecessor(it->right(it->size()-1).toUInt(),ID_SAS);
		}
		else {
			debugWarn("sis","Task:", taskID, " predecessors contain unknown IDs");
		}
	}
}


bool Task::setPriority(const double &prio) {
	if (prio >= 0.0f) {
        itsPriority = prio;
		return true;
	}
	else {
		debugWarn("s", "Task priority cannot be negative");
		return false;
	}
}

AstroDateTime Task::getFirstPossibleDateTime() const {
	AstroDateTime firstdate(firstPossibleDay, windowMinTime);
	return firstdate;
}

AstroDateTime Task::getLastPossibleDateTime() const {
	AstroDateTime lastdate(lastPossibleDay, windowMaxTime);
	return lastdate;
}

const char * Task::getStatusStr(void) const {
	if (itsStatus >= 0 && itsStatus < TASK_STATUS_END)
	{
		return task_states_str[itsStatus];
	}
	debugErr("s","Unknown task status!");
	return "Unknown";
}

const char * Task::getTypeStr(void) const {
    if (itsTaskType >=0 && itsTaskType < TASK_TYPE_END) {
        return task_types_str[itsTaskType];
	}
	debugErr("s","Unknown task type!");
	return "Unknown";
}

QColor Task::getStatusColor(void) const {
	switch (itsStatus) {
	case SCHEDULED:
		return Qt::blue;
	case STARTING:
		return Qt::magenta;
	case ACTIVE:
		return Qt::yellow;
	case COMPLETING:
		return QColor(170,170,255); // (purple background)
	case FINISHED:
		return QColor(40,255,40); // bright greenish
	case ABORTED:
		return QColor(255,135,55); // bright orange
	case OBSOLETE:
		return Qt::darkGray;
	default:
		return Qt::darkYellow;
	}
}

QColor Task::getTypeColor(void) const {
    switch (itsTaskType) {
	case OBSERVATION:
		return Qt::darkBlue;
	case RESERVATION:
		if (itsSASTree.processSubType() == PST_STAND_ALONE) {
			return Qt::black;
		}
		else {
			return Qt::darkRed;
		}
	case MAINTENANCE:
		return Qt::darkCyan;
	case PIPELINE:
		return Qt::darkMagenta;
	case SYSTEM:
		return Qt::darkYellow;
	case UNKNOWN:
		return Qt::darkGreen;
	default: // includes UNKNOWN
		return Qt::darkGreen;
	}
}

/*
 *  diff check for difference between this task and other task.
 *  return true if difference
 *  parameter dif
 */
bool Task::diff(const Task *other, task_diff &dif) const {
    taskID != other->getID() ? dif.task_id = true :  dif.task_id = false;
    itsProjectID.compare(other->getProjectID()) != 0 ? dif.project_ID = true : dif.project_ID = false;
    itsSASTree.itsGroupID != other->getGroupID() ? dif.group_ID = true : dif.group_ID = false;
    itsTaskName.compare(other->getTaskName()) != 0 ? dif.task_name = true : dif.task_name = false;
    itsContactName.compare(other->getContactName()) != 0 ? dif.contact_name = true : dif.contact_name = false;
    itsContactPhone.compare(other->getContactPhone()) != 0 ? dif.contact_phone = true : dif.contact_phone = false;
    itsContactEmail.compare(other->getContactEmail()) != 0 ? dif.contact_email = true : dif.contact_email = false;
    itsTaskType != other->getType() ? dif.task_type = true : dif.task_type = false;
    itsPredecessors != other->getPredecessors() ? dif.predecessors = true : dif.predecessors = false;
    predecessorMinTimeDif != other->getPredecessorMinTimeDif() ? dif.pred_min_time_dif = true : dif.pred_min_time_dif = false;
    predecessorMaxTimeDif != other->getPredecessorMaxTimeDif() ? dif.pred_max_time_dif = true : dif.pred_max_time_dif = false;
    itsPriority != other->getPriority() ? dif.priority = true : dif.priority = false;
    firstPossibleDay != other->getWindowFirstDay() ? dif.first_possible_day = true : dif.first_possible_day = false;
    lastPossibleDay != other->getWindowLastDay() ? dif.last_possible_day = true : dif.last_possible_day = false;
    windowMinTime != other->getWindowMinTime() ? dif.window_minimum_time = true : dif.window_minimum_time = false;
    windowMaxTime != other->getWindowMaxTime() ? dif.window_maximum_time = true : dif.window_maximum_time = false;
    itsDuration != other->getDuration() ? dif.task_duration = true : dif.task_duration = false;
    scheduledStart != other->getScheduledStart() ? dif.scheduled_start = true : dif.scheduled_start = false;
    scheduledEnd != other->getScheduledEnd() ? dif.scheduled_end = true : dif.scheduled_end = false;
    fixed_day != other->getFixedDay() ? dif.fixed_day = true : dif.fixed_day = false;
    fixed_time != other->getFixedTime() ? dif.fixed_time = true : dif.fixed_time = false;
    itsStatus != other->getStatus() ? dif.task_status = true : dif.task_status = false;
    // SAS tree setttings
    itsSASTree.itsDescription.compare(other->itsSASTree.itsDescription) != 0 ? dif.description = true : dif.description = false;

	// task_id on its own should not be detected as a change, only when other properties of that task need to be saved will task_id (if different) also be saved.
    return (dif.project_ID || dif.group_ID || dif.task_name || dif.contact_name || dif.contact_phone || dif.contact_email
        || dif.task_type || dif.predecessors || dif.pred_min_time_dif || dif.pred_max_time_dif || dif.priority
        || dif.first_possible_day || dif.last_possible_day
        || dif.window_minimum_time || dif.window_maximum_time || dif.task_duration || dif.scheduled_start
        || dif.scheduled_end || dif.fixed_day || dif.fixed_time || dif.task_status || dif.description);
}

QString Task::diffString(const task_diff &dif) const {
    QString difstr;
    if (dif.antenna_mode) difstr += QString(SAS_item_names[TP_ANTENNA_MODE]) + ",";
    if (dif.clock_frequency) difstr += QString(SAS_item_names[TP_CLOCK_FREQUENCY]) + ",";
    if (dif.contact_email) difstr += QString(SAS_item_names[TP_CONTACT_EMAIL]) + ",";
    if (dif.contact_name) difstr += QString(SAS_item_names[TP_CONTACT_NAME]) + ",";
    if (dif.contact_phone) difstr += QString(SAS_item_names[TP_CONTACT_PHONE]) + ",";
    if (dif.filter_type) difstr += QString(SAS_item_names[TP_FILTER_TYPE]) + ",";
    if (dif.first_possible_day) difstr += QString(SAS_item_names[TP_FIRST_POSSIBLE_DATE]) + ",";
    if (dif.fixed_day) difstr += QString(SAS_item_names[TP_FIXED_DAY]) + ",";
    if (dif.fixed_time) difstr += QString(SAS_item_names[TP_FIXED_TIME]) + ",";
    if (dif.last_possible_day) difstr += QString(SAS_item_names[TP_LAST_POSSIBLE_DATE]) + ",";
    if (dif.pred_max_time_dif) difstr += QString(SAS_item_names[TP_PRED_MAX_TIME_DIF]) + ",";
    if (dif.pred_min_time_dif) difstr += QString(SAS_item_names[TP_PRED_MIN_TIME_DIF]) + ",";
    if (dif.predecessors) difstr += QString(SAS_item_names[TP_PREDECESSOR]) + ",";
    if (dif.priority) difstr += QString(SAS_item_names[TP_PRIORITY]) + ",";
    if (dif.project_ID) difstr += QString(SAS_item_names[TP_PROJECT_ID]) + ",";
    if (dif.group_ID) difstr += QString(SAS_item_names[TP_GROUP_ID]) + ",";
    if (dif.scheduled_start) difstr += QString(SAS_item_names[TP_SCHEDULED_START]) + ",";
    if (dif.scheduled_end) difstr += QString(SAS_item_names[TP_SCHEDULED_END]) + ",";
    if (dif.task_duration) difstr += QString(SAS_item_names[TP_DURATION]) + ",";
    if (dif.task_name) difstr += QString(SAS_item_names[TP_TASK_NAME]) + ",";
    if (dif.task_status) difstr += QString(SAS_item_names[TP_STATUS]) + ",";
    if (dif.task_type) difstr += QString(SAS_item_names[TP_TYPE]) + ",";
    if (dif.window_minimum_time) difstr += QString(SAS_item_names[TP_WINDOW_MIN_TIME]) + ",";
    if (dif.window_maximum_time) difstr += QString(SAS_item_names[TP_WINDOW_MAX_TIME]) + ",";
    if (dif.description) difstr += QString(SAS_item_names[TP_DESCRIPTION]) + ",";
    if (dif.task_id) difstr += QString(SAS_item_names[TP_TASK_ID]) + ",";

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}
