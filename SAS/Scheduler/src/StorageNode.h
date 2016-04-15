/*
 * StorageNode.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 22-jun-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/StorageNode.h $
 *
 */

#ifndef STORAGENODE_H_
#define STORAGENODE_H_

#include <string>
#include <map>
#include <vector>
#include "lofar_scheduler.h"
#include "storage_definitions.h"
#include "astrodatetime.h"
#include "taskstorage.h"
#include "DataMonitorConnection.h"

class storageClaim {
public:
	storageClaim(unsigned task_id, unsigned claim_id, int raid_id, dataProductTypes dataProduct, const double &claim_size, const double &bandwidth, const AstroDateTime &start, const AstroDateTime &end)
	: taskID(task_id), claimID(claim_id), dataProductType(dataProduct), raidID(raid_id), claimByteSize(claim_size), claimBandWidth(bandwidth), starttime(start), endTime(end)
	{ }
	unsigned taskID, claimID;
	dataProductTypes dataProductType;
	int raidID;
	double claimByteSize;
	double claimBandWidth;
	AstroDateTime starttime, endTime;
};

class capacityLogPoint { // used for logging the remaining disk space and disk write bandwidth for each raid array
public:
	capacityLogPoint(void) {}
	capacityLogPoint(unsigned task_id, unsigned claim_id, const AstroDateTime &log_time, const double &remaining_disk_space, const double &remaining_disk_write_speed)
	: taskID(task_id), claimID(claim_id), time(log_time), remainingDiskSpacekB(remaining_disk_space), remainingDiskWriteBW(remaining_disk_write_speed) { }
	unsigned taskID, claimID;
	AstroDateTime time;
	double remainingDiskSpacekB, remainingDiskWriteBW;
};

// // capacityTimeMap: keeps track of available individual raid capacity and write speed of raid array
// key raidID, value = remaining capacity Log point
typedef std::map<int, std::vector<capacityLogPoint> > capacityTimeMap;

// class that represents a single log point in the storage node's total bandwidth log vector (nodeBandWidthVector itsRemainingBandwidth)
class nodeBandWidthLogPoint {
public:
	nodeBandWidthLogPoint(void) {}
	nodeBandWidthLogPoint(unsigned task_id, const AstroDateTime &log_time, const double &bandwidth_diff, const double &remaining_node_bw)
	: taskID(task_id), time(log_time), nodeBWdiff(bandwidth_diff), remainingNodeBW(remaining_node_bw) { }
	unsigned taskID; // the taskID that caused this log point (multiple log points can have same taskID (on for each individual task file written to this storage node)
	AstroDateTime time; // the time of the log point
	double nodeBWdiff; // the difference in total bandwidth caused by this task log point (negative at task end-time log point)
	double remainingNodeBW; // the remaining total bandwidth for this storage node at time 'time'
};

typedef std::vector<storageClaim> storageClaimsVector;

// nodeBandWidthVector keeps track of the remaining total bandwidth to this storage node
typedef std::vector<nodeBandWidthLogPoint> nodeBandWidthVector;


class StorageNode {
public:
	StorageNode();
	StorageNode(const std::string &name, int nodeID);
	virtual ~StorageNode();

	const std::string &name(void) const {return itsName;}
	int getID() const {return itsID;}
	int getStatus() const {return itsStatus;}
	const capacityTimeMap &getRemainingSpace(void) const {return itsRemainingSpace;}
	const nodeBandWidthVector &getRemainingNodeBandwidth(void) const {return itsRemainingBandwidth;}
	const storagePartitionsMap &getStorageLocations(void) const {return itsStoragePartitions;}
	const storageClaimsVector &getStorageClaims(void) const {return itsClaims;}
	bool mayBeUsed(void) const {return itsMayBeUsed;}
	size_t nrClaims(void) const {return itsClaims.size();}

	void initNode(const StorageHost &host, const AstroDateTime &now, const double &totalBandWidth); // bandWidth units kbit/sec
	bool addPartition(int raidID, const std::string &partition, const AstroDateTime &now, const double &byte_capacity, const double &free_space);
	// check bandwidth requirements don't exceed the nodes bandwidth in the specified (start,end) period
	task_conflict checkBandWidth(const AstroDateTime &start, const AstroDateTime &end, const double &totalBW_kbs) const;
	// check if space is available to add the requested task to the claims of this storage node using the specified raid array (claimSize units: kByte, bandWidth units kbit/sec)
    task_conflict checkSpaceAndWriteSpeed(const AstroDateTime &start, const AstroDateTime &end, const double &claimSize, const double &writeSpeed, int raidID) const;
	// return the ids of the raid arrays that meet the specified bandwidth (kbit/sec) and fileSize (kByte) within the timespan defined by startTime and endTime
	nodeStorageOptions getPossibleRaidArrays(const AstroDateTime &startTime, const AstroDateTime &endTime,
			const double &fileSize, const double &bandWidth, unsigned minNrFiles, std::vector<std::pair<int, task_conflict> > &result) const;
	// addFile adds a single file to raid with id equal to raidID to this node (claimSize units: kByte, bandWidth units kbit/sec)
	bool addClaim(unsigned taskID, const AstroDateTime &start, const AstroDateTime &end, dataProductTypes dp, const double &claimSize, const double &bandWidth, int raidID);
	void removeClaim(unsigned taskID);
	bool checkClaim(unsigned int taskID, dataProductTypes dataProduct, int raidID) const;
	void clearClaims(void);
	void setName(const std::string &name) {itsName = name;}
	void setID(int id) {itsID = id;}
	void setStatus(int status) {itsStatus = status;}
	void setMayBeUsed(bool enabled) {itsMayBeUsed = enabled;}
	void clear(void) {itsStoragePartitions.clear(); itsClaims.clear(); itsRemainingSpace.clear();}

private:
	std::string itsName;
	int itsID;
	int itsStatus;
	bool itsMayBeUsed;
	unsigned itsclaimID; // used for internal bookkeeping to match logpoints to claims
	storagePartitionsMap itsStoragePartitions;
	storageClaimsVector itsClaims;
	capacityTimeMap itsRemainingSpace;
	nodeBandWidthVector itsRemainingBandwidth; // units:kbit/sec
};

class cmp_FreeSpace
{
public:
	bool operator() (const storageOption &s1, const storageOption &s2) const
	{
		return s1.remainingSpacekB < s2.remainingSpacekB;
	}
};

#endif /* STORAGENODE_H_ */
