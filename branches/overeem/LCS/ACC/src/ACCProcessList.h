//#  ACCProcessList.h: class for internal administration of AC
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

#ifndef ACC_ACCPROCESSLIST_H
#define ACC_ACCPROCESSLIST_H

#include <lofar_config.h>

//# Includes
#include <ACC/ACCEvents.h>
#include <ACC/ACCProcess.h>
#include <Common/lofar_list.h>

namespace LOFAR
{

// Description of class.
class ACCProcessList : public list<ACCProcess>
{
public:
	typedef list<ACCProcess>::iterator				iterator;
	typedef list<ACCProcess>::const_iterator		const_iterator;

	// Default constructable
	ACCProcessList();
	~ACCProcessList();

	// Copying is allowed.
	ACCProcessList(const ACCProcessList& that);
	ACCProcessList& 	operator=(const ACCProcessList& that);

	void registerProcess	(const ACCProcess&		theProc);
	void sendAll			(const ACCEvent			theEventType);			
	void shutdownAll		();
};

} // namespace LOFAR

#endif
