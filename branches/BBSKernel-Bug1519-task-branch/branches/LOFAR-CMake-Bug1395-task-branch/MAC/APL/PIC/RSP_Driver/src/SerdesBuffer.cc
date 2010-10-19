//#  SerdesBuffer.cc: one_line_description
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include "SerdesBuffer.h"

namespace LOFAR {
  namespace RSP {

//
// newCommmand (buf, len)
//
bool SerdesBuffer::newCommand   (char* cmdbuf, int	cmdLen)
{
	if (cmdLen > SERDES_BUFFER_SIZE) {
		LOG_ERROR_STR("Serdes buffer is only " << SERDES_BUFFER_SIZE << " bytes tall, not " << cmdLen);
		return (false);
	}
	
	memcpy(&itsBuffer[0], cmdbuf, cmdLen);
	itsNrBytes = cmdLen;
	return (true);
}

//
// appendCommmand (buf, len)
//
bool SerdesBuffer::appendCommand(char* cmdbuf, int	cmdLen)
{
	if (itsNrBytes + cmdLen > SERDES_BUFFER_SIZE) {
		LOG_ERROR_STR("Serdes buffer is only " << SERDES_BUFFER_SIZE << " bytes tall, not " << itsNrBytes + cmdLen);
		return (false);
	}
	
	memcpy(&itsBuffer[itsNrBytes], cmdbuf, cmdLen);
	itsNrBytes += cmdLen;
	return (true);
}

//
// clear()
//
void SerdesBuffer::clear()
{
	memset(itsBuffer, 0, SERDES_BUFFER_SIZE);
	itsNrBytes = 0;
}

  } // namespace RSP
} // namespace LOFAR
