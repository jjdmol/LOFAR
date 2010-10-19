//# DH_Beamlet.h: Beamlet DataHolder
//#
//# Copyright (C) 2000, 2001
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef ONLINEPROTO_DH_BEAMLET_H
#define ONLINEPROTO_DH_BEAMLET_H


#include <lofar_config.h>

#include <Transport/DataHolder.h>
#include <complex>
#include <Common/Lorrays.h>
#include <Common/Debug.h>

using std::complex;

namespace LOFAR
{
class DH_Beamlet: public DataHolder
{
public:
  typedef complex<float> BufferType;

  explicit DH_Beamlet (const string& name, 
		       const int StationID,
		       float FrequencyOffset,
		       float channelWidth,
		       float ElapsedTime,
		       int   nchan);

  explicit DH_Beamlet (const string& name, 
		       int nchan);

  DH_Beamlet(const DH_Beamlet&);

  virtual ~DH_Beamlet();

  DataHolder* clone() const;


  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Accessor functions to the data fileds in the Blob
  BufferType* getBuffer();
  const BufferType* getBuffer() const;
  BufferType* getBufferElement(int freq);
  int getNumberOfChannels () const;
  float getElapsedTime () const;
  void setElapsedTime (float time);
  float getFrequencyOffset () const; 
  float getChannelWidth() const;
  int getStationID () const;


private:
  // Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
  virtual void fillDataPointers();
  
  /// Forbid assignment.
  DH_Beamlet& operator= (const DH_Beamlet&);

  /// pointers to the data fileds in the Blob
  complex<float>* itsBufferptr;  // array containing frequency spectrum.
  int*            itsStationIDptr;
  float*          itsFrequencyOffsetptr;  
  float*          itsChannelWidthptr;     
  float*          itsElapsedTimeptr;      
  int*            itsNumberOfChannelsptr; 
  
  // local attributed to store settings between construction
  // and preprocess()
  int   itsStationID;          // source station ID
  float itsFrequencyOffset;  // frequency offset for this beamlet
  float itsChannelWidth;     // frequency width of each frequency channel
  float itsElapsedTime;      // the hourangle
  int   itsNumberOfChannels;   // number of frequency channels within this beamlet
  //  unsigned int itsBufSize;  

  

};
 
inline DH_Beamlet::BufferType* DH_Beamlet::getBuffer()
  { return itsBufferptr; }

inline const DH_Beamlet::BufferType* DH_Beamlet::getBuffer() const
  { return itsBufferptr; }

inline DH_Beamlet::BufferType* DH_Beamlet::getBufferElement(int freq)
  { return itsBufferptr+freq; }

inline int DH_Beamlet::getNumberOfChannels () const
  { return *itsNumberOfChannelsptr; }

inline float DH_Beamlet::getElapsedTime () const
  { DbgAssertStr(*itsElapsedTimeptr >= 0, 
		 "itsElapsedTime not initialised"); 
    return *itsElapsedTimeptr; 
  }

inline void DH_Beamlet::setElapsedTime (float time)
  {  *itsElapsedTimeptr = time; }

inline float DH_Beamlet::getFrequencyOffset() const
  { DbgAssertStr(*itsFrequencyOffsetptr >= 0, 
		 "itsFrequencyOffset not initialised"); 
  return *itsFrequencyOffsetptr;
  }

inline float DH_Beamlet::getChannelWidth() const
  { DbgAssertStr(*itsChannelWidthptr >= 0, 
		 "itsChannelWidth not initialised"); 
    return *itsChannelWidthptr;
  }

inline int DH_Beamlet::getStationID() const
  { DbgAssertStr(*itsStationIDptr >= 0, 
		 "itsStationID not initialised"); 
  return *itsStationIDptr;
  }

}

#endif 
