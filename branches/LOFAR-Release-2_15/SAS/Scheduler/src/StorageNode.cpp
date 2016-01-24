/*
 * StorageNode.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 22-jun-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/StorageNode.cpp $
 *
 */

#include "StorageNode.h"
#include "Controller.h"
#include <limits>
#include <QDateTime>
#include <cmath>

StorageNode::StorageNode()
: itsID(0), itsStatus(0), itsclaimID(0)
{
}

StorageNode::StorageNode(const std::string &name, int nodeID)
: itsName(name), itsID(nodeID), itsStatus(0), itsclaimID(0)
{
}

StorageNode::~StorageNode() {
}

void StorageNode::initNode(const StorageHost &host, const AstroDateTime &now, const double &totalBandWidth) {
	itsStoragePartitions.clear();
	itsClaims.clear();
	itsRemainingSpace.clear();
	itsRemainingBandwidth.clear();
	itsName = host.itsName;
	itsID = host.itsID;
	itsStatus = host.itsStatus;
	itsMayBeUsed = host.itsMayBeUsed;
	itsRemainingBandwidth.push_back(nodeBandWidthLogPoint(0, now, 0, totalBandWidth));
}

bool StorageNode::addPartition(int raidID, const std::string &partition, const AstroDateTime &now, const double &byte_capacity, const double &free_space) {
	storagePartitionsMap::iterator it = itsStoragePartitions.find(raidID);
	if (it != itsStoragePartitions.end()) {
		return false; // partition ID already taken
	}
	// partition ID not taken -> add the partition
//	std::cout << "node:" << itsName << ", adding partition " << raidID << ", name:" << partition << ", capacity:" << byte_capacity << ", free spacve: " << free_space << std::endl;
	itsStoragePartitions[raidID] = std::pair<std::string, double>(partition, byte_capacity);
//	std::pair<double, double> valPair = std::pair<double, double>(free_space, Controller::theSchedulerSettings.getRaidMaxWriteSpeed());
	itsRemainingSpace[raidID].clear(); // clears or creates this raidID entry in itsremainingspacemap
	itsRemainingSpace[raidID].push_back(capacityLogPoint(0, 0, now, free_space, Controller::theSchedulerSettings.getRaidMaxWriteSpeed())); // set the current values for size and bandwidth
	return true;
}

void StorageNode::clearClaims(void) {
	itsclaimID = 0;
	itsClaims.clear();
	std::vector<capacityLogPoint> newCapacityVec;
	capacityTimeMap newRemainingSpaceMap;
	for (capacityTimeMap::iterator it = itsRemainingSpace.begin(); it != itsRemainingSpace.end(); ++it) {
		newCapacityVec.clear();
		newCapacityVec.push_back(it->second.front());
		newRemainingSpaceMap[it->first] = newCapacityVec;
	}
	itsRemainingSpace = newRemainingSpaceMap;

	nodeBandWidthVector newbwvec;
	newbwvec.push_back(itsRemainingBandwidth.front());
	itsRemainingBandwidth = newbwvec;
}


/*
bool StorageNode::setCurrentValues(int raidID, const AstroDateTime &now, const double &byte_capacity, const double &bandwidth) {
//	capacityTimeMap::iterator it = itsRemainingSpace.find(raidID);
//	if (it != itsRemainingSpace.end()) {
//		QDateTime cT = QDateTime::currentDateTime().toUTC();
//		AstroDateTime now(cT.date().day(), cT.date().month(), cT.date().year(),
//				cT.time().hour(), cT.time().minute(), cT.time().second());
//		it->second.clear();
//	capacityTimeMap;
		std::pair<double, double> valPair = std::pair<double, double>(byte_capacity, bandwidth);
		itsRemainingSpace[raidID].push_back(std::pair<AstroDateTime, std::pair<double, double> >(now, valPair));
		return true;
//	}
//	else return false;
}
*/

task_conflict StorageNode::checkBandWidth(const AstroDateTime &start, const AstroDateTime &end, const double &totalBW_kbs) const {
	//first check if total bandwidth of this node is not exceeded
	if (totalBW_kbs > itsRemainingBandwidth.front().remainingNodeBW) {
		return CONFLICT_STORAGE_NODE_BANDWIDTH;
	}
	for (nodeBandWidthVector::const_iterator it = itsRemainingBandwidth.begin(); it != itsRemainingBandwidth.end(); ++it) {
		if ((it->time >= start) && (it->time <= end)) {
			if (it->remainingNodeBW < totalBW_kbs)
				return CONFLICT_STORAGE_NODE_BANDWIDTH;
		}
		if (it->time > end) break;
	}
	return CONFLICT_NO_CONFLICT;
}

