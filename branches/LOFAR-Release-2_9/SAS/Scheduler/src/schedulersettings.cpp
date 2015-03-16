/*
 * schedulersettings.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date    : $Date$
 * First creation : Aug 6, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/schedulersettings.cpp $
 *
 */

#include "schedulersettings.h"
#include "astrodatetime.h"
#include "Controller.h"
#include <cmath>
#include <QDate>

SchedulerSettings::SchedulerSettings()
	: itsMinTimeBetweenTasks(MIN_TIME_BETWEEN_TASKS_GREGORIAN), uniqueStationID(1), itsAllowMultipleRaidPerNode(false), itsStorageNodeBandWidth(STORAGE_NODE_kbps), itsRaidMaxWriteSpeed(STORAGE_RAID_WRITE_KBS),
	  itsStorageFillPercentage(STORAGE_FILL_PECENTAGE), itsUserAcceptedPenaltyEnabled(false), itsMaxNrOptimizationsEnabled(true), itsAllowUnscheduleFixedTasks(false),
	itsLoadDefaultSettings(true), itsfocusTaskAtClick(true), itsIsTestEnvironment(false), itsUserAcceptedPenalty(0), itsMaxNrOptimizations(MAX_OPTIMIZE_ITERATIONS), itsMaxNrOfFilesPerStorageNode(MAX_FILES_PER_STORAGE_NODE),
    itsPublishLocal(false)
{
	QDate day = QDate::currentDate();
	day = day.addDays(-day.dayOfWeek() + 1); // start on monday
	itsEarliestDay = AstroDate(day.day(), day.month(), day.year());
	day = day.addMonths(6);
	day = day.addDays(7 - day.dayOfWeek()); // end on Sunday
	itsLatestDay = AstroDate(day.day(), day.month(), day.year());
	itsNrOfDaysInSchedule = itsLatestDay.toJulian() - itsEarliestDay.toJulian();
	wrongSunRiseSetValue = std::pair<double, double>(-1,-1);
	itsObservationIDprefix= "L";
}

SchedulerSettings::~SchedulerSettings() {
}

