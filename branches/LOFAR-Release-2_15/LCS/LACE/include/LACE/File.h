//# File.h: Abstract base class for all kind of different sockets.
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

#ifndef LOFAR_LACE_FILE_H
#define LOFAR_LACE_FILE_H

// \file File.h
// Abstract base class for all kind of different sockets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <unistd.h>
#include <Common/LofarTypes.h>
#include <LACE/FileAddress.h>
#include <LACE/ServiceAccessPoint.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class Address;


// class_description
// ...
class File : public ServiceAccessPoint
{
public:
	File();
	virtual ~File();

	void close();
	int  open (const FileAddress&	anAddress);

	int  remove  ();
	int	 truncate(int	length);
	int	 seek    (int	offset, int whence = SEEK_CUR);
	int	 tell	 ();
	
	int	 read (void*		buffer, size_t	nrBytes);
	int	 write(const void*	buffer, size_t	nrBytes);
	
protected:

private:
	//# --- Datamembers ---
	FileAddress		itsAddress;		// installed after succesful 'open'
};

//# ----- inline functions -----
inline int  File::remove  ()
{
	this->close();
	return (::unlink((char*)itsAddress.getAddress()));
}

inline int	 File::truncate(int	length)
{
	return(::truncate((char*)itsAddress.getAddress(), length));
}

inline int	 File::seek    (int	offset, int how)
{
	return (::lseek(getHandle(), offset, how));
}

inline int	 File::tell	 ()
{
	return (::lseek(getHandle(), 0, SEEK_CUR));
}


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
