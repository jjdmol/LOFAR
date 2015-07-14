/*
 * OTDBtree.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 9-feb-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/OTDBtree.cpp $
 * Description    : This 'value' class holds the SAS metadata of a single SAS VIC-tree
 *
 */

#include "OTDBtree.h"
#include "lofar_utils.h"
#include <QSqlRecord>
#include <QVariant>
#include <QDateTime>
#include <iostream>
using std::endl;

const char *SAS_states_strings[15] = {"IDLE", "DESCRIBED", "PREPARED", "APPROVED", "ON_HOLD", "CONFLICT",
			"PRESCHEDULED", "SCHEDULED", "QUEUED", "ACTIVE", "COMPLETING", "FINISHED", "ABORTED", "ERROR", "OBSOLETE"};

// TODO: Upgrade SAS_states_strings to sas_state to string map create a config header which is
//       included in all the sources
std::string sasStateString(int sas_state) {
	switch (sas_state) {
	case SAS_STATE_IDLE:
		return SAS_states_strings[0];
		break;
	case SAS_STATE_DESCRIBED:
		return SAS_states_strings[1];
		break;
	case SAS_STATE_PREPARED:
		return SAS_states_strings[2];
		break;
	case SAS_STATE_APPROVED:
		return SAS_states_strings[3];
		break;
	case SAS_STATE_ON_HOLD:
		return SAS_states_strings[4];
		break;
	case SAS_STATE_CONFLICT:
		return SAS_states_strings[5];
		break;
	case SAS_STATE_PRESCHEDULED:
		return SAS_states_strings[6];
		break;
	case SAS_STATE_SCHEDULED:
		return SAS_states_strings[7];
		break;
	case SAS_STATE_QUEUED:
		return SAS_states_strings[8];
		break;
	case SAS_STATE_ACTIVE:
		return SAS_states_strings[9];
		break;
	case SAS_STATE_COMPLETING:
		return SAS_states_strings[10];
		break;
	case SAS_STATE_FINISHED:
		return SAS_states_strings[11];
		break;
	case SAS_STATE_ABORTED:
		return SAS_states_strings[12];
		break;
	case SAS_STATE_ERROR:
		return SAS_states_strings[13];
		break;
	case SAS_STATE_OBSOLETE:
		return SAS_states_strings[14];
		break;
	}
	return "Unknown";
}

const char * PROCESS_SUBTYPES[NR_PROCESS_SUBTYPES] = {"Averaging Pipeline", "Beam Observation", "Calibration Pipeline",
    "Imaging Pipeline", "Imaging Pipeline MSSS", "Long Baseline Pipeline", "Pulsar Pipeline",
    "Interferometer", "STAND_ALONE", "TBB (piggyback)", "TBB (standalone)", "", "Unknown Process Subtype"};

processSubTypes stringToProcessSubType(const QString &str) {
    for (short i = 0; i < _END_PROCESS_SUBTYPE_ENUM_; ++i) {
        if (str.toUpper().compare(QString(PROCESS_SUBTYPES[i]).toUpper()) == 0) {
            return static_cast<processSubTypes>(i);
        }
    }
    return static_cast<processSubTypes>(PST_UNKNOWN);
}

OTDBtree::OTDBtree()
:	itsTreeID(0), itsMomID(0), itsGroupID(0), itsClassification(0), itsCreator("scheduler"), itsProcessSubtype(PST_UNKNOWN), itsTreeType(0),
	itsTreeState(SAS_STATE_IDLE), itsOriginalTree(0)
{
	// default constructor is ok
}

