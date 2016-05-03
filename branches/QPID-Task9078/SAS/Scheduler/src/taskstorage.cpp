/*
 * taskstorage.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/taskstorage.cpp $
 *
 */

#include "task.h"
#include "taskstorage.h"
#include "observation.h"
#include "pipeline.h"
#include "pulsarpipeline.h"
#include "Controller.h"

//const char *storage_cluster_mode_str[NR_STORAGE_CLUSTERING_MODES] = {"Project preferred nodes", "Data type preferred nodes"};

const char *storage_select_mode_str[NR_STORAGE_SELECTION_MODES] = {"Maximum - data type preferred", "Maximum - project preferred",
        "Minimum - data type preferred", "Minimum - project preferred", "Manual selection"};

TaskStorage::TaskStorage(const Task *owner, const TaskStorage *other)
    : itsOwner(owner), itsRecalcStorageNeeded(true), itsStorageSelectionMode(STORAGE_MODE_MAXIMUM_PROJECT_PREFERRED), itsTotalDataSizekBytes(0.0), itsTotalBandWidth(0.0)
{
    if (other) {
        itsRecalcStorageNeeded = other->itsRecalcStorageNeeded;
        itsStorageSelectionMode = other->itsStorageSelectionMode;
        itsOutputDataFiles = other->itsOutputDataFiles;
        itsInputDataFiles = other->itsInputDataFiles;
        itsEnabledInputData = other->itsEnabledInputData;
        itsEnabledOutputData = other->itsEnabledOutputData;
        itsInputDataProducts = other->itsInputDataProducts;
        itsOutputDataProducts = other->itsOutputDataProducts;
        itsTotalDataSizekBytes = other->itsTotalDataSizekBytes;
        itsTotalBandWidth = other->itsTotalBandWidth;
        itsStorage = other->itsStorage;
        itsInputStorageLocations = other->itsInputStorageLocations;
        itsStorageCheckResult = other->itsStorageCheckResult;
        itsOwner = owner;
    }
}


