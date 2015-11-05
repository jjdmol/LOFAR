//#  SerdesBuffer.h: one_line_description
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef RSP_SERDESBUFFER_H
#define RSP_SERDESBUFFER_H

// \file SerdesBuffer.h
// one_line_description

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_bitset.h>
#include <Common/LofarConstants.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace RSP {

// \addtogroup RSP
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// A SerdesBuffer can contain a serie of Serdes commands. Each Serdes command always
// consists of 4 bytes. 2 bytes for the MDIO Header to address the register and 2 bytes
// for the MDIO Data to set the value of the register.
// Hence the length of a command must always be a multiple of 4 bytes.
class SerdesBuffer
{
public:
	SerdesBuffer () { clear(); }
	~SerdesBuffer() {}

	void clear();
	bool newCommand   (char* cmdbuf, int	cmdLen);
	bool appendCommand(char* cmdbuf, int	cmdLen);

	// bdatabuffer functions
	inline const char*	getBufferPtr()
		{ return (&itsBuffer[0]); }
	inline int			getDataLen	() 
		{ return (itsNrBytes);	}

private:
	// reserve space for a sequence of 16 commands.
	#define	SERDES_BUFFER_SIZE		64

	// Copying is not allowed
	SerdesBuffer(const SerdesBuffer&	that);
	SerdesBuffer& operator=(const SerdesBuffer& that);

	//# --- Datamembers ---
	char						itsBuffer [SERDES_BUFFER_SIZE];
	int							itsNrBytes;
};

//# --- Inline functions ---

// @}
  } // namespace RSP
} // namespace LOFAR

#endif
