//#  tKeyValueLogger.cc: one_line_description
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
#include <Common/lofar_string.h>
#include <MACIO/MACServiceInfo.h>
#include <MACIO/KVT_Protocol.ph>
#include <GCF/TM/EventPort.h>

using namespace LOFAR;
using namespace LOFAR::GCF::TM;

int main (int	argc, char*		argv[])
{
	INIT_LOGGER(basename(argv[0]));

	EventPort		thePort(MAC_SVCMASK_KVTLOGGER, false, KVT_PROTOCOL);	// false = client

	KVTRegisterEvent	registerEvent;
	registerEvent.obsID = 5;
	registerEvent.name  = argv[0];
	thePort.send(&registerEvent);
	KVTRegisterAckEvent		registerAck(thePort.receive());

	sleep(2);

	KVTSendMsgEvent		msgEvent;
	msgEvent.seqnr = 1;
	msgEvent.key   = "LOFAR.ObsSW.ObservationControl.OnlineControl.OLAP.InputAppl.packageLoss{1198368555}";
	msgEvent.value = toString(25);
	thePort.send(&msgEvent);
	KVTSendMsgAckEvent		msgAck(thePort.receive());

	sleep(2);

	// send a bunch of messages
	for (int	i = 0; i < 20; i++) {
		msgEvent.seqnr = i;
		msgEvent.value = toString(25+i);
		thePort.send(&msgEvent);
		KVTSendMsgAckEvent		msgAck(thePort.receive());
	}
		
	sleep(2);

	// now send the same bunch as to pools
	KVTSendMsgPoolEvent		poolEvent;
	poolEvent.seqnr = 1;
	poolEvent.msgCount = 20;
	for (int i = 0; i < 20; i++) {
		poolEvent.keys.theVector.push_back("LOFAR.ObsSW.ObservationControl.OnlineControl.OLAP.InputAppl.packageLoss{1198368555}");
		poolEvent.values.theVector.push_back(toString(50+i));
	}
	thePort.send(&poolEvent);
	KVTSendMsgPoolAckEvent		poolAck(thePort.receive());

	sleep (2);

	return (0);
}

