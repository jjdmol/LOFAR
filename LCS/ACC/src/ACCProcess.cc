//#  ACCProcess.cc: ACCProcess class for internal use by AC
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

//#	NOTE: the class is only used by the AC for its internal administration.
//#		  one might consider to change it into a struct.

#include <ACC/ACCProcess.h>
#include <ACC/ParameterSet.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{

//#-------------------------- creation and destroy ---------------------------
//#
//# Default constructor
//#
ACCProcess::ACCProcess()
{}

//#
//# Construction by reading a parameter file.
//#
ACCProcess::ACCProcess(const string		theName,
					   const string		theExec,
					   const string		theParamFile,
					   const int16		thePortnr) :
	itsName		(theName),
	itsExec		(theExec),
	itsParamFile(theParamFile),
	itsPortnr	(thePortnr),
	itsSocket	(Socket()),
	itsState	(Undefined)
{ }

//#
//# Copying is allowed.
//#
ACCProcess::ACCProcess(const ACCProcess& that) :
	itsName		(that.itsName),
	itsExec		(that.itsExec),
	itsParamFile(that.itsParamFile),
	itsPortnr   (that.itsPortnr),
	itsSocket	(that.itsSocket),
	itsState	(that.itsState)
{ }

//#
//# operator= copying
//#
ACCProcess& ACCProcess::operator=(const ACCProcess& that)
{
	if (this != &that) {
		itsName		= that.itsName;
		itsExec		= that.itsExec;
		itsParamFile= that.itsParamFile;
		itsPortnr   = that.itsPortnr;
		itsSocket	= that.itsSocket;
		itsState	= that.itsState;
	}
	return (*this);
}

//#
//#	Destructor
//#
ACCProcess::~ACCProcess()
{}

} // namespace LOFAR
