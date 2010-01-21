//#  ABSPointing.cc: implementation of the ABS::Pointing class
//#
//#  Copyright (C) 2004-2007
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarTypes.h>
#include <APL/IBS_Protocol/Pointing.h>
#include <MACIO/Marshalling.h>

using namespace LOFAR;
using namespace IBS_Protocol;

//
// Pointing()
//
Pointing::Pointing() : 
	itsAngle2Pi(0.0), itsAnglePi(0.0), itsTime(), itsDuration(0), itsType("NONE")
{}

//
// Pointing(angle, angle, time, type)
//
Pointing::Pointing(double angle0, double angle1, RTC::Timestamp time, uint duration, const string& type) :
	itsAngle2Pi(angle0), itsAnglePi(angle1), itsTime(time), itsDuration(duration), itsType(type)
{}

//
// ~Pointing()
//
Pointing::~Pointing()
{}

//
// overlap(otherPT)
//
bool Pointing::overlap(const Pointing&	that) const
{
	long	thisEnd = itsTime + (long)itsDuration;
	long	thatEnd = that.itsTime + (long)that.itsDuration;
	long	thisBegin = itsTime;
	long	thatBegin = that.itsTime;

	// everlasting pointings are special
	if (itsDuration == 0 && thatEnd > thisBegin)
		return (true);
	if (that.itsDuration == 0 && thisEnd > thatBegin)
		return (true);
	if (itsDuration ==0 && that.itsDuration == 0)
		return (true);

	// limited pointings: do they lay 'behind' each other? then no overlap
	if ((thatEnd <= thisBegin) || (thatBegin >= thisEnd))
		return (false);

	return (true);
}

//
// print
//
ostream& Pointing::print(ostream& os) const
{
	os << "[" << itsAngle2Pi << ", " << itsAnglePi << ", " << itsType << "]@" << 
				 itsTime << " for " << itsDuration << " seconds";
	return (os);
}

// -------------------- routines for streaming the object --------------------
//
// getSize()
//
unsigned int Pointing::getSize()
{
	return (sizeof(double) * 2) + itsTime.getSize() + + sizeof(uint) + MSH_STRING_SIZE(itsType);
}

//
// pack(buffer)
//
unsigned int Pointing::pack  (void* buffer)
{
	unsigned int offset = 0;

	memcpy((char*)buffer + offset, &itsAngle2Pi, sizeof(double));
	offset += sizeof(double);
	memcpy((char*)buffer + offset, &itsAnglePi, sizeof(double));
	offset += sizeof(double);
	offset += itsTime.pack((char*)buffer + offset);
	memcpy((char*)buffer + offset, &itsDuration, sizeof(uint));
	offset += sizeof(uint);
	MSH_PACK_STRING(buffer, offset, itsType);

	return (offset);
}

//
// unpack(buffer)
//
unsigned int Pointing::unpack(void *buffer)
{
	unsigned int offset = 0;

	memcpy(&itsAngle2Pi, (char*)buffer + offset, sizeof(double));
	offset += sizeof(double);
	memcpy(&itsAnglePi, (char*)buffer + offset, sizeof(double));
	offset += sizeof(double);
	offset += itsTime.unpack((char*)buffer + offset);
	memcpy(&itsDuration, (char*)buffer + offset, sizeof(uint));
	offset += sizeof(uint);
	MSH_UNPACK_STRING(buffer , offset, itsType);

	return (offset);
}

