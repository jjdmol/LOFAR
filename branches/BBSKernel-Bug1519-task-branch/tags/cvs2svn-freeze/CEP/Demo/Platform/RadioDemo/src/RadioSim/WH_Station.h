// WH_Station.h: interface for the WH_Station class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_Station_H__
#define _WH_Station_H__

#include "WorkHolder.h"
#include "DH_Antenna.h"
#include "DH_beam.h"

#include <vector>


class WH_Station:public WorkHolder
{
 public:
  WH_Station (int inputs, int outputs);
  virtual ~ WH_Station ();

  void process ();
  void dump ();

  short getInstanceCnt();

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Antenna* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_beam* getOutHolder (int channel); 


 private:

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Antenna*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_beam*> itsOutDataHolders; 

  static short itsInstanceCnt;
  short        myInstanceCnt;
};

inline short       WH_Station::getInstanceCnt()           { return myInstanceCnt; }
inline DH_Antenna* WH_Station::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_beam*    WH_Station::getOutHolder (int channel) { return itsOutDataHolders[channel];}


#endif 

