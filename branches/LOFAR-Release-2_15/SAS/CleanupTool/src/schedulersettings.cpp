/*
 * schedulersettings.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11825 $
 * Last change by : $Author: jong $
 * Change date    : $Date: 2014-09-25 11:35:16 +0000 (Thu, 25 Sep 2014) $
 * First creation : Aug 6, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/schedulersettings.cpp $
 *
 */

#include "schedulersettings.h"
#include "Controller.h"
#include <cmath>
#include <QDate>

SchedulerSettings::SchedulerSettings()
    : itsLoadDefaultSettings(true)
{
	itsObservationIDprefix= "L";
}

SchedulerSettings::~SchedulerSettings() {
}

QDataStream& operator<< (QDataStream &out, const SchedulerSettings &settings) {
    out	<< settings.itsLoadDefaultSettings
	<< settings.itsObservationIDprefix
	<< settings.itsSASUserName
	<< settings.itsSASPassword
	<< settings.itsSASDatabase
	<< settings.itsSASHostName
	<< settings.itsDMUserName
	<< settings.itsDMPassword
	<< settings.itsDMDatabase
    << settings.itsDMHostName
    << settings.itsExcludeStrings;

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

	return out;
}

QDataStream& operator>> (QDataStream &in, SchedulerSettings &settings) {
	quint32 size, size2;

    in >> settings.itsLoadDefaultSettings
	   >> settings.itsObservationIDprefix
	   >> settings.itsSASUserName
	   >> settings.itsSASPassword
	   >> settings.itsSASDatabase
	   >> settings.itsSASHostName
	   >> settings.itsDMUserName
	   >> settings.itsDMPassword
	   >> settings.itsDMDatabase
       >> settings.itsDMHostName
       >> settings.itsExcludeStrings;

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

	return in;
}

QStringList SchedulerSettings::getExludedStrings(void) const {
    QStringList exStrings;
    for (QMap<QString, QPair<bool, QString> >::const_iterator it = itsExcludeStrings.begin(); it != itsExcludeStrings.end(); ++it) {
        if (it.value().first) exStrings.append(it.key());
    }
    return exStrings;
}

void SchedulerSettings::updateSettings(const ScheduleSettingsDialog *settingsDialog) {
	setLoadDefaultSettingsOnStartup(settingsDialog->getLoadDefaultSettingsOnStartUp());
	setSASDatabase(settingsDialog->getSASDatabase());
	setSASHostName(settingsDialog->getSASHostName());
	setSASUserName(settingsDialog->getSASUserName());
	setSASPassword(settingsDialog->getSASPassword());
	setDMDatabase(settingsDialog->getDMDatabase());
	setDMHostName(settingsDialog->getDMHostName());
	setDMUserName(settingsDialog->getDMUserName());
	setDMPassword(settingsDialog->getDMPassword());
    setExcludeStrings(settingsDialog->getExcludeStrings());
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
