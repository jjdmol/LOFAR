/*
 * calibrationpipeline.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/calibrationpipeline.cpp $
 *
 */

#include "calibrationpipeline.h"

CalibrationPipeline::CalibrationPipeline()
    : Pipeline()
{
    itsPipelineType = PIPELINE_CALIBRATION;
    itsSASTree.setProcessSubtype(PST_CALIBRATION_PIPELINE);
}

CalibrationPipeline::CalibrationPipeline(unsigned task_id)
    : Pipeline(task_id)
{
    itsPipelineType = PIPELINE_CALIBRATION;
    itsSASTree.setProcessSubtype(PST_CALIBRATION_PIPELINE);
}

CalibrationPipeline::CalibrationPipeline(const QSqlQuery &query, const OTDBtree &SAS_tree)
: Pipeline(query, SAS_tree)
{
    itsPipelineType = PIPELINE_CALIBRATION;
}

CalibrationPipeline::CalibrationPipeline(unsigned id, const OTDBtree &SAS_tree)
: Pipeline(id, SAS_tree)
{
    itsPipelineType = PIPELINE_CALIBRATION;
}

QDataStream& operator<< (QDataStream &out, const CalibrationPipeline &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const Pipeline &>(task);

        out << task.itsDemixingSettings << task.itsSkyModel;
    }
    return out;
}

QDataStream& operator>> (QDataStream &in, CalibrationPipeline &task) {
    if (in.status() == QDataStream::Ok) {
        in >> static_cast<Pipeline &>(task);

        in >> task.itsDemixingSettings >> task.itsSkyModel;

        task.calculateDataFiles();
    }
    return in;
}

void CalibrationPipeline::calculateDataSize(void) {
    itsStorage->itsTotalDataSizekBytes = 0.0;
    itsStorage->itsTotalBandWidth = 0.0;
    if (itsStorage->itsEnabledOutputData.correlated) { // pipeline will write reduced correlated data
        dataFileMap::const_iterator dit(itsStorage->itsInputDataFiles.find(DP_CORRELATED_UV));
        if (dit != itsStorage->itsInputDataFiles.end()) {
            unsigned reductionFactor = itsDemixingSettings.itsFreqStep * itsDemixingSettings.itsTimeStep;
            const double &inputSize(dit->second.first);
            double outputFileSize(0.0);
            if (reductionFactor) {
                double data_size(inputSize / reductionFactor);
                outputFileSize = data_size + data_size/64 * (1 + reductionFactor) + data_size/2;
            }
            itsStorage->itsOutputDataFiles[DP_CORRELATED_UV] = std::pair<double, unsigned>(outputFileSize ,dit->second.second);
        }
    }
    if (itsStorage->itsEnabledOutputData.instrumentModel) {
        // number of files always equal to number of input (correlated) files (nr files equals nr of subbands)
        // i.e. for each input correlated file specified to the pipeline as an input data product the pipeline generated an output instrumentmodel file
        // instrument data files get determined when the task goes to the SCHEDULED state (see Controller::doScheduleChecks() )
        dataFileMap::const_iterator dit(itsStorage->itsInputDataFiles.find(DP_CORRELATED_UV)); // the number of instrumentModel files created equals the number of input correlated files
        if (dit != itsStorage->itsInputDataFiles.end()) {
            // TODO: add correct size calculation for instrument model
            itsStorage->itsOutputDataFiles[DP_INSTRUMENT_MODEL] = std::pair<double, unsigned>(1000.0, dit->second.second); // for now we set the size of one file to 1MB, which is about correct (small files)
        }
    }
    for (std::map<dataProductTypes, std::pair<double, unsigned> >::const_iterator it = itsStorage->itsOutputDataFiles.begin(); it != itsStorage->itsOutputDataFiles.end(); ++it) {
        itsStorage->itsTotalDataSizekBytes += it->second.first * it->second.second;
    }
    itsStorage->itsTotalBandWidth = (itsStorage->itsTotalDataSizekBytes * 8) / itsDuration.totalSeconds(); // kbit/sec
}


/*
 *  diff check for difference between this task and other task.
 *  return true if difference
 *  parameter dif
 */
bool CalibrationPipeline::diff(const Task *other, task_diff &dif) const {
    bool pipelineDif(Pipeline::diff(other, dif));

    const CalibrationPipeline *otherPipe = dynamic_cast<const CalibrationPipeline *>(other);
    if (otherPipe) {
        const DemixingSettings &otherDemix(otherPipe->demixingSettings());
        dif.demix_always = itsDemixingSettings.itsDemixAlways != otherDemix.demixAlways() ? true : false;
        dif.demix_if_needed = itsDemixingSettings.itsDemixIfNeeded != otherDemix.demixIfNeeded() ? true : false;
        dif.demix_skymodel = itsDemixingSettings.itsSkyModel != otherDemix.skyModel() ? true : false;
        dif.demix_freqstep = itsDemixingSettings.itsDemixFreqStep != otherDemix.demixFreqStep() ? true : false;
        dif.demix_timestep = itsDemixingSettings.itsDemixTimeStep != otherDemix.demixTimeStep() ? true : false;
        dif.freqstep = itsDemixingSettings.itsFreqStep != otherDemix.freqStep() ? true : false;
        dif.timestep = itsDemixingSettings.itsTimeStep != otherDemix.timeStep() ? true : false;
        dif.calibration_skymodel = itsSkyModel != otherPipe->itsSkyModel ? true : false;

        return (pipelineDif || dif.demix_always || dif.demix_if_needed || dif.demix_skymodel ||
                dif.demix_freqstep || dif.demix_timestep || dif.freqstep || dif.timestep || dif.calibration_skymodel);
    }
    else return pipelineDif;
}

QString CalibrationPipeline::diffString(const task_diff &dif) const {
    QString difstr(Pipeline::diffString(dif));
    if (!difstr.isEmpty()) difstr += ",";

    if (dif.demix_always) difstr += QString(SAS_item_names[TP_DEMIX_ALWAYS]) + ",";
    if (dif.demix_if_needed) difstr += QString(SAS_item_names[TP_DEMIX_IF_NEEDED]) + ",";
    if (dif.demix_skymodel) difstr += QString(SAS_item_names[TP_DEMIX_SKYMODEL]) + ",";
    if (dif.demix_freqstep) difstr += QString(SAS_item_names[TP_DEMIX_FREQSTEP]) + ",";
    if (dif.demix_timestep) difstr += QString(SAS_item_names[TP_DEMIX_TIMESTEP]) + ",";
    if (dif.freqstep) difstr += QString(SAS_item_names[TP_AVG_FREQSTEP]) + ",";
    if (dif.timestep) difstr += QString(SAS_item_names[TP_AVG_TIMESTEP]) + ",";
    if (dif.calibration_skymodel) difstr += QString(SAS_item_names[TP_CALIBRATION_SKYMODEL]) + ",";

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}
