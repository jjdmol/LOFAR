// WH_TransB.h: interface for the WH_TransB class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_TransB_H__
#define _WH_TransB_H__

#include "WorkHolder.h"
#include "DH_beamT.h"
#include "DH_beam.h"

#include <vector>


class WH_TransB:public WorkHolder
{
 public:
  WH_TransB (int inputs,
	     int outputs);


  virtual ~ WH_TransB ();
  void process ();
  void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_beamT* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_beam* getOutHolder (int channel); 


private:
  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_beamT*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_beam*> itsOutDataHolders; 

};

inline DH_beamT* WH_TransB::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_beam*  WH_TransB::getOutHolder (int channel) { return itsOutDataHolders[channel];}


#endif 
