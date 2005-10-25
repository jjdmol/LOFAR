// WH_DataProc.h: interface for the WH_DataProc class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_DataProc_H__
#define AFX_WH_DataProc_H__

#include "WorkHolder.h"
#include "DH_Corr.h"
#include "DH_beam.h"

#include <vector>


class WH_DataProc:public WorkHolder
{
 public:
  WH_DataProc (int inputs, int outputs);
  virtual ~ WH_DataProc ();
  void process ();
  void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_beam* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Corr* getOutHolder (int channel); 

 private:

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_beam*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Corr*> itsOutDataHolders; 
};

inline DH_beam* WH_DataProc::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_Corr* WH_DataProc::getOutHolder (int channel) { return itsOutDataHolders[channel];}

#endif 
