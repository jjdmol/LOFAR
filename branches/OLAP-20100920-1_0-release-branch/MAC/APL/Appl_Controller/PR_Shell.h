//#  PR_Shell.h: ProcesRule based on shell scripts
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

#ifndef LOFAR_ACCBIN_PR_SHELL_H
#define LOFAR_ACCBIN_PR_SHELL_H

// \file
// ProcessRule based on shell scripts

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include "ProcRule.h"

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// The PR_Shell class contains all information to (over)rule a process.
// Its known how to start and stop a process and knows its current state.
class PR_Shell : public ProcRule
{
public:
	PR_Shell(const string&	aNodeName, 
			 const string&  aProcName,
			 const string&  aExecName,
			 const string&  aParamfile);
	~PR_Shell() {};

	// Redefine the start and stop commands.
	virtual bool start();
	virtual bool stop();
	virtual string getType() const
		{ return ("PR_Shell"); }
	virtual PR_Shell* clone() const
		{ return (new PR_Shell(*this)); }

private:
	// Default construction not allowed
	PR_Shell();

	//# --- Datamembers ---
};

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
