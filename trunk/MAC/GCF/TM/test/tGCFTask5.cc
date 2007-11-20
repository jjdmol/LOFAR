//#  Class.cc: one_line_description
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
//#  $Id: $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <GCF/TM/GCF_Control.h>
#include <unistd.h>			// sleep
#include "testTask.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR::GCF;

int main(int argc,	char*	argv[]) 
{
	GCFTask::init(argc, argv, basename(argv[0]));

	LOG_INFO("THIS PROGRAM WILL AUTOMATICALLY QUIT AFTER 11.3 SECONDS!!!");
	sleep(2);

	testTask		fast  ("fast",   0.2);
	testTask*		medium(new testTask("medium", 1.1));
	testTask		slow  ("autoQuit",   5.2);

	fast.start();
	medium->start();
	slow.start();

	GCFTask::setDelayedQuit(true);

	GCFTask::run();

	fast.quit();
	LOG_INFO("Destructing the 'medium' task");
	delete medium;
	medium = 0;
	slow.quit();

	GCFTask::run(2.0);

	return (0);
}
