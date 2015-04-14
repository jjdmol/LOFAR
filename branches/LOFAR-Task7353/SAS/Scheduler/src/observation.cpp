/*
 * observation.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/observation.cpp $
 *
 */

#include "observation.h"
#include "Controller.h"
#include "taskstorage.h"

Observation::Observation()
    : StationTask(Task::OBSERVATION), itsReservation(0), itsNightTimeCurve(4), itsNrOfSubbands(0), itsTBBPiggybackAllowed(true), itsAartfaacPiggybackAllowed(true), itsStorage(0)
{
    itsStorage = new TaskStorage(this);
    itsAnalogBeam.directionType = DIR_TYPE_J2000;
}

Observation::Observation(unsigned id)
    : StationTask(id, Task::OBSERVATION), itsReservation(0), itsNightTimeCurve(4), itsNrOfSubbands(0), itsTBBPiggybackAllowed(true), itsAartfaacPiggybackAllowed(true), itsStorage(0)
{
    itsStorage = new TaskStorage(this);
    itsAnalogBeam.directionType = DIR_TYPE_J2000;
}

Observation::Observation(const Observation &other)
    : StationTask(other), itsReservation(other.itsReservation), itsAnalogBeam(other.itsAnalogBeam),
      itsDigitalBeams(other.itsDigitalBeams), itsNightTimeCurve(other.itsNightTimeCurve),
      itsNrOfSubbands(other.itsNrOfSubbands), itsTBBPiggybackAllowed(other.itsTBBPiggybackAllowed), itsAartfaacPiggybackAllowed(other.itsAartfaacPiggybackAllowed),
      itsRTCPsettings(other.itsRTCPsettings), itsStorage(0)
{
    itsStorage = new TaskStorage(this, other.itsStorage);
}

Observation::Observation(unsigned id, const OTDBtree &SAS_tree)
    : StationTask(id, SAS_tree, Task::OBSERVATION), itsReservation(0), itsNightTimeCurve(4), itsNrOfSubbands(0), itsTBBPiggybackAllowed(true), itsAartfaacPiggybackAllowed(true), itsStorage(0)
{
    itsStorage = new TaskStorage(this);
    itsAnalogBeam.directionType = DIR_TYPE_J2000;
}

Observation::Observation(const QSqlQuery &query, const OTDBtree &SAS_tree)
    : StationTask(query, SAS_tree, Task::OBSERVATION), itsReservation(0), itsNightTimeCurve(4), itsNrOfSubbands(0), itsTBBPiggybackAllowed(true), itsAartfaacPiggybackAllowed(true), itsStorage(0)
{
    itsStorage = new TaskStorage(this);
    itsStorage->itsStorageSelectionMode = static_cast<storage_selection_mode>(query.value(query.record().indexOf("storageSelectionMode")).toInt());
    itsNightTimeCurve = query.value(query.record().indexOf("nightTimeWeightFactor")).toUInt();
    itsReservation = query.value(query.record().indexOf("reservation")).toUInt();
    itsAnalogBeam.directionType = DIR_TYPE_J2000; // default initialize value
}

Observation::~Observation() {
    delete itsStorage;
}

QDataStream& operator<< (QDataStream &out, const Observation &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const StationTask &>(task);

        out << task.itsReservation
            << task.itsAnalogBeam;

        // itsDigitalBeams
        out << (quint32) task.itsDigitalBeams.size();
        for (std::map<unsigned, DigitalBeam>::const_iterator dit = task.itsDigitalBeams.begin(); dit != task.itsDigitalBeams.end(); ++dit) {
            out << (quint16) dit->first // digital beam number
                << dit->second;
        }

        out << task.itsNightTimeCurve
            << task.itsNrOfSubbands
            << task.itsTBBPiggybackAllowed
            << task.itsAartfaacPiggybackAllowed
            << task.itsRTCPsettings
            << *task.itsStorage;
    }
    return out;
}

