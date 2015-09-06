/*
 * task.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jan 21, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/task.h $
 *
 */

#ifndef TASK_H_
#define TASK_H_

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <QtSql>
#include <QColor>
#include "lofar_scheduler.h"
#include "storage_definitions.h"

class TaskStorage;

enum dataTypes {
    DATA_TYPE_STOKES_I,
    DATA_TYPE_STOKES_IQUV,
    DATA_TYPE_XXYY,
    DATA_TYPE_UNDEFINED,
    _END_DATA_TYPES
};

// CAUTION: The items in the following enum TASK_PROPERTIES and the subsequent static const array declaration must be kept synchronized!
enum TASK_PROPERTIES { TP_TASK_ID, TP_ANTENNA_MODE, TP_CLOCK_FREQUENCY, TP_CONTACT_EMAIL, TP_CONTACT_NAME, TP_CONTACT_PHONE,
        TP_DATASLOTS, TP_FILTER_TYPE, TP_FIRST_POSSIBLE_DATE, TP_FIXED_DAY, TP_FIXED_TIME, TP_LAST_POSSIBLE_DATE,
		TP_NIGHT_TIME_WEIGHT_FACTOR, TP_PRED_MAX_TIME_DIF, TP_PRED_MIN_TIME_DIF, TP_PREDECESSOR, TP_PRIORITY,
		TP_PROJECT_ID, TP_GROUP_ID, TP_RESERVATION, TP_SCHEDULED_END, TP_SCHEDULED_START, TP_STATION_IDS,
		TP_DURATION, TP_TASK_NAME, TP_STATUS, TP_TYPE, TP_WINDOW_MAX_TIME, TP_WINDOW_MIN_TIME, TP_ANA_BEAM_SETTINGS,
		TP_DIGI_BEAM_SETTINGS, TP_TAB_SETTINGS, TP_RTCP_BANDPASS_CORR, TP_RTCP_DELAY_COMP,
		TP_RTCP_FLYS_EYE, TP_RTCP_COHERENT_DEDISPERSION, TP_RTCP_COHERENT_STOKES_SETTINGS, TP_RTCP_INCOHERENT_STOKES_SETTINGS,
		TP_RTCP_COR_INT_TIME, TP_RTCP_BITS_PER_SAMPLE, TP_RTCP_CHANNELS_PER_SUBBAND, TP_IMAGING_SLICES_PER_IMAGE, TP_IMAGING_SUBBANDS_PER_IMAGE,
        TP_IMAGING_SPECIFY_FOV, TP_IMAGING_FOV, TP_IMAGING_NPIX, TP_IMAGING_CELLSIZE, TP_LONGBASELINE_SB_PER_SBGROUP, TP_LONGBASELINE_SBGROUP_PER_MS,
		TP_DEMIX_ALWAYS, TP_DEMIX_IF_NEEDED, TP_DEMIX_SKYMODEL, TP_DEMIX_FREQSTEP, TP_DEMIX_TIMESTEP, TP_AVG_FREQSTEP, TP_AVG_TIMESTEP,
        TP_CALIBRATION_SKYMODEL, TP_PULSAR_PIPELINE_SETTINGS, TP_OUTPUT_DATA_TYPES, TP_OUTPUT_STORAGE_SETTINGS, TP_OUTPUT_DATA_PRODUCTS, TP_TBB_PIGGYBACK,
        TP_AARTFAAC_PIGGYBACK, TP_INPUT_DATA_PRODUCTS, TP_DESCRIPTION,
		_END_TASK_PROPERTIES_ENUM_};

#define NR_TASK_PROPERTIES _END_TASK_PROPERTIES_ENUM_
extern const char * SAS_item_names[NR_TASK_PROPERTIES];

extern const char * DATA_TYPES[_END_DATA_TYPES];
#define NR_TASK_TYPES 6
extern const char *task_types_str[NR_TASK_TYPES];
#define NR_TASK_STATES	15
extern const char *task_states_str[NR_TASK_STATES];

