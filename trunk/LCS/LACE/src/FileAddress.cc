//# FileAddress.cc: Class for storing a valid filename
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <libgen.h>					// dirname
#include <sys/stat.h>				// fileStat
#include <errno.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <LACE/FileAddress.h>

namespace LOFAR {
  namespace LACE {

//
// FileAddress()
//
FileAddress::FileAddress() :
	Address(FILE_TYPE),
	itsFilename  (""),
	itsAccessMode("")
{}


//
// ~FileAddress()
//
FileAddress::~FileAddress()
{}


//
// set(filename, accessmode)
//
// check if file can be accessed in the given mode.
//
int FileAddress::set(const string&	filename,
					 const string&	accessmode)
{
	// copy setting to internals
	itsFilename   = filename;
	itsAccessMode = accessmode;
	setStatus(false);

	// when file may not be created it must exist.
	bool			fileMustExist (accessmode.find('r') != string::npos);
	struct stat		fileStat;
	if (fileMustExist && (stat(filename.c_str(), &fileStat) != 0)) {
		LOG_ERROR_STR("FileAddress: File '" << filename << "' does not exist");
		return (ENODEV);
	}

	// file may be created: so directory must exist
	if (!fileMustExist) {
		char	fullFilename [1024];
		strncpy(fullFilename, filename.c_str(), 1023);
		if (stat(dirname(fullFilename), &fileStat) != 0) {
			LOG_ERROR_STR("FileAddress: Directory '" << 
						  dirname(fullFilename) << "' does not exist");
			return (ENODEV);
		}
	}

	setStatus(true);
	return (0);
}

//
// deviceName()
//
string FileAddress::deviceName() const
{
	if (!isSet()) {
		return (formatString("Unknown filename %s:%s", 
							 itsFilename.c_str(), itsAccessMode.c_str()));
	}

	return (formatString("%s:%s", itsFilename.c_str(), itsAccessMode.c_str()));
}

//
// operator=
//
FileAddress& FileAddress::operator= (const FileAddress&	that) {
	if (this != &that) { 
		this->itsFilename	= that.itsFilename;
		this->itsAccessMode	= that.itsAccessMode;
	} 
	return (*this); 
}



  } // namespace LACE
} // namespace LOFAR