task_conflict StorageNode::checkSpaceAndWriteSpeed(const AstroDateTime &startTime, const AstroDateTime &endTime, const double &claimSize, const double &writeSpeed, int raidID) const {
	capacityTimeMap::const_iterator cit = itsRemainingSpace.find(raidID);
//	std::cout << "checking storage node: " << itsName << std::endl << "partition: " << raidID << std::endl << "claim size for this node: "
//	<< claimSize << std::endl << "writeSpeed for this node: " << writeSpeed << std::endl << "start time: " << startTime.toString() << std::endl
//	<< "end time: " << endTime.toString() << std::endl;
	if (cit != itsRemainingSpace.end()) {
//		 std::cout << "first free space log: " << cit->second.front().time.toString() << ", space remaining" << cit->second.front().remainingDiskSpacekB << "kB, disk write speed remaining" <<  cit->second.front().remainingDiskWriteBW << "MByte/s";
		if (startTime > cit->second.front().time) { // start time of observation needs to be after 'now' which is the first time in itsRemainingSpace
			for (std::vector<capacityLogPoint>::const_iterator sit = cit->second.begin(); sit != cit->second.end(); ++sit) {
//				std::cout << sit->time.toString() << ", free space:" << sit->remainingDiskSpacekB << "kB, write speed remaining:" << sit->remainingDiskWriteBW << "MB/s" << std::endl;
				if (startTime >= sit->time) { // found the last time that is earlier than the requested start time
					while (sit < cit->second.end()) { // iterate over the following free space log-points to check if space stays sufficient during the task's duration
						if (claimSize > sit->remainingDiskSpacekB) {
							return CONFLICT_STORAGE_NODE_SPACE; // insufficient space
						}
						else if (writeSpeed > sit->remainingDiskWriteBW) {
//							std::cerr << "conflict write speed: " << "requested: " << writeSpeed << ", " << "node remaining write speed at " << sit->time.toString().c_str() << ": " << sit->remainingDiskWriteBW << std::endl;
							return CONFLICT_STORAGE_WRITE_SPEED; // requested write speed to high
						}
						else if ((sit++)->time > endTime) return CONFLICT_NO_CONFLICT;
					}
					return CONFLICT_NO_CONFLICT; // if only the initial entry is logged in itsRemainingSpace we should arrive here.
				}
			}
			return CONFLICT_NO_CONFLICT; // if only the initial entry is logged in itsRemainingSpace we should arrive here.
		}
		else return CONFLICT_STORAGE_TIME_TOO_EARLY; // Error: start time of observation before now
	}
	else return CONFLICT_RAID_ARRRAY_NOT_FOUND; // Error: partition not found
}

