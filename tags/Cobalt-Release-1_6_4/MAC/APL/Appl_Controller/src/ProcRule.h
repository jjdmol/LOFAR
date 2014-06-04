//#  ProcRule.h: information how to rule a process.
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

#ifndef LOFAR_ACCBIN_PROCRULE_H
#define LOFAR_ACCBIN_PROCRULE_H

// \file
// Information how to rule a process.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_iostream.h>

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// The ProcRule class contains all information to (over)rule a process.
// Its known how to start and stop a process and knows its current state.
class ProcRule
{
public:
	ProcRule(const string&	aNodeName,
			 const string&  aProcName,
			 const string&  aExecName,
			 const string&  aParamfile);
	virtual ~ProcRule() {};

	// The start and stop commands to be implemented.
	virtual bool  		start() = 0;
	virtual bool  		stop()  = 0;
	virtual string		getType() const = 0;
	virtual ProcRule*	clone()   const = 0;

	const string&	getName()   const;
	const string&	getNodeName()   const;
	const string&	getExecName()   const;
	const string&	getParamFile()   const;
	bool 			isStarted() const;
	void 			markAsStopped();


	friend std::ostream& operator<<(std::ostream& os, const ProcRule& aPR);

protected:
	// Default construction not allowed
	ProcRule();


	//# --- Datamembers ---
	string		itsNodeName;
	string		itsProcName;
	string		itsExecName;
	string		itsParamfile;
	string		itsStartCmd;
	string		itsStopCmd;
	bool		itsIsStarted;
};

inline bool ProcRule::isStarted() const
{
	return (itsIsStarted);
}

inline const string& ProcRule::getName() const
{
	return (itsProcName);
}

inline const string& ProcRule::getNodeName() const
{
	return (itsNodeName);
}

inline const string& ProcRule::getExecName() const
{
	return (itsExecName);
}

inline const string& ProcRule::getParamFile() const
{
	return (itsParamfile);
}

inline void	ProcRule::markAsStopped()
{
	itsIsStarted = false;
}

// @} addgroup
  } // namespace ACC
} // namespace LOFAR

#endif