QDataStream& operator>> (QDataStream &in, Observation &task) {
    if (in.status() == QDataStream::Ok) {
        in >> static_cast<StationTask &>(task);

        in >> task.itsReservation
           >> task.itsAnalogBeam;

        // itsDigitalBeams
        quint16 beamNr;
        task.itsDigitalBeams.clear();
        DigitalBeam digBeam;
        quint32 nrOfObjects;
        in >> nrOfObjects; // number of digital beams
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            digBeam.clear(); // resets the digiBeam
            in >> beamNr >> digBeam; // digital beam number, digital beam object
            task.itsDigitalBeams.insert(std::map<unsigned, DigitalBeam>::value_type(beamNr, digBeam));
        }

        in >> task.itsNightTimeCurve
           >> task.itsNrOfSubbands
           >> task.itsTBBPiggybackAllowed
           >> task.itsAartfaacPiggybackAllowed
           >> task.itsRTCPsettings;

        // itsStorage
        delete task.itsStorage;
        task.itsStorage = new TaskStorage(&task);
        in >> *task.itsStorage;

        // calculate some info
        task.calcTotalSubbands();
        task.calculateDataFiles(); // calculates the estimate for the total data size for this task
        task.generateFileList();
    }
    return in;
}

Observation & Observation::operator=(const Task &other) {
        if (this != &other) {
            const StationTask *pOther = dynamic_cast<const StationTask *>(&other);
            if (pOther) {
                StationTask::operator=(other);
                const Observation *pOtherObs = dynamic_cast<const Observation *>(&other);
                if (pOtherObs) {
                    itsReservation = pOtherObs->itsReservation;
                    itsAnalogBeam = pOtherObs->itsAnalogBeam;
                    itsDigitalBeams = pOtherObs->itsDigitalBeams;
                    itsNightTimeCurve = pOtherObs->itsNightTimeCurve;
                    itsNrOfSubbands = pOtherObs->itsNrOfSubbands;
                    itsTBBPiggybackAllowed = pOtherObs->itsTBBPiggybackAllowed;
                    itsAartfaacPiggybackAllowed = pOtherObs->itsAartfaacPiggybackAllowed;
                    itsRTCPsettings = pOtherObs->itsRTCPsettings;
                    delete itsStorage;
                    itsStorage = new TaskStorage(this, pOtherObs->storage());
                }
            }
        }
    return *this;
}

QString Observation::getDigitalBeamSubbandStr(unsigned beam) const {
    std::map<unsigned, DigitalBeam>::const_iterator it;
    if ((it = itsDigitalBeams.find(beam)) != itsDigitalBeams.end()) {
        return it->second.subbandsStr();
    }
    else return QString();
}

unsigned Observation::nrRingTABs(void) const {
    unsigned nrTabs(0);
    for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        nrTabs += it->second.nrRingTABs();
    }
    return nrTabs;
}

unsigned Observation::nrTABrings(void) const {
    unsigned nrTabRings(0);
    for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        nrTabRings += it->second.nrTABrings();
    }
    return nrTabRings;
}

unsigned Observation::nrManualTABs(void) const {
    unsigned nrTabs(0);
    for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        nrTabs += it->second.nrManualTABs();
    }
    return nrTabs;
}

unsigned Observation::nrIncoherentTABs(void) const {
    unsigned nrIncoherent(0);
    for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        nrIncoherent += it->second.nrIncoherentTABs();
    }
    return nrIncoherent;
}

unsigned Observation::nrCoherentTABs(void) const {
    unsigned nrCoherent(0);
    for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        nrCoherent += it->second.nrCoherentTABs();
    }
    return nrCoherent;
}

void Observation::setSubbandNotationChange(unsigned beamNr, bool change) {
    std::map<unsigned, DigitalBeam>::iterator it = itsDigitalBeams.find(beamNr);
    if (it != itsDigitalBeams.end()) {
        it->second.setSubbandNotationChange(change);
    }
}

void Observation::resetBeamDurations(void) {
    for (std::map<unsigned, DigitalBeam>::iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        it->second.clearDuration();
    }
}

