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

#include <Common/lofar_complex.h>
#include <Common/Lorrays.h>
#include <Common/Debug.h>


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

  /// Get write access to the Buffer in the DataPacket.
  BufferType* getBuffer();
  /// Get read access to the Buffer in the DataPacket.
  const BufferType* getBuffer() const;
  BufferType* getBufferElement(int freq);
  int getNumberOfChannels () const;
  float getElapsedTime () const;
  void setElapsedTime (float time);
  float getFrequencyOffset () const; 
  float getChannelWidth() const;
  int getStationID () const;

protected:
  // Definition of the DataPacket type.
/*   class DataPacket: public DataHolder::DataPacket */
/*   { */
/*   public: */
/*     DataPacket(){}; */
/*     BufferType itsFill;         // to ensure alignment */

/*     int itsStationID;        // source station ID */
/*     float itsFrequencyOffset;    // frequency offset for this beamlet */
/*     float itsElapsedTime;      // the hourangle */
/*     int itsNumberOfChannels; // number of frequency channels within this beamlet */
/*     float itsChannelWidth;      // frequency width of each frequency channel */
/*   }; */

private:
  /// Forbid assignment.
    DH_Beamlet& operator= (const DH_Beamlet&);

    //    DataPacket*  itsDataPacket;    
    BufferType* itsBuffer;     // array containing frequency spectrum.
    int* itsStationID;          // source station ID
    float* itsFrequencyOffset;  // frequency offset for this beamlet
    float* itsChannelWidth;     // frequency width of each frequency channel
    float* itsElapsedTime;      // the hourangle
    int* itsNumberOfChannels;   // number of frequency channels within this beamlet
    unsigned int itsBufSize;  

    void fillDataPointers();
};

inline DH_Beamlet::BufferType* DH_Beamlet::getBuffer()
  { return itsBuffer; }

inline const DH_Beamlet::BufferType* DH_Beamlet::getBuffer() const
  { return itsBuffer; }

inline DH_Beamlet::BufferType* DH_Beamlet::getBufferElement(int freq)
  { return itsBuffer+freq; }

inline int DH_Beamlet::getNumberOfChannels () const
  { return *itsNumberOfChannels; }

inline float DH_Beamlet::getElapsedTime () const
  { DbgAssertStr(*itsElapsedTime >= 0, "itsElapsedTime not initialised");
    return *itsElapsedTime;
  }

inline void DH_Beamlet::setElapsedTime (float time)
  {  *itsElapsedTime = time; }

inline float DH_Beamlet::getFrequencyOffset() const
  { DbgAssertStr(*itsFrequencyOffset >= 0, "itsFrequencyOffset not initialised");
    return *itsFrequencyOffset; }

inline float DH_Beamlet::getChannelWidth() const 
  { DbgAssertStr(*itsChannelWidth >= 0, "itsChannelWidth not initialised");
    return *itsChannelWidth; }

inline int DH_Beamlet::getStationID() const
  { DbgAssertStr(*itsStationID >= 0, "itsStationID not initialised");
    return *itsStationID;
  }

}

#endif 