QDataStream& operator<< (QDataStream &out, const TaskStorage &storage) {
    if (out.status() == QDataStream::Ok) {

        // itsOwner is already set by the parent task
        out << storage.itsRecalcStorageNeeded;
        // itsStorageSelectionMode
        out << (quint8) storage.itsStorageSelectionMode;

        // itsStorage
        out << (quint32) storage.itsStorage.size();
        for (storageMap::const_iterator it = storage.itsStorage.begin(); it != storage.itsStorage.end(); ++it) {
            out << (quint8) it->first
                << (quint32) it->second.size();
            for (storageVector::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit) {
                out << (quint16) sit->first // storage node ID
                    << (quint16) sit->second; // storage raid ID
            }
        }

        // input data product specifications
        // itsEnabledInputData
        out << storage.itsEnabledInputData.coherentStokes << storage.itsEnabledInputData.correlated
            << storage.itsEnabledInputData.incoherentStokes << storage.itsEnabledInputData.instrumentModel;

        // itsInputDataProducts
        out << (quint32) storage.itsInputDataProducts.size();
        for (std::map<dataProductTypes, TaskStorage::inputDataProduct>::const_iterator it = storage.itsInputDataProducts.begin(); it != storage.itsInputDataProducts.end(); ++it) {
            out << (quint8) it->first;

            out << (quint32) it->second.identifications.size();
            for (QStringList::const_iterator strit = it->second.identifications.begin(); strit != it->second.identifications.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.locations.size();
            for (QStringList::const_iterator strit = it->second.locations.begin(); strit != it->second.locations.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.filenames.size();
            for (QStringList::const_iterator strit = it->second.filenames.begin(); strit != it->second.filenames.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.skip.size();
            for (std::vector<bool>::const_iterator strit = it->second.skip.begin(); strit != it->second.skip.end(); ++strit) {
                out << *strit;
            }

        }

        // input data files (number of files and file size for each input data product)
        // itsInputDataFiles
        out << (quint32) storage.itsInputDataFiles.size();
        for (dataFileMap::const_iterator it = storage.itsInputDataFiles.begin(); it != storage.itsInputDataFiles.end(); ++it) {
            out << (quint8) it->first << it->second.first << it->second.second;
        }

        // output data files (number of files and file size for each output data product)
        // itsOutputDataFiles
        out << (quint32) storage.itsOutputDataFiles.size();
        for (dataFileMap::const_iterator it = storage.itsOutputDataFiles.begin(); it != storage.itsOutputDataFiles.end(); ++it) {
            out << (quint8) it->first << it->second.first << it->second.second;
        }

        // output data product specifications

        // itsEnabledOutputData
        out << storage.itsEnabledOutputData.coherentStokes << storage.itsEnabledOutputData.correlated
            << storage.itsEnabledOutputData.incoherentStokes << storage.itsEnabledOutputData.instrumentModel
            << storage.itsEnabledOutputData.pulsar << storage.itsEnabledOutputData.skyImage
            << storage.itsEnabledOutputData.coherentStokesAssigned << storage.itsEnabledOutputData.complexVoltagesAssigned
            << storage.itsEnabledOutputData.correlatedAssigned	<< storage.itsEnabledOutputData.incoherentStokesAssigned
            << storage.itsEnabledOutputData.instrumentModelAssigned << storage.itsEnabledOutputData.pulsarAssigned << storage.itsEnabledOutputData.skyImageAssigned;

        // itsOutputDataProducts
        out << (quint32) storage.itsOutputDataProducts.size();
        for (std::map<dataProductTypes, TaskStorage::outputDataProduct>::const_iterator it = storage.itsOutputDataProducts.begin(); it != storage.itsOutputDataProducts.end(); ++it) {
            out << (quint8) it->first;

            out << (quint32) it->second.identifications.size();
            for (QStringList::const_iterator strit = it->second.identifications.begin(); strit != it->second.identifications.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.locations.size();
            for (QStringList::const_iterator strit = it->second.locations.begin(); strit != it->second.locations.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.filenames.size();
            for (QStringList::const_iterator strit = it->second.filenames.begin(); strit != it->second.filenames.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.uniqueMointPoints.size();
            for (QStringList::const_iterator strit = it->second.uniqueMointPoints.begin(); strit != it->second.uniqueMointPoints.end(); ++strit) {
                out << strit->toStdString();
            }

            out << (quint32) it->second.skip.size();
            for (std::vector<bool>::const_iterator strit = it->second.skip.begin(); strit != it->second.skip.end(); ++strit) {
                out << *strit;
            }
        }

        out << storage.itsTotalDataSizekBytes << storage.itsTotalBandWidth;

//        storageMap itsInputStorageLocations; // itsStorage contains the 'asked' storage node IDs and raid IDs
//        std::vector<storageResult> itsStorageCheckResult;

    }
    return out;
}

QDataStream& operator>> (QDataStream &in, TaskStorage &storage) {
    if (in.status() == QDataStream::Ok) {
        in >> storage.itsRecalcStorageNeeded;
        // itsStorageSelectionMode
        quint8 storagemode;
        in >> storagemode;
        storage.itsStorageSelectionMode = (storage_selection_mode) storagemode;

        // storage settings
        storage.itsStorage.clear();
        std::pair<quint16,quint16> storagePair;
        storageVector storageVec;
        quint8 dptype;
        quint32 nrOfObjects, nrOfObjects2;

        in >> nrOfObjects;
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            in >> dptype
               >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> storagePair.first // storage node ID
                   >> storagePair.second; // storage raid ID
                storageVec.push_back(storagePair);
            }
            storage.itsStorage.insert(storageMap::value_type((dataProductTypes)dptype, storageVec));
            storageVec.clear();
        }

        // input data product specifications

        in >> storage.itsEnabledInputData.coherentStokes >> storage.itsEnabledInputData.correlated
            >> storage.itsEnabledInputData.incoherentStokes >> storage.itsEnabledInputData.instrumentModel;

        storage.itsInputDataProducts.clear();
        quint8 dpType;
        bool skip;
        std::string tmpString;
        in >> nrOfObjects; // itsInputDataProducts.size()
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            TaskStorage::inputDataProduct tmpInputDP;
            in >> dpType;

            // identifications
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpInputDP.identifications.push_back(tmpString.c_str());
            }
            // locations
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpInputDP.locations.push_back(tmpString.c_str());
            }
            // filenames
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpInputDP.filenames.push_back(tmpString.c_str());
            }
            // skip vector
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> skip;
                tmpInputDP.skip.push_back(skip);
            }

            storage.itsInputDataProducts[static_cast<dataProductTypes>(dpType)] = tmpInputDP;
        }

        // input data files (number of files and file size for each input data product)
        in >> nrOfObjects; //(quint32) storage.itsInputDataFiles.size();
        std::pair<double, unsigned> files;
        for (quint32 j = 0; j < nrOfObjects; ++j) {
            in >> dpType >> files.first >> files.second;
            storage.itsInputDataFiles[static_cast<dataProductTypes>(dpType)] = files;
        }

        // output data files (number of files and file size for each input data product)
        in >> nrOfObjects; //(quint32) storage.itsOutputDataFiles.size();
        for (quint32 j = 0; j < nrOfObjects; ++j) {
            in >> dpType >> files.first >> files.second;
            storage.itsOutputDataFiles[static_cast<dataProductTypes>(dpType)] = files;
        }
        // output data product specifications

        in >> storage.itsEnabledOutputData.coherentStokes >> storage.itsEnabledOutputData.correlated
            >> storage.itsEnabledOutputData.incoherentStokes >> storage.itsEnabledOutputData.instrumentModel >> storage.itsEnabledOutputData.pulsar
            >> storage.itsEnabledOutputData.skyImage >> storage.itsEnabledOutputData.coherentStokesAssigned
            >> storage.itsEnabledOutputData.complexVoltagesAssigned >> storage.itsEnabledOutputData.correlatedAssigned
            >> storage.itsEnabledOutputData.incoherentStokesAssigned >> storage.itsEnabledOutputData.instrumentModelAssigned >> storage.itsEnabledOutputData.pulsarAssigned
            >> storage.itsEnabledOutputData.skyImageAssigned;

        storage.itsOutputDataProducts.clear();
        in >> nrOfObjects; // itsOutputDataProducts.size()
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            TaskStorage::outputDataProduct tmpOutputDP;
            in >> dpType;

            // identifications
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpOutputDP.identifications.push_back(tmpString.c_str());
            }
            // locations
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpOutputDP.locations.push_back(tmpString.c_str());
            }
            // filenames
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpOutputDP.filenames.push_back(tmpString.c_str());
            }
            // mointpoints
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> tmpString;
                tmpOutputDP.uniqueMointPoints.push_back(tmpString.c_str());
            }
            // skip vector
            in >> nrOfObjects2;
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> skip;
                tmpOutputDP.skip.push_back(skip);
            }
            storage.itsOutputDataProducts[static_cast<dataProductTypes>(dpType)] = tmpOutputDP;
        }

        in >> storage.itsTotalDataSizekBytes >> storage.itsTotalBandWidth;
    }
    return in;
}

//WK code commented out
// Returns True if the input and output node locations are equal for
// all the input and output products
// THis function should be moved to the pipeline class?
//bool TaskStorage::getEqualityInputOutputProducts()const
//{
//    // Check we have the same number of dataproduct types
//    if (itsInputDataProducts.size() != itsOutputDataProducts.size())
//        return false;

//    //loop over the input and output data types
//    std::map<dataProductTypes, inputDataProduct >::const_iterator inputTypePair;
//    std::map<dataProductTypes, outputDataProduct >::const_iterator outputTypePair;
//    for (inputTypePair = itsInputDataProducts.begin(),
//         outputTypePair = itsOutputDataProducts.begin();
//         inputTypePair != itsInputDataProducts.end();  // length is the same
//         ++inputTypePair, ++outputTypePair )
//    {
//        // Check if we have the same number of input and output entries
//        if (inputTypePair->second.locations.size() !=
//            outputTypePair->second.locations.size())
//            return false;

//        // Loop over all the input and output locations
//        QStringList::const_iterator inputLoc;
//        QStringList::const_iterator outputLoc;
//        for (inputLoc = inputTypePair->second.locations.begin(),
//             outputLoc = outputTypePair->second.locations.begin();
//             inputLoc != inputTypePair->second.locations.end();
//             ++inputLoc , ++outputLoc)
//        {
//            //return false if the nodes are not the same
//            if (inputLoc->split(":").at(0) != outputLoc->split(":").at(0))
//                return false;
//        }
//    }
//    return true;
//}

void TaskStorage::setInputFileSizes(dataProductTypes dpType, const std::pair<double, unsigned> &inputFileSizes) {
    itsInputDataFiles[dpType] = inputFileSizes;
    itsRecalcStorageNeeded = true;
}