// calculates the total number of subbands used by this task
void Observation::calcTotalSubbands(void) {
    itsNrOfSubbands = 0;
    for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
        itsNrOfSubbands += it->second.nrSubbands();
    }
}

// calculates the estimated total output data size in kBytes
void Observation::calculateDataFiles(void) {
    if (itsDuration.totalSeconds() > 0) {
        calculateDataSize();
    }
    else {
        itsStorage->itsOutputDataFiles.clear();
    }
}

void Observation::calculateDataSize(void) {
    itsStorage->itsTotalDataSizekBytes = 0.0;
    itsStorage->itsTotalBandWidth = 0.0;

    double SamplesPerSecond;
    unsigned sizeOfFComplex(8), nrOfPolarizations(2), sizeOfShort(2);
    unsigned totalFilesCoherent(0), totalFilesCoherentMin(0), totalFilesCoherentSummed(0), totalFilesIncoherent(0), totalFilesIncoherentSummed(0),
            nrTabRings(0), nrCoherent(0), nrIncoherent(0), maxNrSubbandsCoherent(0), maxNrSubbandsIncoherent(0);

    if (itsClockFrequency == clock_160Mhz) SamplesPerSecond = CLOCK160_SAMPLESPERSECOND;
    else SamplesPerSecond = CLOCK200_SAMPLESPERSECOND;
    if (itsStorage->itsEnabledOutputData.coherentStokes || itsStorage->itsEnabledOutputData.incoherentStokes) { // = complex voltages i.e. Beamformed data product
        // calculate the total number of Tied Array Beams (TAB)
        (itsRTCPsettings.coherentType == DATA_TYPE_STOKES_IQUV) || (itsRTCPsettings.coherentType == DATA_TYPE_XXYY) ? nrCoherent = 4 : nrCoherent = 1;
        itsRTCPsettings.incoherentType == DATA_TYPE_STOKES_IQUV ? nrIncoherent = 4 : nrIncoherent = 1;
        for (std::map<unsigned, DigitalBeam>::const_iterator dbit = itsDigitalBeams.begin(); dbit != itsDigitalBeams.end(); ++dbit) {
            const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(dbit->second.tiedArrayBeams());
            for (std::map<unsigned, TiedArrayBeam>::const_iterator tit = tiedArrayBeams.begin(); tit != tiedArrayBeams.end(); ++tit) {
                if (tit->second.isCoherent()) { // coherent TAB, use coherentStokes.which parameter for the number of Stokes
                    maxNrSubbandsCoherent = std::max(maxNrSubbandsCoherent, dbit->second.nrSubbands());
                    totalFilesCoherentMin = nrCoherent;
                    totalFilesCoherentSummed += totalFilesCoherentMin;
                    totalFilesCoherent += nrCoherent * static_cast<unsigned>(ceil(static_cast <float>(dbit->second.nrSubbands()) / itsRTCPsettings.coherentSubbandsPerFile));
                }
                else { // incoherent TAB, use incoherentStokes.which parameter for the number of Stokes
                    totalFilesIncoherentSummed += nrIncoherent;
                    totalFilesIncoherent += nrIncoherent * static_cast<unsigned>(ceil(static_cast <float>(dbit->second.nrSubbands()) / itsRTCPsettings.incoherentSubbandsPerFile));
                    maxNrSubbandsIncoherent = std::max(maxNrSubbandsIncoherent, dbit->second.nrSubbands());
                }
            }
            // add files due to nrTabs and tabRingSize specified in the digital beam (these are always coherent)+
            if (itsStorage->itsEnabledOutputData.coherentStokes) {
                nrTabRings = dbit->second.nrTabRings();
                if (nrTabRings > 0) {
                    maxNrSubbandsCoherent = std::max(maxNrSubbandsCoherent, dbit->second.nrSubbands());
                    totalFilesCoherentMin = (3 * nrTabRings * (nrTabRings + 1) + 1) * nrCoherent;
                    totalFilesCoherentSummed += totalFilesCoherentMin;
                    totalFilesCoherent += totalFilesCoherentMin * static_cast<unsigned>(ceil(static_cast <float>(dbit->second.nrSubbands()) / itsRTCPsettings.coherentSubbandsPerFile));
                }
                // add files due to fly's eye
                if (itsRTCPsettings.flysEye) {
                    maxNrSubbandsCoherent = std::max(maxNrSubbandsCoherent, dbit->second.nrSubbands());
                    totalFilesCoherentMin = itsNrVirtualStations * nrCoherent;
                    totalFilesCoherentSummed += totalFilesCoherentMin;
                    totalFilesCoherent += totalFilesCoherentMin * static_cast<unsigned>(ceil(static_cast <float>(dbit->second.nrSubbands()) / itsRTCPsettings.coherentSubbandsPerFile));
                }
            }
        }

        if ((itsStorage->itsEnabledOutputData.coherentStokes) && totalFilesCoherent > 0) {
            unsigned nrSubbandsPerFile = std::min((unsigned)itsRTCPsettings.coherentSubbandsPerFile, maxNrSubbandsCoherent);
            double sizePerSubband;
            if (itsRTCPsettings.coherentType == DATA_TYPE_XXYY) {
                sizePerSubband = (SamplesPerSecond * 4) * itsDuration.totalSeconds() / 1024;
                itsStorage->itsOutputDataFiles[DP_COHERENT_STOKES] = std::pair<double, unsigned>((double) nrSubbandsPerFile * sizePerSubband, totalFilesCoherent); // size per file, number of files
            }
            else {
                sizePerSubband = (SamplesPerSecond * 4) / itsRTCPsettings.coherentTimeIntegrationFactor * itsDuration.totalSeconds() / 1024;
                itsStorage->itsOutputDataFiles[DP_COHERENT_STOKES] = std::pair<double, unsigned>((double) nrSubbandsPerFile * sizePerSubband, totalFilesCoherent); // size per file, number of files
            }
            itsStorage->itsTotalDataSizekBytes += totalFilesCoherentSummed * maxNrSubbandsCoherent * sizePerSubband;
        }
        else {
            itsStorage->itsOutputDataFiles.erase(DP_COHERENT_STOKES);
        }

        if (itsStorage->itsEnabledOutputData.incoherentStokes && (totalFilesIncoherent > 0)) {
            int channelIntegrationFactor = itsRTCPsettings.incoherentChannelsPerSubband > 0 ? itsRTCPsettings.channelsPerSubband / itsRTCPsettings.incoherentChannelsPerSubband : 1;
            unsigned nrSubbandsPerFile = std::min((unsigned)itsRTCPsettings.incoherentSubbandsPerFile, maxNrSubbandsIncoherent);
            double sizePerSubband = (SamplesPerSecond * 4) / itsRTCPsettings.incoherentTimeIntegrationFactor / channelIntegrationFactor * itsDuration.totalSeconds() / 1024;
            itsStorage->itsOutputDataFiles[DP_INCOHERENT_STOKES] = std::pair<double, unsigned>((double) nrSubbandsPerFile * sizePerSubband, totalFilesIncoherent); // size per file, number of files
            itsStorage->itsTotalDataSizekBytes += totalFilesIncoherentSummed * maxNrSubbandsIncoherent * sizePerSubband;
        }
        else {
            itsStorage->itsOutputDataFiles.erase(DP_INCOHERENT_STOKES);
        }
    }
    if (itsStorage->itsEnabledOutputData.correlated) { // UV data
        // baselines * polarizations * samples * channels * (bytes/sample +	overhead)
        // where baselines = #stations * (#stations + 1) / 2
        // #stations can vary depending on superstations, splitted core stations, etc.
        unsigned nrOfBaseLines = itsNrVirtualStations * (itsNrVirtualStations + 1)/2;
        double integratedSeconds = floor(itsDuration.totalSeconds() / itsRTCPsettings.correlatorIntegrationTime);

        double headerSize(512);
        double dataSize(nrOfBaseLines * itsRTCPsettings.channelsPerSubband * nrOfPolarizations * nrOfPolarizations * sizeOfFComplex);
        dataSize = ceil(dataSize / 512) * 512;
        double nSampleSize(nrOfBaseLines * itsRTCPsettings.channelsPerSubband * sizeOfShort);
        nSampleSize = ceil(nSampleSize / 512) * 512;
        double overhead(600000); // bytes
        double fileSize(((dataSize + nSampleSize + headerSize) * integratedSeconds + overhead) / 1024);
        itsStorage->itsOutputDataFiles[DP_CORRELATED_UV] = std::pair<double, unsigned>(fileSize, itsNrOfSubbands);
        itsStorage->itsTotalDataSizekBytes += fileSize * itsNrOfSubbands;
    }
    else {
        itsStorage->itsOutputDataFiles.erase(DP_CORRELATED_UV);
    }

    // also remove any left-overs from possibly previously specified output data types that now have been switched off.
    if (!itsStorage->itsEnabledOutputData.coherentStokes) {
        itsStorage->itsOutputDataFiles.erase(DP_COHERENT_STOKES);
    }
    if (!itsStorage->itsEnabledOutputData.incoherentStokes) {
        itsStorage->itsOutputDataFiles.erase(DP_INCOHERENT_STOKES);
    }

    itsStorage->itsTotalBandWidth = (itsStorage->itsTotalDataSizekBytes * 8) / itsDuration.totalSeconds(); // kbit/sec
}