// put the non-blocking errors first
enum unscheduled_reasons {
	NO_ERROR,
    USER_WARNING,
	NOT_COMPATIBLE_WITH_RESERVATION,
	BEAM_DURATION_DIFFERENT,
	SCHEDULED_TOO_EARLY,
	TASK_CONFLICT,
	ZERO_DURATION, // from here only ERRORs
	START_TIME_NOT_SET,
	END_TIME_NOT_SET,
	EMPTY_SUBBAND_LIST,
	TOO_MANY_SUBBANDS,
	UNKNOWN_BITMODE,
	OUTSIDE_SCHEDULE_BOUNDS,
	NO_PREDECESSOR_DEFINED,
	PREDECESSOR_NOT_FOUND,
	MAX_DISTANCE_PREDECESSOR,
	PREDECESSOR_UNSCHEDULED,
	PRED_MIN_TIME_GREATER_MAX_TIME,
	TOO_CLOSE_OR_BEFORE_PREDECESSOR,
	FIRST_DATE_TOO_LATE,
	LAST_DATE_TOO_EARLY,
	START_LATER_THEN_END,
	NO_STATIONS_DEFINED,
	NON_EXISTING_STATION,
	WRONG_STATION_COMBINATION,
	NO_MUTUAL_OPENING_FOUND,
	TASK_ON_HOLD,
	TASK_HAS_ERROR,
	ANTENNA_MODE_UNSPECIFIED,
	CLOCK_FREQUENCY_UNSPECIFIED,
	FILTER_TYPE_UNSPECIFIED,
	INCOMPATIBLE_ANTENNA_AND_FILTER,
	INCOMPATIBLE_CLOCK_AND_FILTER,
	SPECIFICATION_INCOMPLETE,
	NO_INCOHERENT_TABS_DEFINED,
	NO_COHERENT_TABS_DEFINED,
	NO_TABS_DEFINED,
	WRONG_CHANNEL_COLLAPSE,
    NON_INTEGER_NR_OUTPUT_FILES,
	NO_DATA_OUTPUT_SELECTED,
	INPUT_DATA_PRODUCT_NOT_FOUND,
	INCOMPATIBLE_DEMIX_SETTINGS,
    UNKNOWN_DEMIX_SOURCE,
    INPUT_OUTPUT_LOCATION_MISMATCH1,
    INPUT_OUTPUT_LOCATION_MISMATCH2,
	UNSCHEDULED_REASON_END
};
#define NR_REASONS	UNSCHEDULED_REASON_END

extern const char *unscheduled_reason_str[NR_REASONS];

#include "astrodatetime.h"
#include "astrotime.h"
#include "astrodate.h"
#include "OTDBtree.h"

struct task_conflicts {
	bool bitmode;
	bool out_of_dataslots;
	bool maintenance;
	bool reservation;
    bool stations;
	bool storage_insufficient;
	bool storage_no_data;
	bool storage_exceeds_bandwidth;
	bool storage_node_inactive;
	bool storage_node_inexistent;
	bool storage_no_nodes;
	bool storage_not_enough_nodes;
	bool storage_minimum_nodes;
	bool storage_no_options;
	bool storage_node_space;
	bool storage_write_speed;
	bool storage_raid_not_found;
	bool storage_time_to_early;
	bool storage_node_bandwidth;
	bool storage_single_file_bw_too_high;
	bool storage_mode_different;
	bool storage_unequal_dataproducts;
	bool storage_no_storage_assigned;
    bool non_integer_nr_output_files;
};

enum id_type {
	ID_SCHEDULER,
	ID_MOM,
	ID_SAS
};

typedef std::vector<std::pair<id_type, quint32> > IDvector;

