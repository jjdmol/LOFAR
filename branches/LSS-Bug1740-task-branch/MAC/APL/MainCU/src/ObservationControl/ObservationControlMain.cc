//#  ObservationControlMain.cc: Main entry for the ObservationControl controller.
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
#include <Common/StringUtil.h>

#include "ObservationControl.h"

using namespace LOFAR;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::MainCU;
using namespace LOFAR::APLCommon;

int main(int argc, char* argv[])
{
	// args: [cntlrname, parentHost, parentService] OR [parameterset]
	bool	standAloneMode(false);
	switch (argc) {
	case 2:
		GCFScheduler::instance()->init(argc, argv, formatString("standAlone_%s", argv[1]));
		standAloneMode = true;
		break;
	case 4:
		GCFScheduler::instance()->init(argc, argv, argv[1]);
		break;
	default:
		cerr << "Expecting parameterset or controllername+hostname+servicename as argument" << endl;
		return (1);
	}

	ChildControl*	cc = ChildControl::instance();
	cc->start();	// make initial transition

	if (!standAloneMode) {
		ParentControl*	pc = ParentControl::instance();
		pc->start();	// make initial transition
	}

	ObservationControl	oc(argv[1], standAloneMode);
	oc.start(); // make initial transition

	GCFScheduler::instance()->run();

	return 0;
}

