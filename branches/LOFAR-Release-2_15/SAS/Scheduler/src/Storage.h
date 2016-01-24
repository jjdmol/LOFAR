/*
 * Storage.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 5-aug-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Storage.h $
 *
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <map>
#include <vector>
#include "lofar_scheduler.h"
#include "storage_definitions.h"
#include "StorageNode.h"
#include "task.h"
#include "schedulersettings.h"


class Storage {
public:
	Storage();
	virtual ~Storage();

	void initStorage(void);
	bool addStorageNode(const std::string &nodeName, int nodeID);// {itsStorageNodes.insert(storageNodesMap::value_type(nodeID, nodeName));}
	void addStoragePartition(int nodeID, unsigned short partitionID, const std::string &path, const double &capacity, const double &free_space);
	void clearStorageClaims(void); // removes all claims from all storage nodes
    std::vector<storageResult> addStorageToTask(Task *pTask, const storageMap &storageLocations);
    std::vector<storageResult> addStorageToTask(Task *pTask, dataProductTypes dataProduct, const storageVector &storageLocations, bool noCheck); //	bool addStorageTask(unsigned taskID, const AstroDateTime &startTime, const AstroDateTime &endTime, const double &claimSize, const double &bandWidth, int storageNodeID, int raidID);
	void removeTaskStorage(unsigned taskID);
    std::vector<storageResult> checkAssignedTaskStorage(Task *pTask, dataProductTypes dataProduct);
	// returns the possible storage locations for the claim.key = node ID, value vector of raidID,free space pairs
	storageLocationOptions getStorageLocationOptions(dataProductTypes dataProduct, const AstroDateTime &startTime, const AstroDateTime &endTime,
			const double &fileSize, const double &bandWidth, unsigned minNrFiles, sortMode sort_mode, const std::vector<int> &nodes = std::vector<int>());
	// returns the last storage check results from calling getStorageLocationOptions
    const std::vector<storageResult> &getLastStorageCheckResult(void) const {return itsLastStorageCheckResult;}
    void setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts);
    const storageNodesMap &storageNodes(void) const {return itsStorageNodes;}

private:
	std::map<unsigned, std::vector<int> > itsTaskStorageNodes; // key:taskID, value: vector holding the storage node IDs used by the task
	storageNodesMap itsStorageNodes;
    std::vector<storageResult> itsLastStorageCheckResult;
};

bool storageLocationsContains(const storageLocationOptions &locs, int node_id, int raid_id);

#endif /* STORAGE_H_ */
