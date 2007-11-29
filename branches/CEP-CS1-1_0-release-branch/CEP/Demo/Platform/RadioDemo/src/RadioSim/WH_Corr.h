// WH_Corr.h: interface for the WH_Corr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_Corr_H__
#define _WH_Corr_H__

#include "WorkHolder.h"
#include "DH_BeamBand.h"
#include "DH_Corr.h"

#include <stdio.h>
#include <vector>


class WH_Corr:public WorkHolder
{
 public:
  WH_Corr (int inputs,
	   int outputs);


  virtual ~ WH_Corr ();
  void process ();

  void openFile(char *filename);
  void closeFile(void);
  void writeFile(float y);

  void dump () const;

  short getInstanceCnt() const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_BeamBand* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Corr* getOutHolder (int channel); 

 private:
  /* Correlation coefficients from Num.Rec. */
  void pearsn(DataBufferType x[], 
	      DataBufferType y[], 
	      unsigned long n, 
	      float *r);
    
  static unsigned int itsCurrentTimeStamp;

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_BeamBand*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Corr*> itsOutDataHolders; 

  static short itsInstanceCnt;
  short        myInstanceCnt;
  static int   itsProcessProfilerState; 

  FILE* itsFile;
  int   itsXOffset;
};

inline DH_BeamBand* WH_Corr::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

inline DH_Corr* WH_Corr::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

inline short WH_Corr::getInstanceCnt() const     { 
  return myInstanceCnt; 
}

#endif // 
