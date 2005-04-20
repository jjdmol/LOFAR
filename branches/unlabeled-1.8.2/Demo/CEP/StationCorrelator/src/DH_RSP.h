//# DH_RSP.h: DataHolder storing RSP raw ethernet frames for 
//#           StationCorrelator demo
//#
//# Copyright (C) 2004
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

#ifndef STATIONCORRELATOR_DH_RSP_H
#define STATIONCORRELATOR_DH_RSP_H


//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Transport/DataHolder.h>
#include <complex>
#include <Common/KeyValueMap.h>

using std::complex;

namespace LOFAR
{

class DH_RSP: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_RSP (const string& name,
		   const KeyValueMap& kvm);

  DH_RSP(const DH_RSP&);

  virtual ~DH_RSP();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void preprocess();

  /// Deallocate the buffers.
  virtual void postprocess();

  /// Reset the buffer size.
  void setBufferSize (const unsigned int bufsize);

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;

  /// Get StationID from first EPA-packet in RSP-frame
  const int getStationID() const;

  /// Get SequenceID from first EPA-packet in RSP-frame
  const int getSeqID() const;

  /// Get BlockID from first EPA-packet in RSP-frame
  const int getBlockID() const;

  /// Set StationID of first EPA-packet in RSP-frame
  void setStationID(int);

  /// Set SequenceID of first EPA-packet in RSP-frame
  void setSeqID(int);

  /// Set BlockID of first EPA-packet in RSP-frame
  void setBlockID(int); 

  /// Reset the buffer
  void resetBuffer();

  /// set an element of the buffer
  void setBufferElement(const int EPApacket, 
			const int beamlet,
			const int polarisation,
			const complex<int16>& value);
  
 private:
  /// Forbid assignment.
  DH_RSP& operator= (const DH_RSP&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  BufferType*  itsBuffer;
  int itsEPAheaderSize;
  int itsNoBeamlets;
  int itsNoPolarisations;
  unsigned int itsBufSize;
};

inline DH_RSP::BufferType* DH_RSP::getBuffer()
  { return itsBuffer; }
   
inline const DH_RSP::BufferType* DH_RSP::getBuffer() const
  { return itsBuffer; }

inline const int DH_RSP::getStationID() const
  { return ((int*)&itsBuffer[2])[0]; }

inline const int DH_RSP::getSeqID() const
  { return ((int*)&itsBuffer[6])[0]; }

inline const int DH_RSP::getBlockID() const
  { return ((int*)&itsBuffer[10])[0]; }

inline void DH_RSP::setStationID(int stationid)
  { memcpy(&itsBuffer[2], &stationid, sizeof(int)); }

inline void  DH_RSP::setSeqID(int seqid)
  { memcpy(&itsBuffer[6], &seqid, sizeof(int)); }

inline void  DH_RSP::setBlockID(int blockid)
  { memcpy(&itsBuffer[10], &blockid, sizeof(int)); }

inline void DH_RSP:: resetBuffer()
  { memset(itsBuffer, 0, itsBufSize); }

#define GETADDRESS(ePacket, beamlet, polarisation) \
  (ePacket + 1)* itsEPAheaderSize \
  + ((ePacket * itsNoBeamlets + beamlet) \
  * itsNoPolarisations + polarisation) \
  * sizeof(complex<int16>)

inline void DH_RSP::setBufferElement(const int EPApacket, 
				     const int beamlet,
				     const int polarisation,
				     const complex<int16>& value)
{
  *((complex<int16>*)&itsBuffer[GETADDRESS(EPApacket, beamlet, polarisation)]) = value;
}


}

#endif 
