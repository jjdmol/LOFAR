//#  ACCProcess.h: class for internal administration of AC
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
//#	 This class implements a container of key-value pairs. The KV pairs can
//#  be read from a file or be merged with the values in another file.
//#  The class also support the creation of subsets.
//#
//#  $Id$

//#	NOTE: the class is only used by the AC for its internal administration.
//#		  one might consider to change it into a struct.

#ifndef ACC_ACCPROCESS_H
#define ACC_ACCPROCESS_H

#include <lofar_config.h>

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>
#include <ACC/Socket.h>

namespace LOFAR
{

// Forward declaration

// Description of class.
class ACCProcess
{
public:
	enum ProcState { Undefined = 0, Booting, Running };

	ACCProcess();
	~ACCProcess();
	ACCProcess(const string		theName,
				   const string		theExec,
				   const string		theParamFile,
				   const int16		thePortnr);
	
	// Copying is allowed.
	ACCProcess(const ACCProcess& that);
	ACCProcess& 	operator=(const ACCProcess& that);

	inline	Socket&		socket()		  { return	itsSocket;		}
	inline	ProcState	state()		const { return	itsState;	 	}
	inline	string		procName()	const { return	itsName;	 	}
	inline	string		execName()	const { return	itsExec;	 	}
	inline	string		paramFile()	const { return	itsParamFile; 	}
	inline	int16		portnr()	const { return	itsPortnr; 		}
	inline	void		setState(const ProcState	theState)
									{ itsState = theState;		}	

private:
	string		itsName;
	string		itsExec;
	string		itsParamFile;
	int16		itsPortnr;
	Socket		itsSocket;
	ProcState	itsState;
};

} // namespace LOFAR

#endif