QDataStream& operator<< (QDataStream &out, const SchedulerSettings &settings) {
	out	<< (quint16) settings.uniqueStationID
	<< (quint16) settings.itsUserAcceptedPenalty
	<< (quint16) settings.itsMaxNrOptimizations
	<< (quint16) settings.itsNrOfDaysInSchedule
	<< (quint16) settings.itsShortTermScheduleDuration
	<< (quint16) settings.itsScheduleDuration
	<< (quint16) settings.itsMaxNrOfFilesPerStorageNode
	<< settings.itsUserAcceptedPenaltyEnabled
	<< settings.itsMaxNrOptimizationsEnabled
	<< settings.itsAllowUnscheduleFixedTasks
	<< settings.itsPublishLocal
	<< settings.itsLoadDefaultSettings
	<< settings.itsIsTestEnvironment
	<< settings.itsObservationIDprefix
	<< settings.itsStorageNodeBandWidth
	<< settings.itsRaidMaxWriteSpeed
	<< (quint8) settings.itsStorageFillPercentage
	<< settings.itsSASUserName
	<< settings.itsSASPassword
	<< settings.itsSASDatabase
	<< settings.itsSASHostName
	<< settings.itsDMUserName
	<< settings.itsDMPassword
	<< settings.itsDMDatabase
	<< settings.itsDMHostName
	<< settings.itsLocalPublishPath
	<< settings.itsSchedulerAccountName
	<< settings.itssWebServerName
	<< settings.itsPrivateKeyFile
	<< settings.itsWebServerPublishPath
	<< settings.itsMinTimeBetweenTasks
	<< settings.itsDemixSources

	// save default template information
	<< (quint32) settings.itsDefaultTemplates.size();
	for (std::map<quint32, DefaultTemplate>::const_iterator it = settings.itsDefaultTemplates.begin(); it != settings.itsDefaultTemplates.end(); ++it) {
		out << it->first
			<< it->second.treeID
			<< it->second.name
			<< (quint16) it->second.status
			<< it->second.processType
			<< it->second.processSubtype
			<< it->second.strategy
			<< it->second.description;
	}

	// write campaigns from settings
	out << (quint32) settings.itsCampaigns.size();
	for (campaignMap::const_iterator it = settings.itsCampaigns.begin(); it != settings.itsCampaigns.end(); ++it) {
		out << it->second.id // id
		<< it->second.name
		<< it->second.title
		<< it->second.PriInvestigator
		<< it->second.CoInvestigator
		<< it->second.contact;
	}

	out << (quint32) settings.itsStationList.size();
	for (stationDefinitionsMap::const_iterator it = settings.itsStationList.begin(); it != settings.itsStationList.end(); ++it) {
		out << it->first
		<< it->second.first
		<< it->second.second;
	}
	out << (quint32) settings.itsStationNameIDMapping.size();
	for (stationNameIDMapping::const_iterator it = settings.itsStationNameIDMapping.begin(); it != settings.itsStationNameIDMapping.end(); ++it) {
		out <<  it->first
		<< (quint16)it->second;
	}
	out << (quint32) settings.itsStationPositions.size();
	for (stationPositionsMap::const_iterator it = settings.itsStationPositions.begin(); it != settings.itsStationPositions.end(); ++it) {
		out << (quint16)it->first
		<< it->second.first
		<< it->second.second;
	}

	// write storage nodes name->ID mapping
	const storageHostsMap &storagenodes = settings.itsController->getStorageNodes();
	out << (quint32)storagenodes.size();
	for (storageHostsMap::const_iterator it = storagenodes.begin(); it != storagenodes.end(); ++it) {
		out << (quint16) it->second.itsID // storage node ID
		<< it->second.itsName // storage node name
		<< it->second.itsMayBeUsed; // may this storage node be used?
	}

	// write storage nodes definitions
	const hostPartitionsMap &partitions = settings.itsController->getStoragePartitions();
	out << (quint32)partitions.size();
	for (hostPartitionsMap::const_iterator it = partitions.begin(); it != partitions.end(); ++it) {
		out << it->first // storage node ID
		<< (quint32)it->second.size(); // size of storagePartitionsMap
		for (dataPathsMap::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
			out << (quint16) sit->first // partition ID
			<< sit->second.first // partition name
			<< (quint64) sit->second.second[0] // total size of partition
			<< (quint64) sit->second.second[1] // used space on partition
			<< (quint64) sit->second.second[2] // claimed space on partition
			<< (quint64) sit->second.second[3]; // free space on partition
		}
	}

	// data distribution scheme
	out << (quint8) settings.itsDataDistributionScheme;

	// preferred storage nodes per data product type
	out << (quint32) settings.itsPreferredDataProductStorage.size();
	for (preferredDataProductStorageMap::const_iterator it = settings.itsPreferredDataProductStorage.begin(); it != settings.itsPreferredDataProductStorage.end(); ++it) {
		out << (quint16) it->first
			<< (quint32) it->second.size();
			for (std::vector<int>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
				out << *sit;
			}
	}

	// preferred storage nodes per project
	out << (quint32) settings.itsPreferredProjectStorage.size();
	for (preferredProjectStorageMap::const_iterator it = settings.itsPreferredProjectStorage.begin(); it != settings.itsPreferredProjectStorage.end(); ++it) {
		out << (quint16)it->first
			<< (quint32) it->second.size();
			for (std::vector<int>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
				out << *sit;
			}
	}

	return out;
}

