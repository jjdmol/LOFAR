#ifndef UVDHeader_h
#define UVDHeader_h 1
    
#pragma aidgroup UVD
#pragma aid UVData UVSet Row Raw Sorted Unsorted Time Timeslot Channel Num
#pragma aid Control MS Integrate Flag Exposure Receptor Antenna IFR
#pragma aid SPW Field UVW Data Integrated Point Source Segment Corr Name
#pragma aid Header Footer Patch XX YY XY YX Chunk Indexing Index
#pragma aid Subtable Type Station Mount Position Offset Dish Diameter
#pragma aid AIPSPP Feed Interval Polarization Response Angle Ref Freq Width
#pragma aid Measure Bandwidth Effective Resolution Total Net Sideband
#pragma aid IF Conv Chain Group Data Desc Polarization

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
    
// field names for Antenna subtable
    FAntennaSubtable = AidAntenna|AidSubtable,
    FStationName     = AidStation|AidName,
    FType            = AidType,
    FMount           = AidMount,
    FPosition        = AidPosition,
    FOffset          = AidOffset,
    FDishDiameter    = AidDish|AidDiameter,

// // field names for Feed subtable
//     FFeedSubtable    = AidFeed|AidSubtable,
//     FFeedIndex       = AidFeed|AidIndex,
//     FInterval        = AidInterval,    
//     FNumReceptors     = AidNum|AidReceptor,
//     FBeamIndex        = AidBeam|AidIndex,
//     FBeamOffset       = AidBeam|AidOffset,
//     FPolznType        = AidPolarization|AidType,
//     FPolznResponse    = AidPolarization|AidResponse,
//     FReceptorAngle    = AidReceptor|AidAngle,
    
// Field names for Polarization subtable
    FPolarizationSubtable = AidPolarization|AidSubtable,
//      FCorr, FFlagRow - already defined

// Field names for SpectralWindow subtable
    FSPWSubtable     = AidSPW|AidSubtable,
    FRefFreq         = AidRef|AidFreq,
    FChannelFreq     = AidChannel|AidFreq,
    FChannelWidth    = AidChannel|AidWidth,
    FMeasFreqRef     = AidMeasure|AidFreq|AidRef,
    FEffectiveBW     = AidEffective|AidBandwidth,
    FResolution      = AidResolution,
    FTotalBW         = AidTotal|AidBandwidth,
    FNetSideband     = AidNet|AidSideband,
    FIFConvChain     = AidIF|AidConv|AidChain,
    FFreqGroup       = AidFreq|AidGroup,
    FFreqGroupName   = AidFreq|AidGroup|AidName,
    
// Field names for Data Description subtable
    FDataDescriptionSubtable = AidData|AidDesc|AidSubtable,
    // FSPWIndex defined above
    FPolarizationIndex = AidPolarization|AidIndex,
    
    
    
    
    

// small function for converting antennas to IFR indices    
    inline int ifrNumber ( int ant1,int ant2 ) 
    {
      return ant1 < ant2
              ? ifrNumber(ant2,ant1)
              : ant1*(ant1+1)/2 + ant2;
    }

};

#endif 
