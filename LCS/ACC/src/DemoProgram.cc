//#  DemoProgram.cc: Program to demonstrate the process control states
//#
//#  Copyright (C) 2002-2003
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

//#  The program demonstrates what functions an ACC executable should implement
//#  in order to function as a proper ACC executable.
//#  The program does nothing more than printing the called states to cout.
//#
//#  NOTE that this program no longer has an own 'main'.

#include <ACC/ProcessState.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

static bool gHalted = true;

//#
//# doCheckPS
//#
//# Do a sanity check on the ParameterSet. Check dependancies, check exists
//# of files, etc. 
//# NOTE: We are allowed to do the actual load of the paramters but we are
//#       NOT allowed to open files, sockets, create threads, etc.
void doCheckPS	(ParameterSet&	theParams)
{
	cout << "MODULE: doCheckPS" << endl;
	cout << theParams;
}

//#
//# doPrepare
//#
//# Load the values of the parameterSet into  oyur own administration and
//# prepare the module for execution: open file, sockets, create threads,
//# allocate large buffers, connect to the persistence broker, etc.
void doPrepare	(ParameterSet&	theParams)
{
	cout << "MODULE: doPrepare" << endl;
	cout << theParams;
}

//#
//# doQuit
//#
//# We are about to quit. Close sockets, files, etc.
//# This routine is the counterpart of doPrepare.
void doQuit()
{
	cout << "MODULE: doQuit" << endl;
}

//#
//# doSayt
//#
//# Implement a sane check to test if we are ready for start.
//# A math library could for instance try to execute one of its functions
//# to see if the other dependant module are available.
bool doSayt	()
{
	try {
		cout << "MODULE: doSayt" << endl;
	}
	catch (Exception&	ex) {
		cerr << ex << endl;
		return (false);
	}

	return (true);
}

//#
//# doExecute
//#
//# Execute the 'main' loop of the program nrOfCycles times.
//#
//# nrOfCycles: 1..n
void doExecute(int	nrOfCycles)
{
	gHalted = false;
	while (!gHalted && nrOfCycles--) {
		cout << "MODULE: doExecute " << nrOfCycles << endl;
	}
}

//#
//# doHalt
//#
//# Stop the program on a logical place.
void doHalt()
{
	gHalted = true;
	cout << "MODULE: doHalt" << endl;
}

//#
//# doSaveState
//#
//# We should save all our objects to the Persistency database.
//#
void doSaveState()
{
	cout << "MODULE: doSaveState" << endl;
}

//#
//# doLoadState
//#
//# We should reconstruct the state of some previous moment.
//# The keys to this state we defined during loadPS.
void doLoadState()
{
	cout << "MODULE: doLoadState" << endl;
}


//#
//# main_init
//#
//# This is the only 'hard' call the ProcessController will make to us.
//# We must deliver a PS object containing all the entry points of the
//# state we have to implement.
//#
void main_init (ProcessState&	theState) 
{
	theState.checkPS	= doCheckPS;
	theState.prepare	= doPrepare;
	theState.sayt		= doSayt;
	theState.execute	= doExecute;
	theState.halt		= doHalt;
	theState.quit		= doQuit;
	theState.saveState	= doSaveState;
	theState.loadState	= doLoadState;
}

} // namespace LOFAR