QDataStream& operator>> (QDataStream &in, SchedulerSettings &settings) {
	quint32 size, size2;
	quint8 uint8;
	quint16 uint16;
	std::string str;
	std::vector<std::pair<double, double> > vec;
	std::pair<double, double> dPair;
	settings.itsStationList.clear();
	settings.itsStationNameIDMapping.clear();
	settings.itsStationPositions.clear();
	settings.itsStationSunMap.clear();

	in >> settings.uniqueStationID
	   >> settings.itsUserAcceptedPenalty
	   >> settings.itsMaxNrOptimizations
	   >> settings.itsNrOfDaysInSchedule
	   >> settings.itsShortTermScheduleDuration
	   >> settings.itsScheduleDuration
	   >> settings.itsMaxNrOfFilesPerStorageNode
	   >> settings.itsUserAcceptedPenaltyEnabled
	   >> settings.itsMaxNrOptimizationsEnabled
	   >> settings.itsAllowUnscheduleFixedTasks
	   >> settings.itsPublishLocal
	   >> settings.itsLoadDefaultSettings
	   >> settings.itsIsTestEnvironment
	   >> settings.itsObservationIDprefix
//	   >> dummy
	   >> settings.itsStorageNodeBandWidth
	   >> settings.itsRaidMaxWriteSpeed
	   >> settings.itsStorageFillPercentage
	   >> settings.itsSASUserName
	   >> settings.itsSASPassword
	   >> settings.itsSASDatabase
	   >> settings.itsSASHostName
	   >> settings.itsDMUserName
	   >> settings.itsDMPassword
	   >> settings.itsDMDatabase
	   >> settings.itsDMHostName
	   >> settings.itsLocalPublishPath
	   >> settings.itsSchedulerAccountName
	   >> settings.itssWebServerName
	   >> settings.itsPrivateKeyFile
	   >> settings.itsWebServerPublishPath
	   >> settings.itsMinTimeBetweenTasks
	   >> settings.itsDemixSources;

	// read default template information
	settings.itsDefaultTemplates.clear();
	DefaultTemplate t;
	quint32 key;
	in >> size;
	while (size--) {
		in >> key >> t.treeID >> t.name >> t.status >> t.processType >> t.processSubtype >> t.strategy >> t.description;
		settings.itsDefaultTemplates.insert(std::map<quint32, DefaultTemplate>::value_type(key,t));
	}

	settings.itsController->defaultTemplatesUpdated();

	// read campaigns from settings
	campaignInfo campaign;
	settings.clearCampaigns();
	in >> size;
	while (size--) {
		in 	>> campaign.id // id
			>> campaign.name
			>> campaign.title
			>> campaign.PriInvestigator
			>> campaign.CoInvestigator
			>> campaign.contact;
		settings.addCampaign(campaign);
	}

	in  >> size;
	while (size--) {
		in >> str >> dPair.first >> dPair.second;
		settings.itsStationList.insert(stationDefinitionsMap::value_type (str, dPair));
	}
	in >> size;
	while (size--) {
		in >> str >> uint16;
		settings.itsStationNameIDMapping.push_back(stationNameIDMapping::value_type (str, uint16));
	}
	in >> size;
	while (size--) {
		in >> uint16 >> dPair.first >> dPair.second;
		settings.itsStationPositions.insert(stationPositionsMap::value_type (uint16, dPair));
	}

	// storage nodes
	storageHostsMap storagenodes;
	// read storage nodes name->ID mapping
	StorageHost newStorageHost;
	newStorageHost.itsStatus = -1; // unknown status

	in >> size;
	while (size--) {
		in >> newStorageHost.itsID // storage node ID
		   >> newStorageHost.itsName // storage node name
		   >> newStorageHost.itsMayBeUsed; // may this storage node be used?
		storagenodes[newStorageHost.itsID] = newStorageHost;
	}
	settings.itsController->setStorageNodes(storagenodes);

	// read storage nodes definitions
	hostPartitionsMap partitions;
	quint16 partitionID;
	dataPathsMap dataPaths;
	quint64 space;
	int nodeID;
	std::vector<quint64> sizeVec;
	std::pair<std::string, std::vector<quint64> > partitionPair;
	in >> size;
	while (size--) { // iterate over itsStorageNodesPartitions
		in >> nodeID // storage node ID
		   >> size2; // size of dataPathsMap
		while (size2--) {
			in >> partitionID // partition ID
			   >> partitionPair.first // partition name
			   >> space; // total size of partition
			sizeVec.push_back(space);
			in >> space; // used space on partition
			sizeVec.push_back(space);
			in >> space; // claimed space on partition
			sizeVec.push_back(space);
			in >> space; // free space on partition
			sizeVec.push_back(space);
			partitionPair.second = sizeVec;
			dataPaths[partitionID] = partitionPair;
			sizeVec.clear();
		}
		partitions[nodeID] = dataPaths;
		dataPaths.clear();
	}
	settings.itsController->setStoragePartitions(partitions);

	// data distribution scheme

	in >> uint8;
	settings.itsDataDistributionScheme = static_cast<storageNodeDistribution>(uint8);

	// preferred storage nodes per data product type
	preferredDataProductStorageMap newDPSMap;
	std::vector<int> nodes;
	quint16 dataProduct;
	in >> size;
	while (size--) {
		in >> dataProduct >> size2;
		while (size2--) {
			in >> nodeID;
			nodes.push_back(nodeID);
		}
		newDPSMap[static_cast<dataProductTypes>(dataProduct)] = nodes;
		nodes.clear();
	}
	settings.itsPreferredDataProductStorage = newDPSMap;

	// preferred storage nodes per project
	preferredProjectStorageMap newPSMap;
	quint16 projectID;
	in >> size;
	while (size--) {
		in >> projectID >> size2;
		while (size2--) {
			in >> nodeID;
			nodes.push_back(nodeID);
		}
		newPSMap[projectID] = nodes;
		nodes.clear();
	}
	settings.itsPreferredProjectStorage = newPSMap;

	// also recalculate sunrises and sundowns again
	for (stationPositionsMap::const_iterator it = settings.itsStationPositions.begin(); it != settings.itsStationPositions.end(); ++it) {
		settings.calculateSunRisesAndSunDowns(it->first);
	}
	settings.calculateScheduleWeeks();

	return in;
}

