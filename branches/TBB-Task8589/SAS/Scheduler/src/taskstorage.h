/*
 * taskstorage.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/taskstorage.h $
 * description    : data class TaskStorage contains storage settings and calculation methods to store storage settings and is used by Tasks that generate storage
 *
 */

#ifndef TASKSTORAGE_H
#define TASKSTORAGE_H

#include "storage_definitions.h"
#include "Storage.h"

class Task;
class task_diff;

extern const char * DATA_PRODUCTS[NR_DATA_PRODUCT_TYPES];

/*
enum storage_clustering_mode {
    STORAGE_PROJECT_PREFERRED,
    STORAGE_DATATYPE_PREFERRED,
    END_STORAGE_CLUSTERING_MODES
};
#define NR_STORAGE_CLUSTERING_MODES END_STORAGE_CLUSTERING_MODES
*/

enum storage_selection_mode {
    STORAGE_MODE_MAXIMUM_DATA_TYPE_PREFERRED,
    STORAGE_MODE_MAXIMUM_PROJECT_PREFERRED,
    STORAGE_MODE_MINIMUM_DATA_TYPE_PREFERRED,
    STORAGE_MODE_MINIMUM_PROJECT_PREFERRED,
    STORAGE_MODE_MANUAL,
    END_STORAGE_SELECTION_MODES
};
#define NR_STORAGE_SELECTION_MODES END_STORAGE_SELECTION_MODES

//extern const char *storage_cluster_mode_str[NR_STORAGE_CLUSTERING_MODES];
extern const char *storage_select_mode_str[NR_STORAGE_SELECTION_MODES];

// dataFileMap: for each data product contains the size per file (kB) and the number of files to be written
typedef std::map<dataProductTypes, std::pair<double, unsigned> > dataFileMap;

class TaskStorage
{
    // TODO: Friend class codesmell ( why does everybody need to look under my skirt?)
    friend class Observation;
    friend class Pipeline;
    friend class PulsarPipeline;
    friend class ImagingPipeline;
    friend class LongBaselinePipeline;
    friend class CalibrationPipeline;

    friend QDataStream& operator<< (QDataStream &out, const TaskStorage &task); // used for writing data to binary file
    friend QDataStream& operator>> (QDataStream &in, TaskStorage &task); // used for reading data from binary file

public:

    class enableDataProdukts {
    public:
        enableDataProdukts() : correlated(false), coherentStokes(false), incoherentStokes(false),
        instrumentModel(false), pulsar(false), skyImage(false), correlatedAssigned(false), coherentStokesAssigned(false), incoherentStokesAssigned(false),
        complexVoltagesAssigned(false), instrumentModelAssigned(false), pulsarAssigned(false), skyImageAssigned(false)
        { }
        bool operator!=(const enableDataProdukts &other) const {
            return ((correlated != other.correlated) ||
                    (coherentStokes != other.coherentStokes) ||
                    (incoherentStokes != other.incoherentStokes) ||
                    (instrumentModel != other.instrumentModel) ||
                    (pulsar != other.pulsar) ||
                    (skyImage != other.skyImage)
                    );
        }

        bool correlated, coherentStokes, incoherentStokes, instrumentModel, pulsar, skyImage;
        bool correlatedAssigned, coherentStokesAssigned, incoherentStokesAssigned, complexVoltagesAssigned, instrumentModelAssigned, pulsarAssigned, skyImageAssigned;
    };

    class inputDataProduct {
    public:
        QStringList identifications;
        QStringList locations;
        QStringList filenames;
        std::vector<bool> skip;
    };

    class outputDataProduct {
    public:
        QStringList identifications;
        QStringList locations;
        QStringList filenames;
        QStringList uniqueMointPoints;
        std::vector<bool> skip;
    };

    TaskStorage(const Task *owner, const TaskStorage *other = 0);

    // getters
    const Task *owner(void) const {return itsOwner;}
    unsigned getNrFiles(void) const;
    std::map<dataProductTypes, int> getMinimumNrOfStorageNodes(void) const;
    const storageMap &getStorageLocations(void) const {return itsStorage;}
    storageVector getStorageLocations(dataProductTypes dpType) const;
    storage_selection_mode getStorageSelectionMode(void) const {return itsStorageSelectionMode;}
    bool checkStorageAssigned(void) const;
    bool hasStorageLocations(void) const;
    double getTotalStoragekBytes(void) const {return itsTotalDataSizekBytes;}
    double getTotalBandWidth(void) const {return itsTotalBandWidth;}
    const dataFileMap &getOutputFileSizes(void) const {return itsOutputDataFiles;}
    std::pair<double, unsigned> getOutputFileSizes(dataProductTypes dp) const;
    const std::vector<storageResult> &getStorageCheckResult(void) const {return itsStorageCheckResult;}
    const std::map<dataProductTypes, outputDataProduct> &generateFileList(void);
    const std::map<dataProductTypes, outputDataProduct> &getOutputDataProducts(void) const {return itsOutputDataProducts;}
    const enableDataProdukts &getOutputDataProductsEnabled(void) const {return itsEnabledOutputData;}
    std::map<dataProductTypes, outputDataProduct> &getOutputDataProductsForChange(void) {return itsOutputDataProducts;}
    const std::map<dataProductTypes, inputDataProduct> &getInputDataProducts(void) const {return itsInputDataProducts;}
    std::map<dataProductTypes, inputDataProduct> &getInputDataProductsForChange(void) {return itsInputDataProducts;}
    const enableDataProdukts &getInputDataProductsEnabled(void) const {return itsEnabledInputData;}
    const storageMap &getInputStorageLocations(void) const {return itsInputStorageLocations;}
    bool isInputDataProduktEnabled(dataProductTypes dpType) const;
    bool isOutputDataProduktEnabled(dataProductTypes dpType) const;
    bool isOutputDataProduktAssigned(dataProductTypes dpType) const;

