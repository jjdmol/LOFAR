#ifndef DEMIXINGSETTINGS_H
#define DEMIXINGSETTINGS_H

#include <QDataStream>
#include <QString>
#include <QStringList>

class DemixingSettings
{
public:
    DemixingSettings() : itsDemixingEnabled(false), itsFreqStep(64), itsTimeStep(10), itsDemixFreqStep(64), itsDemixTimeStep(10) { }

    // getters
    inline const QString &demixAlways(void) const {return itsDemixAlways;}
    QStringList demixAlwaysList(void) const;
    inline const QString &demixIfNeeded(void) const {return itsDemixIfNeeded;}
    QStringList demixIfNeededList(void) const;
    inline const QString &skyModel(void) const {return itsSkyModel;}
    inline quint16 demixFreqStep(void) const {return itsDemixFreqStep;}
    inline quint16 demixTimeStep(void) const {return itsDemixTimeStep;}
    inline quint16 freqStep(void) const {return itsFreqStep;}
    inline quint16 timeStep(void) const {return itsTimeStep;}
    inline bool demixingEnabled(void) const {return itsDemixingEnabled;}

    // setters
    inline void setDemixingEnabled(bool enabled) {itsDemixingEnabled = enabled;}
    inline void setDemixAlways(const QString &demix_sources) {itsDemixAlways = demix_sources;}
    inline void setDemixIfNeeded(const QString &demix_sources) {itsDemixIfNeeded = demix_sources;}
    inline void setDemixFreqStep(quint16 freqstep) {itsDemixFreqStep = freqstep;}
    inline void setDemixTimeStep(quint16 timestep) {itsDemixTimeStep = timestep;}
    inline void setAvgFreqStep(quint16 freqstep) {itsFreqStep = freqstep;}
    inline void setAvgTimeStep(quint16 timestep) {itsTimeStep = timestep;}
    inline void setDemixSkyModel(const QString &sky_model) {itsSkyModel = sky_model;}

    friend QDataStream& operator>> (QDataStream &in, DemixingSettings &d) {
        in >> d.itsDemixingEnabled
           >> d.itsDemixAlways >> d.itsDemixIfNeeded >> d.itsSkyModel
           >> d.itsFreqStep >> d.itsTimeStep >> d.itsDemixFreqStep >> d.itsDemixTimeStep;
        return in;
    }

    friend QDataStream& operator<< (QDataStream &out, const DemixingSettings &d) {
        out << d.itsDemixingEnabled
           << d.itsDemixAlways << d.itsDemixIfNeeded << d.itsSkyModel
           << d.itsFreqStep << d.itsTimeStep << d.itsDemixFreqStep << d.itsDemixTimeStep;
        return out;
    }

public:
    bool itsDemixingEnabled;
    QString itsDemixAlways, itsDemixIfNeeded, itsSkyModel;
    quint16 itsFreqStep, itsTimeStep, itsDemixFreqStep, itsDemixTimeStep;
};

#endif // DEMIXINGSETTINGS_H
