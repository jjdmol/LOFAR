//#  clkctl.h: manual program for testing the CLKCTRL commands of the ClkCtller
//#
//#  Copyright (C) 2009
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

#ifndef CLKCTL_H
#define CLKCTL_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_bitset.h>

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
using	MACIO::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFTask;


class ClkCtl : public GCFTask
{
public:
	explicit ClkCtl(const string& cntlrName);
	~ClkCtl();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult doCommand(GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	ClkCtl();
	ClkCtl(const ClkCtl&);
   	ClkCtl& operator=(const ClkCtl&);

   	void 		doHelp();
   	GCFEvent*	parseOptions(int argc, char **argv);

	// Data members
	GCFTCPPort*				itsClientPort;
	GCFEvent*				itsCommand;
};

} // namespace LOFAR
#endif
