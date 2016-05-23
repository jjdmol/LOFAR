/*
 * observation.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/observation.h $
 *
 */

#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "stationtask.h"
#include "Angle.h"
#include "DigitalBeam.h"
#include "taskstorage.h"

class Observation : public StationTask
{
    friend class TaskStorage;

public:
    enum ref_frame {
        J2000,
        B1950,
        REF_FRAME_END
    };

    class analogBeamSettings {
        friend QDataStream& operator>> (QDataStream &in, analogBeamSettings &beam) {
            if (in.status() == QDataStream::Ok) {
                in >> beam.angle1;
                in >> beam.angle2;
                quint8 beamDirType;
                in >> beamDirType;
                beam.directionType = (beamDirectionType) beamDirType;
                in >> beam.duration;
                in >> beam.startTime;
            }
            return in;
        }

        friend QDataStream& operator<< (QDataStream &out, const analogBeamSettings &beam) {
            if (out.status() == QDataStream::Ok) {
                out << beam.angle1
                    << beam.angle2
                    << (quint8) beam.directionType
                    << beam.duration
                    << beam.startTime;
            }
            return out;
        }

    public:
        Angle angle1;
        Angle angle2;
        beamDirectionType directionType;
        AstroTime startTime;
        AstroTime duration;
    };

    class RTCPsettings {
    public:
        RTCPsettings() : correctBandPass(false), delayCompensation(false), coherentDedisperseChannels(false), flysEye(false),
        channelsPerSubband(64), coherentChannelsPerSubband(0), incoherentChannelsPerSubband(0),
        coherentSubbandsPerFile(512),  incoherentSubbandsPerFile(512), coherentType(DATA_TYPE_STOKES_I), incoherentType(DATA_TYPE_STOKES_I),
        coherentTimeIntegrationFactor(1), incoherentTimeIntegrationFactor(1), correlatorIntegrationTime(1.0), nrBitsPerSample(16)
        { }

        friend QDataStream& operator>> (QDataStream &in, RTCPsettings &rtcp) {
            in >> rtcp.correctBandPass >> rtcp.delayCompensation >> rtcp.coherentDedisperseChannels >> rtcp.flysEye
               >> rtcp.channelsPerSubband >> rtcp.coherentChannelsPerSubband >> rtcp.incoherentChannelsPerSubband
               >> rtcp.coherentSubbandsPerFile >> rtcp.incoherentSubbandsPerFile;
            quint8 data_type;
            in >> data_type;
            rtcp.coherentType = (dataTypes) data_type;
            in >> data_type;
            rtcp.incoherentType = (dataTypes) data_type;
            in >> rtcp.coherentTimeIntegrationFactor >> rtcp.incoherentTimeIntegrationFactor
               >> rtcp.correlatorIntegrationTime >> rtcp.nrBitsPerSample;
            return in;
        }

        friend QDataStream& operator<< (QDataStream &out, const RTCPsettings &rtcp) {
            out << rtcp.correctBandPass << rtcp.delayCompensation << rtcp.coherentDedisperseChannels << rtcp.flysEye
                << rtcp.channelsPerSubband << rtcp.coherentChannelsPerSubband << rtcp.incoherentChannelsPerSubband
                << rtcp.coherentSubbandsPerFile << rtcp.incoherentSubbandsPerFile
                << (quint8)rtcp.coherentType << (quint8)rtcp.incoherentType
                << rtcp.coherentTimeIntegrationFactor << rtcp.incoherentTimeIntegrationFactor
                << rtcp.correlatorIntegrationTime << rtcp.nrBitsPerSample;

            return out;
        }

        bool operator!=(const RTCPsettings &other) const {
            return ((correctBandPass != other.correctBandPass) ||
                    (delayCompensation != other.delayCompensation) ||
                    (coherentDedisperseChannels != other.coherentDedisperseChannels) ||
                    (flysEye != other.flysEye) ||
                    (channelsPerSubband != other.channelsPerSubband) ||
                    (coherentChannelsPerSubband != other.coherentChannelsPerSubband) ||
                    (incoherentChannelsPerSubband != other.incoherentChannelsPerSubband) ||
                    (coherentSubbandsPerFile != other.coherentSubbandsPerFile) ||
                    (incoherentSubbandsPerFile != other.incoherentSubbandsPerFile) ||
                    (coherentType != other.coherentType) ||
                    (incoherentType != other.incoherentType) ||
                    (coherentTimeIntegrationFactor != other.coherentTimeIntegrationFactor) ||
                    (incoherentTimeIntegrationFactor != other.incoherentTimeIntegrationFactor) ||
                    (correlatorIntegrationTime != other.correlatorIntegrationTime) ||
                    (nrBitsPerSample != other.nrBitsPerSample));
        }