void Observation::setNightTimeWeightFactor(unsigned short curve) {
    if ((curve >=1) & (curve <= 7)) {
        itsNightTimeCurve = curve;
        penaltyCalculationNeeded = true;
    }
    else {
//#ifdef DEBUG_SCHEDULER
//		std::cerr << "Warning: night time weight factor must be integer between 1 and 7" << std::endl;
//#endif
    }
}


bool Observation::diff(const Task *other, task_diff &dif) const {
    bool stationTaskDif(StationTask::diff(other, dif));

    const Observation *pOther = dynamic_cast<const Observation *>(other);
    if (pOther) {
        bool storageDif(itsStorage->diff(pOther->storage(), dif));

        itsReservation != pOther->getReservation() ? dif.reservation = true : dif.reservation = false;
        itsNightTimeCurve != pOther->getNightTimeWeightFactor() ? dif.night_time_weight_factor = true : dif.night_time_weight_factor = false;
        itsAnalogBeam.angle1 != pOther->getAnalogBeam().angle1 ? dif.ana_beam_angle1 = true : dif.ana_beam_angle1 = false;
        itsAnalogBeam.angle2 != pOther->getAnalogBeam().angle2  ? dif.ana_beam_angle2 = true : dif.ana_beam_angle2 = false;
        itsAnalogBeam.directionType != pOther->getAnalogBeam().directionType ? dif.ana_beam_direction_type = true : dif.ana_beam_direction_type = false;
        itsAnalogBeam.duration != pOther->getAnalogBeam().duration ? dif.ana_beam_duration = true : dif.ana_beam_duration = false;
        itsAnalogBeam.startTime != pOther->getAnalogBeam().startTime ? dif.ana_beam_starttime = true : dif.ana_beam_starttime = false;

        // check the digital beams
        dif.tiedarray_beam_settings = false;
        const std::map<unsigned, DigitalBeam> &otherDigiBeams = pOther->getDigitalBeams();
        std::map<unsigned, DigitalBeam>::const_iterator oit;
        if (otherDigiBeams.size() != itsDigitalBeams.size()) {
            dif.digital_beam_settings = true;
        }
        else {
            dif.digital_beam_settings = false;
            for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
                oit = otherDigiBeams.find(it->first);
                if (oit != otherDigiBeams.end()) {
                    if ((it->second.target() != oit->second.target()) ||
                            (it->second.angle1() != oit->second.angle1()) ||
                            (it->second.angle2() != oit->second.angle2()) ||
                            (it->second.directionType() != oit->second.directionType()) ||
                            (it->second.nrTABrings() != oit->second.nrTABrings()) ||
                            (fabs(it->second.tabRingSize() - oit->second.tabRingSize()) > std::numeric_limits<double>::epsilon()) ||
                            (it->second.duration() != oit->second.duration()) ||
                            (it->second.startTime() != oit->second.startTime()) ||
                            (it->second.subbandList() != oit->second.subbandList())) {
                        dif.digital_beam_settings = true;
                        break;
                    }
                }
                else {
                    dif.digital_beam_settings = true;
                    break;
                }
            }
        }

        // check differences in tied array beams (which are properties of the digital beams)
        for (std::map<unsigned, DigitalBeam>::const_iterator it = itsDigitalBeams.begin(); it != itsDigitalBeams.end(); ++it) {
            oit = otherDigiBeams.find(it->first);
            if (oit != otherDigiBeams.end()) {
                // tied array beams of this digital beam
                if (it->second.tiedArrayBeams() != oit->second.tiedArrayBeams()) {dif.tiedarray_beam_settings = true; break; }
            }
            else {
                dif.tiedarray_beam_settings = true;
                break;
            }
        }
        dif.TBBPiggybackAllowed = itsTBBPiggybackAllowed != pOther->itsTBBPiggybackAllowed ? true : false;
        dif.AartfaacPiggybackAllowed = itsAartfaacPiggybackAllowed != pOther->itsAartfaacPiggybackAllowed ? true : false;
        // RTCP settings
        dif.RTCP_correct_bandpass = itsRTCPsettings.correctBandPass != pOther->getRTCPsettings().correctBandPass ? true : false;
        dif.RTCP_cor_int_time = itsRTCPsettings.correlatorIntegrationTime != pOther->getRTCPsettings().correlatorIntegrationTime ? true : false;
        dif.RTCP_delay_compensation = itsRTCPsettings.delayCompensation != pOther->getRTCPsettings().delayCompensation ? true : false;
        dif.RTCP_nr_bits_per_sample = itsRTCPsettings.nrBitsPerSample != pOther->getRTCPsettings().nrBitsPerSample ? true : false;
        dif.RTCP_channels_per_subband = itsRTCPsettings.channelsPerSubband != pOther->getRTCPsettings().channelsPerSubband ? true : false;
        dif.RTCP_pencil_flys_eye = itsRTCPsettings.flysEye != pOther->getRTCPsettings().flysEye ? true : false;
        dif.RTCP_coherent_dedispersion = itsRTCPsettings.coherentDedisperseChannels != pOther->getRTCPsettings().coherentDedisperseChannels ? true : false;

        if ((itsRTCPsettings.coherentChannelsPerSubband != pOther->getRTCPsettings().coherentChannelsPerSubband) ||
                (itsRTCPsettings.coherentType != pOther->getRTCPsettings().coherentType) ||
                (itsRTCPsettings.coherentTimeIntegrationFactor != pOther->getRTCPsettings().coherentTimeIntegrationFactor) ||
                (itsRTCPsettings.coherentSubbandsPerFile != pOther->getRTCPsettings().coherentSubbandsPerFile))
            dif.RTCP_coherent_stokes_settings = true;
        else dif.RTCP_coherent_stokes_settings = false;
        if ((itsRTCPsettings.incoherentChannelsPerSubband != pOther->getRTCPsettings().incoherentChannelsPerSubband) ||
                (itsRTCPsettings.incoherentType != pOther->getRTCPsettings().incoherentType) ||
                (itsRTCPsettings.incoherentTimeIntegrationFactor != pOther->getRTCPsettings().incoherentTimeIntegrationFactor) ||
                (itsRTCPsettings.incoherentSubbandsPerFile != pOther->getRTCPsettings().incoherentSubbandsPerFile))
            dif.RTCP_incoherent_stokes_settings = true;
        else dif.RTCP_incoherent_stokes_settings = false;

        // task_id on its own should not be detected as a change, only when other properties of that task need to be saved will task_id (if different) also be saved.

        return (stationTaskDif || storageDif || dif.reservation || dif.night_time_weight_factor
                || dif.ana_beam_angle1 || dif.ana_beam_angle2 || dif.ana_beam_direction_type || dif.ana_beam_duration || dif.ana_beam_starttime
                || dif.digital_beam_settings || dif.tiedarray_beam_settings || dif.RTCP_correct_bandpass || dif.RTCP_cor_int_time || dif.RTCP_delay_compensation
                || dif.RTCP_nr_bits_per_sample || dif.RTCP_channels_per_subband || dif.RTCP_pencil_flys_eye || dif.RTCP_coherent_dedispersion
                || dif.RTCP_coherent_stokes_settings || dif.RTCP_incoherent_stokes_settings || dif.TBBPiggybackAllowed || dif.AartfaacPiggybackAllowed);
    }
    return stationTaskDif;
}

