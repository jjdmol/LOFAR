/*
 * storage_definitions.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 29-july-2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/storage_definitions.h $
 * description    : header file to include all storage definition classes and typedefs
 *
 */

#ifndef STORAGE_DEFINITIONS_H
#define STORAGE_DEFINITIONS_H

#include <map>
#include <vector>
#include <string>

enum pipelineType {
    PIPELINE_IMAGING,
    PIPELINE_CALIBRATION,
    PIPELINE_PULSAR,
    PIPELINE_LONGBASELINE,
    PIPELINE_UNKNOWN
};

enum task_conflict {
    CONFLICT_NO_CONFLICT,
    CONFLICT_BITMODE,
    CONFLICT_OUT_OF_DATASLOTS,
    CONFLICT_MAINTENANCE,
    CONFLICT_RESERVATION,
    CONFLICT_STATIONS,
    CONFLICT_INSUFFICIENT_STORAGE,
    CONFLICT_STORAGE_NO_DATA,
    CONFLICT_STORAGE_EXCEEDS_BANDWIDTH,
    CONFLICT_STORAGE_NODE_INACTIVE,
    CONFLICT_STORAGE_NODE_INEXISTENT,
    CONFLICT_STORAGE_NO_NODES,
    CONFLICT_STORAGE_TOO_FEW_NODES,
    CONFLICT_STORAGE_MINIMUM_NODES,
    CONFLICT_STORAGE_NO_OPTIONS,
    CONFLICT_STORAGE_NODE_SPACE,
    CONFLICT_STORAGE_WRITE_SPEED,
    CONFLICT_RAID_ARRRAY_NOT_FOUND,
    CONFLICT_STORAGE_TIME_TOO_EARLY,
    CONFLICT_STORAGE_NODE_BANDWIDTH,
    CONFLICT_STORAGE_SINGLE_FILE_BW_TOO_HIGH,
    CONFLICT_GROUP_STORAGE_MODE_DIFFERENT,
    CONFLICT_GROUP_STORAGE_UNEQUAL_DATAPRODUCTS,
    CONFLICT_NO_STORAGE_ASSIGNED,
    CONFLICT_NON_INTEGER_OUTPUT_FILES,
    _END_CONFLICTS_
};
extern const char * TASK_CONFLICTS[_END_CONFLICTS_];

enum dataProductTypes {
    _BEGIN_DATA_PRODUCTS_ENUM_,
    DP_CORRELATED_UV = _BEGIN_DATA_PRODUCTS_ENUM_,
    DP_COHERENT_STOKES,
    DP_INCOHERENT_STOKES,
    DP_INSTRUMENT_MODEL,
    DP_PULSAR,
    DP_SKY_IMAGE,
    DP_UNKNOWN_TYPE,
    _END_DATA_PRODUCTS_ENUM_
};
#define NR_DATA_PRODUCT_TYPES	_END_DATA_PRODUCTS_ENUM_

// storageVector: pairs of <storage node ID, location ID> for each subband or beam
typedef std::vector<std::pair<int,int> > storageVector;
//storageMap contains the storage nodes and raid arrays IDs for all dataProductTypes
typedef std::map<dataProductTypes, storageVector> storageMap;

class StorageNode;

class storageOption {
public:
    storageOption(int raid_id, const double &remainingspace_kb, unsigned nr_units)
    : raidID(raid_id), remainingSpacekB(remainingspace_kb), nrUnits(nr_units) {}
    int raidID;
    double remainingSpacekB; // the smallest remaining space on the raid array (during the claim's period)
    unsigned nrUnits; // the number of units of size of the claim which would fit on the raid array according to remaining bandwidth and write speed
};

typedef std::vector<storageOption> nodeStorageOptions;
typedef std::vector<std::pair<int, nodeStorageOptions> > storageLocationOptions;

// storageNodesMap: key=nodeID, value = storageNode object
typedef std::map<int, StorageNode> storageNodesMap;

class storageResult {
public:
    storageResult(dataProductTypes dataProduct, int node_id, int raid_id, task_conflict conflictType)
    : dataProductType(dataProduct), storageNodeID(node_id), storageRaidID(raid_id), conflict(conflictType) { }
    dataProductTypes dataProductType;
    int storageNodeID, storageRaidID;
    task_conflict conflict;
};

// storagePartitionsMap: the storage partitions (IDs) and their total capacity
// key= raidID, value=pair<location string, total capacity of location>
typedef std::map<int, std::pair<std::string, double> > storagePartitionsMap;


#endif // STORAGE_DEFINITIONS_H
