// WH_CPInput.h: interface for the WH_CorrConnect class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_CPINPUT_H__
#define _WH_CPINPUT_H__

#include "WorkHolder.h"
#include "DH_beam.h"
#include "DH_BeamBand.h"

#include <vector>


class WH_CPInput:public WorkHolder
{
 public:
  WH_CPInput (int inputs,
	   int outputs);


  virtual ~ WH_CPInput ();
  void process ();
  void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_beam* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_BeamBand* getOutHolder (int channel); 

private:
  static unsigned int itsCurrentTimeStamp;

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_beam*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_BeamBand*> itsOutDataHolders; 

};

inline DH_beam*   WH_CPInput::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

inline DH_BeamBand* WH_CPInput::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}

#endif 
