/*
 * pipeline.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/pipeline.cpp $
 *
 */

#include "pipeline.h"

Pipeline::Pipeline()
    : Task(), itsStorage(0), itsPipelineType(PIPELINE_UNKNOWN)
{
    itsTaskType = Task::PIPELINE;
    itsStorage = new TaskStorage(this);
}

Pipeline::Pipeline(const Pipeline &other)
    : Task(other), itsStorage(0), itsPipelineType(other.pipelinetype())
{
    itsTaskType = Task::PIPELINE;
    itsStorage = new TaskStorage(this, other.storage());
}

Pipeline::Pipeline(unsigned task_id)
    : Task(task_id), itsStorage(0), itsPipelineType(PIPELINE_UNKNOWN)
{
    itsTaskType = Task::PIPELINE;
    itsStorage = new TaskStorage(this);
    itsStorage->setOwner(this);
}

Pipeline::Pipeline(const QSqlQuery &query, const OTDBtree &SAS_tree)
: Task(query, SAS_tree), itsPipelineType(PIPELINE_UNKNOWN)
{
    itsTaskType = Task::PIPELINE;
    itsStorage = new TaskStorage(this);
    itsStorage->itsStorageSelectionMode = static_cast<storage_selection_mode>(query.value(query.record().indexOf("storageSelectionMode")).toInt());
}

Pipeline::Pipeline(unsigned id, const OTDBtree &SAS_tree)
: Task(id, SAS_tree), itsPipelineType(PIPELINE_UNKNOWN)
{
    itsTaskType = Task::PIPELINE;
    itsStorage = new TaskStorage(this);
}

Pipeline::~Pipeline()
{
    delete itsStorage;
}


QDataStream& operator<< (QDataStream &out, const Pipeline &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const Task &>(task);

        // itsStorage
        out << *task.itsStorage;
        // itsPipelineType
        out << (quint8) task.itsPipelineType;
    }
    return out;
}

QDataStream& operator>> (QDataStream &in, Pipeline &task) {
    if (in.status() == QDataStream::Ok) {
        in >> static_cast<Task &>(task);

        // itsStorage
        delete task.itsStorage;
        task.itsStorage = new TaskStorage(&task);
        in >> *task.itsStorage;
        // itsPipelineType
        quint8 type;
        in >> type;
        task.itsPipelineType = (pipelineType) type;
    }
    return in;
}


