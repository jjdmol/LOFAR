//#  clkctl.cc: Main entry for the ClockControl controller.
//#
//#  Copyright (C) 2009
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
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <MACIO/MACServiceInfo.h>
#include <APL/ClockProtocol/Clock_Protocol.ph>

#include <getopt.h>

#include "clkctl.h"

namespace LOFAR {
	using namespace LOFAR::GCF::TM;

ClkCtl::ClkCtl(const string& name) :
	GCFTask((State)&ClkCtl::doCommand, name)
{
	registerProtocol (CLOCK_PROTOCOL, CLOCK_PROTOCOL_STRINGS);
}

ClkCtl::~ClkCtl()
{
	if (itsCommand)
		delete itsCommand;
}

GCFEvent::TResult ClkCtl::doCommand(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("doCommand: " << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
		itsCommand = parseOptions(GCFScheduler::_argc, GCFScheduler::_argv);
		if (!itsCommand) {
			doHelp();
			GCFScheduler::instance()->stop();
		}
		itsClientPort = new GCFTCPPort(*this, MAC_SVCMASK_CLOCKCTRL, GCFPortInterface::SAP, CLOCK_PROTOCOL);
		itsClientPort->autoOpen(5,0,1);
	}
	break;
	
	case F_CONNECTED:
		itsClientPort->send(*itsCommand);
		break;

	case F_DISCONNECTED:
		cout << "Cannot connect to the ClockController" << endl;
		GCFScheduler::instance()->stop();
		break;
	
	case CLKCTRL_GET_CLOCK_ACK: {
		CLKCTRLGetClockAckEvent	ack(event);
		cout << "Clock is set to " << ack.clock << "MHz" << endl;
		GCFScheduler::instance()->stop();
	}
	break;

	case CLKCTRL_SET_CLOCK_ACK: {
		CLKCTRLSetClockAckEvent	ack(event);
		cout << "Setting the clock was " << ((ack.status == CLKCTRL_NO_ERR) ? "" : "NOT ") << "succesful" << endl;
		GCFScheduler::instance()->stop();
	}
	break;

	case CLKCTRL_GET_SPLITTERS_ACK: {
		CLKCTRLGetSplittersAckEvent	ack(event);
		cout << "Splitters: " << ack.splitters << endl;
		GCFScheduler::instance()->stop();
	}
	break;

	case CLKCTRL_SET_SPLITTERS_ACK: {
		CLKCTRLSetSplittersAckEvent	ack(event);
		cout << "Setting the splitters was " << ((ack.status == CLKCTRL_NO_ERR) ? "" : "NOT ") << "succesful" << endl;
		CLKCTRLGetSplittersEvent	getEvent;
		itsClientPort->send(getEvent);
	}
	break;

	}
	return (GCFEvent::HANDLED);
}


GCFEvent* ClkCtl::parseOptions(int argc, char** argv)
{
	static struct option long_options[] = {
		{ "getclock",		no_argument,		0,	'c'	},
		{ "setclock",		required_argument,	0,	'C'	},
		{ "getsplitters",	no_argument,		0,	's'	},
		{ "setsplitters",	required_argument,	0,	'S'	},
		{ "help",			no_argument,		0,	'h' },
		{ 0,				0,					0,	0,	}
	};

	optind = 0;
	int	option_index = 0;
	int c = getopt_long(argc, argv, "cC:sS:h", long_options, &option_index);	
	if (c == -1) {
		return(0);
	}

	switch (c) {
	case 'c':
		return(new CLKCTRLGetClockEvent());
		break;

	case 'C': {
		CLKCTRLSetClockEvent* event = new CLKCTRLSetClockEvent();
		event->clock = atoi(optarg);
		return(event);
	}
	break;

	case 's':
		return (new CLKCTRLGetSplittersEvent());
		break;

	case 'S': {
		CLKCTRLSetSplittersEvent* event = new CLKCTRLSetSplittersEvent();
		event->splittersOn = atoi(optarg);
		return (event);
	}
	break;

	case 'h':
		break;
	default:
		break;
	}

	return (0);
}

void ClkCtl::doHelp()
{
	cout << endl;
	cout << endl;
	cout << "clkctl syntax:" << endl;
	cout << "clkctl --getclock" << endl;
	cout << "clkctl --setclock=160|200" << endl;
	cout << "clkctl --getsplitters" << endl;
	cout << "clkctl --setsplitters=0|1" << endl;
	cout << endl;
}

} // namespace LOFAR

using namespace LOFAR;
using namespace LOFAR::GCF::TM;

int main(int argc, char* argv[])
{
	if (argc < 2) {
		return (1);
	}

	// args: cntlrname, parentHost, parentService
	GCFScheduler::instance()->init(argc, argv, "clkctl");

	ClkCtl		cc(argv[1]);
	cc.start(); 	// make initial transition

	GCFScheduler::instance()->run();
	return 0;
}

