/*
 * LongBaselinePipeline.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/longbaselinepipeline.cpp $
 *
 */

#include "longbaselinepipeline.h"

LongBaselinePipeline::LongBaselinePipeline()
    : Pipeline(), itsSubbandGroupsPerMS(1), itsSubbandsPerSubbandGroup(1)
{
    itsPipelineType = PIPELINE_LONGBASELINE;
    itsSASTree.setProcessSubtype(PST_LONG_BASELINE_PIPELINE);
}

LongBaselinePipeline::LongBaselinePipeline(unsigned task_id)
    : Pipeline(task_id), itsSubbandGroupsPerMS(1), itsSubbandsPerSubbandGroup(1)
{
    itsPipelineType = PIPELINE_LONGBASELINE;
    itsSASTree.setProcessSubtype(PST_LONG_BASELINE_PIPELINE);
}

LongBaselinePipeline::LongBaselinePipeline(const QSqlQuery &query, const OTDBtree &SAS_tree)
    : Pipeline(query, SAS_tree), itsSubbandGroupsPerMS(1), itsSubbandsPerSubbandGroup(1)
{
    itsPipelineType = PIPELINE_LONGBASELINE;
}

LongBaselinePipeline::LongBaselinePipeline(unsigned id, const OTDBtree &SAS_tree)
    : Pipeline(id, SAS_tree), itsSubbandGroupsPerMS(1), itsSubbandsPerSubbandGroup(1)
{
    itsPipelineType = PIPELINE_LONGBASELINE;
}

LongBaselinePipeline::LongBaselinePipeline(quint16 subbandGroupsPerMS, quint16 subbandsPerSubbandGroup)
    : Pipeline(), itsSubbandGroupsPerMS(subbandGroupsPerMS), itsSubbandsPerSubbandGroup(subbandsPerSubbandGroup)
{
    itsPipelineType = PIPELINE_LONGBASELINE;
    itsSASTree.setProcessSubtype(PST_LONG_BASELINE_PIPELINE);
}

QDataStream& operator>> (QDataStream &in, LongBaselinePipeline &task) {
    if (in.status() == QDataStream::Ok) {
        in >> static_cast<Pipeline &>(task);
        in >> task.itsSubbandGroupsPerMS >> task.itsSubbandsPerSubbandGroup;
        task.calculateDataFiles();
    }
    return in;
}

QDataStream& operator<< (QDataStream &out, const LongBaselinePipeline &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const Pipeline &>(task);
        out << task.itsSubbandGroupsPerMS << task.itsSubbandsPerSubbandGroup;
    }
    return out;
}

void LongBaselinePipeline::calculateDataSize(void) {
    itsStorage->itsTotalDataSizekBytes = 0.0;
    itsStorage->itsTotalBandWidth = 0.0;

    if (itsStorage->itsEnabledOutputData.correlated) {
        // nrImages = (= nr correlated ms input files) / ( nrSubbandGroups * nrSubbandsPerMS )
        dataFileMap::const_iterator dit(itsStorage->itsInputDataFiles.find(DP_CORRELATED_UV));
        if (dit != itsStorage->itsInputDataFiles.end()) {
            if ((itsSubbandGroupsPerMS != 0) && (itsSubbandsPerSubbandGroup != 0)) {
                unsigned nrInputFiles(dit->second.second);
                if (fmod((float)(nrInputFiles), (float)itsSubbandsPerSubbandGroup * itsSubbandGroupsPerMS) == 0) {
                    unsigned nrOutputFiles(nrInputFiles / (itsSubbandsPerSubbandGroup * itsSubbandGroupsPerMS));
                    itsStorage->itsOutputDataFiles[DP_CORRELATED_UV] = std::pair<double, unsigned>(1000.0, nrOutputFiles); // TODO: add correct size calculation for the MS
                    clearConflict(CONFLICT_NON_INTEGER_OUTPUT_FILES);
                }
                else setConflict(CONFLICT_NON_INTEGER_OUTPUT_FILES);
            }
            else setConflict(CONFLICT_NON_INTEGER_OUTPUT_FILES);
        }
    }

    for (std::map<dataProductTypes, std::pair<double, unsigned> >::const_iterator it = itsStorage->itsOutputDataFiles.begin(); it != itsStorage->itsOutputDataFiles.end(); ++it) {
        itsStorage->itsTotalDataSizekBytes += it->second.first * it->second.second;
    }

    itsStorage->itsTotalBandWidth = (itsStorage->itsTotalDataSizekBytes * 8) / itsDuration.totalSeconds(); // kbit/sec
}

bool LongBaselinePipeline::diff(const Task *other, task_diff &dif) const {
    bool pipelineDif(Pipeline::diff(other, dif));

    const LongBaselinePipeline *otherPipe = dynamic_cast<const LongBaselinePipeline *>(other);
    if (otherPipe) {
        dif.LongBaseline_nr_sbgroup_per_MS = itsSubbandGroupsPerMS != otherPipe->subbandGroupsPerMS() ? true : false;
        dif.LongBaseline_nr_sb_per_sbgroup = itsSubbandsPerSubbandGroup != otherPipe->subbandsPerSubbandGroup() ? true : false;

        return (pipelineDif || dif.LongBaseline_nr_sbgroup_per_MS || dif.LongBaseline_nr_sb_per_sbgroup);
    }
    else return pipelineDif;
}

QString LongBaselinePipeline::diffString(const task_diff &dif) const {
    QString difstr(Pipeline::diffString(dif));
    if (!difstr.isEmpty()) difstr += ",";

    if (dif.LongBaseline_nr_sbgroup_per_MS) difstr += QString(SAS_item_names[TP_LONGBASELINE_SBGROUP_PER_MS]) + ",";
    if (dif.LongBaseline_nr_sb_per_sbgroup) difstr += QString(SAS_item_names[TP_LONGBASELINE_SB_PER_SBGROUP]) + ",";

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}