unsigned TaskStorage::getNrFiles(void) const {
    unsigned nrFiles(0);
    for (std::map<dataProductTypes, std::pair<double, unsigned> >::const_iterator it = itsOutputDataFiles.begin(); it != itsOutputDataFiles.end(); ++it) {
        nrFiles += it->second.second;
    }
    return nrFiles;
}

std::map<dataProductTypes, int> TaskStorage::getMinimumNrOfStorageNodes(void) const {
    std::map<dataProductTypes, int> minNrOfNodesMap;
    double bandwidthPerFile, storageNodeBW(Controller::theSchedulerSettings.getStorageNodeBandWidth());
    unsigned maxFilesToNode(0), maxFilesPerNode(Controller::theSchedulerSettings.getMaxNrOfFilesPerStorageNode());
    int durationSec = itsOwner->getDuration().totalSeconds();
    if (durationSec > 0) {
        for (std::map<dataProductTypes, std::pair<double, unsigned> >::const_iterator mit = itsOutputDataFiles.begin(); mit != itsOutputDataFiles.end(); ++mit) {
            bandwidthPerFile = mit->second.first * 8 / (double) durationSec; // kbit/sec
            maxFilesToNode = std::min((unsigned)std::floor(storageNodeBW / bandwidthPerFile), maxFilesPerNode);
            if (maxFilesToNode > 0) {
                minNrOfNodesMap[mit->first] = (int)(std::ceil((float)(mit->second.second) / maxFilesToNode)); // ceil(total_nr_files / max_files_to_node)
            }
            else minNrOfNodesMap[mit->first] = -1; // the bandwidth required for a single file of this dataproduct exceeds the single storage node network bandwidth
        }
    }
    return minNrOfNodesMap;
}


std::pair<double, unsigned> TaskStorage::getOutputFileSizes(dataProductTypes dpType) const {
    dataFileMap::const_iterator it = itsOutputDataFiles.find(dpType);
    if (it != itsOutputDataFiles.end()) {
        return it->second;
    }
    else return std::pair<double, unsigned>(0,0);
}


storageVector TaskStorage::getStorageLocations(dataProductTypes dpType) const {
    storageMap::const_iterator it = itsStorage.find(dpType);
    if (it != itsStorage.end()) {
        return it->second;
    }
    else return storageVector();
}

bool TaskStorage::setStorage(dataProductTypes dataProduct, const QStringList &nodeList, const QStringList &raidList) {
    if (!nodeList.isEmpty() & (nodeList.size() == raidList.size())) {
        std::string nodeName, raidName;
        int nodeID, locationID;
        bool stop(false), result(true);
        storageVector tmpStorageVec;
        for (int idx = 0; idx < nodeList.size(); ++idx) { // nodeList and raidList are of equal size (see check at beginning)
            nodeName = nodeList.at(idx).toStdString();
            raidName = raidList.at(idx).toStdString();
            nodeID = Controller::theSchedulerSettings.getStorageNodeID(nodeName);
            if (nodeID != 0) {
                locationID = Controller::theSchedulerSettings.getStorageRaidID(nodeID, raidName);
                if (locationID != 0) {
                    // look for this entry in storageMap, if it is already inserted then we can stop because of the repeating pattern
                    for (std::vector<std::pair<int, int> >::const_iterator vit = tmpStorageVec.begin(); vit != tmpStorageVec.end(); ++vit) {
                        if ((vit->first == nodeID) & (vit->second == locationID)) {
                            stop = true;
                            //result = true;
                            break;
                        }
                    }
                    if (stop) break; // break out of first for loop
                    tmpStorageVec.push_back(std::pair<int, int>(nodeID, locationID));
                }
                else {
                    qWarning() << "TaskStorage::setStorage, SAS tree: " << itsOwner->getSASTreeID() << ", node: " << nodeName.c_str() << " raid: " << raidName.c_str() << " does not exist";
                    result = false;
                }
            }
            else {
                qWarning() << "TaskStorage::setStorage, SAS tree: " << itsOwner->getSASTreeID() << " storage node:" << nodeName.c_str() << " does not exist";
                result = false;
            }
        }
        if (result == true) {
            itsStorage[dataProduct] = tmpStorageVec;
            return true;
        }
        else return false;
    }
    else {
        qWarning() << "TaskStorage::setStorage, SAS tree: " << itsOwner->getSASTreeID() << "data type: " << DATA_PRODUCTS[dataProduct] << " empty or not of equal length storage nodes and raid array keys!";
        return false;
    }
}


void TaskStorage::addStorage(dataProductTypes dataProductType, const storageVector &storage) {
    if (itsOwner->isObservation()) {
        // sort the nodes for observations (which are not dependent on input data products)
        storageVector cpy(storage);
        sort(cpy.begin(), cpy.end(), cmp_intPairSecond()); // first sort on raid IDs
        sort(cpy.begin(), cpy.end(), cmp_intPairFirst()); // then sort on node ID
        itsStorage[dataProductType] = cpy;
    }
    else {
        itsStorage[dataProductType] = storage;
    }
}


void TaskStorage::unAssignStorage(void) {
    itsEnabledOutputData.complexVoltagesAssigned = false;
    itsEnabledOutputData.correlatedAssigned = false;
    itsEnabledOutputData.coherentStokesAssigned = false;
    itsEnabledOutputData.incoherentStokesAssigned = false;
    itsEnabledOutputData.instrumentModelAssigned = false;
    itsEnabledOutputData.pulsarAssigned = false;
    itsEnabledOutputData.skyImageAssigned = false;
}



void TaskStorage::setInputFilesToBeProcessed(
        const std::map<dataProductTypes, std::vector<bool> > &files)
{
    for (std::map<dataProductTypes, std::vector<bool> >::const_iterator it = files.begin();
         it != files.end(); ++it)
    {
        std::map<dataProductTypes, inputDataProduct>::iterator
                fit(itsInputDataProducts.find(it->first));
        if (fit != itsInputDataProducts.end())
        {
            if (it->second.size() == (unsigned)fit->second.filenames.size())
            {
                fit->second.skip = it->second;
            }
        }
    }
}

