// WH_FFT.h: interface for the WH_FFT class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_FFT_H__
#define _WH_FFT_H__

#include "general.h"
#include "WorkHolder.h"
#include "DH_Antenna.h"
#include "DH_freq.h"

#include <vector>

/**
   Workholder for FFT function. 
   The Process() method of this Workholder reads Buffer from
   DataHolders of type DH_Antenna and writes Buffers to DataHolders of 
   type DH_freq.
   The process step performs an one-dimensional complex FFT on the
   input buffers an stores the Fourier-Transformed data in the
   output buffer; therefore, the number of inputs and outputs must be
   the same. Also the length of the input and output buffer must be identical.
 */
class WH_FFT:public WorkHolder
{
public:

  WH_FFT (int inputs,
	  int outputs);

  virtual ~ WH_FFT ();
  void process ();
  void dump () const;

  short getInstanceCnt() const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Antenna* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_freq* getOutHolder (int channel); 

 private:

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Antenna*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_freq*> itsOutDataHolders; 

  static short itsInstanceCnt;
  short        myInstanceCnt;
};

inline short       WH_FFT::getInstanceCnt() const     { return myInstanceCnt; }
inline DH_Antenna* WH_FFT::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_freq*    WH_FFT::getOutHolder (int channel) { return itsOutDataHolders[channel];}

#endif 





