// WH_Beam.h: interface for the WH_Beam class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_Beam_H__
#define _WH_Beam_H__

#include "WorkHolder.h"
#include "DH_freqT.h"
#include "DH_beamT.h"

#include <vector>


class WH_Beam:public WorkHolder
{
 public:
  WH_Beam (int inputs,int outputs);
  
  virtual ~ WH_Beam ();
  void process ();
  void dump () const;

  short getInstanceCnt() const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_freqT* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_beamT* getOutHolder (int channel); 



 private:
  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_freqT*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_beamT*> itsOutDataHolders; 

  static short itsInstanceCnt;
  short        myInstanceCnt;
};

inline DH_freqT* WH_Beam::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_beamT* WH_Beam::getOutHolder (int channel) { return itsOutDataHolders[channel];}
inline short     WH_Beam::getInstanceCnt() const     { return myInstanceCnt; }

#endif 