class task_diff {
public:
    // TODO: Code smell: Ownership of state.
    // These changed bools should be owned by the respected gui elements
    // each gui element should expose a changed member
    task_diff() : 	task_id(false), antenna_mode(false), clock_frequency (false), contact_email (false), contact_name(false), contact_phone(false),
        dataslots(false), filter_type(false), first_possible_day(false), fixed_day(false), fixed_time(false), last_possible_day(false),
        night_time_weight_factor(false), pred_max_time_dif(false), pred_min_time_dif(false), predecessors(false), priority(false), project_ID(false),
        group_ID(false), reservation(false), scheduled_end(false), scheduled_start(false), station_ids(false), task_duration(false), task_name(false),
        task_status(false), task_type(false),window_maximum_time(false), window_minimum_time(false), ana_beam_angle1(false), ana_beam_angle2(false),
        ana_beam_direction_type(false), ana_beam_starttime(false), ana_beam_duration(false), digital_beam_settings(false), tiedarray_beam_settings(false),
        RTCP_correct_bandpass(false), RTCP_delay_compensation(false), RTCP_pencil_flys_eye(false), RTCP_coherent_dedispersion(false), RTCP_coherent_stokes_settings(false),
        RTCP_incoherent_stokes_settings(false), RTCP_cor_int_time(false), RTCP_nr_bits_per_sample(false), RTCP_channels_per_subband(false), Imaging_nr_slices_per_image(false),
        Imaging_nr_subbands_per_image(false), Imaging_npix(false), Imaging_specify_fov(false), Imaging_fov(false), Imaging_cellsize(false),
        LongBaseline_nr_sb_per_sbgroup(false), LongBaseline_nr_sbgroup_per_MS(false), demix_always(false), demix_if_needed(false),
        demix_skymodel(false), demix_freqstep(false), demix_timestep(false), freqstep(false), timestep(false), calibration_skymodel(false), pulsar_pipeline_settings(false),
        TBBPiggybackAllowed(false), AartfaacPiggybackAllowed(false), input_data_products(false), output_data_types(false), output_storage_settings(false), output_data_products(false),	description(false)
    { }

public:
    bool task_id, antenna_mode, clock_frequency , contact_email , contact_name, contact_phone,
            dataslots, filter_type, first_possible_day, fixed_day, fixed_time, last_possible_day,
            night_time_weight_factor, pred_max_time_dif, pred_min_time_dif, predecessors, priority, project_ID,
            group_ID, reservation, scheduled_end, scheduled_start, station_ids, task_duration, task_name,
            task_status, task_type,window_maximum_time, window_minimum_time, ana_beam_angle1, ana_beam_angle2,
            ana_beam_direction_type, ana_beam_starttime, ana_beam_duration, digital_beam_settings, tiedarray_beam_settings,
            RTCP_correct_bandpass, RTCP_delay_compensation, RTCP_pencil_flys_eye, RTCP_coherent_dedispersion, RTCP_coherent_stokes_settings,
            RTCP_incoherent_stokes_settings, RTCP_cor_int_time, RTCP_nr_bits_per_sample, RTCP_channels_per_subband, Imaging_nr_slices_per_image,
            Imaging_nr_subbands_per_image, Imaging_npix, Imaging_specify_fov, Imaging_fov, Imaging_cellsize,
            LongBaseline_nr_sb_per_sbgroup, LongBaseline_nr_sbgroup_per_MS, demix_always, demix_if_needed,
            demix_skymodel, demix_freqstep, demix_timestep, freqstep, timestep, calibration_skymodel, pulsar_pipeline_settings,
            TBBPiggybackAllowed, AartfaacPiggybackAllowed, input_data_products, output_data_types, output_storage_settings, output_data_products,	description;
};

dataTypes stringToStokesType(const std::string &str);

class Task
{
public:
	enum task_status {
		IDLE = 0,
		ERROR,
		DESCRIBED,// = 100,
		PREPARED,// = 200,
		ON_HOLD,
		UNSCHEDULED,// = 300, = APPROVED in SAS
		CONFLICT, // = 335
		PRESCHEDULED,// = 350,
		SCHEDULED,// = 400, // scheduled means fixed in the schedule and waiting for execution
		STARTING,// = 500,  // = QUEUED in SAS
		ACTIVE,// = 600, // currently being executed
		COMPLETING, // = 900
		FINISHED,// = 1000, (from this state up are inactive states)
		ABORTED,//= 1100,
		OBSOLETE,// = 1200,
		TASK_STATUS_END
	};

