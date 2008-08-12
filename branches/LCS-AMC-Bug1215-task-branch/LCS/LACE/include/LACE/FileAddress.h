//#  FileAddress.h: Class for storing a legal filename
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

#ifndef LOFAR_LACE_FILE_ADDRESS_H
#define LOFAR_LACE_FILE_ADDRESS_H

// \file FileAddress.h
// Class for storing a legal filename.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/LofarTypes.h>
#include <LACE/Address.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace LACE {

// \addtogroup LACE
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//# class ...;


// class_description
// ...
class FileAddress : public Address
{
public:
	FileAddress();
	virtual ~FileAddress();
	
	virtual const void*	getAddress() 	 const { return (itsFilename.c_str()); }
	virtual int			getAddressSize() const { return (itsFilename.size()); 	}
	virtual string		deviceName()	 const;
	string				getMode()		 const { return (itsAccessMode); }

	// Check if the given filename can be opened in the given mode
	// Returns 0 on success and ENODEV on error.
	// mode: [b]r|w|a[+]	see 'man 2 open'
	// b : binairy
	// r : readonly
	// r+: read/write
	// w : write & truncate
	// w+: read/write & truncate
	// a : write & append
	// a+: read/write & append
	int		set(const string&	filename,
				const string&	accessMode = "r");

	// operators for comparison.
	bool		 operator==(const FileAddress&	that);
	bool		 operator!=(const FileAddress&	that);
	FileAddress& operator= (const FileAddress&	that);

protected:

private:
	//# --- Datamembers ---
	string		itsFilename;
	string		itsAccessMode;
};


//# ----- inline functions -----
inline bool FileAddress::operator==(const FileAddress&	that)
{
	return (Address::operator==(that) && 
			itsFilename == that.itsFilename && 
			itsAccessMode == that.itsAccessMode);
}

inline bool FileAddress::operator!=(const FileAddress&	that)
{
	return (!FileAddress::operator==(that));
}


// @}
  } // namespace LACE
} // namespace LOFAR

#endif