void TaskStorage::setInputDataProductEnabled(dataProductTypes dpType, bool enabled) {
    switch (dpType) {
    case DP_COHERENT_STOKES:
        itsEnabledInputData.coherentStokes = enabled;
        break;
    case DP_CORRELATED_UV:
        itsEnabledInputData.correlated = enabled;
        break;
    case DP_INCOHERENT_STOKES:
        itsEnabledInputData.incoherentStokes = enabled;
        break;
    case DP_INSTRUMENT_MODEL:
        itsEnabledInputData.instrumentModel = enabled;
        break;
    case DP_SKY_IMAGE:
        itsEnabledInputData.skyImage = enabled;
        break;
    default:
        break;
    }
}

void TaskStorage::setOutputDataProductEnabled(dataProductTypes dpType, bool enabled) {
    switch (dpType) {
    case DP_COHERENT_STOKES:
        itsEnabledOutputData.coherentStokes = enabled;
        break;
    case DP_CORRELATED_UV:
        itsEnabledOutputData.correlated = enabled;
        break;
    case DP_INCOHERENT_STOKES:
        itsEnabledOutputData.incoherentStokes = enabled;
        break;
    case DP_INSTRUMENT_MODEL:
        itsEnabledOutputData.instrumentModel = enabled;
        break;
    case DP_PULSAR:
        itsEnabledOutputData.pulsar = enabled;
        break;
    case DP_SKY_IMAGE:
        itsEnabledOutputData.skyImage = enabled;
        break;
    default:
        break;
    }
}

void TaskStorage::setOutputDataProductAssigned(dataProductTypes dpType, bool assigned) {
    switch (dpType) {
    case DP_COHERENT_STOKES:
        itsEnabledOutputData.coherentStokesAssigned = assigned;
        break;
    case DP_CORRELATED_UV:
        itsEnabledOutputData.correlatedAssigned = assigned;
        break;
    case DP_INCOHERENT_STOKES:
        itsEnabledOutputData.incoherentStokesAssigned = assigned;
        break;
    case DP_INSTRUMENT_MODEL:
        itsEnabledOutputData.instrumentModelAssigned = assigned;
        break;
    case DP_PULSAR:
        itsEnabledOutputData.pulsarAssigned = assigned;
        break;
    case DP_SKY_IMAGE:
        itsEnabledOutputData.skyImageAssigned = assigned;
        break;
    default:
        break;
    }
}

bool TaskStorage::isInputDataProduktEnabled(dataProductTypes dpType) const {
    switch (dpType) {
    case DP_COHERENT_STOKES:
        return itsEnabledInputData.coherentStokes;
    case DP_CORRELATED_UV:
        return itsEnabledInputData.correlated;
    case DP_INCOHERENT_STOKES:
        return itsEnabledInputData.incoherentStokes;
    case DP_INSTRUMENT_MODEL:
        return itsEnabledInputData.instrumentModel;
    case DP_SKY_IMAGE:
        return itsEnabledInputData.skyImage;
    default:
        break;
    }
    return false;
}

bool TaskStorage::isOutputDataProduktEnabled(dataProductTypes dpType) const {
    switch (dpType) {
    case DP_COHERENT_STOKES:
        return itsEnabledOutputData.coherentStokes;
    case DP_CORRELATED_UV:
        return itsEnabledOutputData.correlated;
    case DP_INCOHERENT_STOKES:
        return itsEnabledOutputData.incoherentStokes;
    case DP_INSTRUMENT_MODEL:
        return itsEnabledOutputData.instrumentModel;
    case DP_PULSAR:
        return itsEnabledOutputData.pulsar;
    case DP_SKY_IMAGE:
        return itsEnabledOutputData.skyImage;
    default:
        break;
    }
    return false;
}

bool TaskStorage::isOutputDataProduktAssigned(dataProductTypes dpType) const {
    switch (dpType) {
    case DP_COHERENT_STOKES:
        return itsEnabledOutputData.coherentStokesAssigned;
    case DP_CORRELATED_UV:
        return itsEnabledOutputData.correlatedAssigned;
    case DP_INCOHERENT_STOKES:
        return itsEnabledOutputData.incoherentStokesAssigned;
    case DP_INSTRUMENT_MODEL:
        return itsEnabledOutputData.instrumentModelAssigned;
    case DP_PULSAR:
        return itsEnabledOutputData.pulsarAssigned;
    case DP_SKY_IMAGE:
        return itsEnabledOutputData.skyImageAssigned;
    default:
        break;
    }
    return false;
}

bool TaskStorage::checkStorageAssigned(void) const {
    if (itsEnabledOutputData.correlated && !itsEnabledOutputData.correlatedAssigned) return false;
    else if (itsEnabledOutputData.coherentStokes && !itsEnabledOutputData.coherentStokesAssigned) return false;
    else if (itsEnabledOutputData.incoherentStokes && !itsEnabledOutputData.incoherentStokesAssigned) return false;
    else if (itsEnabledOutputData.instrumentModel && !itsEnabledOutputData.instrumentModelAssigned) return false;
    else if (itsEnabledOutputData.pulsar && !itsEnabledOutputData.pulsarAssigned) return false;
    else if (itsEnabledOutputData.skyImage && !itsEnabledOutputData.skyImageAssigned) return false;
    else return true;
}

bool TaskStorage::hasStorageLocations(void) const {
    for (storageMap::const_iterator it = itsStorage.begin(); it != itsStorage.end(); ++it) {
        if (!it->second.empty()) return true;
    }
    return false;
}

