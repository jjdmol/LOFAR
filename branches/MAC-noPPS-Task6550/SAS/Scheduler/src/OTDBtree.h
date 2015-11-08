/*
 * OTDBtree.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 9-feb-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/OTDBtree.h $
 * Description    : This 'value' class holds the SAS metadata of a single SAS VIC-tree
 *
 */

#ifndef OTDBtree_H_
#define OTDBtree_H_

enum SAS_task_status {
	SAS_STATE_IDLE   = 0,
	SAS_STATE_DESCRIBED = 100,
	SAS_STATE_PREPARED = 200,
	SAS_STATE_APPROVED = 300, // the state where the observation has been approved by all parties
	SAS_STATE_ON_HOLD = 320,
	SAS_STATE_CONFLICT = 335,
	SAS_STATE_PRESCHEDULED = 350,
	SAS_STATE_SCHEDULED = 400, // the state scheduled if the observation has a start and stop time and can be started
	SAS_STATE_QUEUED = 500, // the state the SAS observation is being started
	SAS_STATE_ACTIVE = 600, // when the SAS observation is running
	SAS_STATE_COMPLETING = 900,
	SAS_STATE_FINISHED = 1000,
	SAS_STATE_ABORTED = 1100,
	SAS_STATE_ERROR = 1150,
	SAS_STATE_OBSOLETE = 1200
};

extern const char *SAS_states_strings[15];

enum processSubTypes {
	PST_AVERAGING_PIPELINE,
	PST_BEAM_OBSERVATION,
	PST_CALIBRATION_PIPELINE,
	PST_IMAGING_PIPELINE,
    PST_MSSS_IMAGING_PIPELINE,
    PST_LONG_BASELINE_PIPELINE,
    PST_PULSAR_PIPELINE,
	PST_INTERFEROMETER,
	PST_STAND_ALONE,
	PST_TBB_PIGGYBACK,
	PST_TBB_STANDALONE,
	PST_EMPTY,
	PST_UNKNOWN,
	_END_PROCESS_SUBTYPE_ENUM_
};

#define NR_PROCESS_SUBTYPES _END_PROCESS_SUBTYPE_ENUM_
extern const char * PROCESS_SUBTYPES[NR_PROCESS_SUBTYPES];

#include <string>
#include <QString>
#include <QSqlQuery>
#include "astrodatetime.h"

extern processSubTypes stringToProcessSubType(const QString &str);
std::string sasStateString(int sas_state);

class OTDBtree {
public:
	OTDBtree();
	OTDBtree(const QSqlQuery &query);
	~OTDBtree();

	inline quint32 treeID(void) const {return itsTreeID;}
	inline quint32 momID(void) const {return itsMomID;}
	inline quint32 groupID(void) const {return itsGroupID;}
	inline quint16 classification(void) const {return itsClassification;}
	inline const std::string &creator(void) const {return itsCreator;}
	inline const AstroDateTime &creationDate(void) const {return itsCreationDate;}
	inline const AstroDateTime &modificationDate(void) const {return itsModificationDate;}
	inline const QString &processType(void) const {return itsProcessType;}
	inline processSubTypes processSubType(void) const {return itsProcessSubtype;}
	inline const char *getProcessSubtypeStr(void) const {return PROCESS_SUBTYPES[itsProcessSubtype];}
	inline const QString &strategy(void) const {return itsStrategy;}
	quint16 type(void) const {return itsTreeType;}
	SAS_task_status state(void) const {return itsTreeState;}
	inline const std::string &description(void) const {return itsDescription;}
	inline int originalTree(void) const {return itsOriginalTree;}
	inline const std::string &campaign(void) const {return itsCampaign;}
	inline const AstroDateTime &startTime(void) const {return itsStarttime;}
	inline const AstroDateTime &stopTime(void) const {return itsStoptime;}

    void setProcessType(const QString &ptype) {itsProcessType = ptype;}
    void setProcessSubtype(processSubTypes pstype) {itsProcessSubtype = pstype;}
    void setStrategy(const QString &strategy) {itsStrategy = strategy;}
    void setOriginalTreeID(int parentID) {itsOriginalTree = parentID;}
	void setState(SAS_task_status state) {itsTreeState = state;}
    void resetTimes(void) {itsStarttime.clear(); itsStoptime.clear();}
    void setStartTime(const AstroDateTime &start) {itsStarttime = start;}
    void setStopTime(const AstroDateTime &stop) {itsStoptime = stop;}

	// Show treeinfo
	std::ostream& print (std::ostream& os) const;

	// let SASConnection have access to my data members
//	friend class SASConnection;
	friend class Task;

	friend std::ostream& operator<< (std::ostream &out, const OTDBtree &meta_data);
	friend QDataStream& operator<< (QDataStream &out, const OTDBtree &meta_data); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, OTDBtree &meta_data); // used for reading data from binary file

private:
	quint32 itsTreeID, itsMomID, itsGroupID;
	quint16 itsClassification;     // development / test / operational
	std::string	itsCreator;
	AstroDateTime itsCreationDate, itsModificationDate; // in SAS software this is a boost ptime object
	QString itsProcessType, /*itsProcessSubtype, */itsStrategy;
	processSubTypes itsProcessSubtype;
	quint16 itsTreeType;			  // hardware / VItemplate / VHtree
	SAS_task_status itsTreeState;			  // idle / configure / ... / active / ...
	std::string itsDescription;  // free text
	quint32	itsOriginalTree;
	std::string itsCampaign;
	AstroDateTime itsStarttime; // in SAS software this is a boost ptime object
	AstroDateTime itsStoptime; // in SAS software this is a boost ptime object
};

#endif /* OTDBtree_H_ */
