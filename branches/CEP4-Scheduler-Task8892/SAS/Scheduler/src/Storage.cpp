/*
 * Storage.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 5-aug-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Storage.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "Storage.h"
#include <QDateTime>
#include "astrodatetime.h"
#include "Controller.h"
#include <map>
#include <algorithm>
#include <cmath>

using std::map;
using std::min;
using std::max;

Storage::Storage() {
}

Storage::~Storage() {
}

bool storageLocationsContains(const storageLocationOptions &locs, int node_id, int raid_id) {
	for (std::vector<std::pair<int, nodeStorageOptions> >::const_iterator it = locs.begin(); it != locs.end(); ++it) {
		if (it->first == node_id) {
			for (std::vector<storageOption>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
				if (sit->raidID == raid_id) return true;
			}
		}
	}
	return false;
}

bool Storage::addStorageNode(const std::string &nodeName, int nodeID) {
	if (itsStorageNodes.find(nodeID) == itsStorageNodes.end()) {
		itsStorageNodes[nodeID] = StorageNode(nodeName, nodeID);
		return true;
	}
	else return false;
}

void Storage::addStoragePartition(int nodeID, unsigned short partitionID, const std::string &path, const double &capacity, const double &free_space) {
	QDateTime cT = QDateTime::currentDateTime().toUTC();
	AstroDateTime now(cT.date().day(), cT.date().month(), cT.date().year(),
			cT.time().hour(), cT.time().minute(), cT.time().second());
	itsStorageNodes[nodeID].addPartition(partitionID, path, now, capacity, free_space);
}

void Storage::clearStorageClaims(void) {
	for (storageNodesMap::iterator it = itsStorageNodes.begin(); it != itsStorageNodes.end(); ++it) {
		it->second.clearClaims();
	}
	itsTaskStorageNodes.clear();
	itsLastStorageCheckResult.clear();
}

void Storage::initStorage(void) {
	itsStorageNodes.clear();
	QDateTime cT = QDateTime::currentDateTime().toUTC();
	const hostPartitionsMap &partitions = Controller::theSchedulerSettings.getStoragePartitions();
	const storageHostsMap &nodes = Controller::theSchedulerSettings.getStorageNodes();
	const double &storageNodeBW = Controller::theSchedulerSettings.getStorageNodeBandWidth();
	StorageNode node;
	AstroDateTime now(cT.date().day(), cT.date().month(), cT.date().year(),
			cT.time().hour(), cT.time().minute(), cT.time().second());
	storageHostsMap::const_iterator nit;
	for (hostPartitionsMap::const_iterator it = partitions.begin(); it != partitions.end(); ++it) {
		nit = nodes.find(it->first);
		if (nit != nodes.end()) {
			node.initNode(nit->second, now, storageNodeBW);
			for (dataPathsMap::const_iterator pit = it->second.begin(); pit != it->second.end(); ++pit) {
				node.addPartition(pit->first, pit->second.first, now, pit->second.second[0], pit->second.second[3]);
			}
			itsStorageNodes[it->first] = node;
			node.clear();
		}
	}
}

std::vector<storageResult> Storage::addStorageToTask(Task *pTask, const storageMap &storageLocations) {
    if (pTask->hasStorage()) {
        TaskStorage *task_storage = pTask->storage();
        storageNodesMap::iterator sit;
        const AstroDateTime &start = pTask->getScheduledStart();
        const AstroDateTime &end = pTask->getScheduledEnd();
        unsigned durationSec = pTask->getDuration().totalSeconds();
        const dataFileMap &dataFiles = task_storage->getOutputFileSizes(); // contains the number of files and the size of an individual file for each output data product of the task
        double claimSize, bandWidth;
        // check node bandwidth requirements (CAUTION: multiple data product types could use the same storage node
        // search for dataproducts that use the same storage node
        std::map<int, double> totalBWPerNodeMap;
        double dpBWPerLocation;
        task_conflict res;
        for (storageMap::const_iterator it = storageLocations.begin(); it != storageLocations.end(); ++it) {
            dataFileMap::const_iterator dit = dataFiles.find(it->first);
            if (dit != dataFiles.end()) {
                // total bandwidth (kbit/sec) required from a storage node to receive the required number of files from the data product
                // ceil(total_number_files / number_of_raids_used) * filesize [kbyte] / duration [seconds] * 8
                //			std::cout << "task:" << pTask->getID() << ", dataproduct:" << DATA_PRODUCTS[dit->first] << std::endl
                //					<< "nr of files:" << dit->second.second << ", size per file:" << dit->second.first << "number of locations:" << it->second.size() << std::endl
                //					<< "BW per location:" << ceil((double)dit->second.second / it->second.size()) * dit->second.first / durationSec * 8 << std::endl;
                dpBWPerLocation = ceil((double)dit->second.second / it->second.size()) * dit->second.first / durationSec * 8;
                for (storageVector::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
                    if (totalBWPerNodeMap.find(sit->first) == totalBWPerNodeMap.end()) { // node not already in totalSizePerNodeMap?
                        // for each time this node is used by this data product increase its totalSizePerNodeMap value accordingly
                        for (storageVector::const_iterator countit = sit; countit !=  it->second.end(); ++countit) {
                            if (countit->first == sit->first) { // for each time this storage node is used in storageVector
                                totalBWPerNodeMap[sit->first] += dpBWPerLocation; // add the amount of bandwidth used by the set of files
                            }
                        }
                    }
                }
            }
        }
        itsLastStorageCheckResult.clear();
        if  (pTask->getOutputDataproductCluster() == "CEP4") { //Can we just skip this for CEP4 ? /AR
            debugWarn("sis","Storage::addStorageToTask: Did not check storage for task:", pTask->getID(), " (CEP4 detected)");
        }
        else {
            // check if the total bandwidths for the nodes used do not exceed the nodes their available bandwidths
            for (std::map<int, double>::const_iterator nit = totalBWPerNodeMap.begin(); nit != totalBWPerNodeMap.end(); ++nit) {
                storageNodesMap::const_iterator nodeit = itsStorageNodes.find(nit->first);
                if (nodeit != itsStorageNodes.end()) {
                    //			std::cout << "Total bandwidth required for node:" << nodeit->second.name() << " = " << nit->second << " kb/s" << std::endl;
                    res = nodeit->second.checkBandWidth(start, end, nit->second);
                    if (res != CONFLICT_NO_CONFLICT) {
                        itsLastStorageCheckResult.push_back(storageResult(_END_DATA_PRODUCTS_ENUM_, nit->first, -1, res));
                    }
                }
            }
            if (itsLastStorageCheckResult.empty()) { // if no total bandwidth error for any node then start the rest of the checks
                for (dataFileMap::const_iterator dfit = dataFiles.begin(); dfit != dataFiles.end(); ++dfit) {
                    storageMap::const_iterator stit = storageLocations.find(dfit->first);
                    if (stit != storageLocations.end()) {
                        if (!stit->second.empty()) {
                            claimSize = (double) dfit->second.first * dfit->second.second / stit->second.size(); // size per file * nrFiles / nr of raid arrays assigned
                            bandWidth = (double) claimSize / 1000 / durationSec; // MByte/sec, the required remaining disk write speed (or bandwidth) for this array

                            // check requested resources
                            for (storageVector::const_iterator it = stit->second.begin(); it != stit->second.end(); ++it) {
                                sit = itsStorageNodes.find(it->first);
                                if (sit != itsStorageNodes.end()) {
                                    // check size requirements
                                    res = sit->second.checkSpaceAndWriteSpeed(start, end, claimSize, bandWidth, it->second); // check space and write speed for every raid array
                                    if (res != CONFLICT_NO_CONFLICT) {
                                        itsLastStorageCheckResult.push_back(storageResult(dfit->first, it->first, it->second, res));
                                        //								itsLastStorageCheckResult[it->first].push_back(std::pair<int, task_conflict>(it->second, res)); // store the error result
                                    }
                                    else { // add the claim
                                        sit->second.addClaim(pTask->getID(), start, end, dfit->first, claimSize, bandWidth, it->second);
                                    }
                                }
                            }
                            // if there were conflicts then remove the claim again from the storage nodes
                            if (!itsLastStorageCheckResult.empty()) {
                                std::vector<int> snd;
                                for (storageVector::const_iterator it = stit->second.begin(); it != stit->second.end(); ++it) {
                                    sit = itsStorageNodes.find(it->first);
                                    if (sit != itsStorageNodes.end()) {
                                        if (std::find(snd.begin(), snd.end(), stit->first) == snd.end()) {
                                            sit->second.removeClaim(pTask->getID()); // only call removeClaim one time for every storage node (it removes all claims found for the task ID)
                                            snd.push_back(stit->first);
                                        }
                                    }
                                }
                            }
                        }
                        else { // no storage has been assigned to this data product type
                            itsLastStorageCheckResult.push_back(storageResult(dfit->first, -1, -1, CONFLICT_NO_STORAGE_ASSIGNED));
                        }
                    }
                    else { // no storage has been assigned to this data product type
                        itsLastStorageCheckResult.push_back(storageResult(dfit->first, -1, -1, CONFLICT_NO_STORAGE_ASSIGNED));
                    }
                }
            }
        }
        if (itsLastStorageCheckResult.empty()) {
            task_storage->unAssignStorage();
            task_storage->setStorage(storageLocations); // sets the new locations in the task
            for (storageMap::const_iterator tsit = storageLocations.begin(); tsit != storageLocations.end(); ++tsit) {
                task_storage->setOutputDataProductAssigned(tsit->first, true);
            }
        }
    }
    else {
        debugWarn("sis","Storage::addStorageToTask: Cannot add storage to task:", pTask->getID(), " (hint:not an observation or pipeline?)");
    }

    return itsLastStorageCheckResult;
}

std::vector<storageResult> Storage::addStorageToTask(Task *pTask, dataProductTypes dataProduct, const storageVector &storageLocations, bool noCheck) {
    if (pTask->hasStorage()) {
        TaskStorage *task_storage = pTask->storage();
        storageNodesMap::iterator sit;
        const AstroDateTime &start = pTask->getScheduledStart();
        const AstroDateTime &end = pTask->getScheduledEnd();
        unsigned durationSec = pTask->getDuration().totalSeconds();
        unsigned taskID = pTask->getID();
        const dataFileMap &dataFiles = task_storage->getOutputFileSizes(); // contains the number of files and the size of an individual file for each output data product of the task
        double claimSize, bandWidth;
        // iterate over all required data products for the task
        //	for (dataFileMap::const_iterator dpit = dataFiles.begin(); dpit != dataFiles.end(); ++dpit) {
        dataFileMap::const_iterator dfit = dataFiles.find(dataProduct);
        itsLastStorageCheckResult.clear();
        if (dfit != dataFiles.end()) {
            // claimsize = size of the claim for this raid array
            claimSize = (double) dfit->second.first * dfit->second.second / storageLocations.size(); // size per file * nrFiles / nr of raid arrays assigned
            bandWidth = (double) claimSize / 1000 / durationSec; // MByte/sec, the required remaining disk write speed (or bandwidth) for this array
            //	std::cout << "total size: " << totalStorageSize << std::endl << "nr of storage locations:" << storageLocations.size() << std::endl << "size per node: " << sizePerNode << std::endl
            //	<< "total bandwidth: " << totalBandWidth  << std::endl << "per node: " << bandWidthPerNode << std::endl;
            task_conflict res(CONFLICT_NO_CONFLICT);
            for (storageVector::const_iterator it = storageLocations.begin(); it != storageLocations.end(); ++it) {
                res = CONFLICT_NO_CONFLICT;
                sit = itsStorageNodes.find(it->first);
                if (sit != itsStorageNodes.end()) {
                    // check size requirements
                    if (!noCheck) {
                        res = sit->second.checkSpaceAndWriteSpeed(start, end, claimSize, bandWidth, it->second); // check space and bandwidth for every raid array
                    }
                    if (res == CONFLICT_NO_CONFLICT) {
                        sit->second.addClaim(taskID, start, end, dataProduct, claimSize, bandWidth, it->second);
                        if (std::find(itsTaskStorageNodes[taskID].begin(), itsTaskStorageNodes[taskID].end(), it->first) == itsTaskStorageNodes[taskID].end()) {
                            itsTaskStorageNodes[taskID].push_back(it->first);
                        }
                    }
                    else {
                        itsLastStorageCheckResult.push_back(storageResult(dataProduct, it->first, it->second, res));
                    }
                }
            }
            if (!storageLocations.empty() && res == CONFLICT_NO_CONFLICT) {
                task_storage->addStorage(dataProduct, storageLocations); // adds the storage to the task
                task_storage->setOutputDataProductAssigned(dataProduct, true);
            }
        }
        else {
            // error: dataProduct not found in dataFiles map of the task!
        }
    }
    else {
        debugWarn("sis","Storage::addStorageToTask: Cannot add storage to task:", pTask->getID(), " (hint:not an observation or pipeline?)");
    }
    return itsLastStorageCheckResult;
}

void Storage::removeTaskStorage(unsigned taskID) {
	std::map<unsigned, std::vector<int> >::iterator it = itsTaskStorageNodes.find(taskID);
	if (it != itsTaskStorageNodes.end()) {
		for (std::vector<int>::iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
			storageNodesMap::iterator snit = itsStorageNodes.find(*sit);
			if (snit != itsStorageNodes.end()) {
				snit->second.removeClaim(taskID);
			}
		}
		itsTaskStorageNodes.erase(it);
	}
}

// function checkAssignedTaskStorage is used for checking if the given task it's claims are registered at the storage nodes assigned to the task
// assuming it is not possible to assign storage to a task if a conflict arises from it, the function doesn't check if the size and bandwidth requirements are fulfilled.
// it assumes that if the task has been registered at the assigned storage nodes that everything is fine.
std::vector<storageResult> Storage::checkAssignedTaskStorage(Task *pTask, dataProductTypes dataProduct) {
    if (pTask->hasStorage()) {
        const storageMap &storageLocations = pTask->storage()->getStorageLocations();
        storageNodesMap::const_iterator snit;
        storageMap::const_iterator stit = storageLocations.find(dataProduct);
        itsLastStorageCheckResult.clear();
        unsigned int taskID(pTask->getID());
        if (stit != storageLocations.end()) {
            for (storageVector::const_iterator sit = stit->second.begin(); sit != stit->second.end(); ++sit) {
                snit = itsStorageNodes.find(sit->first);
                if (snit != itsStorageNodes.end()) {
                    if (!snit->second.checkClaim(taskID, dataProduct, sit->second)) {
                        itsLastStorageCheckResult.push_back(storageResult(dataProduct, sit->first, sit->second, CONFLICT_NO_STORAGE_ASSIGNED));
                        pTask->setConflict(CONFLICT_NO_STORAGE_ASSIGNED);
                    }
                }
                else {
                    itsLastStorageCheckResult.push_back(storageResult(dataProduct, sit->first, sit->second, CONFLICT_STORAGE_NODE_INEXISTENT));
                    pTask->setConflict(CONFLICT_STORAGE_NODE_INEXISTENT);
                }
            }
        }
        else {
            std::cerr << "Storage::checkAssignedTaskStorage, Warning: data product " << DATA_PRODUCTS[dataProduct] << " not specified for task:" << pTask->getID() << std::endl;
        }
    }
    return itsLastStorageCheckResult;
}


storageLocationOptions Storage::getStorageLocationOptions(dataProductTypes dataProduct, const AstroDateTime &startTime, const AstroDateTime &endTime,
		const double &fileSize, const double &bandWidth, unsigned minNrFiles, sortMode sort_mode, const std::vector<int> &nodes) {
	storageLocationOptions locations;
	itsLastStorageCheckResult.clear();
	nodeStorageOptions node_options;
	std::vector<std::pair<int, task_conflict> > checkResult; // raidID, conflict
	// randomize the storage nodes sequence so that storage nodes with the same number of claims are selected at random
	std::vector<int> randomizedStorageNodes;
	storageNodesMap::const_iterator sit;
	for (std::vector<int>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
		sit = itsStorageNodes.find(*it);
		if (sit != itsStorageNodes.end()) {
			if (sit->second.mayBeUsed()) {
				randomizedStorageNodes.push_back(*it);
			}
		}
	}
	std::random_shuffle( randomizedStorageNodes.begin(), randomizedStorageNodes.end() );

	for (std::vector<int>::const_iterator it = randomizedStorageNodes.begin(); it != randomizedStorageNodes.end(); ++it) {
		sit = itsStorageNodes.find(*it);
		checkResult.clear();
		node_options = sit->second.getPossibleRaidArrays(startTime, endTime, fileSize, bandWidth, minNrFiles, checkResult);
		if (!node_options.empty()) {
			locations.push_back(storageLocationOptions::value_type(sit->first, node_options));
		}
		if (!checkResult.empty()) {
			for (std::vector<std::pair<int, task_conflict> >::const_iterator chit = checkResult.begin(); chit != checkResult.end(); ++chit) {
				itsLastStorageCheckResult.push_back(storageResult(dataProduct, sit->first, chit->first, chit->second));
			}
		}
	}
	// sorting requested?
	if (sort_mode == SORT_USAGE) {
		bool inserted;
		storageLocationOptions sortedLocs;
		for (storageLocationOptions::const_iterator slit = locations.begin(); slit != locations.end(); ++slit) {
			inserted = false;
			for (storageLocationOptions::iterator svit = sortedLocs.begin(); svit != sortedLocs.end(); ++svit) {
				if (itsStorageNodes.find(slit->first)->second.nrClaims() < itsStorageNodes.find(svit->first)->second.nrClaims()) {
					sortedLocs.insert(svit, storageLocationOptions::value_type(*slit));
					inserted = true;
					break;
				}
			}
			if (!inserted) {
				sortedLocs.push_back(storageLocationOptions::value_type(*slit));
			}
		}
		return sortedLocs;
	}

	return locations;
}

void Storage::setAllowedStorageHosts(const std::vector<int> &allowedStorageHosts) {
	std::vector<int>::const_iterator it;
	storageNodesMap::iterator sit = itsStorageNodes.begin();
	while (sit != itsStorageNodes.end()) {
		it = find(allowedStorageHosts.begin(),allowedStorageHosts.end(),sit->second.getID());
		if (it != allowedStorageHosts.end()) {
			sit->second.setMayBeUsed(true);
		}
		else {
			sit->second.setMayBeUsed(false);
		}
		++sit;
	}
}
