//#  ProcessState.h: virtual baseclass for all process states
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

#ifndef ACC_PROCESS_STATE_H
#define ACC_PROCESS_STATE_H

//# Includes
#include <ACC/ParameterSet.h>

namespace LOFAR
{

// The ProcessState class is used by the ProcessController to get
// the routine-addresses it should call to control the process.
// So the class only contains function pointers.
class ProcessState
{
public:
	// Default constructable;
	ProcessState()  {};
	~ProcessState() {};
	
	// Copying is allowed.
	//ProcessState(const ProcessState& that);
	//ProcessState& 	operator=(const ProcessState& that);

	void (*checkPS  ) (ParameterSet&	theParams);
	void (*prepare  ) (ParameterSet&	theParams);
	bool (*sayt	    ) (void);
	void (*execute  ) (int	numberOfCycles);
	void (*halt	    ) (void);
	void (*quit	    ) (void);
	void (*saveState) (void);
	void (*loadState) (void);
};

} // namespace LOFAR

#endif
