/*
 * calibrationpipeline.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/calibrationpipeline.h $
 *
 */

#ifndef CALIBRATIONPIPELINE_H
#define CALIBRATIONPIPELINE_H

#include "pipeline.h"
#include "demixingsettings.h"
#include "OTDBtree.h"

class CalibrationPipeline : public Pipeline
{
public:
    CalibrationPipeline();
    CalibrationPipeline(unsigned);
    CalibrationPipeline(const QSqlQuery &query, const OTDBtree &SAS_tree);
    CalibrationPipeline(unsigned id, const OTDBtree &SAS_tree);

    friend QDataStream& operator<< (QDataStream &out, const CalibrationPipeline &task); // used for writing data to binary file
    friend QDataStream& operator>> (QDataStream &in, CalibrationPipeline &task); // used for reading data from binary file

    virtual void clone(const Task *other) {
        if (this != other) {
            const CalibrationPipeline *pOther = dynamic_cast<const CalibrationPipeline *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *pOther;
                taskID = myTaskID;
            }
        }
    }

    // getters
    DemixingSettings &demixingSettings(void) {return itsDemixingSettings;}
    const DemixingSettings &demixingSettings(void) const {return itsDemixingSettings;}
    const QString &skyModel(void) const {return itsSkyModel;}
    bool demixingEnabled(void) const {return itsDemixingSettings.demixingEnabled();}

    inline void setSkyModel(const QString &skymodel) {itsSkyModel = skymodel;}

    // setters
    void setDemixingSettings(const DemixingSettings &demix) {itsDemixingSettings = demix;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

    virtual void calculateDataSize(void);

private:
    DemixingSettings itsDemixingSettings;
    QString itsSkyModel;
};

#endif // CALIBRATIONPIPELINE_H
