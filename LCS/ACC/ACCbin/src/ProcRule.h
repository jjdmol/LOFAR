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
//#  $Id$

#ifndef ACC_PROCRULE_H
#define ACC_PROCRULE_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/lofar_iostream.h>

namespace LOFAR {
  namespace ACC {

// The ProcRule class contains all information to (over)rule a process.
// Its known how to start and stop a process.
class ProcRule
{
public:
	ProcRule(const string&	aName,
			 const string&  aStartCmd,
			 const string&  aStopCmd,
			 const string&  aNodeName);
	//~ProcRule();

	// The start and stop commands.
	bool start();
	bool stop();
	bool isStarted() const;

	string	getName() const;
	void	markAsStopped();

	// Copying is allowed
//	ProcRule(const ProcRule&	that);
//	ProcRule& operator=(const ProcRule& that);

	friend std::ostream& operator<<(std::ostream& os, const ProcRule& aPR);

private:
	// defaultconstruction not allowed
	ProcRule();

	//# Datamembers
	string		itsName;
	string		itsStartCmd;
	string		itsStopCmd;
	string		itsNodeName;
	bool		itsIsStarted;
};

inline bool ProcRule::isStarted() const
{
	return (itsIsStarted);
}

inline string ProcRule::getName() const
{
	return (itsName);
}

inline void	ProcRule::markAsStopped()
{
	itsIsStarted = false;
}

  } // namespace ACC
} // namespace LOFAR

#endif