void SchedulerSettings::updateSettings(const ScheduleSettingsDialog *settingsDialog) {
	setUserAcceptedPenalty(settingsDialog->getUserAcceptedPenalty());
	setUserAcceptedPenaltyEnabled(settingsDialog->getUserAcceptedPenaltyEnabled());
	setMaxNrOptimizations(settingsDialog->getMaxNrOptimizeIterations());
	setmaxNrOptimizationsEnabled(settingsDialog->getmaxNrOptimizationsEnabled());
	setAllowUnscheduleFixedTasks(settingsDialog->getAllowUnscheduleFixedTasks());
	setMinTimeBetweenTasks(settingsDialog->getMinimumTimeBetweenObservations());
	setLoadDefaultSettingsOnStartup(settingsDialog->getLoadDefaultSettingsOnStartUp());
	setShortTermScheduleDurationWeeks(settingsDialog->getShortTermScheduleDurationWeeks());
	setScheduleDurationMonths(settingsDialog->getScheduleDurationMonths());
	setMaxNrOfFilesPerStorageNode(settingsDialog->getMaxNrOfFilesPerStorageNode());
	setSASDatabase(settingsDialog->getSASDatabase());
	setSASHostName(settingsDialog->getSASHostName());
	setSASUserName(settingsDialog->getSASUserName());
	setSASPassword(settingsDialog->getSASPassword());
	setDMDatabase(settingsDialog->getDMDatabase());
	setDMHostName(settingsDialog->getDMHostName());
	setDMUserName(settingsDialog->getDMUserName());
	setDMPassword(settingsDialog->getDMPassword());
	setLocalPublishPath(settingsDialog->getLocalPublishPath());
    setAutoPublish(settingsDialog->getAutoPublish());
	setSchedulerAccountName(settingsDialog->getSchedulerAccountName());
	setPrivateKeyFile(settingsDialog->getPrivateKeyFile());
	setWebServerName(settingsDialog->getWebServerName());
	setWebServerPublishPath(settingsDialog->getWebServerPublishPath());
	setPublishLocal(settingsDialog->publishLocal());
	setStorageNodeBandWidth(settingsDialog->getStorageNodeBandWidth());
	setRaidMaxWriteSpeed(settingsDialog->getRaidMaxWriteSpeed());
	setDemixSources(settingsDialog->getDemixSources());
	// test environment mode?
	setTestEnvironmentMode(settingsDialog->getIsTestEnvironment());
}

void SchedulerSettings::setTestEnvironmentMode(bool enableTestMode) {
	itsIsTestEnvironment = enableTestMode;
	itsObservationIDprefix = enableTestMode ? "T" : "L";
}

