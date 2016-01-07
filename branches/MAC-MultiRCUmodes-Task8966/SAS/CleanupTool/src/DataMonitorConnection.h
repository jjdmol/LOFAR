/*
 * DataMonitorConnection.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 4-aug-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/DataMonitorConnection.h $
 *
 */

#ifndef DATAMONITORCONNECTION_H_
#define DATAMONITORCONNECTION_H_

#include <QSqlDatabase>
#include <vector>
#include <map>

class StorageHost {
public:
	std::string itsName;
	quint16 itsID;
	quint16 itsStatus;
	bool itsMayBeUsed;
};

typedef std::map<int, std::string> statesMap;
// key = hostID, value: pair<hostname, status>
//typedef std::map<int, std::pair<std::string, int> > storageHostsMap;
typedef std::map<int, StorageHost> storageHostsMap;
// key = hostID, value: vector<pair<path, pair<totalspace, pair<usedspace, claimedspace>>>
//typedef std::map<int, std::vector<std::pair<QString, std::pair<unsigned long, std::pair<unsigned long, unsigned long> > > > > dataPathsMap;
// key = raid ID, value = pair<path, vector with four space componennts i.e. [0]=total, [1]=used, [2]=claimed, [3]=free
typedef std::map<int, std::pair<std::string, std::vector<quint64> > > dataPathsMap;
// key = host ID, value = vector of path IDs that belong to this host (the path IDs can be looked up in the dataPathsMap)
//typedef std::map<int, std::vector<int> > hostPartitionIDsMap;
typedef std::map<int, dataPathsMap> hostPartitionsMap;

class DataMonitorConnection {
public:
    DataMonitorConnection();
	virtual ~DataMonitorConnection();

	bool init(const QString &username, const QString &password, const QString &DBName, const QString &hostname);
	bool initRequired(const QString &username, const QString &password, const QString &DBName, const QString &hostname);
	bool connect(void);
//	int testConnect(const QString &username, const QString &password, const QString &DBname, const QString &hostname);
	void disconnect(void);
	bool isOpen(void) const;
	bool updateStorageNodes(void); // gets the the storage nodes and their current status from the DM database

	const storageHostsMap &getStorageNodes(void) const {return itsStorageNodes;}
	std::vector<int> getUsableStorageNodeIDs(void) const;
	const statesMap &getStates(void) const {return itsStates;}
	size_t getNrOfStorageNodesAvailable(void) const;
//	const dataPathsMap &getDataPaths(void) const {return itsDataPaths;}
	const hostPartitionsMap &getHostPartitions(void) const {return itsHostPartitions;}
	int getStorageRaidID(int node_id, const std::string &raidName) const;
//	long double getStorageNodeSpaceRemaining(int node_id, unsigned short partition_id) const;
	int getStorageNodeID(const std::string &name) const;
	std::string getStorageNodeName(int node_id) const;
	std::string getStorageRaidName(int node_id, int raid_id) const;

	void setStorageNodes(const storageHostsMap &nodes) {itsStorageNodes = nodes;}
	void setStoragePartitions(const hostPartitionsMap &partitions) {itsHostPartitions = partitions;}

	void setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts);

	friend class Controller;

private:
	statesMap itsStates;
	storageHostsMap itsStorageNodes;
	hostPartitionsMap itsHostPartitions;
};

#endif /* DATAMONITORCONNECTION_H_ */
