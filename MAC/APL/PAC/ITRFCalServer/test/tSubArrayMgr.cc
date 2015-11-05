//#  Class.cc: one_line_description
//#
//#  Copyright (C) 2010
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
//#  $Id: tAnaBeamMgr.cc 15142 2010-03-05 10:25:51Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>

#include <blitz/array.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <APL/ICAL_Protocol/ICAL_Protocol.ph>
#include <ITRFCalServer/SubArrayMgr.h>

using namespace LOFAR;
using namespace ICAL;
using namespace GCF::TM;

class dummyTask : public GCFTask 
{ 
public:
  dummyTask(const string& name) : GCFTask((State)&dummyTask::dummyFunc, name) {}
  GCFEvent::TResult dummyFunc(GCFEvent&, GCFPortInterface&) { return (GCFEvent::HANDLED); }
};


int main(int	argc, char*	argv[]) 
{
	INIT_LOGGER("tSubArrayMgr");

	// first construct a SubArrayMgr instance
	SubArrayMgr		SAM;

	// ----- Try calling some functions on empty admin. -----
	LOG_INFO("----- Testing some function on an empty admin -----");
	RCUmask_t	RCUresult;
	RCUresult = SAM.getRCUs(5);
	ASSERTSTR(!RCUresult.any(), "Empty admin should not contain RCU's: " << RCUresult);
	LOG_INFO("getRCUs(5) on empty admin is OK");

	ASSERTSTR(!SAM.scheduleRemove("notThere"), "Empty admin should return 'false' on remove-command");
	LOG_INFO("scheduleRemove(notThere) on empty admin is OK");

	ASSERTSTR(!SAM.getByName("notThere"), "Empty admin should return NULL on getByName-command");
	LOG_INFO("getByName(notThere) on empty admin is OK");

	SubArrayMap	saMap = SAM.getSubArrays("");
	ASSERTSTR(saMap.empty(), "getSubArrays should be empty with empty admin");
	LOG_INFO("getSubArrays() on empty admin is OK");

	// ----- basic add test -----
	LOG_INFO("----- Add subarray with rcus 0..23 for mode 4 -----");
	RCUmask_t	rcuMask1;
	for (int rcu = 0; rcu < 24; rcu++) rcuMask1.set(rcu);
	SubArray*	SA1   = new SubArray("firstArr", "LBA_INNER", rcuMask1, true, 200.0e6, 1);
	GCFTask*	dTask = new dummyTask("dummyTask");
	GCFTCPPort*	port1 = new GCFTCPPort(*dTask, MAC_SVCMASK_CALSERVER, GCFPortInterface::SAP, ICAL_PROTOCOL);
	LOG_INFO_STR("Adding subarray: " << *SA1);
	SAM.scheduleAdd(SA1, port1);

	LOG_INFO("  --- SubArray may not be visible before creater() is called ---");
	RCUresult = SAM.getRCUs(4);
	ASSERTSTR(!RCUresult.any(), "Inactive SubArray should not contain RCU's for rcumode 4: " << RCUresult);
	LOG_INFO("getRCUs(4) on admin with one inactive subArray is OK");

	RCUresult = SAM.getRCUs(3);
	ASSERTSTR(!RCUresult.any(), "Inactive SubArray should not contain RCU's for rcumode 3: " << RCUresult);
	LOG_INFO("getRCUs(3) on admin with one inactive subArray is OK");

	LOG_INFO("  --- SubArray must be visible after creater() is called ---");
	SAM.activateArrays();
	RCUresult = SAM.getRCUs(4);
	ASSERTSTR(rcuMask1 == RCUresult, "Added rcumask differs from retrieved one: " << rcuMask1 << " vs. " << RCUresult);
	LOG_INFO("getRCUs(4) on admin with one active subArray is OK");

	RCUresult = SAM.getRCUs(3);
	ASSERTSTR(!RCUresult.any(), "Added SubArray should not contain RCU's for rcumode 3: " << RCUresult);
	LOG_INFO("getRCUs(3) on admin with one active subArray is OK");

	// ----- add multimode subarray -----
	LOG_INFO("----- Add subarray with rcus 40..47 for mode 2/4 -----");
	RCUmask_t	rcuMask2;
	for (int rcu = 40; rcu < 48; rcu++) rcuMask2.set(rcu);
	SubArray*	SA2   = new SubArray("secondArr", "LBA_X", rcuMask2, true, 200.0e6, 1);
	LOG_INFO_STR("Adding subarray: " << *SA2);
	SAM.scheduleAdd(SA2, port1);

	SAM.activateArrays();
	LOG_INFO_STR("getRCUs(1):" << SAM.getRCUs(1));
	LOG_INFO_STR("getRCUs(2):" << SAM.getRCUs(2));
	LOG_INFO_STR("getRCUs(3):" << SAM.getRCUs(3));
	LOG_INFO_STR("getRCUs(4):" << SAM.getRCUs(4));
	LOG_INFO_STR("getRCUs(5):" << SAM.getRCUs(5));

	// ----- testing getByName(...) -----
	SubArray*	sap = SAM.getByName("notThere");
	ASSERTSTR(!sap, "function getByName does not work properly, should have returned a null pointer");
	sap = SAM.getByName("firstArr");
	ASSERTSTR(sap && sap->name()=="firstArr", "function getByName does not work properly, should have returned a pointer");
	sap = SAM.getByName("secondArr");
	ASSERTSTR(sap && sap->name()=="secondArr", "function getByName does not work properly, should have returned a pointer");
	LOG_INFO("The function 'getbyName' works OK");

	// ----- testing getSubArrays -----
	LOG_INFO_STR("Active subarrays:");
	saMap = SAM.getSubArrays("");
	SubArrayMap::iterator	iter = saMap.begin();
	SubArrayMap::iterator	end  = saMap.end();
	while (iter != end) {
		LOG_INFO_STR(*(iter->second));
		++iter;
	}

	// ----- testing remove -----
	SAM.scheduleRemove("firstArr");
	LOG_INFO_STR("Active subarrays after scheduled removal of 'firstArr':");
	saMap = SAM.getSubArrays("");
	iter = saMap.begin();
	end  = saMap.end();
	while (iter != end) {
		LOG_INFO_STR(*(iter->second));
		++iter;
	}

	SAM.removeDeadArrays();
	LOG_INFO_STR("Active subarrays after removal of 'firstArr':");
	saMap = SAM.getSubArrays("");
	iter = saMap.begin();
	end  = saMap.end();
	while (iter != end) {
		LOG_INFO_STR(*(iter->second));
		++iter;
	}
}