	// define task types
	enum task_type {
		OBSERVATION,
		RESERVATION,
		MAINTENANCE,
		PIPELINE,
		SYSTEM,
		UNKNOWN,
		TASK_TYPE_END
	};

	enum task_color_mode {
		TASK_TYPE_COLOR_MODE,
		TASK_STATUS_COLOR_MODE
	};

	Task();
    Task(unsigned task_id);
#ifdef HAS_SAS_CONNECTION
    Task(unsigned task_id, const OTDBtree &SAS_tree); // constructor used for incomplete SAS tasks that have no stored scheduler properties
	Task(const QSqlQuery &query, const OTDBtree &SAS_tree);
#endif
	virtual ~Task();

    friend QDataStream& operator<< (QDataStream &out, const Task &task); // used for writing data to binary file
    friend QDataStream& operator>> (QDataStream &in, Task &task); // used for reading data from binary file

	// make me an exact copy of the original task but keep my ID
    virtual void clone(const Task *other) {
        if (this != other) {
            unsigned myTaskID = taskID;
            *this = *other;
            taskID = myTaskID;
        }
	}

    virtual Task & operator=(const Task &);
	bool operator==(unsigned int taskID) const {return (this->getID() == taskID);}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

	//get methods
	unsigned getID(void) const {return taskID;}
	quint32 getMomID(void) const {return itsSASTree.itsMomID;}
	quint32 getGroupID(void) const {return itsSASTree.itsGroupID;}
	quint32 getSASTreeID(void) const {return itsSASTree.itsTreeID;}
	SAS_task_status getSAStreeState(void) const {return itsSASTree.state();}
	int getOriginalTreeID(void) const {return itsSASTree.itsOriginalTree;}
	QColor getTypeColor(void) const;
	QColor getStatusColor(void) const;
	bool isScheduled(void) const {return ((itsStatus == Task::STARTING) || (itsStatus == Task::ACTIVE) ||
										  (itsStatus == Task::PRESCHEDULED) || (itsStatus == Task::SCHEDULED));}
	//	predecessorVector const &getPredecessors() const { return predecessors; }; // returns the list of predecessors
	bool hasPredecessors(void) const {return !itsPredecessors.empty();}
	bool hasSuccesors(void) const {return !itsSuccessors.empty();}
	bool hasLinkedTask(void) const {return (hasPredecessors() | hasSuccesors());}
	const IDvector &getPredecessors(void) const { return itsPredecessors; } // returns the predecessor vector
    IDvector &getPredecessorsForChange(void) { return itsPredecessors; } // returns the predecessor vector as a non-const
    QString getPredecessorsString(const QChar &separater = QChar(',')) const;
	const IDvector &getSuccessors(void) const { return itsSuccessors; } // returns the successors vector
    bool isReservation(void) const {return itsTaskType == Task::RESERVATION;}
    bool isMaintenance(void) const {return itsTaskType == Task::MAINTENANCE;}
    bool isObservation(void) const {return itsTaskType == Task::OBSERVATION;}
    bool isStationTask(void) const {return ((itsTaskType == Task::OBSERVATION) || (itsTaskType == Task::RESERVATION) || (itsTaskType == Task::MAINTENANCE));}
    bool isPipeline(void) const {return itsTaskType == Task::PIPELINE;}
    bool isSystem(void) const {return itsTaskType == Task::SYSTEM;}
    bool isUnknown(void) const {return itsTaskType == Task::UNKNOWN;}
	const AstroTime &getPredecessorMinTimeDif() const {return predecessorMinTimeDif;}
	const AstroTime &getPredecessorMaxTimeDif() const {return predecessorMaxTimeDif;}
	unsigned int getPenalty() const { return itsPenalty; }//if (penaltyCalculationNeeded) { calculatePenalty(); } return itsPenalty; }
//	bool isPenaltyCalculationNeeded(void) const {return penaltyCalculationNeeded;}
	unsigned int calculatePenalty(void) { if (penaltyCalculationNeeded) { doCalculatePenalty(); }; return itsPenalty; }
    const double &getPriority() const { return itsPriority; }
	inline Task::task_status getStatus() const { return itsStatus; }
	const char * getStatusStr(void) const;
	const QString &getProcessType(void) const {return itsSASTree.itsProcessType;}
	processSubTypes getProcessSubtype(void) const {return itsSASTree.itsProcessSubtype;}
	inline const char *getProcessSubtypeStr(void) const {return itsSASTree.getProcessSubtypeStr();}
	const QString &getStrategy(void) const {return itsSASTree.itsStrategy;}
    Task::task_type getType() const { return itsTaskType; }
	const char * getTypeStr(void) const;
	std::vector<std::string> getTypesStrings(void) const;
	const char * getProjectName(void) const {return itsProjectName.c_str();}
	const char * getProjectID(void) const {return itsProjectID.c_str();}
	const char * getProjectPI(void) const {return itsProjectPI.c_str();}
	const char * getProjectCO_I(void) const {return itsProjectCO_I.c_str();}
	const char * getTaskName(void) const {return itsTaskName.c_str();}
	const char * getTaskDescription(void) const {return itsSASTree.itsDescription.c_str();}
	const char * getContactName(void) const {return itsContactName.c_str();}
	const char * getContactPhone(void) const {return itsContactPhone.c_str();}
	const char * getContactEmail(void) const {return itsContactEmail.c_str();}
	const AstroDateTime &getScheduledStart(void) const { return scheduledStart; }
	const AstroDateTime &getScheduledEnd(void) const { return scheduledEnd; }
    const AstroDateTime &getRealStart(void) const {return itsSASTree.startTime();}
    const AstroDateTime &getRealEnd(void) const {return itsSASTree.stopTime();}
    const AstroDate &getWindowFirstDay() const { return firstPossibleDay; }
	const AstroDate &getWindowLastDay() const { return lastPossibleDay; }
	const AstroTime &getWindowMinTime() const { return windowMinTime; }
	const AstroTime &getWindowMaxTime() const { return windowMaxTime; }
	AstroDateTime getFirstPossibleDateTime() const;
	AstroDateTime getLastPossibleDateTime() const;
	const AstroTime &getDuration() const { return itsDuration; }
	bool getFixedDay(void) const { return fixed_day; }
	bool getFixedTime(void) const { return fixed_time; }
	const std::string &getReason() const {return itsReason;}
	bool getPenaltyCalculationNeeded(void) const {return penaltyCalculationNeeded;}
	bool getShiftDirection(void) const {return itsShiftDirection;}
    const task_conflicts &getConflicts(void) const {return itsConflicts;}

