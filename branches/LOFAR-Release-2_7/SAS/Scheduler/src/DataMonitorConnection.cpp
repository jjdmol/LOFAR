/*
 * DataMonitorConnection.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 4-aug-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DataMonitorConnection.cpp $
 *
 */

#include "DataMonitorConnection.h"
#include "lofar_utils.h"
#include <QSqlRecord>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlResult>
#include <QSqlError>
#include <algorithm>

DataMonitorConnection::DataMonitorConnection(Controller *controller)
: itsController(controller)/*, itsIsConnected(false)*/ {
	QSqlDatabase dataMonitorDB = QSqlDatabase::addDatabase("QMYSQL","datamonitorDB");
}

DataMonitorConnection::~DataMonitorConnection() {
	{
		QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
		if (dataMonitorDB.isValid()) {
			if (dataMonitorDB.isOpen()) dataMonitorDB.close();
		}
	}
	QSqlDatabase::removeDatabase("datamonitorDB");
}

bool DataMonitorConnection::isOpen(void) const {
	QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
	if (dataMonitorDB.isOpen()) {
		QSqlQuery query("SELECT * FROM states", dataMonitorDB);
		if (query.next()) {
			query.finish();
			return true;
		}
		else {
			query.finish();
			return false;
		}
	}
	else return false;
	//	return dataMonitorDB.isOpen();
}

bool DataMonitorConnection::initRequired(const QString &username, const QString &password, const QString &DBName, const QString &hostname) {
	QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
	if (username.compare(dataMonitorDB.userName()) != 0) return true;
	else if (hostname.compare(dataMonitorDB.hostName()) != 0) return true;
	else if (password.compare(dataMonitorDB.password()) != 0) return true;
	else if (DBName.compare(dataMonitorDB.databaseName()) != 0) return true;
	// if the same wrong settings where applied two times the settings are not changed but the connection should still be re-initialized to make sure the settings are checked
	else return (!dataMonitorDB.isOpen());
}

bool DataMonitorConnection::init(const QString &username, const QString &password, const QString &DBName, const QString &hostname) {
	QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
	if (!dataMonitorDB.isValid()) {
		dataMonitorDB = QSqlDatabase::addDatabase("QMYSQL","datamonitorDB");
	}

	if (dataMonitorDB.isOpen()) {
		dataMonitorDB.close();
	}

	bool init_required(false);
	if (username.compare(dataMonitorDB.userName()) != 0) init_required = true;
	else if (hostname.compare(dataMonitorDB.hostName()) != 0) init_required = true;
	else if (password.compare(dataMonitorDB.password()) != 0) init_required = true;
	else if (DBName.compare(dataMonitorDB.databaseName()) != 0) init_required = true;

	if (init_required) {
		dataMonitorDB.setHostName(hostname);
		dataMonitorDB.setDatabaseName(DBName);
		dataMonitorDB.setUserName(username);
		dataMonitorDB.setPassword(password);
	}

    // get all status types from database
	if (dataMonitorDB.open()) {
		itsStates.clear();
		QSqlQuery query("SELECT * FROM states", dataMonitorDB);
		int idxId = query.record().indexOf("id");
		int idxStateName = query.record().indexOf("statename");
		while (query.next()) {
			itsStates[query.value(idxId).toInt()] = query.value(idxStateName).toString().toStdString();
		}
		query.finish();
		dataMonitorDB.close();
		return true;
	}
	return false;
}

bool DataMonitorConnection::connect(void) {
	QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
	if (!dataMonitorDB.isValid()) {
		dataMonitorDB = QSqlDatabase::addDatabase("QMYSQL","datamonitorDB");
	}
	QVariant value;
    if (dataMonitorDB.open()) {
     	return true;
    }
    else {
    	debugErr("sssssss","Could not establish a connection to the Data Monitor! Settings used:\n",
    			"Username: ", dataMonitorDB.userName().toStdString().c_str(),
    			", database: ", dataMonitorDB.databaseName().toStdString().c_str(),
    			", hostname: ", dataMonitorDB.hostName().toStdString().c_str()
    			);
    	return false; // could not connect to SAS database
    }
}
/*
bool DataMonitorConnection::isOpen(void) const {
	QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
	return dataMonitorDB.isOpen();
}
*/
void DataMonitorConnection::disconnect(void) {
	{
		QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
		dataMonitorDB.close();
	} // dataMonitorDB needs to go out of scope before removeDatabase is called
	QSqlDatabase::removeDatabase("datamonitorDB");
}

