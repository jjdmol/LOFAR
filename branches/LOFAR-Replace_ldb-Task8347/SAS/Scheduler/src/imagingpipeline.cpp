/*
 * imagingpipeline.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/imagingpipeline.cpp $
 *
 */

#include "imagingpipeline.h"

ImagingPipeline::ImagingPipeline()
    : Pipeline(), itsSpecifyFOV(true), itsSlicesPerImage(1), itsSubbandsPerImage(1), itsNrOfPixels(1024), itsFov(0)
{
    itsPipelineType = PIPELINE_IMAGING;
    itsSASTree.setProcessSubtype(PST_IMAGING_PIPELINE);
}

ImagingPipeline::ImagingPipeline(unsigned task_id)
    : Pipeline(task_id), itsSpecifyFOV(true), itsSlicesPerImage(1), itsSubbandsPerImage(1), itsNrOfPixels(1024), itsFov(0)
{
    itsPipelineType = PIPELINE_IMAGING;
    itsSASTree.setProcessSubtype(PST_IMAGING_PIPELINE);
}

ImagingPipeline::ImagingPipeline(const QSqlQuery &query, const OTDBtree &SAS_tree)
    : Pipeline(query, SAS_tree), itsSpecifyFOV(true), itsSlicesPerImage(1), itsSubbandsPerImage(1), itsNrOfPixels(1024), itsFov(0)
{
    itsPipelineType = PIPELINE_IMAGING;
}

ImagingPipeline::ImagingPipeline(unsigned id, const OTDBtree &SAS_tree)
    : Pipeline(id, SAS_tree), itsSpecifyFOV(true), itsSlicesPerImage(1), itsSubbandsPerImage(1), itsNrOfPixels(1024), itsFov(0)
{
    itsPipelineType = PIPELINE_IMAGING;
}

ImagingPipeline::ImagingPipeline(bool specifyFOV, quint16 slicesPerImage, quint16 subbandsPerImage, quint16 nrOfPixels,
                const double &fov, const QString &cellSize)
    : Pipeline(), itsSpecifyFOV(specifyFOV), itsSlicesPerImage(slicesPerImage), itsSubbandsPerImage(subbandsPerImage),
    itsNrOfPixels(nrOfPixels), itsFov(fov), itsCellSize(cellSize)
{
    itsPipelineType = PIPELINE_IMAGING;
    itsSASTree.setProcessSubtype(PST_IMAGING_PIPELINE);
}

QDataStream& operator>> (QDataStream &in, ImagingPipeline &task) {
    if (in.status() == QDataStream::Ok) {
        in >> static_cast<Pipeline &>(task);
        in >> task.itsSpecifyFOV >> task.itsSlicesPerImage >> task.itsSubbandsPerImage
           >> task.itsNrOfPixels >> task.itsCellSize >> task.itsFov;
        task.calculateDataFiles();
    }
    return in;
}

QDataStream& operator<< (QDataStream &out, const ImagingPipeline &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const Pipeline &>(task);
        out << task.itsSpecifyFOV << task.itsSlicesPerImage << task.itsSubbandsPerImage
            << task.itsNrOfPixels << task.itsCellSize << task.itsFov;
    }
    return out;
}

void ImagingPipeline::calculateDataSize(void) {
    itsStorage->itsTotalDataSizekBytes = 0.0;
    itsStorage->itsTotalBandWidth = 0.0;

    if (itsStorage->itsEnabledOutputData.skyImage) {
        // nrImages = input_nrSubbands (= nr correlated ms input files) / ( nrSlicesPerImage * nrSubbandsPerImage )
        dataFileMap::const_iterator dit(itsStorage->itsInputDataFiles.find(DP_CORRELATED_UV));
        if (dit != itsStorage->itsInputDataFiles.end()) {
            if ((itsSlicesPerImage != 0) && (itsSubbandsPerImage != 0)) {
                unsigned nrInputSubbands(dit->second.second);
                if (fmod((float)(nrInputSubbands), (float)itsSubbandsPerImage * itsSlicesPerImage) == 0) {
                    unsigned nrImages(nrInputSubbands / (itsSubbandsPerImage * itsSlicesPerImage));
                    itsStorage->itsOutputDataFiles[DP_SKY_IMAGE] = std::pair<double, unsigned>(1000.0, nrImages); // TODO: add correct size calculation for SkyImage (See RedMine #3045)
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

bool ImagingPipeline::diff(const Task *other, task_diff &dif) const {
    bool pipelineDif(Pipeline::diff(other, dif));

    const ImagingPipeline *otherPipe = dynamic_cast<const ImagingPipeline *>(other);
    if (otherPipe) {
        dif.Imaging_nr_slices_per_image = itsSlicesPerImage != otherPipe->slicesPerImage() ? true : false;
        dif.Imaging_nr_subbands_per_image = itsSubbandsPerImage != otherPipe->subbandsPerImage() ? true : false;
        dif.Imaging_specify_fov = itsSpecifyFOV != otherPipe->specifyFov() ? true : false;
        dif.Imaging_fov = itsFov != otherPipe->fov() ? true : false;
        dif.Imaging_cellsize = itsCellSize != otherPipe->cellSize() ? true : false;
        dif.Imaging_npix = itsNrOfPixels != otherPipe->nrOfPixels() ? true : false;

        return (pipelineDif || dif.Imaging_nr_slices_per_image || dif.Imaging_nr_subbands_per_image
                || dif.Imaging_specify_fov || dif.Imaging_fov || dif.Imaging_npix || dif.Imaging_cellsize);
    }
    else return pipelineDif;
}

QString ImagingPipeline::diffString(const task_diff &dif) const {
    QString difstr(Pipeline::diffString(dif));
    if (!difstr.isEmpty()) difstr += ",";

    if (dif.Imaging_nr_slices_per_image) difstr += QString(SAS_item_names[TP_IMAGING_SLICES_PER_IMAGE]) + ",";
    if (dif.Imaging_nr_subbands_per_image) difstr += QString(SAS_item_names[TP_IMAGING_SUBBANDS_PER_IMAGE]) + ",";
    if (dif.Imaging_specify_fov) difstr += QString(SAS_item_names[TP_IMAGING_SPECIFY_FOV]) + ",";
    if (dif.Imaging_fov) difstr += QString(SAS_item_names[TP_IMAGING_FOV]) + ",";
    if (dif.Imaging_cellsize) difstr += QString(SAS_item_names[TP_IMAGING_CELLSIZE]) + ",";
    if (dif.Imaging_npix) difstr += QString(SAS_item_names[TP_IMAGING_NPIX]) + ",";

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}
