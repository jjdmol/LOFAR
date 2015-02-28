/*
 * pulsarpipeline.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/pulsarpipeline.cpp $
 *
 */

#include "pulsarpipeline.h"

PulsarPipeline::PulsarPipeline()
    : Pipeline(), itsNoFold(false), itsNoPdmp(false), itsNoRFI(false), itsRawTo8bit(false), itsRrats(false),
    itsSinglePulse(false), itsSkipDspsr(false), itsSkipDynamicSpectrum(false), itsSkipPrepfold(false),
    itsDecodeNblocks(100), itsDecodeSigma(3), itsTsubint(-1), itsEightBitConvSigma(5.0), itsDynamicSpectrumAvg(0.5), itsRratsDmRange(5.0),
    itsCoherentType(DATA_TYPE_UNDEFINED), itsIncoherentType(DATA_TYPE_UNDEFINED)
{
    itsPipelineType = PIPELINE_PULSAR;
    itsSASTree.setProcessSubtype(PST_PULSAR_PIPELINE);
}

PulsarPipeline::PulsarPipeline(unsigned task_id)
    : Pipeline(task_id), itsNoFold(false), itsNoPdmp(false), itsNoRFI(false), itsRawTo8bit(false), itsRrats(false),
    itsSinglePulse(false), itsSkipDspsr(false), itsSkipDynamicSpectrum(false), itsSkipPrepfold(false),
    itsDecodeNblocks(100), itsDecodeSigma(3), itsTsubint(-1), itsEightBitConvSigma(5.0), itsDynamicSpectrumAvg(0.5), itsRratsDmRange(5.0),
    itsCoherentType(DATA_TYPE_UNDEFINED), itsIncoherentType(DATA_TYPE_UNDEFINED)
{
    itsPipelineType = PIPELINE_PULSAR;
    itsSASTree.setProcessSubtype(PST_PULSAR_PIPELINE);
}

PulsarPipeline::PulsarPipeline(const QSqlQuery &query, const OTDBtree &SAS_tree)
    : Pipeline(query, SAS_tree), itsNoFold(false), itsNoPdmp(false), itsNoRFI(false), itsRawTo8bit(false), itsRrats(false),
      itsSinglePulse(false), itsSkipDspsr(false), itsSkipDynamicSpectrum(false), itsSkipPrepfold(false),
      itsDecodeNblocks(100), itsDecodeSigma(3), itsTsubint(-1), itsEightBitConvSigma(5.0), itsDynamicSpectrumAvg(0.5), itsRratsDmRange(5.0),
      itsCoherentType(DATA_TYPE_UNDEFINED), itsIncoherentType(DATA_TYPE_UNDEFINED)
{
    itsPipelineType = PIPELINE_PULSAR;
}

PulsarPipeline::PulsarPipeline(unsigned id, const OTDBtree &SAS_tree)
    : Pipeline(id, SAS_tree), itsNoFold(false), itsNoPdmp(false), itsNoRFI(false), itsRawTo8bit(false), itsRrats(false),
      itsSinglePulse(false), itsSkipDspsr(false), itsSkipDynamicSpectrum(false), itsSkipPrepfold(false),
      itsDecodeNblocks(100), itsDecodeSigma(3), itsTsubint(-1), itsEightBitConvSigma(5.0), itsDynamicSpectrumAvg(0.5), itsRratsDmRange(5.0),
      itsCoherentType(DATA_TYPE_UNDEFINED), itsIncoherentType(DATA_TYPE_UNDEFINED)
{
    itsPipelineType = PIPELINE_PULSAR;
}

QDataStream& operator<< (QDataStream &out, const PulsarPipeline &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const Pipeline &>(task);

        out << task.itsNoFold << task.itsNoPdmp << task.itsNoRFI << task.itsRawTo8bit
            << task.itsRrats << task.itsSinglePulse << task.itsSkipDspsr << task.itsSkipDynamicSpectrum << task.itsSkipPrepfold
            << task.itsTwoBf2fitsExtra << task.itsDigifilExtra << task.itsDspsrExtra << task.itsPrepDataExtra << task.itsPrepFoldExtra
            << task.itsPrepSubbandExtra << task.itsPulsar << task.itsRfiFindExtra
            << task.itsDecodeNblocks << task.itsDecodeSigma << task.itsTsubint
            << task.itsEightBitConvSigma << task.itsDynamicSpectrumAvg << task.itsRratsDmRange
            << (quint8) task.itsCoherentType << (quint8) task.itsIncoherentType;
    }
    return out;
}

QDataStream& operator>> (QDataStream &in, PulsarPipeline &task) {
    if (in.status() == QDataStream::Ok) {
        in >> static_cast<Pipeline &>(task);

        in >> task.itsNoFold >> task.itsNoPdmp >> task.itsNoRFI >> task.itsRawTo8bit
           >> task.itsRrats >> task.itsSinglePulse >> task.itsSkipDspsr >> task.itsSkipDynamicSpectrum >> task.itsSkipPrepfold
           >> task.itsTwoBf2fitsExtra >> task.itsDigifilExtra >> task.itsDspsrExtra >> task.itsPrepDataExtra >> task.itsPrepFoldExtra
           >> task.itsPrepSubbandExtra >> task.itsPulsar >> task.itsRfiFindExtra
           >> task.itsDecodeNblocks >> task.itsDecodeSigma >> task.itsTsubint
           >> task.itsEightBitConvSigma >> task.itsDynamicSpectrumAvg >> task.itsRratsDmRange;

        quint8 type;
        in >> type;
        task.itsCoherentType = (dataTypes) type;
        in >> type;
        task.itsIncoherentType = (dataTypes) type;

        task.calculateDataFiles();
    }
    return in;
}


