//#  ProcRule.cc: one line description
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
#include "ProcRule.h"

namespace LOFAR {
  namespace ACC {

ProcRule::ProcRule(const string&  aNodeName,
				   const string&  aProcName,
				   const string&  aExecName,
				   const string&  aParamfile) :
	itsNodeName (aNodeName),
	itsProcName (aProcName),
	itsExecName (aExecName),
	itsParamfile(aParamfile),
	itsIsStarted(false)
{}

std::ostream& operator<< (std::ostream& os, const ProcRule& aPR)
{
	os << "ProcName: " << aPR.itsProcName << endl;
	os << "Type    : " << aPR.getType()   << endl;
	os << "StartCmd: " << aPR.itsStartCmd << endl;
	os << "StopCmd : " << aPR.itsStopCmd  << endl;
	os << "NodeName: " << aPR.itsNodeName << endl;

	return (os);
}

  } // namespace ACC
} // namespace LOFAR