// return the ids of the raid arrays that meet the specified bandwidth (kbit/sec) and claimSize within the timespan defined by startTime and endTime
nodeStorageOptions StorageNode::getPossibleRaidArrays(const AstroDateTime &startTime, const AstroDateTime &endTime, const double &fileSize,
		const double &bandWidth, unsigned minNrFiles, std::vector<std::pair<int, task_conflict> > &result) const {
	bool conflict(false);
	result.clear();
	nodeStorageOptions locations;
	double minBWreq = minNrFiles * bandWidth; // kbit/sec
	double minSizeReq = minNrFiles * fileSize;
	storagePartitionsMap::const_iterator spit;
//	bool debugOut(false);
//	qDebug() << "node: " << itsName.c_str() << " looking for free space: " << minSizeReq << " and bandwidth: " << minBWreq;
//	std::cout << "node: " << itsName.c_str() << " looking for free space: " << minSizeReq << " and bandwidth: " << minBWreq << std::endl;
//	if (itsName.compare("locus001") == 0) debugOut = true;

//	if (debugOut) {
//		std::cout << "storage node id:" << itsID << std::endl
//				<< "itsRemainingBandwidth size:" << itsRemainingBandwidth.size() << std::endl
//				<< "itsRemainingSpace size:" << itsRemainingSpace.size() << std::endl
//				<< "itsClaims size:" << itsClaims.size() << std::endl;
//	}

	// check if start time of observation is later than first time in itsRemainingBandwidth
	if (startTime > itsRemainingBandwidth.front().time) { // start time of observation needs to be after 'now' which is the first time in itsRemainingSpace
		// first check if enough total bandwidth to this node is remaining
		if (itsRemainingBandwidth.size() > 1) {
//			std::cout << "number of log points in itsRemainingBandwidth:" << itsRemainingBandwidth.size() << std::endl;
			for (nodeBandWidthVector::const_iterator it = itsRemainingBandwidth.begin(); it != itsRemainingBandwidth.end()-2; ++it) {
//				if (debugOut)  {
//					std::cout << "log point time:" << it->time.toString() << "BW:" << it->remainingNodeBW << "kbit/s" << std::endl;
//				}
				if (startTime <= (it+1)->time) { // found the first log point later than the start time of the observation
					while (endTime > it->time) {
//						if (debugOut) {
//						std::cout << "log point time:" << it->time.toString() << "BW:" << it->remainingNodeBW << "kbit/s" << std::endl;
//						}
						if (minBWreq > it->remainingNodeBW) {
//							if (debugOut) {
//								std::cout << " not enough total bandwidth to this storage node: " << itsName.c_str() << std::endl;
//							}
							result.push_back(std::pair<int, task_conflict>(-1, CONFLICT_STORAGE_NODE_BANDWIDTH));
							return locations; // don't check the raid arrays because there is not enough total bandwidth
						}
						if (++it == itsRemainingBandwidth.end()) break; // iterate to next logpoint in itsRemainingBandWidth
					}
					break; // break out of for loop (no conflicts so far)
				}
			}
		}
		else { // only one element in itsRemainingBandwidth
			if (minBWreq > itsRemainingBandwidth.front().remainingNodeBW) {
//				if (debugOut)
//					std::cout << " not enough total bandwidth to this storage node: " << itsName.c_str() << std::endl;
				result.push_back(std::pair<int, task_conflict>(-1, CONFLICT_STORAGE_NODE_BANDWIDTH));
				return locations; // don't check the raid arrays because there is not enough total bandwidth
			}
		}
		// start checking the raid arrays
//		if (debugOut) std::cout << "start checking raid arrays" << std::endl;

		short unsigned fillPercentage(Controller::theSchedulerSettings.getStorageFillPercentage());
		double lowerLimitFreekB;
		for (capacityTimeMap::const_iterator cit = itsRemainingSpace.begin(); cit != itsRemainingSpace.end(); ++cit) { // iterates over the raid arrays
			spit = itsStoragePartitions.find(cit->first);
			if (spit != itsStoragePartitions.end()) {
				lowerLimitFreekB = spit->second.second * (double) (100.0-fillPercentage) / 100.0;
			}
			else {
				lowerLimitFreekB = 0;
			}
			conflict = false;
			// do check on space and bandwidth on the first entry in itsRemainingSpace (first entry is the total capacity and total bandwidth of this array
			double 	smallest_free = cit->second.front().remainingDiskSpacekB;
			double lowest_write_speed = cit->second.front().remainingDiskWriteBW;
			if (minSizeReq > smallest_free) {
				conflict = true; // not enough space
//				if (debugOut)
//					std::cout << " not enough total free space on raid: " << cit->first << std::endl;
				result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_NODE_SPACE));
//								break; // insufficient space
			}
			else if (minBWreq > lowest_write_speed * 8) { // minBWreq (kbit/s), lowest_write_speed (kbyte/sec)
//				if (debugOut)
//					std::cout << " not enough write bandwidth on raid: " << cit->first << std::endl;
				result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_WRITE_SPEED));
				conflict = true; // not enough bandwidth
