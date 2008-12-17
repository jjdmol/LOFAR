//#  ServiceAccessPoint.h: Base class for a point for IO.
//#
//#  Copyright (C) 2008
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

#ifndef LOFAR_LACE_SERVICE_ACCESS_POINT_H
#define LOFAR_LACE_SERVICE_ACCESS_POINT_H

// \file ServiceAccessPoint.h
// Base class for a point for IO.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
//#include <otherPackage/file.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//#class ...;

typedef	int			SAPHandle;

// class_description
// ...
class ServiceAccessPoint
{
public:
	void		setHandle(SAPHandle	aHandle);
	SAPHandle	getHandle()	const			 { return (itsHandle); }
	
	int		setBlocking(bool	blocking);
	bool	getBlocking() const				 { return (itsIsBlocking); }

protected:
	// Default construction is not allowed, its a baseclass.
	ServiceAccessPoint();
	~ServiceAccessPoint();

private:
	//# --- Datamembers ---
	SAPHandle	itsHandle;
	bool		itsIsBlocking;
};


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
