//  DH_Correlations.h: DataHolder containing DataPacket with station-frequency matrix 
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////

#ifndef PIPELINE_DH_CORRELATIONS_H
#define PIPELINE_DH_CORRELATIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "myComplex.h"

#include "CEPFrame/DataHolder.h"
#include "Common/Debug.h"

/**
   This class is a DataHolder containing correlator outputs
*/

class DH_Correlations: public DataHolder
{
public:
  typedef myComplex32 DataType;

  explicit DH_Correlations (const string& name,
			    short stations,
			    short channels,
			    short polarisations);

  virtual ~DH_Correlations();

  /// Allocate the buffers.
  virtual void preprocess();

  /// fill the whole array with zeroes
  void reset();
  
  DataType* getBuffer(short stationA, short stationsB, short pol=0);
  const DataType* getBuffer(short stationA, short stationsB, short pol=0) const;
  
  void setStartTime(int time);
  void setEndTime(int time);
  void setStartChannel(int channel);
  void setEndChannel(int channel);
  int getStartTime();
  int getEndTime();
  int getStartChannel();
  int getEndChannel();

protected:
  class DataPacket: public DataHolder::DataPacket
  {
  public:
    DataPacket() {};
    int itsStartTime;
    int itsEndTime;
    short itsStartChannel;
    short itsEndChannel;
    DataType itsBuffer[];
  };

private:
  /// Forbid copy constructor.
  DH_Correlations (const DH_Correlations&);
  /// Forbid assignment.
  DH_Correlations& operator= (const DH_Correlations&);

  DataPacket* itsDataPacket;
  short itsStations;
  short itsChannels;
  short itsPols;
  short itsBaselines;
  short itsWindows;
  };


inline DH_Correlations::DataType* DH_Correlations::getBuffer(short stationA, short stationB, short pol) { 
  DbgAssertStr(stationA >= 0 && stationA < itsStations, "stationA not in range: " 
	       << stationA << " [0.." << itsStations << ")");
  DbgAssertStr(stationB >= 0 && stationB <= stationA   , "stationB not in range"
	       << stationB << " [0.." << stationA << "]");
  DbgAssertStr(pol >= 0 && pol < itsPols      , "pol not in range"
	       << pol << " [0.." << itsPols << ")");
  return &(itsDataPacket->itsBuffer[(stationA*(stationA+1)/2+stationB)*itsPols + pol]); 
}

inline const DH_Correlations::DataType* DH_Correlations::getBuffer(short stationA, short stationB, short pol) const { 
  DbgAssertStr(stationA >= 0 && stationA < itsStations, "stationA not in range: " 
	       << stationA << " [0.." << itsStations << ")");
  DbgAssertStr(stationB >= 0 && stationB <= stationA   , "stationB not in range"
	       << stationB << " [0.." << stationA << "]");
  DbgAssertStr(pol >= 0 && pol < itsPols      , "pol not in range"
	       << pol << " [0.." << itsPols << ")");
  return &(itsDataPacket->itsBuffer[(stationA*(stationA+1)/2+stationB)*itsPols + pol]); 
}

inline void  DH_Correlations::setStartTime(int time) {itsDataPacket->itsStartTime = time;}
inline void  DH_Correlations::setEndTime(int time) {itsDataPacket->itsEndTime = time;}
inline void  DH_Correlations::setStartChannel(int channel) {itsDataPacket->itsStartChannel = channel;}
inline void  DH_Correlations::setEndChannel(int channel){itsDataPacket->itsEndChannel = channel;}


inline int  DH_Correlations::getStartTime() {return itsDataPacket->itsStartTime;}
inline int  DH_Correlations::getEndTime() {return itsDataPacket->itsEndTime;}
inline int  DH_Correlations::getStartChannel() {return itsDataPacket->itsStartChannel;}
inline int  DH_Correlations::getEndChannel(){return itsDataPacket->itsEndChannel;}
#endif 
