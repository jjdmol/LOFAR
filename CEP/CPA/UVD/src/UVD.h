#ifndef UVDHeader_h
#define UVDHeader_h 1
    
#pragma aidgroup UVD
#pragma aid UVData UVSet Row Raw Sorted Unsorted Time Timeslot Channel Num
#pragma aid Control MS Integrate Flag Exposure Receptor Antenna IFR
#pragma aid SPW Field UVW Data Integrated Point Source Segment Corr Name
#pragma aid Header Footer Patch XX YY XY YX Chunk Indexing Index
#pragma aid Subtable Type Station Mount Pos Offset Dish Diameter
#pragma aid Feed Interval Polarization Response Angle Ref Freq Width
#pragma aid Bandwidth Effective Resolution Total Net Sideband
#pragma aid IF Conv Chain Group Data Desc Polarization Code Poly Delay
#pragma aid Dir Phase Pointing Lines Calibration Group Proper Motion Sigma Weight
#pragma aid Origin Target Tracking Beam Product Meas Centroid
#pragma aid AIPSPP

#include "UVD/AID-UVD.h"
#include "DMI/HIID.h"
#include "DMI/DataField.h"
#include "DMI/DataRecord.h"
    
// these typedefs are used as long as we lack a true DataTable class
typedef DataField DataTable;
typedef DataRecord DataTableRow;


// some predefined field names for UVDatasets
namespace UVD 
{
  static int dummy = aidRegistry_UVD();
  
  const HIID 
    FUVSetIndex     = AidUVSet|AidIndex,
    FPatchIndex     = AidPatch|AidIndex,
    FSegmentIndex   = AidSegment|AidIndex,
    FFieldIndex     = AidField|AidIndex,
    FFieldName      = AidField|AidName,
    FSourceName     = AidSource|AidName,
    FCorr           = AidCorr,
    FCorrName       = AidCorr|AidName,
    FNumTimeslots   = AidNum|AidTimeslot,
    FNumChannels    = AidNum|AidChannel,
    FNumBaselines   = AidNum|AidIFR,
    FChunkIndexing  = AidChunk|AidIndexing,

    FName           = AidName,
    FRowIndex       = AidRow|AidIndex,
    FRowFlag        = AidRow|AidFlag,
    FTimeSlotIndex  = AidTimeslot|AidIndex,
    FTime           = AidTime,
    FExposure       = AidExposure,
    FInterval       = AidInterval,
    FTimeCentroid   = AidTime|AidCentroid,
    FSigma          = AidSigma,
    FWeight         = AidWeight,
    FReceptorIndex  = AidReceptor|AidIndex,
    FAntennaIndex   = AidAntenna|AidIndex,
    FIFRIndex       = AidIFR|AidIndex,
    FSPWIndex       = AidSPW|AidIndex,
    FUVW            = AidUVW,
    FData           = AidData,
    FDataFlag       = AidData|AidFlag,
    FNumIntTimes    = AidNum|AidIntegrated|AidTimeslot,
    FNumIntPixels   = AidNum|AidIntegrated|AidPoint,
    FRowIndexing    = AidRow|AidIndexing,

    FAipsMSName     = AidAIPSPP|AidMS|AidName,
    
// field names for Antenna subtable
    FAntennaSubtable = AidAntenna|AidSubtable,
    FStationName     = AidStation|AidName,
    FType            = AidType,
    FMount           = AidMount,
    FPosition        = AidPos,
    FOffset          = AidOffset,
    FDishDiameter    = AidDish|AidDiameter,

// field names for Pointing subtable
    FPointingSubtable = AidPointing|AidSubtable,
    // FAntennaIndex, FTime, FName, FInterval already defined
    FNumPoly          = AidNum|AidPoly,
    FTimeOrigin       = AidTime|AidOrigin,
    FDirection        = AidDir,
    FTarget           = AidTarget,
    FTracking         = AidTracking,

// field names for Feed subtable
     FFeedSubtable    = AidFeed|AidSubtable,
     // FAntennaIndex, FTime, FInterval, FPosition, FNumRecptors already defined
     FNumReceptors    = AidNum|AidReceptor,
     FFeedIndex       = AidFeed|AidIndex,
     FBeamIndex       = AidBeam|AidIndex,
     FBeamOffset      = AidBeam|AidOffset,
     FPolznType       = AidPolarization|AidType,
     FPolznResponse   = AidPolarization|AidResponse,
     FReceptorAngle   = AidReceptor|AidAngle,
    
// Field names for Polarization subtable
    FPolarizationSubtable = AidPolarization|AidSubtable,
    FCorrType         = AidCorr|AidType,
    FCorrProduct      = AidCorr|AidProduct,
//      FCorr, FFlagRow - already defined

// Field names for SpectralWindow subtable
    FSPWSubtable     = AidSPW|AidSubtable,
    FRefFreq         = AidRef|AidFreq,
//    FNumChannels already defined
    FChannelFreq     = AidChannel|AidFreq,
    FChannelWidth    = AidChannel|AidWidth,
    FMeasFreqRef     = AidMeas|AidFreq|AidRef,
    FEffectiveBW     = AidEffective|AidBandwidth,
    FResolution      = AidResolution,
    FTotalBW         = AidTotal|AidBandwidth,
    FNetSideband     = AidNet|AidSideband,
    FIFConvChain     = AidIF|AidConv|AidChain,
    FFreqGroup       = AidFreq|AidGroup,
    FFreqGroupName   = AidFreq|AidGroup|AidName,
    
// Field names for SOURCE subtable
    FSourceSubtable   = AidSource|AidSubtable,
    FSourceIndex      = AidSource|AidIndex,
    // FName, FTime, FInterval, FSPWIndex already defined
    FNumLines         = AidNum|AidLines,
    FCalibrationGroup = AidCalibration|AidGroup,
    FCode             = AidCode,
    // FDirection, FPosition already defined
    FProperMotion     = AidProper|AidMotion,

// Field names for Data Description subtable
    FDataDescriptionSubtable = AidData|AidDesc|AidSubtable,
    // FSPWIndex defined above
    FPolarizationIndex = AidPolarization|AidIndex,
    
// Field  names for Field subtable
    FFieldSubtable      = AidField|AidSubtable,
    // FCode, FTime, FNumPoly already defined
    FDelayDirMeas       = AidDelay|AidDir|AidMeas,
    FPhaseDirMeas       = AidPhase|AidDir|AidMeas,
    FRefDirMeas         = AidRef|AidDir|AidMeas,
    // FSourceIndex already defined
    
    
    // this lets us end the list above with a comma
    FThisIsTheLastDeclaration = HIID();

    // flag value for missing data
    const int FlagMissing = 0xFFFFFFFF;


// small function for converting antennas to IFR indices    
    inline int ifrNumber ( int ant1,int ant2 ) 
    {
      return ant1 < ant2
              ? ifrNumber(ant2,ant1)
              : ant1*(ant1+1)/2 + ant2;
    }

};

#endif 
