//#  Event.h: definition of the events exchanged between the AC and the PC
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

#ifndef ACC_EVENT_H
#define ACC_EVENT_H

#include <lofar_config.h>

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

// QAD implementation of the events exchanged between the Application
// Controller and the processController.
class Event
{

public:
	~Event();
	Event(const int16		theType,
		  const uint32		theTime,
		  const string&		theInfo);
	// The Event may be construction by reading a ParameterSet string
	explicit Event(const string&		ParamString);
	
	// Copying is allowed.
	Event(const Event& that);
	Event& 	operator=(const Event& that);

	inline	int16		eventType() const	{ return itsType;	}
	inline	uint32		eventTime() const	{ return itsTime;	}
	inline	string		eventInfo() const	{ return itsInfo;	}

private:
	// NOT default constructable, makes no sense
	Event();

	int16		itsType;
	uint32		itsTime;
	string		itsInfo;
};

} // namespace LOFAR

#endif
