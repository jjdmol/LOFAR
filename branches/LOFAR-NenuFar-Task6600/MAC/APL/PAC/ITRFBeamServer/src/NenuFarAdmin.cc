//#  NenuFarAdmin.h: implementation of the NenuFarAdmin class
//#
//#  Copyright (C) 2014
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
//#  $Id: NenuFarAdmin.cc 16850 2010-12-06 08:23:46Z schoenmakers $

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_bitset.h>
#include "NenuFarAdmin.h"

using namespace LOFAR;
using namespace BS;
using namespace IBS_Protocol;
using namespace RTC;
using namespace std;

//
// ~NenuFarAdmin
//
NenuFarAdmin::~NenuFarAdmin()
{
	// clear the pointings
	itsBeams.clear();
}

//
// addBeam(...)
//
void NenuFarAdmin::addBeam(const string& 				 beamName, 
						   const string& 				 antennaSet, 
						   const RCUmask_t&				 RCUselection, 
						   int 							 rank, 
						   const IBS_Protocol::Pointing& pointing, 
						   const vector<string>&		 extraInfo)
{
	if (!itsIsActive) {
		return;
	}

	LOG_INFO_STR("NenuFarAdmin: adding " << beamName);
	itsBeams.push_back(BeamInfo(beamName, antennaSet, RCUselection, rank, pointing, extraInfo));
}


//
// abortBeam(const string& beamName)
//
bool NenuFarAdmin::abortBeam(const string& beamName)
{
	if (!itsIsActive) {
		return(false);
	}

	list<BeamInfo>::iterator	iter = itsBeams.begin();
	list<BeamInfo>::iterator	end  = itsBeams.end();
	while (iter != end) {
		if (iter->beamName == beamName) {
			LOG_INFO_STR("NenuFarAdmin: aborting " << beamName);
			if (iter->comm_state == BeamInfo::BS_NONE) {
				LOG_INFO_STR("NenuFarAdmin: removing " << beamName);
				itsBeams.erase(iter);
			}
			else {
				iter->beam_state = BeamInfo::BS_ABORT;
			}
			return (true);
		}
		++iter;
	}
	return (false);
}

//
// abortAllBeams()
//
void NenuFarAdmin::abortAllBeams()
{
	if (!itsIsActive) {
		return;
	}

	LOG_INFO("NenuFarAdmin: aborting all beams");
	itsAllBeamsAborted = true;
	itsBeams.clear();
}

//
// firstCommand(time_t)
//
NenuFarAdmin::BeamInfo NenuFarAdmin::firstCommand(const time_t time)
{
	// first cleanup beams that became obsolete (now>=endTime && comm_state!=bs_new)
	list<BeamInfo>	tmpBeamList;		 // NOTE: 'erase' reorders the elements of a list!!!
	tmpBeamList.swap(itsBeams);
	list<BeamInfo>::iterator	iter = tmpBeamList.begin();
	list<BeamInfo>::iterator	end  = tmpBeamList.end();
	while (iter != end) {
		if (time < iter->endTime || iter->comm_state == BeamInfo::BS_NEW) {
			// check state of beam before copying it.
			if (time >= iter->endTime && iter->beam_state != BeamInfo::BS_ABORT) {
				iter->beam_state = BeamInfo::BS_ENDED;
			}
			itsBeams.push_back(*iter);
		}
		else {
			LOG_INFO_STR(iter->beamName << " became obsolete.");
		}
		++iter;
	}
	tmpBeamList.clear();

	// anything left?
	if (itsBeams.empty()) {
		return (BeamInfo());
	}

	// any aborted beams?
	iter = itsBeams.begin();
	end  = itsBeams.end();
	while (iter != end) {
		if (iter->beam_state == BeamInfo::BS_ABORT) {
			return (BeamInfo(*iter));
		}
		++iter;
	}

	// no aborted beams, check for other differences in state
	iter = itsBeams.begin();
	end  = itsBeams.end();
	while (iter != end) {
		if (iter->beam_state > iter->comm_state) {
			return (BeamInfo(*iter));
		}
		++iter;
	}

	return (BeamInfo());
}

//
// setCommState(beam, state)
//
bool	NenuFarAdmin::setCommState(const string& beamName, int new_comm_state) 
{
	// sanity check
	if (new_comm_state < BeamInfo::BS_NEW || new_comm_state > BeamInfo::BS_ABORT) {
		return (false);
	}

	list<BeamInfo>::iterator	iter = itsBeams.begin();
	list<BeamInfo>::iterator	end  = itsBeams.end();
	while (iter != end) {
		if (iter->beamName == beamName) {
			iter->comm_state = new_comm_state;
			LOG_INFO_STR("NenuFarAdmin: commstate of " << iter->name() << " set to " << BeamInfo::stateName(new_comm_state));
			// final state reached? delete the entry.
			if (new_comm_state == BeamInfo::BS_ABORT) {
				itsBeams.erase(iter);
			}
			return (true);
		}
		++iter;
	}

	return (false);
}

//
// asParset()
//
ParameterSet	NenuFarAdmin::BeamInfo::asParset() const
{
	ParameterSet	ps;
	ps.add("beamName", 	 beamName);
	ps.add("antennaSet", antennaSet);
	ps.add("rcus", 		 RCUselection.to_string<char,char_traits<char>,allocator<char> >());
	ps.add("angle2Pi", 	 toString(angle2Pi));
	ps.add("anglePi", 	 toString(anglePi));
	ps.add("coordFrame", coordFrame);
	ps.add("rankNumber", toString(rankNr));
	ps.add("startTime",  toString(startTime));
	ps.add("stopTime", 	 toString(endTime));
	for (size_t i = 0; i < extra.size(); ++i) {
		size_t	pos = extra[i].find('=');
		if (pos == string::npos) {
			LOG_ERROR_STR("Extra argument missing equal-sign: " << extra[i]);
		}
		else {
			ps.add(extra[i].substr(0,pos), extra[i].substr(pos+1));
		}
	} // for
	return (ps);
}

//
// print function for operator<<
//
ostream& NenuFarAdmin::print (ostream& os) const
{
	list<BeamInfo>::const_iterator	iter = itsBeams.begin();
	list<BeamInfo>::const_iterator	end  = itsBeams.end();
	while (iter != end) {
		os << "# " << iter->name() << " bs=" << iter->beamState() << ", cs=" << iter->commState() << endl;
		++iter;
	}
	return (os);
}
