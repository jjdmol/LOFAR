//#  OfflineControlMain.cc: Main entry for the OfflineControl controller.
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

#include "OfflineControl.h"

using namespace LOFAR;
using namespace LOFAR::GCF::TM;
using namespace LOFAR::CEPCU;

int main(int argc, char* argv[])
{
	// args: cntlrname
    if(argc < 2)
	{
	  printf("Unexpected number of arguments: %d\n",argc);
	  printf("%s usage: %s <controller name>\n",argv[0],argv[0]);
	  exit(-1);
	}
	GCFTask::init(argc, argv);

	ParentControl*	pc = ParentControl::instance();
	pc->start();	// make initial transition

	OfflineControl	ofc(argv[1]);
	ofc.start(); 	// make initial transition

	GCFTask::run();

	return 0;
}