OTDBtree::OTDBtree(const QSqlQuery &query) {
	// treeid
	itsTreeID = query.value(query.record().indexOf("treeid")).toInt();
	// momid
    itsMomID = query.value(query.record().indexOf("momid")).toInt();
	// groupid
    itsGroupID = query.value(query.record().indexOf("groupid")).toInt();
	// classification
    itsClassification = query.value(query.record().indexOf("classification")).toInt();
	// creator
	itsCreator = query.value(query.record().indexOf("creator")).toString().toStdString();
	// creationDate
    itsCreationDate = AstroDateTime(query.value(query.record().indexOf("creationdate")).toDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
	// creationDate
    itsModificationDate = AstroDateTime(query.value(query.record().indexOf("modificationdate")).toDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
    // type
    itsTreeType = query.value(query.record().indexOf("type")).toInt();
	// state
	itsTreeState = static_cast<SAS_task_status>(query.value(query.record().indexOf("state")).toInt());
	// processType
    itsProcessType = query.value(query.record().indexOf("processtype")).toString().toUpper();
	// processSubtype
    itsProcessSubtype = stringToProcessSubType(query.value(query.record().indexOf("processsubtype")).toString());
	// strategy
	itsStrategy = query.value(query.record().indexOf("strategy")).toString();
	// originalTree
    itsOriginalTree = query.value(query.record().indexOf("originaltree")).toInt();
	// campaign
	itsCampaign = query.value(query.record().indexOf("campaign")).toString().toStdString();
	// starttime
	if (!query.value(query.record().indexOf("starttime")).isNull())
		itsStarttime = AstroDateTime(query.value(query.record().indexOf("starttime")).toDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
	// stoptime
	if (!query.value(query.record().indexOf("stoptime")).isNull())
		itsStoptime = AstroDateTime(query.value(query.record().indexOf("stoptime")).toDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
	// description
	itsDescription = query.value(query.record().indexOf("description")).toString().toStdString();
}

OTDBtree::~OTDBtree() {
	// default destructor is ok
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
std::ostream& OTDBtree::print (std::ostream& os) const
{
	os
	<< "treeID          : " << itsTreeID << endl
	<< "momID           : " << itsMomID << endl
	<< "groupID         : " << itsGroupID << endl
	<< "classification  : " << itsClassification << endl
	<< "creator         : " << itsCreator << endl
	<< "creationDate    : " << itsCreationDate.toString() << endl
	<< "modificationDate: " << itsModificationDate.toString() << endl
	<< "type            : " << itsTreeType << endl
	<< "state           : " << itsTreeState << endl
	<< "processType     : " << itsProcessType.toStdString() << endl
	<< "processSubType  : " << std::string(getProcessSubtypeStr()) << endl
	<< "strategy        : " << itsStrategy.toStdString() << endl
	<< "originalTree    : " << itsOriginalTree << endl
	<< "campaign        : " << itsCampaign << endl
	<< "starttime       : " << itsStarttime.toString() << endl
	<< "stoptime        : " << itsStoptime.toString() << endl
	<< "description     : " << itsDescription << endl;

	return (os);
}

std::ostream& operator<< (std::ostream &out, const OTDBtree &meta_data) {
	out << meta_data.itsTreeID
	    << meta_data.itsMomID
	    << meta_data.itsGroupID
	    << meta_data.itsOriginalTree
	    << meta_data.itsClassification
	    << meta_data.itsTreeType
	    << (quint16) meta_data.itsTreeState
	    << meta_data.itsProcessType.toStdString() << std::string(PROCESS_SUBTYPES[meta_data.itsProcessSubtype]) << meta_data.itsStrategy.toStdString()
	    << meta_data.itsCreator
	    << meta_data.itsCampaign
	    << meta_data.itsDescription
	    << meta_data.itsCreationDate.toString()
	    << meta_data.itsModificationDate.toString()
	    << meta_data.itsStarttime.toString()
	    << meta_data.itsStoptime.toString();

	return out;
}

QDataStream& operator<< (QDataStream &out, const OTDBtree &meta_data) {
	out << meta_data.itsTreeID
	    << meta_data.itsMomID
	    << meta_data.itsGroupID
	    << meta_data.itsOriginalTree
	    << meta_data.itsClassification
	    << meta_data.itsTreeType
	    << (quint16) meta_data.itsTreeState
	    << meta_data.itsProcessType.toStdString() << std::string(PROCESS_SUBTYPES[meta_data.itsProcessSubtype]) << meta_data.itsStrategy.toStdString()
	    << meta_data.itsCreator
	    << meta_data.itsCampaign
	    << meta_data.itsDescription
	    << meta_data.itsCreationDate
	    << meta_data.itsModificationDate
	    << meta_data.itsStarttime
	    << meta_data.itsStoptime;

	return out;
}

QDataStream& operator>> (QDataStream &in, OTDBtree &meta_data) {
	in >> meta_data.itsTreeID
	   >> meta_data.itsMomID
	   >> meta_data.itsGroupID
	   >> meta_data.itsOriginalTree
	   >> meta_data.itsClassification
	   >> meta_data.itsTreeType;
	quint16 treestate;
	QString strVal;
	in >> treestate;
	meta_data.itsTreeState = (SAS_task_status) treestate;
    in >> meta_data.itsProcessType;
    in >> strVal;
    meta_data.itsProcessSubtype = stringToProcessSubType(strVal);
    in >> meta_data.itsStrategy
	   >> meta_data.itsCreator
	   >> meta_data.itsCampaign
	   >> meta_data.itsDescription
	   >> meta_data.itsCreationDate
	   >> meta_data.itsModificationDate
	   >> meta_data.itsStarttime
	   >> meta_data.itsStoptime;

	return in;
}
