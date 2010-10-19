//# Address.h: Base class for the address of any device.
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

#ifndef LOFAR_LACE_ADDRESS_H
#define LOFAR_LACE_ADDRESS_H

// \file Address.h
// Base class for the address of any device.

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


enum {
	UNDEFINED = 0,
	INET_TYPE,
	FILE_TYPE
};

// class_description
// ...
class Address
{
public:
	explicit Address (int	anAddressType) :
		itsType(anAddressType), itsIsSet(false) {}
	virtual ~Address() {};

	virtual const void*	getAddress() 	 const { return (0); }
	virtual int			getAddressSize() const { return (0); }
	virtual string		deviceName() 	 const 
		{ return ("Address: deviceName not implemented"); }

	int		getType() const { return (itsType);  }
	bool	isSet()   const { return (itsIsSet); }

	// operators for comparison.
	bool	operator==(const Address&	that)
		{ return (itsType == that.itsType); }
	bool	operator!=(const Address&	that)
		{ return (itsType != that.itsType); }


protected:
	void	setStatus(bool	newState)
		{ itsIsSet = newState; }

private:
	// Default construction is not allowed.
	Address();

	//# --- Datamembers ---
	int		itsType;
	bool	itsIsSet;
};


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
