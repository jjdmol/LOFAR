#ifndef UVDHeader_h
#define UVDHeader_h 1
    
#pragma aidgroup UVD
#pragma aid UVData UVSet Row Raw Sorted Unsorted Time Timeslot Channel Num
#pragma aid Control MS Integrate Flag Exposure Receptor Antenna IFR
#pragma aid SPW Field UVW Data Integrated Point Source Segment Corr Name
#pragma aid Header Footer Patch XX YY XY YX Chunk Indexing Index

#include "UVD/AID-UVD.h"
#include "DMI/HIID.h"

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
    FRowIndexing    = AidRow|AidIndexing;

// small function for converting antennas to IFR indices    
    inline int ifrNumber ( int ant1,int ant2 ) 
    {
      return ant1 < ant2
              ? ifrNumber(ant2,ant1)
              : ant1*(ant1+1)/2 + ant2;
    }

};

#endif 