void PulsarPipeline::calculateDataSize(void) {
    itsStorage->itsTotalDataSizekBytes = 0.0;
    itsStorage->itsTotalBandWidth = 0.0;

    // TODO: pulsar integration: set the correct number of ouptut files and file size estimate for the pulsar pipeline
    if (itsStorage->itsEnabledOutputData.pulsar) {
        unsigned totalNrOutputFiles(0);
        // the number of pulsar outputfiles is equal to the sum of coherent and incoherent input files
        dataFileMap::const_iterator dit(itsStorage->itsInputDataFiles.find(DP_COHERENT_STOKES)); // the number of pulsar outputfiles created for now set equal to the number of input coherent
        if (dit != itsStorage->itsInputDataFiles.end()) {
            // itsRTCPsettings.coherentType for the pulsar pipeline is set to the predecessor it's coherent data type in Controller::setInputFilesForPipeline
            if (itsCoherentType == DATA_TYPE_XXYY) { // for complex voltages the 'polarizations' XXYY are tarred into one output file
                totalNrOutputFiles += dit->second.second / 4;
            }
            else {
                totalNrOutputFiles += dit->second.second;
            }
        }
        dit = itsStorage->itsInputDataFiles.find(DP_INCOHERENT_STOKES);
        if (dit != itsStorage->itsInputDataFiles.end()) {
            totalNrOutputFiles += dit->second.second;
        }

        itsStorage->itsOutputDataFiles[DP_PULSAR] = std::pair<double, unsigned>(1000.0, totalNrOutputFiles);
    }

    for (std::map<dataProductTypes, std::pair<double, unsigned> >::const_iterator it = itsStorage->itsOutputDataFiles.begin(); it != itsStorage->itsOutputDataFiles.end(); ++it) {
        itsStorage->itsTotalDataSizekBytes += it->second.first * it->second.second;
    }

    itsStorage->itsTotalBandWidth = (itsStorage->itsTotalDataSizekBytes * 8) / itsDuration.totalSeconds(); // kbit/sec
}

bool PulsarPipeline::diff(const Task *other, task_diff &dif) const {
    bool pipelineDif(Pipeline::diff(other, dif));

    const PulsarPipeline *otherPipe = dynamic_cast<const PulsarPipeline *>(other);
    if (otherPipe) {
        // TODO: Why not assign/test the tertairy test case emediately?
        // copy paste codesmell
        dif.pulsar_pipeline_settings |= itsNoRFI != otherPipe->noRFI() ? true : false;  // == dif.pulsar_pipeline_settings |= (itsNoRFI != otherPipe->noRFI())  // The () is not even needed
        dif.pulsar_pipeline_settings |= itsSkipDspsr != otherPipe->skipDspsr() ? true : false;
        dif.pulsar_pipeline_settings |= itsNoPdmp != otherPipe->noPdmp() ? true : false;
        dif.pulsar_pipeline_settings |= itsRawTo8bit != otherPipe->rawTo8Bit() ? true : false;
        dif.pulsar_pipeline_settings |= itsRrats != otherPipe->rrats() ? true : false;
        dif.pulsar_pipeline_settings |= itsSinglePulse != otherPipe->singlePulse() ? true : false;
        dif.pulsar_pipeline_settings |= itsSkipDynamicSpectrum != otherPipe->skipDynamicSpectrum() ? true : false;
        dif.pulsar_pipeline_settings |= itsSkipPrepfold != otherPipe->skipPrepfold() ? true : false;
        dif.pulsar_pipeline_settings |= itsTwoBf2fitsExtra != otherPipe->twoBf2fitsExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsDigifilExtra != otherPipe->digifilExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsDspsrExtra != otherPipe->dspsrExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsPrepDataExtra != otherPipe->prepDataExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsPrepFoldExtra != otherPipe->prepFoldExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsPrepSubbandExtra != otherPipe->prepSubbandExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsPulsar != otherPipe->pulsarName() ? true : false;
        dif.pulsar_pipeline_settings |= itsRfiFindExtra != otherPipe->rfiFindExtra() ? true : false;
        dif.pulsar_pipeline_settings |= itsDecodeNblocks != otherPipe->decodeNblocks() ? true : false;
        dif.pulsar_pipeline_settings |= itsDecodeSigma != otherPipe->decodeSigma() ? true : false;
        dif.pulsar_pipeline_settings |= itsTsubint != otherPipe->tsubInt() ? true : false;
        dif.pulsar_pipeline_settings |= itsEightBitConvSigma != otherPipe->eightBitConversionSigma() ? true : false;
        dif.pulsar_pipeline_settings |= itsDynamicSpectrumAvg != otherPipe->dynamicSpectrumAvg() ? true : false;
        dif.pulsar_pipeline_settings |= itsRratsDmRange != otherPipe->rratsDmRange() ? true : false;

        return (pipelineDif || dif.pulsar_pipeline_settings);
    }
    else return pipelineDif;
}

QString PulsarPipeline::diffString(const task_diff &dif) const {
    QString difstr(Pipeline::diffString(dif));
    if (!difstr.isEmpty()) difstr += ",";

    if (dif.pulsar_pipeline_settings) difstr += QString(SAS_item_names[TP_PULSAR_PIPELINE_SETTINGS]);

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}