// calculates the estimated total output data size in kBytes
void Pipeline::calculateDataFiles(void) {
    if (itsDuration.totalSeconds() > 0) {
        calculateDataSize();
    }
    else {
        itsStorage->itsOutputDataFiles.clear();
    }
}
/*
void Pipeline::calculateDataSize(void) {
    itsStorage->itsTotalDataSizekBytes = 0.0;
    itsStorage->itsTotalBandWidth = 0.0;
    if (itsStorage->itsEnabledOutputData.correlated) { // pipeline will write reduced correlated data
        dataFileMap::const_iterator dit(itsStorage->itsInputDataFiles.find(DP_CORRELATED_UV));
        if (dit != itsInputDataFiles.end()) {
            unsigned reductionFactor = itsDemixingSettings.freqStep * itsDemixingSettings.timeStep;
            const double &inputSize(dit->second.first);
            double outputFileSize(0.0);
            if (reductionFactor) {
                double data_size(inputSize / reductionFactor);
                outputFileSize = data_size + data_size/64 * (1 + reductionFactor) + data_size/2;
            }
            itsOutputDataFiles[DP_CORRELATED_UV] = std::pair<double, unsigned>(outputFileSize ,dit->second.second);
        }
    }
    if (itsEnabledOutputData.instrumentModel) {
        // number of files always equal to number of input (correlated) files (nr files equals nr of subbands)
        // i.e. for each input correlated file specified to the pipeline as an input data product the pipeline generated an output instrumentmodel file
        // instrument data files get determined when the task goes to the SCHEDULED state (see Controller::doScheduleChecks() )
        dataFileMap::const_iterator dit(itsInputDataFiles.find(DP_CORRELATED_UV)); // the number of instrumentModel files created equals the number of input correlated files
        if (dit != itsInputDataFiles.end()) {
            // TODO: add correct size calculation for instrument model
            itsOutputDataFiles[DP_INSTRUMENT_MODEL] = std::pair<double, unsigned>(1000.0, dit->second.second); // for now we set the size of one file to 1MB, which is about correct (small files)
        }
    }
    // TODO: pulsar integration: set the correct number of ouptut files and file size estimate for the pulsar pipeline
    if (itsEnabledOutputData.pulsar) {
        unsigned totalNrOutputFiles(0);
        // the number of pulsar outputfiles is equal to the sum of coherent and incoherent input files
        dataFileMap::const_iterator dit(itsInputDataFiles.find(DP_COHERENT_STOKES)); // the number of pulsar outputfiles created for now set equal to the number of input coherent
        if (dit != itsInputDataFiles.end()) {
            // itsRTCPsettings.coherentType for the pulsar pipeline is set to the predecessor it's coherent data type in Controller::setInputFilesForPipeline
            if (itsRTCPsettings.coherentType == DATA_TYPE_XXYY) { // for complex voltages the 'polarizations' XXYY are tarred into one output file
                totalNrOutputFiles += dit->second.second / 4;
            }
            else {
                totalNrOutputFiles += dit->second.second;
            }
        }
        dit = itsInputDataFiles.find(DP_INCOHERENT_STOKES);
        if (dit != itsInputDataFiles.end()) {
            totalNrOutputFiles += dit->second.second;
        }

        itsOutputDataFiles[DP_PULSAR] = std::pair<double, unsigned>(1000.0, totalNrOutputFiles);
    }
    if (itsEnabledOutputData.skyImage) {
        // nrImages = input_nrSubbands (= nr correlated ms input files) / ( nrSlicesPerImage * nrSubbandsPerImage )
        dataFileMap::const_iterator dit(itsInputDataFiles.find(DP_CORRELATED_UV));
        if (dit != itsInputDataFiles.end()) {
            if ((itsImagingSettings.slicesPerImage != 0) && (itsImagingSettings.subbandsPerImage != 0)) {
                unsigned nrInputSubbands(dit->second.second);
                if (fmod((float)(nrInputSubbands), (float)itsImagingSettings.subbandsPerImage * itsImagingSettings.slicesPerImage) == 0) {
                    unsigned nrImages(nrInputSubbands / (itsImagingSettings.subbandsPerImage * itsImagingSettings.slicesPerImage));
                    itsOutputDataFiles[DP_SKY_IMAGE] = std::pair<double, unsigned>(1000.0, nrImages); // TODO: add correct size calculation for SkyImage (See RedMine #3045)
                    clearConflict(CONFLICT_NON_INTEGER_OUTPUT_FILES);
                }
                else setConflict(CONFLICT_NON_INTEGER_OUTPUT_FILES);
            }
            else setConflict(CONFLICT_NON_INTEGER_OUTPUT_FILES);
        }
    }
    for (std::map<dataProductTypes, std::pair<double, unsigned> >::const_iterator it = itsOutputDataFiles.begin(); it != itsOutputDataFiles.end(); ++it) {
        itsTotalDataSizekBytes += it->second.first * it->second.second;
    }
    itsTotalBandWidth = (itsTotalDataSizekBytes * 8) / itsDuration.totalSeconds(); // kbit/sec
}
*/

bool Pipeline::diff(const Task *other, task_diff &dif) const {
    bool taskDif(Task::diff(other, dif));

    const Pipeline *otherPipe(dynamic_cast<const Pipeline *>(other));
    if (otherPipe) {
        bool storageDif(itsStorage->diff(otherPipe->storage(), dif));
        return (taskDif || storageDif);
    }
    else return taskDif;
}


// TODO: I have seen this exact function on three other classes
//      Copy paste code smell
QString Pipeline::diffString(const task_diff &dif) const {
    QString difstr(Task::diffString(dif));

    if (!difstr.isEmpty()) difstr += ",";

    difstr += itsStorage->diffString(dif);

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}