    // add all data products of a certain type (clears previous data products of that type if they where already added)
    void addStorage(dataProductTypes dataProduct, const storageVector &storage);
    void setStorage(const storageMap &storage) {itsStorage = storage;}
    bool setStorage(dataProductTypes dataProduct, const QStringList &nodeList, const QStringList &raidList);
    void unAssignStorage(void);
    void setInputFilesForDataProduct(dataProductTypes dpType, const QStringList &filenames, const QStringList &locations, const std::vector<bool> &skipVector) {
        itsInputDataProducts[dpType].filenames = filenames;
        itsInputDataProducts[dpType].locations = locations;
        itsInputDataProducts[dpType].skip = skipVector;
    }
    void setOutputFilesForDataProduct(dataProductTypes dpType, const QStringList &filenames, const QStringList &locations, const std::vector<bool> &skipVector) {
        itsOutputDataProducts[dpType].filenames = filenames;
        itsOutputDataProducts[dpType].locations = locations;
        itsOutputDataProducts[dpType].skip = skipVector;
    }

    // setters
    void setOwner(const Task *owner) {itsOwner = owner;}
    void addInputStorageLocations(dataProductTypes dpType, const storageVector &storage) {itsInputStorageLocations[dpType] = storage;}
    // add an identifications list for a specific input data product type (creates a dependency on that dataproduct)
    void addInputDataProductID(dataProductTypes dpType, const QStringList &identification_list) {itsInputDataProducts[dpType].identifications = identification_list;}
    void setInputDataProductEnabled(dataProductTypes dpType, bool enabled);
    void setInputFilesToBeProcessed(const std::map<dataProductTypes, std::vector<bool> > &files);
    void setOutputDataProductEnabled(dataProductTypes dp_type, bool enabled);
    void setEnabledOutputDataProducts(const enableDataProdukts &outputDataTypes) {itsEnabledOutputData = outputDataTypes;}
    void setOutputDataProductAssigned(dataProductTypes dp_type, bool assigned);
    void addOutputDataProductID(dataProductTypes dpType, const QStringList &identification_list) {itsOutputDataProducts[dpType].identifications = identification_list;}
    void setInputFileSizes(dataProductTypes dpType, const std::pair<double, unsigned> &inputFileSizes);
    void setOutputFileNames(dataProductTypes dpType, const QStringList &filenames) {itsOutputDataProducts[dpType].filenames = filenames;}
    void setOutputFileLocations(dataProductTypes dpType, const QStringList &file_locations) {itsOutputDataProducts[dpType].locations = file_locations;}
    void setOutputFileMountpoints(dataProductTypes dpType, const QStringList &mountPoints) {itsOutputDataProducts[dpType].uniqueMointPoints = mountPoints;}
    void clearOutputFileMountpoints(dataProductTypes dpType) {itsOutputDataProducts[dpType].uniqueMointPoints.clear();}

    void setStorageCheckResult(const std::vector<storageResult> &result) {itsStorageCheckResult = result;}
    void clearStorageCheckResults(void) {itsStorageCheckResult.clear();}
    void setStorageSelectionMode(storage_selection_mode mode) {itsStorageSelectionMode = mode;}
    void setRecalcStorageNeeded(void) {itsRecalcStorageNeeded = true;}

    bool diff(const TaskStorage *other, task_diff &dif) const;
    QString diffString(const task_diff &dif) const;

//WK code commented out
//    bool getEqualityInputOutputProducts()const;

private:
    const Task *itsOwner;
    bool itsRecalcStorageNeeded;
    storage_selection_mode itsStorageSelectionMode;
    dataFileMap itsOutputDataFiles, itsInputDataFiles; // for each data product contains the size per file and the number of files to be written
    enableDataProdukts itsEnabledInputData, itsEnabledOutputData;
    std::map<dataProductTypes, inputDataProduct> itsInputDataProducts;
    std::map<dataProductTypes, outputDataProduct> itsOutputDataProducts;
    double itsTotalDataSizekBytes;
    double itsTotalBandWidth; // Gbit/sec
    storageMap itsStorage, itsInputStorageLocations; // itsStorage contains the 'asked' storage node IDs and raid IDs
    std::vector<storageResult> itsStorageCheckResult;
};

inline bool operator==(const TaskStorage::inputDataProduct &lhs, const TaskStorage::inputDataProduct &rhs) {
    return ((lhs.identifications == rhs.identifications) &&
            (lhs.locations == rhs.locations) &&
            (lhs.filenames == rhs.filenames));
}

inline bool operator==(const TaskStorage::outputDataProduct &lhs, const TaskStorage::outputDataProduct &rhs) {
    return (
            (lhs.identifications == rhs.identifications) &&
            (lhs.locations == rhs.locations) &&
            (lhs.filenames == rhs.filenames) &&
            (lhs.uniqueMointPoints == rhs.uniqueMointPoints));
}

#endif // TASKSTORAGE_H
