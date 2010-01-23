//#
//#  beamctl.cc: implementation of beamctl class
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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include <APL/RSP_Protocol/RCUSettings.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_Control.h>

#include <iostream>
#include <sys/time.h>
#include <blitz/array.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>

#include "beamctl.h"

#define BEAMCTL_BEAM  			"beamctl_beam"
#define SKYSCAN_STARTDELAY 		30.0
#define	BEAMLET_RING_OFFSET		1000

using namespace blitz;
namespace LOFAR {
  using namespace RTC;
  using namespace CAL_Protocol;
  using namespace IBS_Protocol;
  using namespace GCF::TM;
  namespace BS {

//
// beamctl(...)
//
beamctl::beamctl(const string&	name) :
	GCFTask((State)&beamctl::checkUserInput, name), 
	itsCalServer	(0),
	itsBeamServer	(0),
	itsRCUmode		(-1)
{
	registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);
	registerProtocol(IBS_PROTOCOL, IBS_PROTOCOL_STRINGS);

	itsCalServer  = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER,  GCFPortInterface::SAP, CAL_PROTOCOL);
	itsBeamServer = new GCFTCPPort(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::SAP, IBS_PROTOCOL);
	ASSERTSTR(itsCalServer,  "Cannot allocate port for CalServer");
	ASSERTSTR(itsBeamServer, "Cannot allocate port for BeamServer");

	itsSkyScanTotalTime = globalParameterSet()->getInt32("beamctl.SKYSCAN_TOTAL_TIME", 3600);
	itsSkyScanPointTime = globalParameterSet()->getInt32("beamctl.SKYSCAN_POINT_TIME", 2);
	itsSkyScanWaitTime  = globalParameterSet()->getInt32("beamctl.SKYSCAN_WAIT_TIME",  10);
}

//
// ~beamctl()
//
beamctl::~beamctl()
{
	if (itsBeamServer) { itsBeamServer->close();	}
	if (itsCalServer)  { itsCalServer->close();		}
}

//
// checkUserInput(event, port)
//
GCFEvent::TResult beamctl::checkUserInput(GCFEvent& event, GCFPortInterface& port)
{
	LOG_DEBUG_STR("checkUserInput: " << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY:
		break;
	case F_INIT:
		// create memory for storing the arguments
		if (parseOptions(GCFScheduler::_argc, GCFScheduler::_argv) && checkOptions()) {
			TRAN(beamctl::con2beamserver);
		}
		else {
			TRAN(beamctl::final);
		}
		break;
	}

	return (GCFEvent::HANDLED);
}


//
// con2beamserver(event, port)
//
GCFEvent::TResult beamctl::con2beamserver(GCFEvent& event, GCFPortInterface& port)
{
	switch(event.signal) {
	case F_ENTRY: {
		cout << "Connecting to BeamServer... " << endl;
		itsBeamServer->autoOpen(5, 0, 2);
	}
	break;

	case F_CONNECTED: {
//		TRAN(beamctl::validate_pointings);
		TRAN(beamctl::con2calserver);
	}
	break;

	case F_DISCONNECTED: {
		// retry once every 10 seconds
		cout << "Can not connect to the BeamServer, is it running?" << endl;
		port.close();
		port.setTimer(10.0);
	}
	break;

	case F_TIMER: {
		// try again
		cout << "Still waiting for BeamServer... " << endl;
		itsBeamServer->autoOpen(5, 0, 2);
	}
	break;

	default:
		return (GCFEvent::NOT_HANDLED);
	break;
	}

	return (GCFEvent::HANDLED);
}

//
// con2calserver(event, port)
//
GCFEvent::TResult beamctl::con2calserver(GCFEvent& event, GCFPortInterface& port)
{
	switch(event.signal) {
	case F_ENTRY: {
		cout << "Connecting to CalServer... " << endl;
		itsCalServer->autoOpen(5, 0, 2);
	}
	break;

	case F_CONNECTED: {
		TRAN(beamctl::create_subarray);
	}
	break;

	case F_DISCONNECTED: {
		if (&port == itsBeamServer) {
			cout << "Lost connection with BeamServer, trying to reconnect." << endl;
			itsBeamServer->close();
			TRAN(beamctl::con2beamserver);
		}
		else {
			cout << "Can not connect to the CalServer, is it running?" << endl;
			port.close();
			port.setTimer(10.0);
		}
	}
	break;

	case F_TIMER: {
		// try again
		cout << "Stil waiting for CalServer... " << endl;
		itsCalServer->autoOpen(5, 0, 2);
	}
	break;

	default:
		return (GCFEvent::NOT_HANDLED);
	break;
	}

	return (GCFEvent::HANDLED);
}