quint32 SchedulerSettings::getSASDefaultTreeID(const QString &ptype, const QString & pstype, const QString &strategy) const {
	for (std::map<quint32, DefaultTemplate>::const_iterator it = itsDefaultTemplates.begin(); it != itsDefaultTemplates.end(); ++it) {
		if ((it->second.processType == ptype) && (it->second.processSubtype == pstype) && (it->second.strategy == strategy)) return it->first;
	}
	return 0;
}

quint32 SchedulerSettings::getSASDefaultTreeID(const Task *pTask) const {
	for (std::map<quint32, DefaultTemplate>::const_iterator it = itsDefaultTemplates.begin(); it != itsDefaultTemplates.end(); ++it) {
		if ((it->second.processType == pTask->getProcessType()) && (it->second.processSubtype == pTask->getProcessSubtypeStr()) && (it->second.strategy == pTask->getStrategy())) return it->first;
	}
	return 0;
}


QStringList SchedulerSettings::getAllProcessTypes(void) const {
	QStringList ptypes;
	for (std::map<quint32, DefaultTemplate>::const_iterator it = itsDefaultTemplates.begin(); it != itsDefaultTemplates.end(); ++it) {
		if (it->second.status != SAS_STATE_OBSOLETE) {
			if (!ptypes.contains(it->second.processType)) ptypes.append(it->second.processType);
		}
	}
	ptypes.sort();
	return ptypes;
}


QStringList SchedulerSettings::getAllProcessSubTypes(const QString &processType) const {
	QStringList pstypes;
	for (std::map<quint32, DefaultTemplate>::const_iterator it = itsDefaultTemplates.begin(); it != itsDefaultTemplates.end(); ++it) {
		if (it->second.status != SAS_STATE_OBSOLETE) {
			if (it->second.processType == processType) {
				if (!pstypes.contains(it->second.processSubtype)) pstypes.append(it->second.processSubtype);
			}
		}
	}
	pstypes.sort();
	return pstypes;
}


QStringList SchedulerSettings::getAllStrategies(const QString &processType, const QString &processSubtype) const {
	QStringList strategies;
	for (std::map<quint32, DefaultTemplate>::const_iterator it = itsDefaultTemplates.begin(); it != itsDefaultTemplates.end(); ++it) {
		if (it->second.status != SAS_STATE_OBSOLETE) {
			if ((it->second.processType == processType) & (it->second.processSubtype == processSubtype)) {
				if (!strategies.contains(it->second.strategy)) strategies.append(it->second.strategy);
			}
		}
	}
	strategies.sort();
	return strategies;
}

const std::map<quint32, DefaultTemplate> &SchedulerSettings::updateDefaultTemplates(void) {
	clearDefaultTemplates();
	itsSchedulerDefaultTemplate = 0;
	itsDefaultTemplates.clear();
	std::vector<DefaultTemplate> defaultTemplates = itsController->getDefaultTemplatesFromSAS();
	for (std::vector<DefaultTemplate>::const_iterator it = defaultTemplates.begin(); it != defaultTemplates.end(); ++it) {
		itsDefaultTemplates.insert(std::map<quint32, DefaultTemplate>::value_type(it->treeID, *it));
		if (it->name == "Scheduler default template") {
			itsSchedulerDefaultTemplate = it->treeID;
		}
	}
	if (itsSchedulerDefaultTemplate == 0) {
		debugErr("s","Warning: Scheduler default template does not exist in SAS database!");
	}
	itsController->defaultTemplatesUpdated();
	return itsDefaultTemplates;
}

void SchedulerSettings::defineScheduleAndStations(const stationDefinitionsMap &stations, const AstroDate &start_day, const AstroDate &end_day) {
	if (end_day > start_day) {
		if ((start_day != itsEarliestDay) || (end_day != itsLatestDay)) {
			itsEarliestDay = start_day;
			itsLatestDay = end_day;
			itsNrOfDaysInSchedule = end_day.toJulian() - start_day.toJulian();
			itsStationSunMap.clear();
		}
		defineStations(stations, true);
		calculateScheduleWeeks();
	} else {
		itsEarliestDay.set_MJDay(0);
		itsNrOfDaysInSchedule = 0;
		std::cerr << "ERROR: last scheduling day is earlier than first scheduling day"	<< std::endl;
	}
}