//								break;
			}
			if (!conflict) {
//				if (debugOut) {
//					std::cout << "nr of log points:" << cit->second.size() << std::endl;
//				}
//				if (cit->second.size() >= 2) {
				bool pastEnd(false);
				for (std::vector<capacityLogPoint>::const_iterator logpointIt = cit->second.begin(); logpointIt != cit->second.end(); ++logpointIt) {

					if (!pastEnd && (startTime >= logpointIt->time)) { // found first entry that has a time before start ti
						while (logpointIt < cit->second.end()) { // iterate over the following free space log-points to check if space stays sufficient during the task's duration
//							if (claimSize > sit->remainingDiskSpacekB) {

							// check if disk usage limit is reached
							if (logpointIt->remainingDiskSpacekB < lowerLimitFreekB) {
								conflict = true;
								result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_NODE_SPACE));
								break;
							}

//					if (startTime <= (logpointIt + 1)->time) { // found the first log point which has a time later than startTime of the task
//						if (debugOut)
//							std::cout << "start:" << startTime.toString() << ", log time:" << (logpointIt + 1)->time.toString() << std::endl;
						// do check on space and bandwidth for logPoint
						smallest_free = std::min(smallest_free, logpointIt->remainingDiskSpacekB);
						lowest_write_speed = std::min(lowest_write_speed, logpointIt->remainingDiskWriteBW);
						if (minSizeReq > smallest_free) {
							conflict = true; // not enough space
//							if (debugOut)
//								std::cout << "(1)node: " << itsName.c_str() << " raid: " << cit->first << " not enough space at: " << logpointIt->time.toString().c_str() << ", space available:" << logpointIt->remainingDiskSpacekB << std::endl;
							result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_NODE_SPACE));
							break; // insufficient space
						}
						else if (minBWreq > lowest_write_speed * 8) { // minBWreq (kbit/s), lowest_write_speed (kbyte/sec)
//							if (debugOut)
//								std::cout << "(2)node: " << itsName.c_str() << " raid: " << cit->first << " not enough bandwidth at: " << logpointIt->time.toString().c_str() << ", write speed available:" << logpointIt->remainingDiskWriteBW << std::endl;
							result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_NODE_BANDWIDTH));
							conflict = true; // not enough bandwidth
							break;
						}
						else if (logpointIt->time >= endTime) {
							pastEnd = true;
							break; //reach the end time of the task
						}
						else {
							if (++logpointIt == cit->second.end()){
								pastEnd = true;
								break;
							}
						}
//						while (endTime > (++logpointIt)->time) { // repeat checks upto the end of the task
//							// do check on space and bandwidth for logPoint +1
//							smallest_free = std::min(smallest_free, (logpointIt + 1)->remainingDiskSpacekB);
//							lowest_write_speed = std::min(lowest_write_speed, (logpointIt + 1)->remainingDiskWriteBW);
//							if (minSizeReq > smallest_free) {
//								conflict = true; // not enough space
////								if (debugOut)
////									std::cout << "(3)node: " << itsName.c_str() << " raid: " << cit->first << " not enough space at: " << (logpointIt + 1)->time.toString().c_str() << ", space available:" << (logpointIt + 1)->remainingDiskSpacekB << ", logpoint taskID:" << logpointIt->taskID << std::endl;
//								result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_NODE_SPACE));
//								break; // insufficient space
//							}
//							else if (minBWreq > lowest_write_speed * 8) { // minBWreq (kbit/s), lowest_write_speed (kbyte/sec)
////								if (debugOut)
////									std::cout << "(4)node: " << itsName.c_str() << " raid: " << cit->first << " not enough bandwidth at: " << (logpointIt + 1)->time.toString().c_str() << ", write speed available:" << (logpointIt + 1)->remainingDiskWriteBW << std::endl;
//								result.push_back(std::pair<int, task_conflict>(cit->first, CONFLICT_STORAGE_NODE_BANDWIDTH));
//								conflict = true; // not enough bandwidth
//								break;
//							}
//							if (logpointIt == cit->second.end()) break; // prevent infinite loop
//						}
//						break; // no conflicts break out of for loop
						}
					}
					else break; // past end, break out of for loop
				}
//			}
				// ELSE only one lop point means no claims yet for this raid array
			}
			// if no conflict then enough space and bandwidth are available
			if (!conflict) {
//				std::cout << "node: " << itsName << " has suitable raid: " << cit->first << std::endl;
				unsigned nrFiles = std::min(static_cast<unsigned>(smallest_free / fileSize), static_cast<unsigned>(lowest_write_speed * 8 / bandWidth)); // nr files size calimSize which would fit on this raid array according to remaining space and write speed
				locations.push_back(storageOption(cit->first, smallest_free, nrFiles));
			}
		}
	}
	else {
		result.push_back(std::pair<int, task_conflict>(-1, CONFLICT_STORAGE_TIME_TOO_EARLY));
	}
	if (!locations.empty()) {
		sort(locations.begin(), locations.end(), cmp_FreeSpace()); // sort according to smallest free space from small to large
	}
	return locations;
}