        bool correctBandPass, delayCompensation, coherentDedisperseChannels;
        bool flysEye;
        quint16 channelsPerSubband, coherentChannelsPerSubband, incoherentChannelsPerSubband, coherentSubbandsPerFile, incoherentSubbandsPerFile;
        dataTypes coherentType, incoherentType;
        quint16 coherentTimeIntegrationFactor, incoherentTimeIntegrationFactor;
        double correlatorIntegrationTime;
        quint8 nrBitsPerSample;

    };

    Observation();
    Observation(const Observation &);
    Observation(unsigned task_id);
    Observation(unsigned id, const OTDBtree &SAS_tree);
    Observation(const QSqlQuery &query, const OTDBtree &SAS_tree);

    ~Observation();

    virtual Observation & operator=(const Task &other);

    friend QDataStream& operator<< (QDataStream &out, const Observation &task);
    friend QDataStream& operator>> (QDataStream &in, Observation &task);

    virtual void clone(const Task *other) {
        if (this != other) {
            const Observation *pOther = dynamic_cast<const Observation *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *other;
                taskID = myTaskID;
            }
        }
    }

    // getters
    const std::map<unsigned, DigitalBeam> &getDigitalBeams(void) const {return itsDigitalBeams;}
    QString getDigitalBeamSubbandStr(unsigned beam) const;
    const analogBeamSettings &getAnalogBeam(void) const {return itsAnalogBeam;}
    const superStationMap &getSuperStations(void) const {return itsSuperStations;}
    unsigned getNrOfDataslotsPerRSPboard(void) const {return itsNrOfDataslotsPerRSPboard;}
    unsigned getNrOfBeams(void) const { return itsDigitalBeams.size(); }
    unsigned short getNightTimeWeightFactor(void) const { return itsNightTimeCurve; }
    const RTCPsettings & getRTCPsettings(void) const {return itsRTCPsettings;}
    unsigned getReservation() const { return itsReservation; } // returns the reservation ID (0 = not part of reservation)
    bool getTBBPiggybackAllowed(void) const {return itsTBBPiggybackAllowed;}
    bool getAartfaacPiggybackAllowed(void) const {return itsAartfaacPiggybackAllowed;}
    unsigned short getBitMode(void) const {return itsRTCPsettings.nrBitsPerSample;}
    unsigned getNrOfSubbands(void) const { return itsNrOfSubbands; }
    unsigned nrRingTABs(void) const;
    unsigned nrTABrings(void) const;
    unsigned nrManualTABs(void) const;
    unsigned totalNrTABs(void) const {return nrRingTABs() + nrManualTABs();}
    unsigned nrIncoherentTABs(void) const;
    unsigned nrCoherentTABs(void) const;
    virtual const std::vector<storageResult> &getStorageCheckResult(void) const { return itsStorage->getStorageCheckResult(); }

    // setters
    inline void setAnalogBeamSettings(const analogBeamSettings &analog_beam) {itsAnalogBeam = analog_beam;}
    inline void setAnalogBeamDirectionType(const beamDirectionType &dirType) {itsAnalogBeam.directionType = dirType;}
    inline void setAnalogBeamAngle1(const double &radian) {itsAnalogBeam.angle1.setRadianAngle(radian);}
    inline void setAnalogBeamAngle2(const double &radian) {itsAnalogBeam.angle2.setRadianAngle(radian);}
    inline void setAnalogBeamAngle1(const Angle &angle) {itsAnalogBeam.angle1 = angle;}
    inline void setAnalogBeamAngle2(const Angle &angle) {itsAnalogBeam.angle2 = angle;}
    inline void setAnalogBeamStartTime(int seconds) {itsAnalogBeam.startTime.clearTime(); itsAnalogBeam.startTime.addSeconds(seconds);}
    inline void setAnalogBeamDuration(int seconds) {itsAnalogBeam.duration.clearTime(); itsAnalogBeam.duration.addSeconds(seconds);}
    inline void setNrOfDataslotsPerRSPboard(unsigned nrDataslotsPerRSPboard) {itsNrOfDataslotsPerRSPboard = nrDataslotsPerRSPboard;}
    inline void setTBBPiggybackAllowed(bool TBBPiggybackAllowed) {itsTBBPiggybackAllowed = TBBPiggybackAllowed;}
    inline void setAartfaacPiggybackAllowed(bool aartfaacPiggybackAllowed) {itsAartfaacPiggybackAllowed = aartfaacPiggybackAllowed;}
    inline void setRTCPsettings(const RTCPsettings &RTCP_settings) {itsRTCPsettings = RTCP_settings; /*itsRecalcStorageNeeded = true;*/}

    inline void setChannelsPerSubband(unsigned channels) {itsRTCPsettings.channelsPerSubband = channels;}
    inline void setBitsPerSample(unsigned short bits_per_sample) {itsRTCPsettings.nrBitsPerSample = bits_per_sample;}
    inline void setIncoherentDataType(dataTypes data_type) {itsRTCPsettings.incoherentType = data_type;}
    inline void setCoherentDataType(dataTypes data_type) {itsRTCPsettings.coherentType = data_type;}
    inline void setIncoherentTimeIntegration(unsigned ti_factor) {itsRTCPsettings.incoherentTimeIntegrationFactor = ti_factor;}
    inline void setCoherentTimeIntegration(unsigned ti_factor) {itsRTCPsettings.coherentTimeIntegrationFactor = ti_factor;}
    inline void setCoherentChannelsPerSubband(unsigned cps) {itsRTCPsettings.coherentChannelsPerSubband = cps;}
    inline void setIncoherentChannelsPerSubband(unsigned cps) {itsRTCPsettings.incoherentChannelsPerSubband = cps;}
    inline void setCoherentSubbandsPerFile(unsigned subbands_per_file) {itsRTCPsettings.coherentSubbandsPerFile = subbands_per_file;}
    inline void setIncoherentSubbandsPerFile(unsigned subbands_per_file) {itsRTCPsettings.incoherentSubbandsPerFile = subbands_per_file;}
    inline void setBandPassCorrection(bool enabled) {itsRTCPsettings.correctBandPass = enabled;}
    inline void setDelayCompensation(bool enabled) {itsRTCPsettings.delayCompensation = enabled;}

    inline void setCorrelatorIntegrationTime(const double &integration_time) {itsRTCPsettings.correlatorIntegrationTime = integration_time;}
    inline void setFlysEye(bool enabled) {itsRTCPsettings.flysEye = enabled;}
    inline void setCoherentDedispersion(bool enabled) {itsRTCPsettings.coherentDedisperseChannels = enabled;}
     // sets the properties a digital beam. If the beam does not yet exist it will be created
    void resetBeamDurations(void);
    void setDigitalBeam(unsigned beamNr, const DigitalBeam &beam) {itsDigitalBeams[beamNr] = beam; calcTotalSubbands(); /*itsRecalcStorageNeeded = true;*/}
    void setDigitalBeams(const std::map<unsigned, DigitalBeam> &digitalBeams) {itsDigitalBeams = digitalBeams; calcTotalSubbands(); /*itsRecalcStorageNeeded = true;*/}
    void clearDigitalBeams(void) {itsDigitalBeams.clear(); itsNrOfSubbands = 0; /*itsRecalcStorageNeeded = true;*/}
    void deleteDigitalBeam(unsigned beamNr) {itsDigitalBeams.erase(beamNr); calcTotalSubbands();  /*itsRecalcStorageNeeded = true;*/} // deletes the digital beam with index beamNr
    void setSubbandNotationChange(unsigned beamNr, bool change);
    void setNightTimeWeightFactor(unsigned short curve);
    inline void setReservation(unsigned reservationID) {itsReservation = reservationID;}

    const std::map<dataProductTypes, TaskStorage::outputDataProduct> &generateFileList(void) {return itsStorage->generateFileList();}

    // calculate the output data sizes for observations
    virtual void calculateDataFiles(void);
    inline bool hasStorage(void) const {return itsStorage != 0;}
    TaskStorage *storage(void) {return itsStorage;}
    const TaskStorage *storage(void) const {return itsStorage;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

private:
    virtual void doCalculatePenalty();

private:
    // calculates the total number of subbands used by this task and stores it in itsNrOfSubbands
    void calcTotalSubbands(void);
    // calculates the amount of data files and their sizes that will be written to the storage nodes
    void calculateDataSize(void);

private:
    quint32 itsReservation;
    analogBeamSettings itsAnalogBeam;
    std::map<unsigned, DigitalBeam> itsDigitalBeams;
    quint8 itsNightTimeCurve; // (curve 1-7) the weight factor curve is used to calculate the penalty for daytime scheduling
    quint16 itsNrOfSubbands; // the total number of sub bands used by this task
    bool itsTBBPiggybackAllowed, itsAartfaacPiggybackAllowed;

    // RTCP settings
    RTCPsettings itsRTCPsettings;

    // storage settings
    TaskStorage *itsStorage;
};

#endif // OBSERVATION_H
