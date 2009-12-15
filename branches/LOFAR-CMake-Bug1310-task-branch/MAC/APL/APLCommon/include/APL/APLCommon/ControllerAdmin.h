//#  ControllerAdmin.h: Container class for Controller information
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

#ifndef APLCOMMON_CONTROLLERADMIN_H
#define APLCOMMON_CONTROLLERADMIN_H

// \file 
// Container class for Controller information

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_list.h>
#include <APL/APLCommon/ControllerInfo.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace APLCommon {

// \addtogroup package
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class ChildControl;


// (singleton) Container class for managing ControllerInfo classes.
class ControllerAdmin
{
public:
	static ControllerAdmin*	instance();
	~ControllerAdmin();

	typedef list<ControllerInfo>::iterator			CIiter;
	typedef list<ControllerInfo>::const_iterator	const_CIiter;
	CIiter	findController(const string&	name);
	bool	isController(CIiter	controller)
		{	return (controller != itsList.end());	}

	friend class ChildControl;

private:
	// Default construction and copying is not allowed
	ControllerAdmin() {};
	ControllerAdmin(const ControllerAdmin&	that);
	ControllerAdmin& operator=(const ControllerAdmin& that);

	//# --- Datamembers ---
	list<ControllerInfo>		itsList;
};


// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
