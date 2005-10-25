// WH_MakeMS.h: interface for the WH_MakeMS class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_MakeMS_H__
#define _WH_MakeMS_H__

#include "WorkHolder.h"
#include "DH_Corr.h"
#include "DH_Empty.h"
#include "LSFiller.h"

#include <vector>


class WH_MakeMS: public WorkHolder
{
public:
  WH_MakeMS (int ninputs, LSFiller* filler);

  virtual ~ WH_MakeMS ();
  void process ();
  void dump () const;
  short getInstanceCnt() const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Corr* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Empty* getOutHolder (int channel); 

private:
  /// Vector with pointers to the input dataholders,
  vector<DH_Corr*> itsInDataHolders; 

  /// Output dataholder.
  DH_Empty itsOutDataHolder; 

  static short theirInstanceCnt;
  short        itsInstanceCnt;

  /// Pointer to the filler object.
  LSFiller* itsFiller;

  /// Counter how often written.
  int itsCounter;
};


inline DH_Corr* WH_MakeMS::getInHolder (int channel)
  { return itsInDataHolders[channel]; }

inline DH_Empty* WH_MakeMS::getOutHolder (int)
  { return &itsOutDataHolder;}

inline short WH_MakeMS::getInstanceCnt() const
  { return itsInstanceCnt; }


#endif 

