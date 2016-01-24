/*
 * pipeline.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/pipeline.h $
 *
 */

#ifndef PIPELINE_H
#define PIPELINE_H

#include "storage_definitions.h"
#include "taskstorage.h"
#include "task.h"
#include "demixingsettings.h"
#include "OTDBtree.h"

class Pipeline : public Task
{
public:
    friend class TaskStorage;

    Pipeline();
    Pipeline(const Pipeline &);
    Pipeline(unsigned task_id);
    Pipeline(const QSqlQuery &query, const OTDBtree &SAS_tree);
    Pipeline(unsigned id, const OTDBtree &SAS_tree);

    ~Pipeline();

    friend QDataStream& operator<< (QDataStream &out, const Pipeline &task); // used for writing data to binary file
    friend QDataStream& operator>> (QDataStream &in, Pipeline &task); // used for reading data from binary file

    Pipeline & operator=(const Pipeline &other) {
        if (this != &other) {
            Task::operator=(other);
            itsPipelineType = other.itsPipelineType;
            delete itsStorage;
            itsStorage = new TaskStorage(this, other.storage());
        }
        return *this;
    }

    virtual void clone(const Task *other) {
        if (this != other) {
            const Pipeline *pOther = dynamic_cast<const Pipeline *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *pOther;
                taskID = myTaskID;
            }
        }
    }

    pipelineType pipelinetype(void) const {return itsPipelineType;}
    bool isCalibrationPipeline(void) const {return itsPipelineType == PIPELINE_CALIBRATION;}
    bool isImagingPipeline(void) const {return itsPipelineType == PIPELINE_IMAGING;}
    bool isPulsarPipeline(void) const {return itsPipelineType == PIPELINE_PULSAR;}
    bool isLongBaselinePipeline(void) const {return itsPipelineType == PIPELINE_LONGBASELINE;}

    // calculate the output data sizes for pipeline
    const std::map<dataProductTypes, TaskStorage::outputDataProduct> &generateFileList(void) {return itsStorage->generateFileList();}

    virtual const std::vector<storageResult> &getStorageCheckResult(void) const { return itsStorage->getStorageCheckResult(); }
    inline bool hasStorage(void) const {return true;}
    TaskStorage *storage(void) {return itsStorage;}
    const TaskStorage *storage(void) const {return itsStorage;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

    void calculateDataFiles(void);

private:
    virtual void calculateDataSize(void) {}


protected:
    TaskStorage *itsStorage;
    pipelineType itsPipelineType;
};

#endif // PIPELINE_H
