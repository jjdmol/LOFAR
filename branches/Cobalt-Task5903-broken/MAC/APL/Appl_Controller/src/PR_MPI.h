//#  PR_MPI.h: ProcesRule based on mpirun
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_PR_MPI_H
#define LOFAR_ACCBIN_PR_MPI_H

// \file
// ProcessRule based on mpirun

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

#include <Common/lofar_vector.h>
#include <Common/StringUtil.h>
#include "ProcRule.h"

namespace LOFAR {
  namespace ACC {

// \addtogroup ACCbin
// @{

// The PR_MPI starts and stop an mpi program. One MPI-job is 
// controlled by one PR_MPI. This means there are usually multiple
// processes on multiple hosts involved.

// The PR_MPI class contains all information to (over)rule a process.
// Its known how to start and stop a process and knows its current state.
class PR_MPI : public ProcRule {
public:
	PR_MPI (const string&			aHostName,
			const string& 			aJobName, 
			const vector<string>& 	nodes,
			const string& 			aExecutable,
			const string& 			aParamfile,
			uint16					nrProcs);

	PR_MPI(const PR_MPI&	other);

	~PR_MPI();

	// Redefine the start and stop commands.
	virtual bool start();
	virtual bool stop();
	virtual string getType() const;
	virtual PR_MPI* clone() const;

private:
	// Default construction not allowed
	PR_MPI();

	//# --- Datamembers ---
};

	// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