	// set methods
	void setID(unsigned id) {taskID = id;}
	void setStatus(const task_status stat);
	void setStatus(const std::string &stat);
	void setStatusSAS(SAS_task_status status); // set the task status according to the SAS states
	void setType(const task_type tasktype);
	void setType(const QString &processType, processSubTypes processSubtype, const QString &strategy);
    void syncStartStopTimes(void); // synchronizes the start and stop times in the meta-data of the OTDBtree with the scheduledStart and scheduledEnd times
	inline void setProcessType(const QString &processType) {itsSASTree.itsProcessType = processType;}
	inline void setProcessSubtype(processSubTypes process_subtype) {itsSASTree.itsProcessSubtype = process_subtype;}
	inline void setStrategy(const QString &strategy) {itsSASTree.itsStrategy = strategy;}
	inline void setPenalty(unsigned int penalty) {this->itsPenalty = penalty; this->penaltyCalculationNeeded = false;}
	inline void clearPredecessors(void) {itsPredecessors.clear();}
	inline void clearSuccessors(void) {itsSuccessors.clear();}
	void addPredecessor(unsigned int taskID, id_type type = ID_SCHEDULER) {itsPredecessors.push_back(IDvector::value_type(type, taskID));}
	void setPredecessors(const QString &predecessors);
	inline void addSuccessor(unsigned int taskID, id_type type = ID_SCHEDULER) {itsSuccessors.push_back(IDvector::value_type(type, taskID));}
	inline void setPredecessorMinTimeDif(const AstroTime &minTime) { predecessorMinTimeDif = minTime; }
	inline void setPredecessorMaxTimeDif(const AstroTime &maxTime) { predecessorMaxTimeDif = maxTime; }
	inline void setProjectName(const std::string &project_name) {itsProjectName = project_name;}
	inline void setProjectID(const std::string &project_id) {itsProjectID = project_id; itsSASTree.itsCampaign = project_id;}
	inline void setProjectPI(const std::string &project_leader) {itsProjectPI = project_leader;}
	inline void setProjectCO_I(const std::string &project_coi) {itsProjectCO_I = project_coi;}
	inline void setTaskName(const std::string &task_name) {itsTaskName = task_name;}
	inline void setTaskDescription(const std::string &description) {itsSASTree.itsDescription = description;}
	inline void setContactName(const std::string &contact_name) {itsContactName = contact_name;}
	inline void setContactPhone(const std::string &contact_phone) {itsContactPhone = contact_phone;}
	inline void setContactEmail(const std::string &contact_email) {itsContactEmail = contact_email;}
	inline void setOriginalTreeID(int original_tree) {itsSASTree.itsOriginalTree = original_tree;}
    bool setPriority(const double &);
	inline void setWindowFirstDay(const QString &day) { firstPossibleDay = day; }
	inline void setWindowFirstDay(const AstroDate &day) { firstPossibleDay = day;}
	inline void setWindowFirstDay(unsigned long int day) { firstPossibleDay = AstroDate(day);}
	inline void setWindowLastDay(const QString &day) {lastPossibleDay = day; }
	inline void setWindowLastDay(const AstroDate &day) {lastPossibleDay = day; }
	inline void setWindowLastDay(unsigned long int day) { lastPossibleDay = AstroDate(day);}
	inline void setWindowMinTime(const AstroTime &time) { windowMinTime = time;}
	inline void setWindowMaxTime(const AstroTime &time) {windowMaxTime = time; }
	void setScheduledStart(const AstroDateTime &start) { scheduledStart = start; scheduledEnd = scheduledStart + itsDuration; penaltyCalculationNeeded = true;}
	void setScheduledStart(double start) { scheduledStart = start; scheduledEnd = scheduledStart + itsDuration; penaltyCalculationNeeded = true;}
	void setScheduledStart(const QString &start) { scheduledStart = start;	scheduledEnd = scheduledStart + itsDuration; penaltyCalculationNeeded = true;}
    void setScheduledEnd(const AstroDateTime &end) { scheduledEnd = end; itsDuration = scheduledEnd - scheduledStart; penaltyCalculationNeeded = true; /*itsRecalcStorageNeeded = true;*/}
    void setScheduledEnd(double end) { scheduledEnd = end; itsDuration = scheduledEnd - scheduledStart; penaltyCalculationNeeded = true; /*itsRecalcStorageNeeded = true;*/}
    void setScheduledEnd(const QString &end) { scheduledEnd = end; itsDuration = scheduledEnd - scheduledStart; penaltyCalculationNeeded = true; /*itsRecalcStorageNeeded = true;*/}
    void resetRealTimes(void) {itsSASTree.resetTimes();}
	void setScheduledPeriod(const AstroDateTime &start, const AstroDateTime &end);
    void setDuration(const AstroTime &dur) { itsDuration = dur; scheduledEnd = scheduledStart + itsDuration; /*itsRecalcStorageNeeded = true;*/}
	inline void setFixDay(bool fix_day) { fixed_day = fix_day; }
	inline void setFixTime(bool fix_time) { fixed_time = fix_time; }
	inline void setReason(const std::string &reason) {itsReason = reason;}
	void setReason(unscheduled_reasons reason);
	inline void clearReason(void) {itsReason = "";}
	inline void clearPenalty(void) {itsPenalty = 0; penaltyCalculationNeeded = true;}
	inline void setShiftDirection(bool direction) {itsShiftDirection = direction;}
    inline void setSASTree(const OTDBtree &tree) {itsSASTree = tree;}
	inline void setSASTreeID(int treeID) {itsSASTree.itsTreeID = treeID;}
	inline void setGroupID(unsigned groupID) {itsSASTree.itsGroupID = groupID;}
	inline void setMoMID(int momID) {itsSASTree.itsMomID = momID;}

