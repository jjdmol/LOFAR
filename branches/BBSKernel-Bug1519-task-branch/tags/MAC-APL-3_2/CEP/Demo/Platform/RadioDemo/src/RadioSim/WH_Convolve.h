// WH_Convolve.h: interface for the WH_Convolve class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_Convolve_H__
#define _WH_Convolve_H__

#include "WorkHolder.h"
#include "DH_Corr.h"

#include <vector>


class WH_Convolve:public WorkHolder
{
 public:
  WH_Convolve (int inputs,
	     int outputs);


  virtual ~ WH_Convolve ();
  void process ();
  void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Corr* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Corr* getOutHolder (int channel); 

 private:
  std::vector<DH_Corr> *convolve_input;
  std::vector<DH_Corr> *convolve_output;
  
  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Corr*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Corr*> itsOutDataHolders; 

};

inline DH_Corr* WH_Convolve::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_Corr* WH_Convolve::getOutHolder (int channel) { return itsOutDataHolders[channel];}

#endif 


