//#  tEventPort.cc: Program to test the EventPort class
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
//#  $Id$

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/GCF_ServiceInfo.h>
#include <GCF/GCF_Defines.h>
#include <GCF/GCF_PVTypes.h>
#include <GCF/LogSys/CEPKeyValueLogger.h>

using namespace LOFAR;
using namespace LOFAR::GCF;
using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::LogSys;

int main (int32	argc, char*argv[]) {

	INIT_LOGGER("tKVLlogger");
	
	CEPKeyValueLogger	kvlPort;

	kvlPort.logKeyValue("LOFAR.ObsSW.Observation.nyquistZone", GCFPVInteger(2), KVL_ORIGIN_MAC);

	return (0);
}