QString Observation::diffString(const task_diff &dif) const {
    QString difstr(StationTask::diffString(dif));
    if (!difstr.isEmpty()) difstr += ",";

    if (dif.night_time_weight_factor) difstr += QString(SAS_item_names[TP_NIGHT_TIME_WEIGHT_FACTOR]) + ",";
    if (dif.reservation) difstr += QString(SAS_item_names[TP_RESERVATION]) + ",";
    if (dif.ana_beam_angle1 | dif.ana_beam_angle2 | dif.ana_beam_direction_type | dif.ana_beam_duration | dif.ana_beam_starttime)
        difstr += QString(SAS_item_names[TP_ANA_BEAM_SETTINGS]) + ",";
    if (dif.digital_beam_settings) difstr += QString(SAS_item_names[TP_DIGI_BEAM_SETTINGS]) + ",";
    if (dif.tiedarray_beam_settings) difstr += QString(SAS_item_names[TP_TAB_SETTINGS]) + ",";
    if (dif.RTCP_correct_bandpass) difstr += QString(SAS_item_names[TP_RTCP_BANDPASS_CORR]) + ",";
    if (dif.RTCP_cor_int_time) difstr += QString(SAS_item_names[TP_RTCP_COR_INT_TIME]) + ",";
    if (dif.RTCP_delay_compensation) difstr += QString(SAS_item_names[TP_RTCP_DELAY_COMP]) + ",";
    if (dif.RTCP_nr_bits_per_sample) difstr += QString(SAS_item_names[TP_RTCP_BITS_PER_SAMPLE]) + ",";
    if (dif.RTCP_channels_per_subband) difstr += QString(SAS_item_names[TP_RTCP_CHANNELS_PER_SUBBAND]) + ",";
    if (dif.RTCP_pencil_flys_eye) difstr += QString(SAS_item_names[TP_RTCP_FLYS_EYE]) + ",";
    if (dif.RTCP_coherent_dedispersion) difstr += QString(SAS_item_names[TP_RTCP_COHERENT_DEDISPERSION]) + ",";
    if (dif.RTCP_incoherent_stokes_settings) difstr += QString(SAS_item_names[TP_RTCP_INCOHERENT_STOKES_SETTINGS]) + ",";
    if (dif.RTCP_coherent_stokes_settings) difstr += QString(SAS_item_names[TP_RTCP_COHERENT_STOKES_SETTINGS]) + ",";
    if (dif.TBBPiggybackAllowed) difstr += QString(SAS_item_names[TP_TBB_PIGGYBACK]) + ",";
    if (dif.AartfaacPiggybackAllowed) difstr += QString(SAS_item_names[TP_AARTFAAC_PIGGYBACK]) + ",";

    difstr += itsStorage->diffString(dif);

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}