	void clearAllStorageConflicts(void);

    virtual TaskStorage *storage(void) {return 0;}
    virtual const TaskStorage *storage(void) const {return 0;}
    virtual bool hasStorage(void) const {return false;}
    virtual void calculateDataFiles() { }

    void setConflict(task_conflict conflict, bool enable = true);
	void clearConflict(task_conflict conflict) {setConflict(conflict, false);}
	void clearAllConflicts(void);
    // calculate the output data sizes, needs to be implemented in derived classes observation and pipeline

private:
    virtual void doCalculatePenalty() {itsPenalty = 0;}

protected:
	// attributes
    std::string itsProjectID, itsProjectName, itsProjectPI, itsProjectCO_I, itsTaskName, itsContactName, itsContactPhone, itsContactEmail;
	quint32 taskID; // unique task ID
    double itsPriority; // priority of this task from 1 (high) to 5 (low)
	task_status itsStatus; // the current status of this task
    task_type itsTaskType; // the major type of this task e.g. PIPELINE / OBSERVATION / RESERVATION
	bool fixed_day; // if set to true fixes the day
	bool fixed_time; // if set to true fixes the day
	quint16 itsPenalty; // the penalty for this task
	bool penaltyCalculationNeeded; // determines if penalty needs to be recalculated
	std::string itsReason;
	bool itsShiftDirection;
	IDvector itsPredecessors, itsSuccessors;
	task_conflicts itsConflicts;

