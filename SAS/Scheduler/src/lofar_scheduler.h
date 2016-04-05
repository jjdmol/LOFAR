/*
 * lofar_scheduler.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 20, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/lofar_scheduler.h $
 *
 */

#ifndef LOFAR_SCHEDULER_H_
#define LOFAR_SCHEDULER_H_

#define SCHEDULER_VERSION "v2.6"

#define HAS_SAS_CONNECTION
#define DEBUG_SCHEDULER

// undefine to compile away the debug output:
//#define QT_NO_DEBUG_OUTPUT

//#undef Q_OS_UNIX
//#define Q_OS_WIN

#include <vector>
#include <map>
#include <string>
#include <limits>

#define FILE_WRITE_VERSION 10

#define J2000_EPOCH 2451545LL
// pi
#define PI	3.14159265358979323846264338327950288419716939937510
#define PI_DIV2  1.57079632679489661923132169163975144209858469968755
#define TWO_PI	6.28318530717958647692528676655900576839433879875021

#define RAD2GRAD	57.29577951308232087679815481410517033240547246656432 // 180/pi
#define GRAD2RAD	0.01745329251994329576923690768488612713442871888542  // pi/180
// TODO: Ask Josh if the following constant is WSRT related and should thus be changed in a station dependent variable
// Sine of sun rise and sun set elevation for elevation = -0.7 deg
#define SINSUNRISESETEL -1.22170008352471688802e-2
// Earth's rotation per hour in radians (needed for hour angle calculations)
//#define EARTH_RAD_PER_HOUR 2.61799387799149436539e-1
//#define EARTH_RAD_PER_HOUR 15
#define DU_LST_LINEAR 24.06570982441908
#define DU_LST_CONST 18.697374558336
#define LST_DU_LINEAR 4.15528975427151E-02

#define MINIMUM_CORRELATOR_INTEGRATION_TIME 0.1 // minimum correlator integration time in seconds (typically minimum is 0.1 sec)

#define CLOCK160_CHANNELWIDTH		    610.3515625
#define STR_CLOCK160_CHANNELWIDTH	   "610.3515625"
#define CLOCK160_SAMPLESPERSECOND	    155648
#define STR_CLOCK160_SAMPLESPERSECOND "155648"
#define CLOCK160_SUBBANDWIDTH			156.250
#define STR_CLOCK160_SUBBANDWIDTH 	   "156.250"
#define CLOCK160_SAMPLECLOCK		    160
#define STR_CLOCK160_SAMPLECLOCK	   "160"
#define CLOCK200_CHANNELWIDTH		    762.939453125
#define STR_CLOCK200_CHANNELWIDTH	   "762.939453125"
#define CLOCK200_SAMPLESPERSECOND	    196608
#define STR_CLOCK200_SAMPLESPERSECOND "196608"
#define CLOCK200_SUBBANDWIDTH		    195.3125
#define STR_CLOCK200_SUBBANDWIDTH	   "195.3125"
#define CLOCK200_SAMPLECLOCK		    200
#define STR_CLOCK200_SAMPLECLOCK	   "200"

// dataslot and RSP board limits
//#define MAX_DATASLOTS_2_BITS 1952
#define MAX_DATASLOTS_4_BITS 976
#define MAX_DATASLOTS_8_BITS 488
#define MAX_DATASLOTS_16_BITS 244
#define MAX_DATASLOT_PER_RSP_4_BITS  243
#define MAX_DATASLOT_PER_RSP_8_BITS  121
#define MAX_DATASLOT_PER_RSP_16_BITS 60

#define STORAGE_NODE_kbps				2000000 // units: kbit/sec
#define STORAGE_RAID_WRITE_KBS			4000000 // units: kByte/sec
#define STORAGE_FILL_PECENTAGE          90
//#define DEFAULT_NAME_MASK "L${YEAR}_${MSNUMBER}/SB${SUBBAND}.MS"

