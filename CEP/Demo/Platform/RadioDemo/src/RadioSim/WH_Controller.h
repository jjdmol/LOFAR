// WH_Controller.h: interface for the WH_Controller class.
//
// This workholder is for a time controller step. It broadcasts a timestamp
// to other nodes, to synchronize the processes
//////////////////////////////////////////////////////////////////////

#ifndef _WH_Controller_H__
#define _WH_Controller_H__

#include "general.h"
#include "WorkHolder.h"
#include "DH_Empty.h"

#ifdef NOMPI
#include "TH_Mem.h"
#else
#include "TH_MPI.h"
#endif 

#include <vector>


class WH_Controller:public WorkHolder
{
 public:
  WH_Controller ();
  virtual ~ WH_Controller ();
  void process ();
  void dump () const;
  static unsigned long timeStamp;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Empty* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Empty* getOutHolder (int channel); 


 private:

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Empty*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Empty*> itsOutDataHolders; 
};

inline DH_Empty* WH_Controller::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_Empty* WH_Controller::getOutHolder (int channel) { return itsOutDataHolders[channel];}


#endif