void SchedulerSettings::setStationPosition(unsigned int stationID, const double &latitude, const double &longitude) {
	itsStationPositions[stationID] = std::pair<double, double>(latitude, longitude);
}

unsigned int SchedulerSettings::getStationID(const std::string &station_name) const {
	for (stationNameIDMapping::const_iterator it = itsStationNameIDMapping.begin(); it != itsStationNameIDMapping.end(); ++it) {
		if (it->first.compare(station_name) == 0) return it->second;
	}
	return 0;
}

std::string SchedulerSettings::getStationName(unsigned int station_id) const {
	for (stationNameIDMapping::const_iterator it = itsStationNameIDMapping.begin(); it != itsStationNameIDMapping.end(); ++it) {
		if (it->second == station_id) {
			return it->first;
		}
	}
	return "";
}

bool SchedulerSettings::getStationPosition(unsigned int stationID, std::pair<double, double> &position) {
	stationPositionsMap::const_iterator it = itsStationPositions.find(stationID);
	if (it != itsStationPositions.end()) {
		position = it->second;
		return true;
	}
	else {
		position.first = 0;
		position.second = 0;
		return false;
	}
}

const std::pair<double, double> & SchedulerSettings::getStationSunRiseAndSet(unsigned int stationID, const AstroDate & day) const {
	if ((day >= itsEarliestDay) & (day < itsEarliestDay + itsNrOfDaysInSchedule) ) {
		stationSunMap::const_iterator cit = itsStationSunMap.find(stationID);
		if (cit != itsStationSunMap.end()) {
			return (cit->second)[(day - itsEarliestDay).toJulian()];
		}
	}
	return wrongSunRiseSetValue; //itsStationSunMap.begin()->second.front(); // return 'wrong' value
}

const std::vector<std::pair<double, double> > &SchedulerSettings::getStationSunVector(unsigned int stationID) const {
	stationSunMap::const_iterator cit = itsStationSunMap.find(stationID);
	return cit->second;
}