#define MULTIPLE_VALUE_TEXT "(MIXED)"
#define NOT_SET_TEXT "not set"

// the file name masks and directory masks for the different output data products (data types)
#define NAMEMASK_COHERENT_STOKES 		"L${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_INCOHERENT_STOKES		"L${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_CORRELATED				"L${OBSID}_SAP${SAP}_SB${SUBBAND}_uv.MS"
#define NAMEMASK_INSTRUMENT_MODEL		"L${OBSID}_SAP${SAP}_SB${SUBBAND}_inst.INST"
// TODO: Pulsar Pipeline: set the correct file name mask
#define NAMEMASK_PULSAR                 "L${OBSID}_SAP${SAP}_B${BEAM}_bf.tar.gz"
#define NAMEMASK_SKY_IMAGE				"L${OBSID}_SBG${SBG}_sky.h5"
#define NAMEMASK_COHERENT_STOKES_TEST 	"T${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_INCOHERENT_STOKES_TEST	"T${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.h5"
#define NAMEMASK_CORRELATED_TEST		"T${OBSID}_SAP${SAP}_SB${SUBBAND}_uv.MS"
#define NAMEMASK_INSTRUMENT_MODEL_TEST	"T${OBSID}_SAP${SAP}_SB${SUBBAND}_inst.INST"
// TODO: Pulsar Pipeline: set the correct file name mask for test
#define NAMEMASK_PULSAR_TEST            "T${OBSID}_SAP${SAP}_B${BEAM}_bf.tar.gz"
#define NAMEMASK_SKY_IMAGE_TEST			"T${OBSID}_SBG${SBG}_sky.IM"
#define NAMEMASK_TRIGGER_TEST			"T${OBSID}_SAP${SAP}_B${BEAM}_S${STOKES}_P${PART}_bf.trigger"
#define DIRMASK_COHERENT_STOKES			"L${OBSID}"
#define DIRMASK_INCOHERENT_STOKES		"L${OBSID}"
#define DIRMASK_CORRELATED				"L${OBSID}"
#define DIRMASK_TRIGGER					"L${OBSID}"
#define DIRMASK_INSTRUMENT_MODEL		"L${OBSID}"
#define DIRMASK_PULSAR          		"L${OBSID}"
#define DIRMASK_SKY_IMAGE				"L${OBSID}"
#define DIRMASK_COHERENT_STOKES_TEST	"L${OBSID}"
#define DIRMASK_INCOHERENT_STOKES_TEST	"L${OBSID}"
#define DIRMASK_CORRELATED_TEST			"L${OBSID}"
#define DIRMASK_TRIGGER_TEST			"L${OBSID}"
#define DIRMASK_INSTRUMENT_MODEL_TEST	"L${OBSID}"
#define DIRMASK_PULSAR_TEST        		"L${OBSID}"
#define DIRMASK_SKY_IMAGE_TEST			"L${OBSID}"


class campaignInfo {
public:
	unsigned id;
	std::string name;
	std::string title;
	std::string PriInvestigator;
	std::string CoInvestigator;
	std::string contact;
};

typedef std::map<std::string ,campaignInfo> campaignMap;

// DEFINE CONVENIENT TYPES

// tableChanges will contain all changes made by the user in the table view
// its structure is: pair < pair< taskID, column>, change string> >
// first.first = task ID
// first.second = column
// second = changed value

#define PROGRAM_DEFAULT_SETTINGS_FILENAME ".default_settings.set"
#define PROGRAM_PREFERENCES_FILENAME ".scheduler_preferences.pre"

// The following two defines must be kept 'synchronized'
#define MIN_TIME_BETWEEN_TASKS_GREGORIAN AstroTime("00:15:00")
#define MIN_TIME_BETWEEN_TASKS_JULIAN 0.01041666666666666667 // approximately equals AstroTime("00:15:00")