const std::map<dataProductTypes, TaskStorage::outputDataProduct> &TaskStorage::generateFileList(void) {
    // for some types of task we need some private information, cast a pointer if needed
    QString nodeName,raidSet;
    QString observationIDStr;
    const QString & ObsIDPrefix(Controller::theSchedulerSettings.getObservationIDprefix());
    unsigned treeID(itsOwner->SASTree().treeID());
    if (treeID) {
        observationIDStr = ObsIDPrefix + QString::number(treeID); // Lxxxxx, no padding
    }
    else {
        observationIDStr = ObsIDPrefix + "?????";
    }
    QString locationStr("/" + observationIDStr + "/");
    int nrFiles;
    std::map<dataProductTypes, outputDataProduct>::iterator oit;

    const Observation *pObs(0);
    const CalibrationPipeline *pCalPipe(0);
    const ImagingPipeline *pImagingPipe(0);
    const LongBaselinePipeline *pLongBasePipe(0);
    const PulsarPipeline *pPulsarPipe(0);

    if ((pObs = dynamic_cast<const Observation *>(itsOwner))) { }
    else if ((pCalPipe = dynamic_cast<const CalibrationPipeline *>(itsOwner))) { }
    else if ((pLongBasePipe = dynamic_cast<const LongBaselinePipeline *>(itsOwner))) { }
    else if ((pImagingPipe = dynamic_cast<const ImagingPipeline *>(itsOwner))) { }
    else if ((pPulsarPipe = dynamic_cast<const PulsarPipeline *>(itsOwner))) { }
    else {
        qDebug() << "TaskStorage::generateFileList Unknown task type for task ID:" << itsOwner->getID() << ", cannot update the output file list";
        return itsOutputDataProducts;
    }


    for (dataProductTypes dp = _BEGIN_DATA_PRODUCTS_ENUM_; dp < _END_DATA_PRODUCTS_ENUM_-1; dp = dataProductTypes(dp + 1)) {
        outputDataProduct dFile;
        if (isOutputDataProduktEnabled(dp)) {
            oit = itsOutputDataProducts.find(dp);
            if (oit != itsOutputDataProducts.end()) {
                if (!oit->second.identifications.empty()) {
                    // copy already existing information on the output data product which is already in the task (e.g. identifications array)
                    dFile.identifications = oit->second.identifications;
                }
            }

            storageMap::const_iterator it = itsStorage.find(dp);
            dataFileMap::const_iterator dit = itsOutputDataFiles.find(dp);
            if( dit != itsOutputDataFiles.end()) {
                nrFiles = dit->second.second;
                if (it != itsStorage.end()) {
                    if (!it->second.empty()) {
                        // generate stringlist of storage node names concatenated with raid sets for round-robin file distribution pattern
                        int firstNode(-1), firstRaid(-1);
                        if (it->second.size() > 1) {
                            firstNode = it->second.begin()->first;
                            firstRaid = it->second.begin()->second;
                            nodeName = Controller::theSchedulerSettings.getStorageNodeName(firstNode).c_str();
                            raidSet = Controller::theSchedulerSettings.getStorageRaidName(firstNode, firstRaid).c_str();
                            if (!nodeName.isEmpty() && !raidSet.isEmpty()) {
                                dFile.uniqueMointPoints.push_back(nodeName + ":" + raidSet);
                            }
                        }
                        else {
                            nodeName = Controller::theSchedulerSettings.getStorageNodeName(it->second.front().first).c_str();
                            raidSet = Controller::theSchedulerSettings.getStorageRaidName(it->second.front().first, it->second.front().second).c_str();
                            if (!nodeName.isEmpty() && !raidSet.isEmpty()) {
                                dFile.uniqueMointPoints.push_back(nodeName + ":" + raidSet);
                            }
                        }
                        for (storageVector::const_iterator sit = it->second.begin()+1; sit != it->second.end(); ++sit) {
                            if ((sit->first == firstNode) && (sit->second == firstRaid)) break; // stop if we are back at a previous location
                            nodeName = Controller::theSchedulerSettings.getStorageNodeName(sit->first).c_str();
                            raidSet = Controller::theSchedulerSettings.getStorageRaidName(sit->first, sit->second).c_str();
                            if (!nodeName.isEmpty() && !raidSet.isEmpty()) {
                                dFile.uniqueMointPoints.push_back(nodeName + ":" + raidSet);
                            }
                        }
                    }
                    else {
                        dFile.uniqueMointPoints.push_back("??:??");
                    }
                }
                else {
                    dFile.uniqueMointPoints.push_back("??:??");
                }

                if (pObs) {
                    const Observation::RTCPsettings &RTCP(pObs->getRTCPsettings());

                    switch (dp) {
                    case DP_INCOHERENT_STOKES:
                    {
                        unsigned fileIdxCounter(0);
                        unsigned nrParts(0);
                        int sz(dFile.uniqueMointPoints.size());
                        unsigned nrComponents = RTCP.incoherentType == DATA_TYPE_STOKES_IQUV ? 4 : 1;
                        QString fileName_p1, fileName_p2, fileName_p3, fileName_p4;
                        for (std::map<unsigned, DigitalBeam>::const_iterator dbit = pObs->itsDigitalBeams.begin(); dbit != pObs->itsDigitalBeams.end(); ++dbit) {
                            nrParts = static_cast<unsigned>(ceil((double)dbit->second.nrSubbands() / RTCP.incoherentSubbandsPerFile));
                            // add the SAP identifier to the filename
                            fileName_p1 = observationIDStr + "_SAP"  + QString("%1").arg(dbit->first, 3, 10, QChar('0'));
                            const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(dbit->second.tiedArrayBeams());
                            for (std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.begin(); tit != tiedArrayBeams.end(); ++tit) {
                                // add the tied array beam identifier to the filename
                                fileName_p2 = "_B" + QString("%1").arg(tit->first, 3, 10, QChar('0'));
                                if (!tit->second.isCoherent()) {
                                    if (nrComponents == 1) { // only I is written
                                        // add the part identifier to the filename
                                        for (unsigned part = 0; part < nrParts; ++part) {
                                            fileName_p3 = "_S0_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                            dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3);
                                            dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                        }
                                    }
                                    else { // QUIV is written
                                        for (short unsigned which = 0; which < nrComponents; ++which) {
                                            // add the which identifier _Sx to the filename
                                            fileName_p3 = "_S" + QString("%1").arg(which, 1, 10, QChar('0'));
                                            // add the part identifier to the filename
                                            for (unsigned part = 0; part < nrParts; ++part) {
                                                fileName_p4 = "_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                                dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3 + fileName_p4);
                                                dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        itsOutputDataProducts[dp] = dFile;
                    }
                        break;

                    case DP_COHERENT_STOKES:
                    {
                        unsigned fileIdxCounter(0), nrParts(0);
                        int sz(dFile.uniqueMointPoints.size());
                        unsigned nrComponents = RTCP.coherentType == DATA_TYPE_STOKES_IQUV || RTCP.coherentType == DATA_TYPE_XXYY ? 4 : 1;
                        QString fileName_p1, fileName_p2, fileName_p3, fileName_p4;
                        for (std::map<unsigned, DigitalBeam>::const_iterator dbit = pObs->itsDigitalBeams.begin(); dbit != pObs->itsDigitalBeams.end(); ++dbit) {
                            unsigned TABNr(0);
                            nrParts = static_cast<unsigned>(ceil((double)dbit->second.nrSubbands() / RTCP.coherentSubbandsPerFile));
                            // add the SAP identifier to the filename
                            fileName_p1 = observationIDStr + "_SAP"  + QString("%1").arg(dbit->first, 3, 10, QChar('0'));

                            // generate files for MANUAL TABs
                            const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(dbit->second.tiedArrayBeams());
                            for (std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.begin(); tit != tiedArrayBeams.end(); ++tit) {
                                // add the tied array beam identifier to the filename
                                if (tit->second.isCoherent()) {
                                    fileName_p2 = "_B" + QString("%1").arg(tit->first, 3, 10, QChar('0'));
                                    if (nrComponents == 1) { // only I is written
                                        // add the part identifier to the filename
                                        for (unsigned part = 0; part < nrParts; ++part) {
                                            fileName_p3 = "_S0_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                            dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3);
                                            dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                        }
                                    }
                                    else { // IQUV is written
                                        for (short unsigned which = 0; which < nrComponents; ++which) {
                                            // add the which identifier _Sx to the filename
                                            fileName_p3 = "_S" + QString("%1").arg(which, 1, 10, QChar('0'));
                                            // add the part identifier to the filename
                                            for (unsigned part = 0; part < nrParts; ++part) {
                                                fileName_p4 = "_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                                dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3 + fileName_p4);
                                                dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                            }
                                        }
                                    }
                                }
                            }

                            TABNr = tiedArrayBeams.size();

                            // generate files for NR_TAB_RINGS
                            if (dbit->second.nrTabRings() > 0) {
                                unsigned nrRingTabs(dbit->second.nrRingTABs());
                                if (nrComponents == 1) {
                                    for (unsigned t = 0; t < nrRingTabs; ++t) {
                                        // add the part identifier to the filename
                                        fileName_p2 = "_B" + QString("%1").arg(TABNr++, 3, 10, QChar('0'));
                                        for (unsigned part = 0; part < nrParts; ++part) {
                                            fileName_p3 = "_S0_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                            dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3);
                                            dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                        }
                                    }
                                }
                                else {
                                    for (unsigned t = 0; t < nrRingTabs; ++t) {
                                        fileName_p2 = "_B" + QString("%1").arg(TABNr++, 3, 10, QChar('0'));
                                        for (short unsigned which = 0; which < nrComponents; ++which) {
                                            // add the which identifier _Sx to the filename
                                            fileName_p3 = "_S" + QString("%1").arg(which, 1, 10, QChar('0'));
                                            // add the part identifier to the filename
                                            for (unsigned part = 0; part < nrParts; ++part) {
                                                fileName_p4 = "_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                                dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3 + fileName_p4);
                                                dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                            }
                                        }
                                    }
                                }
                            }

                            // generate files for fly's eye
                            if (RTCP.flysEye) {
                                nrParts = static_cast<unsigned>(ceil((double)pObs->getNrOfSubbands() / RTCP.coherentSubbandsPerFile)); // here we use the total number of subbands of all SAP's together (is that ok?)
                                for (unsigned t = 0; t < pObs->getNrVirtualStations(); ++t) {
                                    fileName_p2 = "_B" + QString("%1").arg(TABNr++, 3, 10, QChar('0'));
                                    if (nrComponents == 1) {
                                        for (unsigned part = 0; part < nrParts; ++part) {
                                            fileName_p3 = "_S0_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                            dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3);
                                            dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                        }
                                    }
                                    else {
                                        for (short unsigned which = 0; which < nrComponents; ++which) {
                                            // add the which identifier _Sx to the filename
                                            fileName_p3 = "_S" + QString("%1").arg(which, 1, 10, QChar('0'));
                                            // add the part identifier to the filename
                                            for (unsigned part = 0; part < nrParts; ++part) {
                                                fileName_p4 = "_P" + QString("%1").arg(part, 3, 10, QChar('0')) + "_bf.h5";
                                                dFile.filenames.push_back(fileName_p1 + fileName_p2 + fileName_p3 + fileName_p4);
                                                dFile.locations.push_back(dFile.uniqueMointPoints.at(fileIdxCounter++ % sz) + locationStr); // round-robin storage locations
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        itsOutputDataProducts[dp] = dFile;
                    }
                        break;

                    case DP_CORRELATED_UV:
                    {
                        if (!dFile.uniqueMointPoints.empty()) {
                            int sz(dFile.uniqueMointPoints.size());
                            for (int i=0; i < nrFiles; ++i) {
                                dFile.locations.push_back(dFile.uniqueMointPoints.at(i % sz) + "/" + observationIDStr + "/"); // round-robin storage locations
                            }

                            // generate the array for correlated uv data
                            int curDigiBeam(0), beamSBBorder(0);
                            QString SAPstr;
                            std::map<unsigned, DigitalBeam>::const_iterator dbit;
                            const std::map<unsigned, DigitalBeam> &digitalBeams(pObs->getDigitalBeams());
                            for (int subIdx=0; subIdx < nrFiles; ++subIdx) { // one file per subband
                                if (subIdx == beamSBBorder) {
                                    SAPstr = QString("_SAP%1").arg(curDigiBeam,3,10,QChar('0'));
                                    dbit = digitalBeams.find(curDigiBeam++);
                                    if (dbit != digitalBeams.end()) {
                                        beamSBBorder += dbit->second.nrSubbands(); // next digital beam subband border
                                    }
                                }
                                dFile.filenames.push_back(observationIDStr + SAPstr + "_SB" + QString("%1").arg(subIdx,3,10,QChar('0'))	+ "_uv.MS");
                            }
                        }
                        itsOutputDataProducts[dp] = dFile;
                    }
                        break;

                    default:
                        break;
                    } // end switch(dp)
                }
                else if (pCalPipe) {
                    if (dp == DP_CORRELATED_UV) {
                        outputDataProduct &output(itsOutputDataProducts[DP_CORRELATED_UV]);
                        output.locations.clear();
                        output.filenames.clear();
                        std::map<dataProductTypes, inputDataProduct>::iterator idpit(itsInputDataProducts.find(DP_CORRELATED_UV));
                        if (idpit != itsInputDataProducts.end()) {
                            if (idpit->second.filenames.size() == nrFiles) {
                                if (dFile.uniqueMointPoints.empty()) {
                                    dFile.uniqueMointPoints.push_back("??:??");
                                }
                                int sz(dFile.uniqueMointPoints.size());
                                output.uniqueMointPoints = dFile.uniqueMointPoints;
                                for (int i=0; i < nrFiles; ++i) {
                                    output.filenames.push_back(QString(idpit->second.filenames.at(i)).remove(QRegExp("_SAP[0-9]{1,}")).replace(QRegExp(ObsIDPrefix + "[0-9]{1,}"), observationIDStr).replace("_uv.MS","_uv.dppp.MS"));
                                    output.locations.push_back(dFile.uniqueMointPoints.at(i % sz) + "/" + observationIDStr + "/"); // round-robin storage locations
                                }
                                output.skip = idpit->second.skip;
                            }
                        }
                    }
                    // for the output data product type instrumentModel we now also determine the files if this output type is enabled
                    // it generates one file for each input correlated data product file. (i.e. the number of files is the same as the number of input correlated files
                    else if (dp == DP_INSTRUMENT_MODEL) { // is intrumentModel output data product enabled?
                        outputDataProduct &output(itsOutputDataProducts[DP_INSTRUMENT_MODEL]);
                        output.locations.clear();
                        output.filenames.clear();
                        std::map<dataProductTypes, inputDataProduct>::iterator idpit(itsInputDataProducts.find(DP_CORRELATED_UV));
                        if (idpit != itsInputDataProducts.end()) {
                            if (idpit->second.filenames.size() == nrFiles) {
                                if (dFile.uniqueMointPoints.empty()) {
                                    dFile.uniqueMointPoints.push_back("??:??");
                                }
                                int sz(dFile.uniqueMointPoints.size());
                                output.uniqueMointPoints = dFile.uniqueMointPoints;
                                for (int i=0; i < nrFiles; ++i) {
                                    output.filenames.push_back(QString(idpit->second.filenames.at(i)).replace("_uv.MS","_inst.INST").replace(QRegExp(ObsIDPrefix + "[0-9]{1,}"), observationIDStr)); // replace _uv.MS extension of the correlated input files with _inst.INST extension to get the output instrument filenames
                                    output.locations.push_back(dFile.uniqueMointPoints.at(i % sz) + "/" + observationIDStr + "/"); // round-robin storage locations
                                }
                                output.skip = idpit->second.skip;
                            }
                        }
                    }
                }
                else if (pPulsarPipe && dp == DP_PULSAR) { // is pulsar output data product enabled?
                    outputDataProduct &output(itsOutputDataProducts[DP_PULSAR]);
                    output.locations.clear();
                    output.filenames.clear();
                    output.skip.clear();
                    std::vector<dataProductTypes> dpv;
                    dpv.push_back(DP_COHERENT_STOKES);
                    dpv.push_back(DP_INCOHERENT_STOKES);
                    std::map<dataProductTypes, TaskStorage::inputDataProduct>::iterator idpit;
                    for (std::vector<dataProductTypes>::const_iterator dpi = dpv.begin(); dpi != dpv.end(); ++dpi) {
                        if (isInputDataProduktEnabled(*dpi)) {
                            idpit = itsInputDataProducts.find(*dpi);
                            if (idpit != itsInputDataProducts.end()) {
                                if (idpit->second.filenames.size() == idpit->second.locations.size()) {
                                    if (dFile.uniqueMointPoints.empty()) {
                                        dFile.uniqueMointPoints.push_back("??:??");
                                    }
                                    output.uniqueMointPoints = dFile.uniqueMointPoints;

                                    QString fileName, location, outFile, regexp;
                                    // All file parts (_Pxxx) and polarizations (_Sy) are tarred into one output file for complex voltage (XXYY)
                                    // for other coherent data types each input file gets its own output tar.gz file (so keep _Sn identifier in filenames)
                                    // TODO: for Stokes IQUV with number of parts > 1 all files merge to one tarball so _P[0-9]+ should be removed
                                    if (*dpi == DP_COHERENT_STOKES) {
                                        regexp = (pPulsarPipe->coherentType() == DATA_TYPE_STOKES_IQUV) ? "_P[0-9]+" : "_S[0-9]_P[0-9]+";
                                    }
                                    else {
                                        regexp = (pPulsarPipe->incoherentType() == DATA_TYPE_STOKES_IQUV) ? "_P[0-9]+" : "_S[0-9]_P[0-9]+";
                                    }
                                    for (int i=0; i < idpit->second.filenames.size(); ++i) {
                                        fileName = idpit->second.filenames.at(i);
                                        location = idpit->second.locations.at(i);
                                        // replace _bf.h5 extension of the coherent stokes input files with _bf.tar.gz extension
                                        outFile = fileName.replace("_bf.h5","_bf.tar.gz").replace(QRegExp(regexp),"").replace(QRegExp(ObsIDPrefix + "[0-9]+"), observationIDStr);
                                        if (!output.filenames.contains(outFile)) {
                                            output.filenames.push_back(outFile);
                                            output.locations.push_back(location.replace(QRegExp(ObsIDPrefix + "[0-9]+"), observationIDStr));
                                        }
                                    }
                                    output.skip.assign(output.filenames.size(), false);
                                }
                            }
                        }
                    }
                }
                else if (pImagingPipe && dp == DP_SKY_IMAGE) {
                    outputDataProduct &output(itsOutputDataProducts[DP_SKY_IMAGE]);
                    output.locations.clear();
                    output.filenames.clear();
                    std::map<dataProductTypes, TaskStorage::inputDataProduct>::iterator idpit(itsInputDataProducts.find(DP_CORRELATED_UV));
                    if (idpit != itsInputDataProducts.end()) {
                        if (dFile.uniqueMointPoints.empty()) {
                            dFile.uniqueMointPoints.push_back("??:??");
                        }
                        int sz(dFile.uniqueMointPoints.size());
                        output.uniqueMointPoints = dFile.uniqueMointPoints;
                        unsigned nrInputSubbands(idpit->second.filenames.size());
                        // nr of skyImage files = nr_input_correlated files (= total subbands) / (slices_per_image * subbands_per_image)
                        // if the division above is not an integer value, that is an error, needs to be checked in Controller::doPrescheduleChecks
                        unsigned slicesPerImage(pImagingPipe->slicesPerImage()), subbandsPerImage(pImagingPipe->subbandsPerImage());
                        if ((slicesPerImage != 0) && (subbandsPerImage != 0)) {
                            if (fmod((float)(nrInputSubbands), (float)subbandsPerImage * slicesPerImage) == 0) {
                                unsigned nrImages(nrInputSubbands / (subbandsPerImage * slicesPerImage));
                                for (unsigned sbg = 0; sbg < nrImages; ++sbg) {
                                    output.filenames.push_back(observationIDStr + "_SBG" + QString("%1").arg(sbg,3,10,QChar('0')) + "_sky.h5");
                                    output.locations.push_back(dFile.uniqueMointPoints.at(sbg % sz) + "/" + observationIDStr + "/"); // round-robin storage locations
                                }
                                output.skip.assign(output.filenames.size(), false);
                            }
                        }
                    }
                }
                else if (pLongBasePipe && dp == DP_CORRELATED_UV) {
                    outputDataProduct &output(itsOutputDataProducts[DP_CORRELATED_UV]);
                    output.locations.clear();
                    output.filenames.clear();
                    std::map<dataProductTypes, TaskStorage::inputDataProduct>::iterator idpit(itsInputDataProducts.find(DP_CORRELATED_UV));
                    if (idpit != itsInputDataProducts.end()) {
                        if (dFile.uniqueMointPoints.empty()) {
                            dFile.uniqueMointPoints.push_back("??:??");
                        }
                        int sz(dFile.uniqueMointPoints.size());
                        output.uniqueMointPoints = dFile.uniqueMointPoints;
                        unsigned nrInputFiles(idpit->second.filenames.size());
                        // nr of output MS = nr of input files (= total subbands) / (* )
                        // if the division above is not an integer value, that is an error, needs to be checked in Controller::doPrescheduleChecks
                        unsigned subbandsPerSubbandGroup(pLongBasePipe->subbandsPerSubbandGroup()), subbandGroupsPerMS(pLongBasePipe->subbandGroupsPerMS());
                        if ((subbandsPerSubbandGroup != 0) && (subbandGroupsPerMS != 0)) {
                            if (fmod((float)(nrInputFiles), (float)subbandsPerSubbandGroup * subbandGroupsPerMS) == 0) {
                                unsigned nrOutputFiles(nrInputFiles / (subbandsPerSubbandGroup * subbandGroupsPerMS));
                                for (unsigned sbg = 0; sbg < nrOutputFiles; ++sbg) {
                                    output.filenames.push_back(observationIDStr + "_SBG" + QString("%1").arg(sbg,3,10,QChar('0')) + "_uv.MS");
                                    output.locations.push_back(dFile.uniqueMointPoints.at(sbg % sz) + "/" + observationIDStr + "/"); // round-robin storage locations
                                }
                                output.skip.assign(output.filenames.size(), false);
                            }
                        }
                    }
                }
            }
            else if (!itsOutputDataProducts.empty()) { // this output data product does not have output files defined (could be because of missing specification)
                oit = itsOutputDataProducts.find(dp);
                if (oit != itsOutputDataProducts.end()) {
                    if (oit->second.identifications.empty()) { // if an identification for this data product type exists then don't remove it
                        // if it doesn't exist, it is save to remove this output data product type altogether
                        itsOutputDataProducts.erase(dp);
                    }
                    else {
                        oit->second.filenames.clear();
                        oit->second.locations.clear();
                        oit->second.uniqueMointPoints.clear();
                    }
                }
            }
        }
        else { // this output data product type is not enabled (anymore)
            oit = itsOutputDataProducts.find(dp);
            if (oit != itsOutputDataProducts.end()) {
                if (oit->second.identifications.empty()) { // if an identification for this data product type exists then don't remove it
                    // if it doesn't exist, it is save to remove this output data product type altogether
                    itsOutputDataProducts.erase(dp);
                }
                else {
                    oit->second.filenames.clear();
                    oit->second.locations.clear();
                    oit->second.uniqueMointPoints.clear();
                }
            }
        }
    }

    return itsOutputDataProducts;
}

bool TaskStorage::diff(const TaskStorage *other, task_diff &dif) const {
    // storage settings
    itsEnabledOutputData != other->getOutputDataProductsEnabled() ? dif.output_data_types = true : dif.output_data_types = false;
    itsOutputDataProducts != other->getOutputDataProducts() ? dif.output_data_products = true : dif.output_data_products = false;
    if (itsStorageSelectionMode != other->getStorageSelectionMode()) dif.output_storage_settings = true;
    else dif.output_storage_settings = false;

    // input data products
    // w'll need a special compare for differences for input data products, where a file that has been disabled (in the enabled vector)
    // is not marked different from that file not being there in the input data product list of the other task

    dif.input_data_products = false;
    if (itsEnabledInputData != other->getInputDataProductsEnabled()) {
        dif.input_data_products = true;
    }
    else {
        std::map<dataProductTypes, inputDataProduct>::const_iterator odit;
        const std::map<dataProductTypes, inputDataProduct> &otherInput(other->getInputDataProducts());
        for (std::map<dataProductTypes, inputDataProduct>::const_iterator it = itsInputDataProducts.begin(); it != itsInputDataProducts.end(); ++it) {
            odit = otherInput.find(it->first);
            if (odit != otherInput.end()) {
                if (it->second.skip != odit->second.skip) dif.input_data_products = true;
                if (itsOwner->getStatus() >= Task::PRESCHEDULED) { // input data products locations don't need to be saved (diff done) for tasks that have status < PRESCHEDULED
                    if (it->second.locations != odit->second.locations) dif.input_data_products = true;
                }
                if (it->second.filenames != odit->second.filenames) dif.input_data_products = true;
                else if (it->second.identifications != odit->second.identifications) dif.input_data_products = true;
            }
            else dif.input_data_products = true;
        }
    }
    return (dif.input_data_products || dif.output_data_types || dif.output_storage_settings || dif.output_data_products);
}


QString TaskStorage::diffString(const task_diff &dif) const {
    QString difstr;
    if (dif.input_data_products) difstr += QString(SAS_item_names[TP_INPUT_DATA_PRODUCTS]) + ",";
    if (dif.output_data_types) difstr += QString(SAS_item_names[TP_OUTPUT_DATA_TYPES]) + ",";
    if (dif.output_storage_settings) difstr += QString(SAS_item_names[TP_OUTPUT_STORAGE_SETTINGS]) + ",";
    if (dif.output_data_products) difstr += QString(SAS_item_names[TP_OUTPUT_DATA_PRODUCTS]) + ",";

    if (difstr.endsWith(',')) {
        difstr = difstr.remove(difstr.length()-1,1);
    }
    return difstr;
}