void SchedulerSettings::defineStations(const stationDefinitionsMap &stations, bool recalculateExistingStations) {
	itsStationList = stations;
	// clean up no longer existing stations from itsStationPositions,  itsStationSunMap
	// and itsStationNameIDMapping
	if (recalculateExistingStations) {
		itsStationSunMap.clear();
	}
	// check which stations to keep and which to delete
	std::vector<std::pair<std::string, unsigned int> > deleteStations;
	bool keep;
	for (stationNameIDMapping::const_iterator it = itsStationNameIDMapping.begin(); it != itsStationNameIDMapping.end(); ++it) {
		keep = false;
		for (stationDefinitionsMap::const_iterator sit = itsStationList.begin(); sit != itsStationList.end(); ++sit) { // check if existing station name is still in the new list (itsStationList)
			if (sit->first.compare(it->first) == 0) {
				keep = true;
				break;
			}
		}
		if (!keep) {
			deleteStations.push_back(*it);
		}
	}

	// now delete references for the 'stations to delete' in the other station maps
	if (!deleteStations.empty()) {
		stationPositionsMap::iterator spit;
		stationSunMap::iterator ssit;
		for (std::vector<std::pair<std::string, unsigned int> >::const_iterator it = deleteStations.begin(); it != deleteStations.end(); ++it) {
			for (stationNameIDMapping::iterator snit = itsStationNameIDMapping.begin(); snit != itsStationNameIDMapping.end(); ++snit) {
				if (snit->second == it->second) { // same station ID?
					itsStationNameIDMapping.erase(snit);
					break;
				}
			}
			if ((spit = itsStationPositions.find(it->second)) != itsStationPositions.end())
				itsStationPositions.erase(spit);
			if ((ssit = itsStationSunMap.find(it->second)) != itsStationSunMap.end())
				itsStationSunMap.erase(ssit);
		}
	}
	// at last we create the stations that have been newly added and we possibly update the stations that existed but might have different coordinates
	stationNameIDMapping::const_iterator sit;
	if (!(stations.empty())) {
		for (stationDefinitionsMap::const_iterator it = stations.begin(); it != stations.end(); ++it) {
			for (sit = itsStationNameIDMapping.begin(); sit != itsStationNameIDMapping.end(); ++sit) {
				if (sit->first.compare(it->first) == 0) break; // found
			}
			if (sit == itsStationNameIDMapping.end()) { // new station name
				itsStationNameIDMapping.push_back(stationNameIDMapping::value_type(it->first, uniqueStationID));
				itsStationPositions[uniqueStationID] = it->second;
				calculateSunRisesAndSunDowns(uniqueStationID++);
			}
			else { // existing station, see if position was changed by user or we need to recalc because e.g. schedule time span has changed
				if ((fabs(itsStationPositions[sit->second].first - it->second.first) > 1e-15) ||
						(fabs(itsStationPositions[sit->second].second - it->second.second) > 1e-15) ||
						recalculateExistingStations) {
					itsStationPositions[sit->second] = it->second;
					calculateSunRisesAndSunDowns(sit->second);
				}
			}
		}
		// now sort the stations in itsStationNameIDMapping according to the
		// sorting sequence: CSxxx, RSxxx and then all other stations alphabetically
		// currently they are sorted alphabetically by the map functionality itself
		stationNameIDMapping sortedMappingCS, sortedMappingRS, sortedMappingOther;
		for (stationNameIDMapping::const_iterator snit = itsStationNameIDMapping.begin(); snit != itsStationNameIDMapping.end(); ++snit) {
			if (snit->first[0] == 'C') {
				sortedMappingCS.push_back(*snit);
			}
		}
		for (stationNameIDMapping::const_iterator snit = itsStationNameIDMapping.begin(); snit != itsStationNameIDMapping.end(); ++snit) {
			if (snit->first[0] == 'R') {
				sortedMappingRS.push_back(*snit);
			}
		}
		for (stationNameIDMapping::const_iterator snit = itsStationNameIDMapping.begin(); snit != itsStationNameIDMapping.end(); ++snit) {
			if ((snit->first[0] != 'R') & (snit->first[0] != 'C')) {
				sortedMappingOther.push_back(*snit);
			}
		}
		sort(sortedMappingCS.begin(), sortedMappingCS.end());
		sort(sortedMappingRS.begin(), sortedMappingRS.end());
		sort(sortedMappingOther.begin(), sortedMappingOther.end());
		itsStationNameIDMapping = sortedMappingCS;
		itsStationNameIDMapping.insert(itsStationNameIDMapping.end(), sortedMappingRS.begin(), sortedMappingRS.end());
		itsStationNameIDMapping.insert(itsStationNameIDMapping.end(), sortedMappingOther.begin(), sortedMappingOther.end());
	}
}

