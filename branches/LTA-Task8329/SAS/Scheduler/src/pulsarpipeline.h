/*
 * pulsarpipeline.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/pulsarpipeline.h $
 *
 */

#ifndef PULSARPIPELINE_H
#define PULSARPIPELINE_H

#include <QString>
#include "pipeline.h"

class PulsarPipeline : public Pipeline
{
    friend class TaskStorage;

public:
    PulsarPipeline();
    PulsarPipeline(unsigned task_id);
    PulsarPipeline(const QSqlQuery &query, const OTDBtree &SAS_tree);
    PulsarPipeline(unsigned id, const OTDBtree &SAS_tree);

    friend QDataStream& operator<< (QDataStream &out, const PulsarPipeline &task); // used for writing data to binary file
    friend QDataStream& operator>> (QDataStream &in, PulsarPipeline &task); // used for reading data from binary file

    virtual void clone(const Task *other) {
        if (this != other) {
            const PulsarPipeline *pOther = dynamic_cast<const PulsarPipeline *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *pOther;
                taskID = myTaskID;
            }
        }
    }

    // getters
    inline bool noRFI(void) const {return itsNoRFI;}
    inline bool skipDspsr(void) const {return itsSkipDspsr;}
    inline bool noFold(void) const {return itsNoFold;}
    inline bool noPdmp(void) const {return itsNoPdmp;}
    inline bool rawTo8Bit(void) const {return itsRawTo8bit;}
    inline bool singlePulse(void) const {return itsSinglePulse;}
    inline bool rrats(void) const {return itsRrats;}
    inline bool skipDynamicSpectrum(void) const {return itsSkipDynamicSpectrum;}
    inline bool skipPrepfold(void) const {return itsSkipPrepfold;}
    inline const QString & twoBf2fitsExtra(void) const {return itsTwoBf2fitsExtra;}
    inline const QString & digifilExtra(void) const {return itsDigifilExtra;}
    inline const QString & dspsrExtra(void) const {return itsDspsrExtra;}
    inline const QString & prepDataExtra(void) const {return itsPrepDataExtra;}
    inline const QString & prepFoldExtra(void) const {return itsPrepFoldExtra;}
    inline const QString & prepSubbandExtra(void) const {return itsPrepSubbandExtra;}
    inline const QString & pulsarName(void) const {return itsPulsar;}
    inline const QString & rfiFindExtra(void) const {return itsRfiFindExtra;}
    inline int decodeNblocks(void) const {return itsDecodeNblocks;}
    inline int decodeSigma(void) const {return itsDecodeSigma;}
    inline int tsubInt(void) const {return itsTsubint;}
    inline const double & eightBitConversionSigma(void) const {return itsEightBitConvSigma;}
    inline const double & dynamicSpectrumAvg(void) const {return itsDynamicSpectrumAvg;}
    inline const double & rratsDmRange(void) const {return itsRratsDmRange;}
    dataTypes coherentType(void) const {return itsCoherentType;}
    dataTypes incoherentType(void) const {return itsIncoherentType;}

    // setters
    inline void setNoRFI(bool bVal) {itsNoRFI = bVal;}
    inline void setSkipDspsr(bool bVal) {itsSkipDspsr = bVal;}
    inline void setNoFold(bool bVal) {itsNoFold = bVal;}
    inline void setNoPdmp(bool bVal) {itsNoPdmp = bVal;}
    inline void setRawTo8Bit(bool bVal) {itsRawTo8bit = bVal;}
    inline void setSinglePulse(bool bVal) {itsSinglePulse = bVal;}
    inline void setRRATS(bool bVal) {itsRrats = bVal;}
    inline void setSkipDynamicSpectrum(bool bVal) {itsSkipDynamicSpectrum = bVal;}
    inline void setSkipPrepfold(bool bVal) {itsSkipPrepfold = bVal;}
    inline void setTwoBf2fitsExtra(const QString & text) {itsTwoBf2fitsExtra = text;}
    inline void setDigifilExtra(const QString &text) {itsDigifilExtra = text;}
    inline void setDspsrExtra(const QString &text) {itsDspsrExtra = text;}
    inline void setPrepDataExtra(const QString &text) {itsPrepDataExtra = text;}
    inline void setPrepFoldExtra(const QString &text) {itsPrepFoldExtra = text;}
    inline void setPrepSubbandExtra(const QString &text) {itsPrepSubbandExtra = text;}
    inline void setPulsarName(const QString &text) {itsPulsar = text;}
    inline void setRFIfindExtra(const QString &text) {itsRfiFindExtra = text;}
    inline void setDecodeNblocks(int value) {itsDecodeNblocks = value;}
    inline void setDecodeSigma(int value) {itsDecodeSigma = value;}
    inline void setTsubInt(int value) {itsTsubint = value;}
    inline void setEightBitConversionSigma(const double &value) {itsEightBitConvSigma = value;}
    inline void setDynamicSpectrumAvg(const double &value) {itsDynamicSpectrumAvg = value;}
    inline void setRratsDmRange(const double &value) {itsRratsDmRange = value;}
    inline void setCoherentType(dataTypes type) {itsCoherentType = type;}
    inline void setIncoherentType(dataTypes type) {itsIncoherentType = type;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

    virtual void calculateDataSize(void);

private:
    bool itsNoFold, itsNoPdmp, itsNoRFI, itsRawTo8bit, itsRrats, itsSinglePulse, itsSkipDspsr, itsSkipDynamicSpectrum, itsSkipPrepfold;
    QString itsTwoBf2fitsExtra, itsDigifilExtra, itsDspsrExtra, itsPrepDataExtra, itsPrepFoldExtra, itsPrepSubbandExtra, itsPulsar, itsRfiFindExtra;
    int itsDecodeNblocks, itsDecodeSigma, itsTsubint;
    double itsEightBitConvSigma, itsDynamicSpectrumAvg, itsRratsDmRange;
    dataTypes itsCoherentType, itsIncoherentType;
};

#endif // PULSARPIPELINE_H
