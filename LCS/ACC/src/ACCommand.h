//#  ACCommand.h: Structure containing one AC command
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

#ifndef ACC_ACCOMMAND_H
#define ACC_ACCOMMAND_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <time.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR {
  namespace ACC {

// Description of class.
class ACCommand
{
public:
	ACCommand (int16			aCommand,
				time_t			aScheduleTime,
				time_t			aWaitTime = 0,
				const string&	aOptions = "",
				const string&	aProcessList = "",
				const string&	aNodeList = "");

	~ACCommand() {};

	int16		itsCommand;
	time_t		itsScheduleTime;
	time_t		itsWaitTime;
	string		itsOptions;
	string		itsProcList;
	string		itsNodeList;
};

  } // namespace ACC
} // namespace LOFAR

#endif