void SchedulerSettings::calculateSunRisesAndSunDowns(unsigned int stationID) {
	// algorithm taken from:
	// The Astronomical Almanac for the year 2008, chapter C24
	// low precision formulas for the Sun's coordinates and the equation of time
	double sunrise, sunset;
	double n, L, g, lambda, eps, ra, dec, lst_rise, lst_set, ha, fday, lam_quadrant, RA_quadrant;
/*
#ifdef DEBUG_SCHEDULER
	for (stationNameIDMapping::const_iterator it = itsStationNameIDMapping.begin(); it != itsStationNameIDMapping.end(); ++it) {
		if (it->second == stationID) {
			std::cout << "Calculating sun rise and set times for station: " << it->first << std::endl;
			std::cout << "Station latitude: " << itsStationPositions[stationID].first << ", longitude: "
			<< itsStationPositions[stationID].second << std::endl;
			break;
		}
	}
#endif
*/
	for (int day = itsEarliestDay.toJulian(); day != itsLatestDay.toJulian(); ++day) {
		// day is the integer day number at noon counted from 0, where 0 equals the day set in julianStartDay
		// returns sunrise and sunset in UTC. To local time add +2 hours
		n = static_cast<double>(day);
		// mean longitude of the sun L, put into range of 0 - 360 degrees
		L = fmod((280.46 + 0.9856474 * n), 360) * GRAD2RAD;
		// Mean anomaly g, put into range of 0 - 360 degrees
		g = fmod((357.528 + 0.9856003 * n), 360) * GRAD2RAD;
		// Ecliptic longitude lambda, put into the range of 0 - 2PI
		lambda = fmod(L + (1.915 * sin(g) + 0.02 * sin(2 * g)) * GRAD2RAD, TWO_PI);
		// Obliquity of ecliptic epsilon
		eps = (23.439 - 0.0000004 * n) * GRAD2RAD;
		ra = fmod(atan(cos(eps) * tan(lambda)), TWO_PI);
		// put ra in the same quadrant as lambda
		lam_quadrant = floor(lambda / PI_DIV2) * PI_DIV2;
		RA_quadrant = floor(ra / PI_DIV2) * PI_DIV2;
		ra = ra + (lam_quadrant - RA_quadrant);

		//Declination
		dec = asin(sin(eps) * sin(lambda));
		ha = fmod(acos(SINSUNRISESETEL - tan(itsStationPositions[stationID].first * GRAD2RAD) * tan(dec)), TWO_PI);
		lst_rise = fmod((ra - ha) * 12 / PI, 24);
		lst_set  = fmod((ra + ha) * 12 / PI, 24);

		fday = fmod((n - 0.5) * DU_LST_LINEAR + DU_LST_CONST + itsStationPositions[stationID].second / 15, 24);
		if (fday > lst_rise) {
			sunrise = n + (lst_rise - (fday - 24)) * LST_DU_LINEAR;
		} else {
			sunrise = n + (lst_rise - fday) * LST_DU_LINEAR;
		}
		if (fday > lst_set) {
			sunset = n + (lst_set - (fday - 24)) * LST_DU_LINEAR;
		} else {
			sunset = n + (lst_set - fday) * LST_DU_LINEAR;
		}
		itsStationSunMap[stationID].push_back(std::pair<double, double>(sunrise, sunset));
	}
}

bool SchedulerSettings::stationExist(const std::string &stationName) const {
	for (stationNameIDMapping::const_iterator it = itsStationNameIDMapping.begin(); it != itsStationNameIDMapping.end(); ++it) {
		if (it->first.compare(stationName) == 0) {
			return true;
		}
	}
	return false;
}

bool SchedulerSettings::addCampaign(const campaignInfo &campaign) {
	std::pair<campaignMap::iterator, bool> retValue;
	retValue = itsCampaigns.insert(campaignMap::value_type(campaign.name, campaign));
	return retValue.second;
}

const campaignInfo & SchedulerSettings::getCampaignInfo(const std::string &campaign_name) {
	campaignMap::const_iterator it = itsCampaigns.find(campaign_name);
	if (it != itsCampaigns.end()) return it->second;
	else return itsCampaigns[""];
}


int SchedulerSettings::getStorageNodeID(const std::string &name) const {return itsController->getStorageNodeID(name);}
std::string SchedulerSettings::getStorageNodeName(int node_id) const {return itsController->getStorageNodeName(node_id);}
std::string SchedulerSettings::getStorageRaidName(int node_id, int raid_id) const {return itsController->getStorageRaidName(node_id, raid_id);}
const storageHostsMap &SchedulerSettings::getStorageNodes(void) const {return itsController->getStorageNodes();}
const statesMap &SchedulerSettings::getStorageNodesStates(void) const {return itsController->getStorageNodesStates();}
size_t SchedulerSettings::getNrOfStorageNodesAvailable(void) const {return itsController->getNrOfStorageNodesAvailable();}
const hostPartitionsMap &SchedulerSettings::getStoragePartitions(void) const {return itsController->getStoragePartitions();}
int SchedulerSettings::getStorageRaidID(int node_id, const std::string &raidName) const {return itsController->getStorageRaidID(node_id,raidName);}


void SchedulerSettings::calculateScheduleWeeks(void) {
	itsScheduleWeeks.clear();
	// first week of schedule
	unsigned day = itsEarliestDay.getDayOfTheWeek();
	AstroDate monday = itsEarliestDay.subtractDays(day);
	// last week of schedule
	day = itsLatestDay.getDayOfTheWeek();
	AstroDate lastMonday = itsLatestDay.subtractDays(day);
	while (monday <= lastMonday) {
		itsScheduleWeeks.push_back(std::pair<unsigned, AstroDate>(monday.getWeek(), monday));
		monday = monday.addDays(7);
	}
}