// adds the task without checking (use checkSpace function before this one on all required storage nodes before adding the task
bool StorageNode::addClaim(unsigned taskID, const AstroDateTime &startTime, const AstroDateTime &endTime, dataProductTypes dataProduct,
		const double &claimSize, const double &bandWidth, int raidID) {
	// update the total bandwidth remaining vector

	storagePartitionsMap::iterator it = itsStoragePartitions.find(raidID); // check if the raidIDbelongs to this node
	if (it != itsStoragePartitions.end()) {

	bool inserted(false);
	nodeBandWidthLogPoint newLogPoint;
	if (startTime > itsRemainingBandwidth.front().time) { // startTime > now?
		newLogPoint.taskID = taskID;

		unsigned idx;
		nodeBandWidthVector::iterator vit;

		for (idx = 0; idx < itsRemainingBandwidth.size(); ++idx) {
			vit = itsRemainingBandwidth.begin() +  idx;
			if (taskID == vit->taskID) { // already a log point at this node for this task (from another file / subband)
				vit->nodeBWdiff += bandWidth;
				vit->remainingNodeBW -= bandWidth;
				// now search for the end log point of this task
				while ((++idx < itsRemainingBandwidth.size()) && (vit->time <= endTime)) {
					vit = itsRemainingBandwidth.begin() + idx;
					if (taskID == vit->taskID) { // this is the end -time log point. free up the bandwidth again
//						vit->remainingNodeBW += bandWidth;
						vit->nodeBWdiff -= bandWidth;
						inserted = true;
						break;
					}
					else {
						vit->remainingNodeBW -= bandWidth;
					}
				}
				break;
			}
			if (startTime < vit->time) { // next element (at position idx) is later than the startTime of the task to be inserted
				newLogPoint.time = startTime;
				newLogPoint.remainingNodeBW = (vit-1)->remainingNodeBW - bandWidth;
				newLogPoint.nodeBWdiff = bandWidth;
				vit = itsRemainingBandwidth.insert(vit, newLogPoint); // vit now points to the newly inserted element
				// now continue subtracting the used bandwidth from the later elements in itsRemainingBandwidth upto the end time of the task
				while ((++idx < itsRemainingBandwidth.size()) && (vit->time <= endTime)) {
					vit = itsRemainingBandwidth.begin() + idx;
					vit->remainingNodeBW -= bandWidth;
				}
				vit = itsRemainingBandwidth.begin() + idx;
				newLogPoint.time = endTime;
				newLogPoint.remainingNodeBW = (vit-1)->remainingNodeBW + bandWidth;
				newLogPoint.nodeBWdiff = -bandWidth; // at the end of the task the used bandwidth is freed, hence the minus sign
				itsRemainingBandwidth.insert(vit, newLogPoint);
				inserted = true;
				break;
			}
		}
		if (!inserted) {
			// insert start time log point
//			vit = itsRemainingBandwidth.begin();
			newLogPoint.time = startTime;
			newLogPoint.remainingNodeBW = itsRemainingBandwidth.back().remainingNodeBW - bandWidth;
			newLogPoint.nodeBWdiff = bandWidth;
			itsRemainingBandwidth.push_back(newLogPoint);
			// insert end time log point
			newLogPoint.time = endTime;
			newLogPoint.remainingNodeBW = vit->remainingNodeBW;
			newLogPoint.nodeBWdiff = -bandWidth; // at the end of the task the used bandwidth is freed, hence the minus sign
			itsRemainingBandwidth.push_back(newLogPoint);
		}

/*
		if (itsName == "locus003") {
			std::cout << std::setprecision(16) << "locus003" << std::endl;
			std::cout << "task:" << taskID << ", dataproduct:" << DATA_PRODUCTS[dataProduct] << ", raidID: " << raidID << std::endl;
		for (nodeBandWidthVector::iterator nbit = itsRemainingBandwidth.begin(); nbit != itsRemainingBandwidth.end(); ++nbit) {
			std::cout << "time:" << nbit->time.toString() << ", task:" << nbit->taskID << ", remaining:" << nbit->remainingNodeBW << ", diff:" << nbit->nodeBWdiff << std::endl;
		}
		}
*/
		/*
		for (nodeBandWidthVector::iterator vit = itsRemainingBandwidth.begin(); vit < itsRemainingBandwidth.end()-1; ++vit) {
			if (startTime < vit->time) { // next element (at position vit) is later than startTime of task to be inserted
				// insert element at position pointed to by vit
				newLogPoint.time = startTime;
				newLogPoint.remainingNodeBW = vit->remainingNodeBW - bandWidth;
				newLogPoint.nodeBWdiff = bandWidth;
				vit = itsRemainingBandwidth.insert(vit, newLogPoint); // vit now points to the newly inserted element
				// update all later elements their size and bandwidth
				++vit; // go to element after newly inserted element
				while ((vit < itsRemainingBandwidth.end()) && (vit->time <= endTime)) { // continue to end of task updating total remaining bandwidth
					vit->remainingNodeBW -= bandWidth;
					++vit;
				}
				// now add the end-time log point of the task
				newLogPoint.time = endTime;
				newLogPoint.remainingNodeBW = vit->remainingNodeBW;
				newLogPoint.nodeBWdiff = -bandWidth; // at the end of the task the used bandwidth is freed, hence the minus sign
				itsRemainingBandwidth.insert(vit, newLogPoint);
				inserted = true;
				break;
			}
		}
		*/
/*
		if (!inserted) {
			// not yet inserted means either there is only one element in the vector (which is the total size and bandwidth), in which case the task has to be inserted after the first element
			// or the startTime of this task is later than the last element in the vector, in which case the task has to be added at the end of the vector
			// either case this means that the task should be inserted at the back of the vector
			// startTime logpoint:
			newLogPoint.time = startTime;
			newLogPoint.remainingNodeBW = itsRemainingBandwidth.front().remainingNodeBW - bandWidth;
			newLogPoint.nodeBWdiff = bandWidth;
			itsRemainingBandwidth.push_back(newLogPoint);
			// endTime logpoint:
			newLogPoint.time = endTime;
			newLogPoint.remainingNodeBW = itsRemainingBandwidth.front().remainingNodeBW;
			newLogPoint.nodeBWdiff = -bandWidth;
			itsRemainingBandwidth.push_back(newLogPoint);
		}
*/
			// add the claim to the itsClaims vector
			itsClaims.push_back(storageClaim(taskID, ++itsclaimID, raidID, dataProduct, claimSize, bandWidth, startTime, endTime));
	}

	inserted = false;
		capacityTimeMap::iterator cit = itsRemainingSpace.find(raidID);
		if (cit != itsRemainingSpace.end()) {
//			std::cout << "space map for node:" << itsName << ", adding claim for task: " << taskID << std::endl;
//			for (std::vector<capacityLogPoint>::const_iterator spaceit = cit->second.begin(); spaceit != cit->second.end(); ++spaceit) {
//				std::cout << spaceit->time.toString() << ", " << "task:" << spaceit->taskID << ", space:" << spaceit->remainingDiskSpacekB << std::endl;
//			}

//			bool inserted(false);
			capacityLogPoint newCapacityLogPoint(taskID, itsclaimID, startTime, 0, 0);
			for (std::vector<capacityLogPoint>::iterator vit = cit->second.begin()+1; vit < cit->second.end()-1; ++vit) {
				if (startTime < vit->time) { // next element is later than startTime of task to be inserted
					// insert element at position pointed to by vit
					newCapacityLogPoint.remainingDiskSpacekB = (vit-1)->remainingDiskSpacekB - claimSize;
					newCapacityLogPoint.remainingDiskWriteBW = (vit-1)->remainingDiskWriteBW - bandWidth / 8; // diskwrite speed units kbyte/s, bandWidth units kbit/sec
					vit = cit->second.insert(vit, newCapacityLogPoint); // vit now points to the newly inserted logpoint
					// update all later elements their size and bandwidth (vit now points to the newly inserted element
					while (++vit != cit->second.end()) { // subtract claimed disk space from elements after this
						vit->remainingDiskSpacekB -= claimSize;
						vit->remainingDiskWriteBW -= bandWidth / 8; // diskwrite speed units kbyte/s, bandWidth units kbit/sec
					}
					inserted = true;
					break;
				}
			}
			if (!inserted) {
				// not yet inserted that means either there is only one element in the vector (which is the total size and bandwidth), in which case the task has to be inserted after the first element
				// or the startTime of this task is later than the last element in the vector, in which case the task has to be added at the end of the vector
				// either case this means that the task should be inserted at the back of the vector
				newCapacityLogPoint.remainingDiskSpacekB = cit->second.back().remainingDiskSpacekB - claimSize;
				newCapacityLogPoint.remainingDiskWriteBW = cit->second.back().remainingDiskWriteBW - (bandWidth / 8); // diskwrite speed units kbyte/s, bandWidth units kbit/sec

				cit->second.push_back(newCapacityLogPoint);
			}
//			std::cout << "after insertion task:" << taskID << " size of claim:" << claimSize << std::endl;
//			for (std::vector<capacityLogPoint>::const_iterator spaceit = cit->second.begin(); spaceit != cit->second.end(); ++spaceit) {
//				std::cout << spaceit->time.toString() << ", " << "task:" << spaceit->taskID << ", space:" << spaceit->remainingDiskSpacekB << std::endl;
//			}

		}
		else {
			qWarning() << "StorageNode::addClaim, storageNode: " << itsName.c_str() << ", task:" << taskID << " raid: " << raidID << " was not found in the remainingSpace map";
			return false;
		}
	}
	else {
		qWarning() << "StorageNode::addClaim, storageNode: " << itsName.c_str() << ", task:" << taskID << " asking for raid: " << raidID << " which is not on this storage node";
		return false;
	}
	return true;
}

