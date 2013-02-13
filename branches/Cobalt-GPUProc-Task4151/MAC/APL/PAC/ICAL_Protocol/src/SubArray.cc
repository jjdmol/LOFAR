//#  -*- mode: c++ -*-
//#  SubArray.cc: implementation of the SubArray class
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
//#  $Id: SubArray.cc 14564 2009-11-30 08:32:42Z loose $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/hexdump.h>
#include <ApplCommon/AntennaSets.h>

#include <MACIO/Marshalling.tcc>
#include <APL/RTCCommon/MarshallBlitz.h>
#include <APL/ICAL_Protocol/SubArray.h>

namespace LOFAR {
  namespace ICAL {
	using namespace blitz;

// forward declaration
class CalibrationInterface;
    
//
// SubArray()
//
SubArray::SubArray() : 
	itsSPW("undefined", 0, 0, false),
	itsGains(new AntennaGains)
{
	LOG_TRACE_OBJ("SubArray()");
}

//
// SubArray(name, antennaSet, select, freq, nyquist, nrSubbands, rcuControl)
//
SubArray::SubArray (const string&			name,
					const string&			antennaSet,
					RCUmask_t				RCUmask,
					bool					LBAfilterOn,
					double 					sampling_frequency,
					int    					nyquist_zone) :
	itsName		  (name),
	itsAntennaSet (antennaSet),
	itsSPW		  (name + "_spw", sampling_frequency, nyquist_zone, LBAfilterOn),
	itsRCUmask	  (RCUmask & globalAntennaSets()->RCUallocation(antennaSet))
{
	LOG_DEBUG(formatString("SubArray(%s,%f,%d,%s)", 
							name.c_str(), sampling_frequency, nyquist_zone, (LBAfilterOn ? "ON" : "OFF")));

	// create calibration result objects [ant x pol x subbands]
	itsGains = new AntennaGains(itsRCUmask.count(), MAX_SUBBANDS); // TODO: does this work with non contiguous RCUmasks????
	ASSERT(itsGains);
	// fill rcumode array
	string	RCUinputs(globalAntennaSets()->RCUinputs(itsAntennaSet));
	itsRCUmodes.resize(MAX_RCUS);
	itsRCUmodes = 0;
	for (int rcu = 0; rcu < MAX_RCUS; rcu++) {
		if (itsRCUmask.test(rcu)) {
			switch (RCUinputs[rcu]) {
			case 'l': itsRCUmodes(rcu) = (LBAfilterOn ? 2 : 1);	break;
			case 'h': itsRCUmodes(rcu) = (LBAfilterOn ? 4 : 3);	break;
			case 'H': itsRCUmodes(rcu) = itsSPW.rcumodeHBA();	break;
			case '.': itsRCUmodes(rcu) = 0; 						break;
			default: ASSERTSTR(false, "RCUinput #" << rcu << " contains illegal specification");
			} // switch
			itsRCUuseFlags.set(itsRCUmodes(rcu));
		} // if
	} // for
}

SubArray::~SubArray()
{
	LOG_DEBUG_STR("SubArray destructor: " << itsName);	
	if (itsGains) {
		delete itsGains;
	}
}

SubArray& SubArray::operator=(const SubArray& that)
{
	LOG_DEBUG_STR("SubArray operator= : " << that.itsName);	

	if (this != &that) {
		itsName			= that.itsName;
		itsAntennaSet	= that.itsAntennaSet;
		itsSPW 			= that.itsSPW;
		itsRCUmask 		= that.itsRCUmask;
		itsRCUmodes.resize(that.itsRCUmodes.shape());
		itsRCUmodes		= that.itsRCUmodes.copy();
		itsGains 		= that.itsGains->clone();
	}

	return (*this);
}

bool SubArray::usesRCUmode(int	rcumode) const 
{ 
	ASSERTSTR(rcumode >= 0 && rcumode <= NR_RCU_MODES, "RCUmode must be in the range 0..7");

	return (itsRCUuseFlags.test(rcumode)); 
}

//
// RCUMask(rcumode)
//
RCUmask_t	 SubArray::RCUMask(uint	rcumode) const
{
	RCUmask_t	result;
	if (!itsRCUuseFlags.test(rcumode)) {	// is this mode used anywhere?
		return (result);
	}

	// mode is used somewhere, is this the only used mode?
	if (itsRCUuseFlags.count() == 1) {
		return (itsRCUmask);
	}

	// several modes are used, compose the right mask.
	for (int rcu = 0; rcu < MAX_RCUS; rcu++) {
		if (itsRCUmodes(rcu) == rcumode) {
			result.set(rcu);
		}
	} // for
	return (result);
}

//
// writeGains()
//
void SubArray::writeGains()
{
	time_t now   = time(0);
	struct tm* t = gmtime(&now);
	char filename[PATH_MAX];

	snprintf(filename, PATH_MAX, "%s_%04d%02d%02d_%02d%02d%02d_gain_%dx%dx%d.dat",
						itsName.c_str(),
						t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
						t->tm_hour, t->tm_min, t->tm_sec,
						itsGains->getGains().extent(firstDim),
						itsGains->getGains().extent(secondDim),
						itsGains->getGains().extent(thirdDim));
	LOG_DEBUG_STR("writeGains(" << name() << ") to " << filename);

	FILE* gainFile = fopen(filename, "w");

	if (!gainFile) {
		LOG_ERROR_STR("failed to open file: " << filename);
		return;
	}

	if (fwrite(itsGains->getGains().data(), sizeof(complex<double>), itsGains->getGains().size(), gainFile) != 
			(size_t)itsGains->getGains().size()) {
		LOG_ERROR_STR("failed to write to file: " << filename);
	}

	(void)fclose(gainFile);
}

//
// print function for operator<<
//
ostream& SubArray::print (ostream& os) const
{
	os << "SubArray " << itsName << ":AntennaSet=" << itsAntennaSet << ", SPW={" << itsSPW;
	os << "}, RCUmask=" << itsRCUmask << ", RCUmodeFlags=" << itsRCUuseFlags;
	return (os);
}

//
// ---------- pack and unpack routines ----------
//
size_t SubArray::getSize() const
{
  return
      MSH_size(itsName)
    + MSH_size(itsAntennaSet)
    + MSH_size(itsRCUmask)
    + itsSPW.getSize();
}

size_t SubArray::pack(char* buffer) const
{
	size_t offset = 0;

	MSH_pack(buffer, offset, itsName);
	MSH_pack(buffer, offset, itsAntennaSet);
	MSH_pack(buffer, offset, itsRCUmask);
	offset += itsSPW.pack(buffer + offset);

	return offset;
}

size_t SubArray::unpack(const char* buffer)
{
	size_t offset = 0;

	MSH_unpack(buffer, offset, itsName);
	MSH_unpack(buffer, offset, itsAntennaSet);
	MSH_unpack(buffer, offset, itsRCUmask);
	offset += itsSPW.unpack(buffer + offset);

	return offset;
}

// -------------------- SubArrayMap --------------------

size_t SubArrayMap::getSize() const
{
	return MSH_size(*this);
}

size_t SubArrayMap::pack(char* buffer) const
{
	size_t	offset = 0;
	MSH_pack(buffer, offset, (*this));
	return (offset);
}

size_t SubArrayMap::unpack(const char* buffer)
{
	size_t offset = 0;
	MSH_unpack(buffer, offset, (*this));
	return (offset);
}

  } // namespace ICAL
} // namespace LOFAR
