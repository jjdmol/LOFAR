//# DH_Socket.h: DataHolder with one large buffer
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

#ifndef LIBTRANSPORT_TEST_DH_SOCKET_H
#define LIBTRANSPORT_TEST_DH_SOCKET_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/LofarTypes.h>
#include <Transport/DataHolder.h>

namespace LOFAR
{

// This class is an example DataHolder which is only used in the
// Example programs.

class DH_Socket: public DataHolder
{
public:
	typedef uchar BufferType;

	explicit DH_Socket (const string& name="dh_socket",
		       		  uint32 initialNrElements = 10);

	DH_Socket(const DH_Socket&);

	virtual ~DH_Socket();

	virtual DataHolder* clone() const;

	/// Set the Counter attribute.
	void setCounter (int32 counter);
	/// Get the Counter attribute.
	int32 getCounter() const;

	/// Allocate the buffers.
	virtual void init();

	/// Reset the buffer size.
	void setBufferSize (uint32 nelements);
	/// Return the buffe size.
	uint32 getBufferSize() const;

	/// Get write access to the Buffer.
	BufferType* getBuffer();
	/// Get read access to the Buffer.
	const BufferType* getBuffer() const;

 private:
	/// Forbid assignment.
	DH_Socket& operator= (const DH_Socket&);

	// Fill the pointers (itsCounter and itsBuffer) to the data in the blob.
	virtual void fillDataPointers();


	int32*		itsCounter;
	BufferType*	itsBuffer;
	uint32		itsBufSize;
};


inline void DH_Socket::setCounter (int32 counter)
  { *itsCounter = counter; }

inline int32 DH_Socket::getCounter() const
  { return *itsCounter; }

inline DH_Socket::BufferType* DH_Socket::getBuffer()
  { return itsBuffer; }
   
inline const DH_Socket::BufferType* DH_Socket::getBuffer() const
  { return itsBuffer; }

inline uint32 DH_Socket::getBufferSize() const
  { return itsBufSize; }


} // namespace LOFAR

#endif 