	// objects
//	std::vector<unsigned int> successors;
	AstroTime predecessorMinTimeDif;
	AstroTime predecessorMaxTimeDif;
	AstroTime windowMinTime; // scheduling time window minimum time
	AstroTime windowMaxTime; // scheduling time window maximum time
	AstroDateTime scheduledStart; // the currently scheduled start time
	AstroDateTime scheduledEnd; // the currently scheduled end time
	AstroTime itsDuration;
	AstroDate firstPossibleDay; // time window first possible scheduling date
	AstroDate lastPossibleDay; // time window last possible scheduling date

	// SAS parameters
//#ifdef HAS_SAS_CONNECTION
	OTDBtree itsSASTree;
public :
	const OTDBtree &SASTree(void) const {return itsSASTree;}
//#endif
};


// the following class is needed to be able to sort a vector of pointers to Task objects
// according to the priority attribute or the scheduled start time of the Task objects

class cmp_TaskPriority
{
public:
	bool operator() (const Task * t1, const Task * t2) const
	{
		return t1->getPriority() > t2->getPriority();
	}
};

class cmp_taskScheduledStart
{
public:
	bool operator() (const Task *t1, const Task *t2) const
	{
		return t1->getScheduledStart() < t2->getScheduledStart();
	}
};

class cmp_taskFirstPossibleDateTime
{
public:
	bool operator() (const Task *t1, const Task *t2) const
	{
		return t1->getFirstPossibleDateTime() < t2->getFirstPossibleDateTime();
	}
};

class cmp_intPairFirst
{
public:
	bool operator() (const std::pair<int,int> &p1, const std::pair<int,int> &p2) const
	{
		return p1.first < p2.first;
	}
};

class cmp_intPairSecond
{
public:
	bool operator() (const std::pair<int,int> &p1, const std::pair<int,int> &p2) const
	{
		return p1.second < p2.second;
	}
};

// static helper functions
Task *cloneTask(const Task *pTask); // creates an exact clone of the supplied task
Task::task_status taskStatusFromString(const std::string &statusStr);
Task::task_type taskTypeFromString(std::string processType);
Task::task_status convertSASstatus(SAS_task_status sas_state);

#endif /* TASK_H_ */