#define MAX_STATION_ID 20
#define MAX_UNSIGNED std::numeric_limits<unsigned int>::max()
#define MAX_TASK_ID MAX_UNSIGNED

#define MAX_DURATION_HOURS 1000

#define MAX_OPTIMIZE_ITERATIONS 1000 // the default maximum number of optimize iterations after which the optimizer will stop

#define MAX_FILES_PER_STORAGE_NODE 20 // the default value for the maximum number of files to write per task to one storage node

#define MAX_TASK_PENALTY 100
#define MAX_DAY_TIME_PENALTY 100 // the maximum penalty assigned to a task when it is fully scheduled at day time
#define UNSCHEDULED_TASK_PENALTY 100

#define MIN_NR_SUBBANDS 0
#define MAX_NR_SUBBANDS 248

#define SHIFT_RIGHT true
#define SHIFT_LEFT false

void debugInfo( std::string szTypes, ... ); // debug info
void debugWarn( std::string szTypes, ... ); // debug warning
void debugErr(  std::string szTypes, ...); // debug error

// x = [0:1:100] = percentage of task scheduled at daytime
// y = night time weight factor (for penalty calculation [0.0, 1.0]
// curve 1: y = 1  ( in other words: task may not have overlap with daytime)
// curve 2: y = 1-c^-X  // c = 1.1
// curve 3: y = 1-c^-x  // c = 1.04
// curve 4: y = x
// curve 5: y = c^(x-100) - 0.019800040113920 // c = 1.04
// curve 6: y = c^(x-100) - 0.000072565715901  // c = 1.1
// curve 7: y = 0 ( in other words: task may be scheduled at daytime without penalty)

// ATTENTION: The sequence of the following enumeration of data headers determines the column sequence of
// the scheduler's table view (amongst others)
enum data_headers {
	TASK_ID = 0,
	SAS_ID,
	MOM_ID,
	GROUP_ID,
	PROJECT_ID,
	TASK_NAME,
	PLANNED_START,
	PLANNED_END,
	TASK_DURATION,
	TASK_TYPE,
	TASK_STATUS,
    CLUSTER_NAME,
    UNSCHEDULED_REASON,
    TASK_DESCRIPTION,
	STATION_ID,
	RESERVATION_NAME,
	PRIORITY,
	FIXED_DAY,
	FIXED_TIME,
	FIRST_POSSIBLE_DATE,
	LAST_POSSIBLE_DATE,
	WINDOW_MINIMUM_TIME,
	WINDOW_MAXIMUM_TIME,
	ANTENNA_MODE,
	CLOCK_FREQUENCY,
	FILTER_TYPE,
	NR_OF_SUBBANDS,
	CONTACT_NAME,
	CONTACT_PHONE,
	CONTACT_EMAIL,
	PREDECESSORS,
	PRED_MIN_TIME_DIF,
	PRED_MAX_TIME_DIF,
	NIGHT_TIME_WEIGHT_FACTOR,
	STORAGE_SIZE,
	_END_DATA_HEADER_ENUM_
};
// define the labels of the columns (sequence has to match the sequence of the data_headers enumeration above.
#define NR_DATA_HEADERS	_END_DATA_HEADER_ENUM_
extern const char * DATA_HEADERS[NR_DATA_HEADERS];

typedef std::map <unsigned int, std::vector<data_headers> > errorTasksMap;

enum selector_types {
    SEL_ALL_TASKS,
    SEL_OBSERVATIONS,
    SEL_MAINTENANCE,
    SEL_RESERVATION,
    SEL_SYSTEM,
    SEL_CALIBRATOR_PIPELINES,
    SEL_TARGET_PIPELINES,
    SEL_IMAGING_PIPELINES,
    SEL_LONGBASELINE_PIPELINES,
    SEL_PREPROCESSING_PIPELINES,
    SEL_PULSAR_PIPELINES,
    SEL_UNKNOWN
};

#endif /* LOFAR_SCHEDULER_H_ */
