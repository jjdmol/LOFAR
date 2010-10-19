// WH_TransF.h: interface for the WH_TransF class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_TransF_H__
#define _WH_TransF_H__


#include "WorkHolder.h"
#include "DH_freq.h"
#include "DH_freqT.h"

#include <vector>


class WH_TransF:public WorkHolder
{
 public:
  WH_TransF (int inputs,
	     int outputs);

    virtual ~ WH_TransF ();
  void process ();
  void dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_freq* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_freqT* getOutHolder (int channel); 


 private:
  std::vector<DH_freq> *transf_input;
  std::vector<DH_freqT> *transf_output;

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_freq*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_freqT*> itsOutDataHolders; 
};

inline DH_freq*  WH_TransF::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_freqT* WH_TransF::getOutHolder (int channel) { return itsOutDataHolders[channel];}


#endif