void Observation::doCalculatePenalty() {

    // x = [0:1:100] = percentage of task scheduled at daytime
    // y = night time weight factor (for penalty calculation [0.0, 1.0]
    // curve 1: y = 1  ( in other words: task may not be scheduled during daytime)
    // curve 2: y = 1-c^-x  // c = 1.1
    // curve 3: y = 1-c^-x  // c = 1.04
    // curve 4: y = x
    // curve 5: y = c^(x-100) - 0.019800040113920 // c = 1.04
    // curve 6: y = c^(x-100) - 0.000072565715901  // c = 1.1
    // curve 7: y = 0 ( in other words: task may be scheduled at daytime without penalty)

    // penalty contribution of (partially) day time scheduling
    itsPenalty = 0;
    if (!itsStations.empty()) {
        double dayOverlap(0), wf(0);

        if (itsDuration > 0.5) { // no penalty for scheduling during day if duration is more than 12 hours
            penaltyCalculationNeeded = false;
            return;
        }

        if ( (scheduledStart >= Controller::theSchedulerSettings.getEarliestSchedulingDay()) &
             (scheduledEnd <= Controller::theSchedulerSettings.getLatestSchedulingDay()) ) {

            for (taskStationsMap::const_iterator it = itsStations.begin(); it != itsStations.end(); ++it) {
                double startDaySunRise = Controller::theSchedulerSettings.getStationSunRise(it->second, scheduledStart);
                double startDaySunSet = Controller::theSchedulerSettings.getStationSunSet(it->second, scheduledStart);
                double endDaySunRise = Controller::theSchedulerSettings.getStationSunRise(it->second, scheduledEnd);
                double endDaySunSet  = Controller::theSchedulerSettings.getStationSunSet(it->second, scheduledEnd);

                dayOverlap = 0;

                //step 1: calculate overlap with day time at both start and end of observation

                if ((scheduledStart > startDaySunRise) & (scheduledStart < startDaySunSet)) { // start during day?
                    if (scheduledEnd < startDaySunSet) { // ends also during same day?
                        dayOverlap = itsDuration.toJulian();
                    }
                    else { // ends during night
                        dayOverlap = startDaySunSet - scheduledStart.toJulian();
                    }
                }

                if ((scheduledEnd > endDaySunRise) & (scheduledEnd < endDaySunSet)) { // end during day?
                    if (scheduledStart > endDaySunRise) { // starts also during same day?
                        dayOverlap = itsDuration.toJulian();
                    }
                    else {
                        dayOverlap += scheduledEnd.toJulian() - endDaySunRise;
                    }
                }
                // step 2: calculate overlap percentage according to task duration
                double overlapRatio = dayOverlap / itsDuration.toJulian();

                // step 3: calculate night time weight factor according to user chosen curve (1 to 7) (this is factor y)
                //TODO re-determine the different night time weight factor curves to work with ratio overlap in stead of percentage overlap
                switch (itsNightTimeCurve) {
                case 1:
                    wf = 1;
                    break;
                case 2:
                    wf = 1 - pow(1.1, -overlapRatio);
                    break;
                case 3:
                    wf = 1 - pow(1.04, -overlapRatio);
                    break;
                case 4:
                default:
                    wf = overlapRatio;
                    break;
                case 5:
                    wf = pow(1.04, overlapRatio - 100) - 0.019800040113920;
                    break;
                case 6:
                    wf = pow(1.1, overlapRatio - 100) - 0.000072565715901;
                    break;
                case 7:
                    wf = 0;
                    break;
                }
                // step 4: calculate penalty contribution for partially day time scheduling with:
                // penalty contribution = d * y,  where d is the maximum penalty for full day time scheduling

                itsPenalty += static_cast<int>(MAX_DAY_TIME_PENALTY * wf);
            }
            itsPenalty /= itsStations.size();
            penaltyCalculationNeeded = false;
        }
        else {
            debugWarn("sis","Task: ",taskID, " penalty calculation aborted because task is out of schedule bounds");
        }
    }
    else {
        debugWarn("sis","Task: ",taskID, " penalty calculation aborted because task does not have stations defined");
    }
}