bool DataMonitorConnection::updateStorageNodes(void) {
	QSqlDatabase dataMonitorDB = QSqlDatabase::database("datamonitorDB");
	//itsStorageNodes.clear();
	storageHostsMap itsNewStorageNodes;
	itsHostPartitions.clear();
	if (dataMonitorDB.isOpen()) {
		//fetch all storage nodes
		StorageHost newHost;
	    QSqlQuery query("SELECT * FROM hosts WHERE groupid=3", dataMonitorDB);
	    storageHostsMap::const_iterator stit;
	    while (query.next()) {
	    	newHost.itsID = query.value(query.record().indexOf("id")).toInt();
	    	newHost.itsName = query.value(query.record().indexOf("hostname")).toString().toStdString();
	    	newHost.itsStatus = query.value(query.record().indexOf("statusid")).toInt();
	    	// look up the host in the old host table to see if it may be used
	    	stit = itsStorageNodes.find(newHost.itsID);
	    	if (stit != itsStorageNodes.end()) { // host did exist before?
	    		if (newHost.itsName == stit->second.itsName) {
	    			newHost.itsMayBeUsed = stit->second.itsMayBeUsed;
	    		}
	    		else if (newHost.itsStatus == 1) {
	    			newHost.itsMayBeUsed = true;
	    		}
	    		else {
	    			newHost.itsMayBeUsed = false;
	    		}
	    	}
    		else if (newHost.itsStatus == 1) {
    			newHost.itsMayBeUsed = true;
    		}
    		else {
    			newHost.itsMayBeUsed = false;
    		}
	    	itsNewStorageNodes[newHost.itsID] = newHost;
	    }
		query.finish();
		itsStorageNodes = itsNewStorageNodes;
		// now update the free space of every partition available from this storage node
		int locationID;
		std::string path;
		std::vector<quint64> spaceVector;
		dataPathsMap dataPaths;
		quint64 totalSpace, claimedSpace, usedSpace, freeSpace;
		for (storageHostsMap::const_iterator it = itsStorageNodes.begin(); it != itsStorageNodes.end(); ++it) {
			query.exec("SELECT * FROM datapaths WHERE hostid=" + QString::number(it->first));
			while (query.next()) { // get all partitions for this node from the database
				locationID = query.value(query.record().indexOf("id")).toInt();
				path = query.value(query.record().indexOf("path")).toString().toStdString();
				totalSpace = query.value(query.record().indexOf("totalspace")).toULongLong();
				usedSpace = query.value(query.record().indexOf("usedspace")).toULongLong();
				claimedSpace = query.value(query.record().indexOf("claimedspace")).toULongLong();
				spaceVector.push_back(totalSpace);
				spaceVector.push_back(usedSpace);
				spaceVector.push_back(claimedSpace);
				if (usedSpace + claimedSpace > totalSpace) freeSpace = 0;
				else freeSpace = totalSpace - usedSpace - claimedSpace;
				spaceVector.push_back(freeSpace);
				dataPaths[locationID] = std::pair<std::string, std::vector<quint64> >(path, spaceVector);
				spaceVector.clear();
			}
			itsHostPartitions[it->first] = dataPaths;
			dataPaths.clear();
			query.finish();
		}
		return true;
	}
	else return false;
}

void DataMonitorConnection::setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts) {
	std::vector<int>::const_iterator it;
	storageHostsMap::iterator sit = itsStorageNodes.begin();
	while (sit != itsStorageNodes.end()) {
		it = find(allowedStorageHosts.begin(),allowedStorageHosts.end(),sit->second.itsID);
		if (it != allowedStorageHosts.end()) {
			sit->second.itsMayBeUsed = true;
		}
		else {
			sit->second.itsMayBeUsed = false;
		}
		++sit;
	}
}

std::vector<int> DataMonitorConnection::getUsableStorageNodeIDs(void) const {
	std::vector<int> nodes;
	for (storageHostsMap::const_iterator it = itsStorageNodes.begin(); it != itsStorageNodes.end(); ++it) {
		if (it->second.itsMayBeUsed) {
			nodes.push_back(it->first);
		}
	}
	return nodes;
}

int DataMonitorConnection::getStorageNodeID(const std::string &name) const {
	storageHostsMap::const_iterator it = itsStorageNodes.begin();
	while (it != itsStorageNodes.end()) {
		if (it->second.itsName.compare(name.c_str()) == 0) return it->first;
		++it;
	}
	return 0;
}

std::string DataMonitorConnection::getStorageNodeName(int node_id) const {
	storageHostsMap::const_iterator it = itsStorageNodes.find(node_id);
	if (it != itsStorageNodes.end()) return it->second.itsName;
	else return "";
}

int DataMonitorConnection::getStorageRaidID(int node_id, const std::string &raidName) const {
	hostPartitionsMap::const_iterator it = itsHostPartitions.find(node_id);
	if (it != itsHostPartitions.end()) {
		for (dataPathsMap::const_iterator dit = it->second.begin(); dit != it->second.end(); ++dit) {
			if (dit->second.first.compare(raidName) == 0) return dit->first;
		}
		return 0;
	}
	else return 0;
}

size_t DataMonitorConnection::getNrOfStorageNodesAvailable(void) const {
	size_t nrAvailable(0);
	for (storageHostsMap::const_iterator it = itsStorageNodes.begin(); it != itsStorageNodes.end(); ++it) {
		if (it->second.itsMayBeUsed) ++nrAvailable;
	}
	return nrAvailable;
}

std::string DataMonitorConnection::getStorageRaidName(int node_id, int raid_id) const {
	hostPartitionsMap::const_iterator it = itsHostPartitions.find(node_id);
	if (it != itsHostPartitions.end()) {
		dataPathsMap::const_iterator dit = it->second.find(raid_id);
		if (dit != it->second.end())
			return dit->second.first;
	}
	return "";
}
