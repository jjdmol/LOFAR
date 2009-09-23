//#  StationControlMain.cc: Main entry for the StationControl controller.
//#
//#  Copyright (C) 2006
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
#include <Common/SystemUtil.h>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/ParentControl.h>

#include "StationControl.h"

using namespace LOFAR;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::APLCommon;
using namespace LOFAR::StationCU;

int main(int argc, char* argv[])
{
	// args: cntlrname, parentHost, parentService
	GCFScheduler::instance()->init(argc, argv, basename(argv[0]));

	ParentControl*	pc = ParentControl::instance();
	pc->start();	// make initial transition

	ChildControl*	cc = ChildControl::instance();
	cc->start();	// make initial transition

	string		myName;
	if (argc < 2) {		// started by swlevel?
		myName = formatString("%s:%s", myHostname(false).c_str(),  basename(argv[0]));
	}
	else {
		myName = argv[1];
	}
	StationControl	sc(myName.c_str());
	sc.start(); 	// make initial transition

	GCFScheduler::instance()->run();

	return 0;
}

