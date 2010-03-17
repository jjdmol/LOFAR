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
#include <APL/BS_Protocol/Pointing.h>

#include <AMCBase/Converter.h>
#include <AMCBase/RequestData.h>
#include <AMCBase/ResultData.h>

#include <cmath>

using namespace LOFAR;
using namespace BS_Protocol;
using namespace AMC;

//
// Pointing()
//
Pointing::Pointing() : 
	m_angle0(0.0), m_angle1(0.0), m_time(), m_type(LOFAR_LMN)
{}

//
// Pointing(angle, angle, time, type)
//
Pointing::Pointing(double angle0, double angle1, RTC::Timestamp time, Type type) :
	m_angle0(angle0), m_angle1(angle1), m_time(time), m_type(type)
{}

//
// ~Pointing()
//
Pointing::~Pointing()
{}

//
// convert(converter, position, requestedType)
//
// Converts a pointing to another coordinatesystem.
// Supports: 
//	J2000 --> AZEL
//	J2000 --> LMN
//	AZEL  --> LMN
//
Pointing Pointing::convert(Converter* conv, Position* pos, Pointing::Type reqType)
{
	if (getType() == reqType) {	// no conversion needed?
		return (*this);
	}

	ASSERT(conv && pos);		// always need converter and position

	// start with current coordinates
	// angle0 = longitude = RA, Az or l
	// angle1 = latitude  = Dec, El or m
	Direction resultDir = Direction(angle0(), angle1());
	double
		mjd      = 0.0,
		fraction = 0.0,
		az       = angle0(),
		el       = angle1(),
		l        = angle0(),
		m        = angle1(),
		n        = 0.0;

	// setup the AMC classes.
	time().convertToMJD(mjd, fraction); // mjd,fraction in UTC
	RequestData 	request(resultDir, *pos, Epoch(mjd, fraction));
	ResultData 		resultdata;

	switch (getType()) {		// determines when conversion chain begins
	case J2000:
		// convert J2000 to LMN
		LOG_TRACE_VAR_STR("J2000(rad)=(" << resultDir.longitude() << ", " 
										 << resultDir.latitude() << ")");
		conv->j2000ToAzel(resultdata, request);
		resultDir = resultdata.direction[0];
		az = resultDir.longitude();
		el = resultDir.latitude();

		// now convert from azel to lmn by falling through to AZEL label
		// Note: break omitted intentionally 

	case AZEL:
		// convert AZEL to LMN
		LOG_TRACE_VAR_STR("AZEL(rad)=(" << az << ", " << el << ")");

		if (reqType == AZEL) {					// return AZEL pointing?
			return (Pointing(az, el, time(), AZEL));
		}

		// See LOFAR-ASTRON-MEM-105, top of page 7
		l = ::cos(el) * ::sin(az);
		m = ::cos(el) * ::cos(az);
		n = ::sin(el);

		// Note: break omitted intentionally

	case LOFAR_LMN:
		// coordinates are already in LMN format
		LOG_DEBUG_STR("lmn=(" << l << ", " << m << ", " << n << ")");
		break;

	default:
		LOG_FATAL("invalid switch value");
		exit(EXIT_FAILURE);
		break;
	}

	return Pointing(l, m, time(), LOFAR_LMN);	// return LOFAR_LMN pointing
}

// -------------------- routines for streaming the object --------------------
//
// getSize()
//
unsigned int Pointing::getSize()
{
	return (sizeof(double) * 2) + m_time.getSize() + sizeof(uint8);
}

//
// pack(buffer)
//
unsigned int Pointing::pack  (void* buffer)
{
	unsigned int offset = 0;

	memcpy((char*)buffer + offset, &m_angle0, sizeof(double));
	offset += sizeof(double);
	memcpy((char*)buffer + offset, &m_angle1, sizeof(double));
	offset += sizeof(double);
	offset += m_time.pack((char*)buffer + offset);
	uint8 	type = m_type;
	memcpy((char*)buffer + offset, &type, sizeof(uint8));
	offset += sizeof(uint8);

	return (offset);
}

//
// unpack(buffer)
//
unsigned int Pointing::unpack(void *buffer)
{
	unsigned int offset = 0;

	memcpy(&m_angle0, (char*)buffer + offset, sizeof(double));
	offset += sizeof(double);
	memcpy(&m_angle1, (char*)buffer + offset, sizeof(double));
	offset += sizeof(double);
	offset += m_time.unpack((char*)buffer + offset);
	uint8 	type = 0;
	memcpy(&type, (char*)buffer + offset, sizeof(uint8));
	m_type = (Type)type;
	offset += sizeof(uint8);

	return (offset);
}

