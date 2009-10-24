//#  tACCMain.cc: testproces to test ACCmain
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
#include <PLC/ACCmain.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>
#include "APCmdImpl.h"

using namespace LOFAR;
using namespace LOFAR::ACC;
using namespace LOFAR::ACC::PLC;

int main(int argc, char *argv[])
{
	// Always bring up the logger first
	ConfigLocator	aCL;
	string			progName(basename(argv[0]));
#ifdef HAVE_LOG4CPLUS
	string			logPropFile(progName + ".log_prop");
	INIT_VAR_LOGGER (aCL.locate(logPropFile).c_str(), progName + "-" + argv[3]);
#else
	string logPropFile(progName + ".debug");
	INIT_LOGGER (aCL.locate(logPropFile).c_str());	
#endif

	try {
		LOFAR::ACC::APCmdImpl	 myProcessImpl;
		return (LOFAR::ACC::PLC::ACCmain(argc, argv, &myProcessImpl));
	} //try
	catch(...) {
		std::cerr << "** PROBLEM **: Unhandled exception caught." << std::endl;
		return -3;
	}
}
