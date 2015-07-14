//# ServiceAccessPoint.h: Base class for a point for IO.
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

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
