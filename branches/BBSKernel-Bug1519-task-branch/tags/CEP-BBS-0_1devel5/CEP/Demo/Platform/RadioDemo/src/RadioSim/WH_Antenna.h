// WH_Antenna.h: interface for the WH_Antenna class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _WH_ANTENNA_H__
#define _WH_ANTENNA_H__


#include "WorkHolder.h"
#include "DH_Antenna.h"
#include "PH_Antenna.h"

#include <vector>


class WH_Antenna:public WorkHolder
{
public:
  WH_Antenna (int inputs, int outputs);

  virtual ~ WH_Antenna ();

  void setPosition(float aXpos, float aYpos);

  void process ();
  void dump () const;
  //  static float itsTime;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Antenna* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Antenna* getOutHolder (int channel); 

 private:
  PH_Antenna itsPH;
  float itsDelay;
  float itsTime;
  unsigned int itsTimeStamp;
  void simulateInput(); 

  /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Antenna*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Antenna*> itsOutDataHolders; 
 
};

inline DH_Antenna* WH_Antenna::getInHolder (int channel)  { return itsInDataHolders[channel]; }
inline DH_Antenna* WH_Antenna::getOutHolder (int channel) { return itsOutDataHolders[channel];}

#endif 










