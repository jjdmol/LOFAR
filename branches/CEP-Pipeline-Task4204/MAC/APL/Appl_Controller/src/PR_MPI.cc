//#  PR_MPI.cc: ProcessRule based on mpirun
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <Common/SystemUtil.h>
#include "PR_MPI.h"
#include "forkexec.h"

namespace LOFAR {
  namespace ACC {

//
// PR_MPI(jobname, nodes, executable, paramfile)
//
PR_MPI::PR_MPI(const string&			aHostName,
			   const string&  			aJobName,
			   const vector<string>& 	nodes,
			   const string&  			aExecutable,
			   const string&  			aParamFile,
			   uint16					nrProcs)
	: ProcRule("MPIjob", aJobName, aExecutable, aParamFile)
{
	LOG_TRACE_OBJ_STR ("PR_MPI: constructor");
	// create machinefile
	string 		machinefileName(aJobName + ".machinefile");
	ofstream 	machinefile;
	machinefile.open(machinefileName.c_str(), ofstream::out);
	vector<string>::const_iterator node;
	for (node = nodes.begin(); node != nodes.end(); node++) {
		machinefile << *node << endl;
	}
	machinefile.close();

	// start processes on another node?
	if (aHostName != myHostname(false) &&
		aHostName != myHostname(true)) {
		// copy files to other machine
		remoteCopy(aParamFile, aHostName, aParamFile);
		remoteCopy(machinefileName, aHostName, machinefileName);

		itsStartCmd = formatString("ssh %s \"( cd /opt/lofar/bin ; ./startMPI.sh %s %s %s %s %d )\"", 
									aHostName.c_str(),
									aJobName.c_str(),
									machinefileName.c_str(),
									aExecutable.c_str(),
									aParamFile.c_str(),
									nrProcs);
		itsStopCmd  = formatString("ssh %s \"( cd /opt/lofar/bin ; ./stopMPI.sh %s ) \"", 
									aHostName.c_str(),
									aExecutable.c_str());
	}
	else { // start/stop on local machine
		itsStartCmd = formatString("./startMPI.sh %s %s %s %s %d", 
									aJobName.c_str(),
									machinefileName.c_str(),
									aExecutable.c_str(),
									aParamFile.c_str(),
									nrProcs);
		itsStopCmd  = formatString("./stopMPI.sh %s", 
									aExecutable.c_str());
	}
}

//
// PR_MPI(PR_MPI&)
//
PR_MPI::PR_MPI(const PR_MPI& other)
	: ProcRule(other)
{
	LOG_TRACE_OBJ_STR ("PR_MPI: copy constructor");
}

//
// ~PR_MPI()
//
PR_MPI::~PR_MPI()
{
	LOG_TRACE_OBJ_STR ("PR_MPI: destructor");
}

//
// start()
//
bool PR_MPI::start()
{
	if (itsIsStarted) {
		LOG_TRACE_OBJ_STR("PR_MPI:" << itsProcName << " is already started");
		return (true);
	}

	LOG_TRACE_OBJ_STR ("PR_MPI: " << itsStartCmd);

	if (forkexec (itsStartCmd.c_str()) == 0) {
		itsIsStarted = true;
	}

	return (itsIsStarted);
}

//
// stop()
//
bool PR_MPI::stop()
{
	LOG_TRACE_OBJ_STR ("PR_MPI: " << itsStopCmd);

	if (forkexec (itsStopCmd.c_str()) == 0) {
		itsIsStarted = false;
	}

	return (!itsIsStarted);
}

//
// getType()
//
string PR_MPI::getType() const
{ 
	return ("PR_MPI");
}

//
// clone()
//
PR_MPI* PR_MPI::clone() const
{ 
	return (new PR_MPI(*this)); 
}


  } // namespace ACC
} // namespace LOFAR