void StorageNode::removeClaim(unsigned taskID) {
	capacityTimeMap::iterator cit;
	size_t claimIdx = 0;
	double bandWidthCorrection(0), spaceCorrection(0);
	bool correctBandWidth(false);
	while (claimIdx < itsClaims.size()) {
		const storageClaim &claim = itsClaims.at(claimIdx);
		if (claim.taskID == taskID) {
			// directly update itsRemainingSpace log to remove the task from the different raid arrays
			cit = itsRemainingSpace.find(claim.raidID); // find the raid array for this claim in itsRemainingSpace
			if (cit != itsRemainingSpace.end()) {
				size_t idx = 0;
				while (idx < cit->second.size()) {
					capacityLogPoint &logPoint = cit->second.at(idx);
					if (logPoint.claimID == claim.claimID) { // is this the logpoint belonging to this claim?
						bandWidthCorrection += claim.claimBandWidth;
						spaceCorrection += claim.claimByteSize;
						correctBandWidth = true;
						// erase the element
						cit->second.erase(cit->second.begin()+idx);
					}
					else if (correctBandWidth) {
						logPoint.remainingDiskSpacekB -= spaceCorrection; // correct space
						if (logPoint.time <= claim.endTime) { // bandwidth should only be updated upto the end of the task
							logPoint.remainingDiskWriteBW -= bandWidthCorrection;
						}
						++idx;
					}
					else ++idx;
				}
			}
			// erase the claim from itsClaims
			itsClaims.erase(itsClaims.begin()+claimIdx);
		}
		else ++claimIdx;
		// search for more of this task's claims in itsClaims, continue for loop
	}

	//also update the node's total bandwidth log
	size_t i=0;
	while (i < itsRemainingBandwidth.size()) {
		nodeBandWidthLogPoint &logpoint = itsRemainingBandwidth.at(i);
		if (logpoint.taskID == taskID) {
			bandWidthCorrection += logpoint.nodeBWdiff;
			// only correct the bandwidth for the logpoints in between the task start (-bandwidth) and task end time (+bandwidth)
			// if we are past both logpoints these two should cancel each other out (bandWidthCorrection ~0).
			// After the task's end logpoint we don't want to correct the bandwidth for the following logpoints
			fabs(bandWidthCorrection) < 0.01 ? correctBandWidth = true : correctBandWidth = false;
			// erase the element
			itsRemainingBandwidth.erase(itsRemainingBandwidth.begin() + i);
		}
		else if (correctBandWidth) {
			logpoint.remainingNodeBW -= bandWidthCorrection; // correct the bandwidth for removed logpoints
			++i;
		}
		else ++i;
	}
}

bool StorageNode::checkClaim(unsigned int taskID, dataProductTypes dataProduct, int raidID) const {
	for (storageClaimsVector::const_iterator it = itsClaims.begin(); it != itsClaims.end(); ++it) {
		if ((it->taskID == taskID) && (it->dataProductType == dataProduct) && (it->raidID == raidID)) return true;
	}
	return false;
}
/*
const double &StorageNode::getSpaceAtTime(int raidID, const AstroDateTime &time) {

}
*/
