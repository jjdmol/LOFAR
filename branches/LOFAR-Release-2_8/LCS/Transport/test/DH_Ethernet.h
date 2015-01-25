//# DH_Ethernet.h: DataHolder for TH_Ethernet test purposes
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LIBTRANSPORT_TEST_DH_ETHERNET_H
#define LIBTRANSPORT_TEST_DH_ETHERNET_H

#ifndef USE_NO_TH_ETHERNET


//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Transport/DataHolder.h>
#include <linux/if_ether.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// ExampleEthernet program.

class DH_Ethernet: public DataHolder
{
public:
  typedef char BufferType;

  explicit DH_Ethernet (const string& name="dh_ethernet",
		       unsigned int initialNelements = ETH_DATA_LEN,
		       bool useExtra = false);

  DH_Ethernet(const DH_Ethernet&);

  virtual ~DH_Ethernet();

  DataHolder* clone() const;

  /// Allocate the buffers.
  virtual void init();

  /// Reset the buffer size.
  void setBufferSize (unsigned int nelements);

  /// Get write access to the Buffer.
  BufferType* getBuffer();
  
  /// Get read access to the Buffer.
  const BufferType* getBuffer() const;
  
  /// Get Buffer element.
  BufferType& getBufferElement(unsigned int element);

 private:
  /// Forbid assignment.
  DH_Ethernet& operator= (const DH_Ethernet&);

  // Fill the pointers (itsBuffer) to the data in the blob.
  virtual void fillDataPointers();

  BufferType*  itsBuffer;
  unsigned int itsBufSize;
};

inline DH_Ethernet::BufferType* DH_Ethernet::getBuffer()
  { return itsBuffer; }
   
inline const DH_Ethernet::BufferType* DH_Ethernet::getBuffer() const
  { return itsBuffer; }

}

#endif

#endif 