//
// create_subarray(event, port)
//
GCFEvent::TResult beamctl::create_subarray(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	//
	// Create a new subarray
	//
	switch (event.signal) {
	case F_ENTRY: {
		CALStartEvent start;
		start.name   = BEAMCTL_BEAM + formatString("_%d", getpid());
		start.parent = itsAntSet;
		start.subset = getRCUMask();
		start.rcumode().resize(1);
		start.rcumode()(0).setMode((RSP_Protocol::RCUSettings::Control::RCUMode)itsRCUmode);

		LOG_INFO(formatString("Rcumode(dec)=%06d", start.rcumode()(0).getRaw()));
		LOG_INFO(formatString("Rcumode(hex)=%06X", start.rcumode()(0).getRaw()));
		LOG_INFO_STR("Creating subarray: " << start.name);

		itsCalServer->send(start);
	}
	break;

	case CAL_STARTACK: {
		CALStartackEvent ack(event);
		if (ack.status != CAL_Protocol::CAL_SUCCESS) {
			cerr << "Error: failed to start calibration" << endl;
			TRAN(beamctl::final);
		} else {
			cout << "Calserver accepted settings." << endl;
			TRAN(beamctl::create_beam);
		}
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		cerr << "Error: unexpected disconnect" << endl;
		TRAN(beamctl::final);
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}

//
// create_beam(event, port)
//
GCFEvent::TResult beamctl::create_beam(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	//
	// Create a new subarray
	//
	switch (event.signal) {
	case F_ENTRY: {
		IBSBeamallocEvent 	alloc;
		alloc.beamName	   = BEAMCTL_BEAM + formatString("_%d", getpid());
		alloc.antennaSet   = itsAntSet;
		alloc.rcumask	   = getRCUMask();
		// assume beamletnumbers are right so the ring can be extracted from those numbers.
		// when the user did this wrong the BeamServer will complain.
		alloc.ringNr	   = itsBeamlets.front() >= BEAMLET_RING_OFFSET;

		list<int>::iterator its = itsSubbands.begin();
		list<int>::iterator itb = itsBeamlets.begin();
		for (; its != itsSubbands.end() && itb != itsBeamlets.end(); ++its, ++itb) {
			if (((*itb) >= BEAMLET_RING_OFFSET ? 1 : 0) != alloc.ringNr) {	// all beamlets in the same ring?
				cerr << "Beamlet " << *itb << " does not lay in ring " << alloc.ringNr << endl;
				TRAN(beamctl::final);
				break;
			}
			alloc.allocation()[(*itb) % BEAMLET_RING_OFFSET] = (*its);
		}
		itsBeamServer->send(alloc);

		LOG_INFO_STR("name        : " << alloc.beamName);
		LOG_INFO_STR("antennaSet  : " << alloc.antennaSet);
		LOG_INFO_STR("rcumask     : " << alloc.rcumask);
		LOG_INFO_STR("ringNr      : " << alloc.ringNr);
//		LOG_INFO_STR(alloc.allocation);
	}
	break;

	case IBS_BEAMALLOCACK: {
		IBSBeamallocackEvent ack(event);
		if (ack.status != IBS_Protocol::IBS_NO_ERR) {
			cerr << "Error: " << errorName(ack.status) << endl;
			TRAN(beamctl::final);
		} else {
			cout << "BeamServer accepted settings" << endl;
			itsBeamHandle = ack.beamName;
			LOG_DEBUG(formatString("got beam_handle=%s for %s", itsBeamHandle.c_str(), ack.antennaGroup.c_str()));
			TRAN(beamctl::sendPointings);
		}
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		cerr << "Error: unexpected disconnect" << endl;
		TRAN(beamctl::final);
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}

//
// sendPointings(event, port)
//
GCFEvent::TResult beamctl::sendPointings(GCFEvent& event, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;
	static bool		sendingDigitalPts(true);
	static list<Pointing>::const_iterator	ptIter = itsDigPointings.begin();

	//
	// Create a new subarray
	//
	switch (event.signal) {
	case F_ENTRY: {
		// update iterator to current pointing
		if (sendingDigitalPts && ptIter == itsDigPointings.end()) {
			sendingDigitalPts = false;
			ptIter = itsAnaPointings.begin();
		}
		if (!sendingDigitalPts && ptIter == itsAnaPointings.end()) {
			cout << "All pointings sent and accepted" << endl;
			return (GCFEvent::HANDLED);
		}
			
		IBSPointtoEvent 	alloc;
		alloc.beamName = BEAMCTL_BEAM + formatString("_%d", getpid());
		alloc.pointing = *ptIter;
		alloc.analogue = !sendingDigitalPts;
		alloc.rank	   = 6;				// always less important than MAC scheduled beams
		itsBeamServer->send(alloc);

		cout << "sending pointing: " << *ptIter << endl;

		++ptIter;
	}
	break;

	case IBS_POINTTOACK: {
		IBSPointtoackEvent ack(event);
		if (ack.status != IBS_Protocol::IBS_NO_ERR) {
			cerr << "Error: " << errorName(ack.status) << endl;
			TRAN(beamctl::final);
		} else {
			TRAN(beamctl::sendPointings);	// tran to myself to exec the ENTRY state again.
		}
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		cerr << "Error: unexpected disconnect" << endl;
		TRAN(beamctl::final);
	}
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}




GCFEvent::TResult beamctl::final(GCFEvent& event, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(event.signal) {
			case F_ENTRY:
		GCFScheduler::instance()->stop();
		break;
			
			case F_EXIT:
		break;

			default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

void beamctl::send_direction(double	longitude, double	latitude, const string&	dirType, bool	isAnalogue)
{
	IBSPointtoEvent pointto;
	pointto.beamName = itsBeamHandle;
	pointto.pointing.setDirection(longitude, latitude);
	pointto.analogue = isAnalogue;
	// TODO UPDATE THIS TO THE NEW CASACORE INTERFACE
	if (dirType == "J2000") {
		pointto.pointing.setType(dirType);
	} else if (dirType == "AZEL") {
		pointto.pointing.setType(dirType);
	} else if (dirType == "LOFAR_LMN") {
		pointto.pointing.setType(dirType);
	} else if (dirType != "SKYSCAN") {
		LOG_FATAL_STR("Error: invalid coordinate type '" << dirType << "'");
		exit(EXIT_FAILURE);
	}

	if (dirType != "SKYSCAN") {
		itsBeamServer->send(pointto);
		return;
	}

	// SKYSCAN
	Timestamp time, end_time;
	time.setNow(SKYSCAN_STARTDELAY); // start after appropriate delay
	// TODO HOW TO CONVERT THIS?
//	pointto.pointing.setType(Pointing::LOFAR_LMN);

	// step through l and m
	int l_steps = static_cast<int>(longitude);
	int m_steps = static_cast<int>(latitude);
	end_time = time + static_cast<long>(itsSkyScanTotalTime);

	double m_increment = 2.0 / (m_steps - 1);
	double l_increment = 2.0 / (l_steps - 1);
	double eps = 5.6e-16;

	do { 
		for (double m = -1.0; m <= 1.0 + eps; m += m_increment) {
			for (double l = -1.0; l <= 1.0 + eps; l+= l_increment) {
				if (l*l+m*m <= 1.0 + eps) {
					pointto.pointing.setTime(time);
					pointto.pointing.setDirection(l, m);
					itsBeamServer->send(pointto);
					time = time + static_cast<long>(itsSkyScanPointTime); // advance seconds
				}
			}
		}
		pointto.pointing.setTime(time);
		pointto.pointing.setDirection(0.0, 0.0);
		itsBeamServer->send(pointto);
		time = time + static_cast<long>(itsSkyScanWaitTime); // advance seconds
	} while (time < end_time);
}


void beamctl::usage() const
{
	cout <<
		"Usage: beamctl <rcuspec> <dataspec> <digpointing> [<digpointing> ...] FOR LBA ANTENNAS\n"
		"       beamctl <rcuspec> <anapointing> [<anapointing> ...] [<dataspec> <digpointing> [<digpointing> ...]] FOR HBA ANTENNAS\n"
		"where:\n"
		"  <rcuspec>      = --antennaset [--rcus] --rcumode \n"
		"  <dataspec>     = --subbands --beamlets \n"
		"  <digpointing>  = --digdir \n"
		"  <anapointing>  = --anadir \n"
		"with option arguments: \n"
		"  --antennaset=name # name of the antenna (sub)field the RCU's are part of\n"
		"  --rcus=<set>      # optional subselection of RCU's\n"
		"  --rcumode=0..7    # RCU mode to use (may not conflict with antennaset\n"
		"  --subbands=<set>  # set of subbands to use for this beam\n"
		"  --beamlets=<list> # list of beamlets on which to allocate the subbands\n" 
		"                    # beamlet range = 0..247 when Serdes splitter is OFF\n"
		"                    # beamlet range = 0..247 + 1000..1247 when Serdes splitter is ON\n"
		"  --digdir=longitude,latitude,type[,duration]\n"
		"                    # lon,lat are floating point values specified in radians\n"
		"                    # type is SKYSCAN or olmost any other coordinate system\n"
		"                    # SKYSCAN will scan the sky with a L x M grid in the (l,m) plane\n"
		"  --anadir=longitude,latitude,type[,duration]\n"
		"                    # direction of the analogue HBA beam\n"
		"  --help            # print this usage\n"
		"\n"
		"The order of the arguments is trivial\n"
		"\n"
		"This utility connects to the CalServer to create a subarray of --array\n"
		"containing the selected RCU's. The CalServer sets those RCU's in the mode\n"
		"specified by --rcumode. Another connection is made to the BeamServer to create a\n"
		"beam on the created subarray pointing in the direction specified with --direction.\n"
	<< endl;
}

bitset<LOFAR::MAX_RCUS> beamctl::getRCUMask() const
{
	bitset<LOFAR::MAX_RCUS> mask;
	
	mask.reset();
	list<int>::const_iterator it;
	for (it = itsRCUs.begin(); it != itsRCUs.end(); ++it) {
		if (*it < LOFAR::MAX_RCUS)
			mask.set(*it);
	}
	return mask;
}

list<int> beamctl::strtolist(const char* str, int max) const
{
	string inputstring(str);
	char* start = (char*)inputstring.c_str();
	char* end   = 0;
	bool  range = false;
	long prevval = 0;
	list<int> resultset;

	resultset.clear();

	while (start) {
		long val = strtol(start, &end, 10); // read decimal numbers
		start = (end ? (*end ? end + 1 : 0) : 0); // advance
		if (val >= max || val < 0) {
			cerr << formatString("Error: value %ld out of range",val) << endl;
			resultset.clear();
			return resultset;
		}

		if (end) {
			switch (*end) {
				case ',':
				case 0: {
					if (range) {
						if (0 == prevval && 0 == val)
							val = max - 1;
						if (val < prevval) {
				cerr << "Error: invalid range specification" << endl;
							resultset.clear();
							return resultset;
						}
						for (long i = prevval; i <= val; i++)
							resultset.push_back(i);
					} 
					else {
						resultset.push_back(val);
					}
					range=false;
				}
				break;

				case ':':
					range=true;
				break;

				default:
					cerr << formatString("Error: invalid character %c",*end) << endl;
					resultset.clear();
					return resultset;
				break;
			}
		}
		prevval = val;
	}

	return resultset;
}

void beamctl::printList(list<int>&		theList) const
{
	list<int>::iterator		iter = theList.begin();
	list<int>::iterator		end  = theList.end();
	while (iter != end) {
		cout << *iter;
		++iter;
		if (iter != end) {
			cout << ",";
		}	
	}
	cout << endl;
}

bool beamctl::checkOptions()
{
	// antennaSet OR rcus must be specified.
	if (itsAntSet.empty() && itsRCUs.empty()) {
		cerr << "Error: antennaSet or rcu selection is required." << endl;
		return (false);
	}

	if (itsRCUmode < 0 || itsRCUmode > 7) {
		cerr << "Error: --rcumode=" << itsRCUmode << ", is out of range [0..7]." << endl;
		return (false);
	}

	// at least one direction must be entered
	if (itsAnaPointings.empty() && itsDigPointings.empty()) {
		cerr << "Error: no direction(s) specified." << endl;
		return (false);
	}

	// subbands must match beamlets
	if (itsSubbands.size() != itsBeamlets.size()) {
		cerr << "Error: the number of subbands must match the number of beamlets." << endl;
		return (false);
	}

	// if a digbeam is setup, we must have subbands
	if (!itsDigPointings.empty() && itsSubbands.empty()) {
		cerr << "No subbands specified for the digital beam." << endl;
		return (false);
	}

	return (true);
}

//
// parseOptions
//
bool beamctl::parseOptions(int	myArgc, char** myArgv)
{
	if (myArgc == 1) {
		usage();
		GCFScheduler::instance()->stop();
		return (false);
	}

	static struct option long_options[] = {
		{ "antennaset",	 required_argument, 0, 'a' },
		{ "rcus",      	 required_argument, 0, 'r' },
		{ "rcumode",   	 required_argument, 0, 'm' },
		{ "digdir", 	 required_argument, 0, 'D' },
		{ "anadir", 	 required_argument, 0, 'A' },
		{ "subbands",  	 required_argument, 0, 's' },
		{ "beamlets",  	 required_argument, 0, 'b' },
		{ "help",      	 no_argument,       0, 'h' },
		{ 0, 0, 0, 0 },
	};

	Timestamp		lastDigPtTime;
	Timestamp		lastAnaPtTime;
	lastDigPtTime.setNow();
	lastAnaPtTime.setNow();
	optind = 0; // reset option parsing
	while (true) {
		int option_index = 0;
		int c = getopt_long(myArgc, myArgv, "a:r:m:A:D:s:b:h", long_options, &option_index);
		if (c == -1) {
			break;
		}
		if (c != 'h') {		// only 'h' does not need an argument
			// note: --xxx  results in !optarg when it is the last option
			if (!optarg)
				continue;
			// note: --xxx=  result in optarg being empty
			//       --xxx --yyy=zzz  result in --yyy=zzz being the optarg of xxx !!!!!!!!
			if ((optarg[0]=='\0') || (optarg[0]=='-' && optarg[1]=='-')) { 
				cerr << "Error: missing value for option " << long_options[option_index].name << endl;
				continue;
			}
		}

		switch (c) {
		case 'a': {		// antennaset
			itsAntSet = optarg;
			cout << "antennaSet : " << itsAntSet << endl;
		}
		break;

		case 'r': {		// optional rcu subset
			itsRCUs = strtolist(optarg, LOFAR::MAX_RCUS);
			cout << "rcus     : ";  printList(itsRCUs);
		}
		break;

		case 'm': {		// optional rcumode
			itsRCUmode = atoi(optarg);
			cout << "rcumode  : " << itsRCUmode << endl;
		}
		break;

		case 'A': 
		case 'D':  {
			double	lat, lon;
			int		duration(0);	// forever
			char	dirType[20];
			int	nargs = sscanf(optarg, "%lf,%lf,%[A-Za-z0-9],%d", &lon, &lat, dirType, &duration);
			if (nargs < 3) {
				cerr << "Error: invalid number of parameters for " << long_options[option_index].name << endl;
			} else {
				if (c == 'D')  {
					Pointing	pt(lon, lat, dirType, lastDigPtTime, duration);
					itsDigPointings.push_back(pt);
					cout << "digdir   : " << pt << endl;
					lastDigPtTime = pt.endTime();
				}
				else {
					Pointing	pt(lon, lat, dirType, lastAnaPtTime, duration);
					itsAnaPointings.push_back(pt);
					cout << "anadir   : " << pt << endl;
					lastAnaPtTime = pt.endTime();
				}
			}
		}
		break;

		case 's': {
			itsSubbands = strtolist(optarg, LOFAR::MAX_SUBBANDS);
			cout << "subbands : "; printList(itsSubbands);
		}
		break;

		case 'b': {
			itsBeamlets = strtolist(optarg, BEAMLET_RING_OFFSET + LOFAR::MAX_BEAMLETS);
			cout << "beamlets : "; printList(itsBeamlets);
		}
		break;

		case 'h':
		default:
			usage();
			GCFScheduler::instance()->stop();
			return (false);
		break;
		} // switch (c)
	} 

	return (true);

}

  }; // namespace BS
}; // namespace LOFAR

using namespace LOFAR;
using namespace BS;
using namespace GCF::TM;

//
// main
//
int main(int argc, char** argv)
{
	GCFScheduler::instance()->init(argc, argv, "beamctl");

	try {
		beamctl beamctlTask("beamctl");
		beamctlTask.start();
		GCFScheduler::instance()->run();
	} catch (Exception& e) {
		cerr << "Exception: " << e.text() << endl;
		exit(EXIT_FAILURE);
	}

	LOG_INFO("Normal termination of program");

	return (0);
}
