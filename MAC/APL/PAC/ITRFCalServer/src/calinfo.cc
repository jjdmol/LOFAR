//#  calinfo: Utilities for showing the subarrays the Calserver is managing
//#
//#  Copyright (C) 2007
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
//#  $Id: calinfo.cc 14634 2009-12-09 08:15:10Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <APL/ICAL_Protocol/ICAL_Protocol.ph>
#include <MACIO/MACServiceInfo.h>
#include "calinfo.h" 

namespace LOFAR {
  using namespace GCF;
  using namespace GCF::TM;
  namespace CAL {

calinfo::calinfo(const string&	name) : 
	GCFTask((State)&calinfo::initial, "calinfo"),
	itsSAname(name)
{
	registerProtocol(ICAL_PROTOCOL, ICAL_PROTOCOL_STRINGS);

	itsCalPort.init(*this, MAC_SVCMASK_CALSERVER, GCFPortInterface::SAP, ICAL_PROTOCOL);
}

calinfo::~calinfo()
{}

GCFEvent::TResult	calinfo::initial(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("initial:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_INIT:
		break;
	case F_ENTRY:
		itsCalPort.open();
		break;
	case F_CONNECTED:
		TRAN(calinfo::getInfo);
		break;
	case F_DISCONNECTED:
		LOG_ERROR("No connection with CalServer");
		port.setTimer(1.0);
		port.close();
		break;
	case F_TIMER:
		port.open();
		break;
	default:
		return (GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

GCFEvent::TResult	calinfo::getInfo(GCFEvent&	event, GCFPortInterface&	port)
{
	LOG_DEBUG_STR("getInfo:" << eventName(event) << "@" << port.getName());

	switch (event.signal) {
	case F_ENTRY: {
			ICALGetsubarrayEvent		request;
			request.subarrayname = itsSAname;
			itsCalPort.send(request);
		}
		break;

	case ICAL_GETSUBARRAYACK: {
		ICALGetsubarrayackEvent		answer(event);
		if (answer.status != ICAL_Protocol::ICAL_SUCCESS) {
			cout << "CalServer returned error " << answer.status << endl;
			TRAN(calinfo::finish);
			break;
		}
		cout << "Received " << answer.subarraymap.size() << " entries" << endl;
		SubArrayMap::iterator	iter = answer.subarraymap.begin();
		SubArrayMap::iterator	end  = answer.subarraymap.end();
		while (iter != end) {
			cout << "name          :" << iter->first << endl;
			cout << "spectralwindow:" << iter->second->getSPW().getName() << endl;
			cout << "RCUmask       :" << iter->second->getRCUMask() << endl;
			iter++;
		}
		TRAN(calinfo::finish);
	}
	break;

	case F_DISCONNECTED:
		LOG_ERROR("Lost connection with CalServer");
		TRAN(calinfo::finish);
		port.close();
		break;

	default:
		return (GCFEvent::NOT_HANDLED);
	}

	return (GCFEvent::HANDLED);
}

GCFEvent::TResult	calinfo::finish(GCFEvent&	event, GCFPortInterface&	port) {
	GCFScheduler::instance()->stop();
	return (GCFEvent::HANDLED);
}




  } // namespace CAL
} // namespace LOFAR

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::CAL;

int main(int	argc,	char*	argv[])
{
	string	saName;
	if (argc > 1) {
		saName = argv[1];
	}

	GCFScheduler::instance()->init(argc, argv, "calinfo");
	LOG_INFO(formatString("Program %s has started", argv[0]));

	CAL::calinfo		ciTask(saName);
	ciTask.start();

	GCFScheduler::instance()->run();

	return (0);
}
