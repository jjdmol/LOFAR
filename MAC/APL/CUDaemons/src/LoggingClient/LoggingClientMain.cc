//#  LoggingClientMain.cc: main loop of LoggingClient
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

#include <GCF/TM/GCF_Control.h>
#include "LoggingClient.h"

using namespace LOFAR::GCF::TM;
using namespace LOFAR::CUDaemons;

int main(int argc, char *argv[])
{
	GCFTask::init(argc, argv, "LoggingClient");

//	LOG_INFO("MACProcessScope: LOFAR_PermSW_LoggingClient");

	LoggingClient LC("LoggingClient"); 
	LC.start(); // make initial transition

	GCFTask::run();

	return (0);
}
