//#  File.cc: Abstract base class for several kinds of sockets
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <unistd.h>					// file stuff
#include <fcntl.h>					// mode flags
#include <Common/LofarLogger.h>
#include <LACE/File.h>

namespace LOFAR {
  namespace LACE {

//
// File()
//
File::File()
{}


//
// ~File()
//
File::~File()
{}


//
// close()
//
void File::close()
{
	if (getHandle() != 0) {
		::close(getHandle());
		setHandle(0);
	}
}


//
// open(address)
//
int File::open(const FileAddress&		anAddress)
{
	// translate accesmode of address into the right flags
	int	flags = 0;
	string	accessmode = anAddress.getMode();
	if (accessmode.find('+') != string::npos) {
		flags |= O_RDWR;
	}
	else {
		if (accessmode.find('r') != string::npos) {
			flags |= O_RDONLY;
		}
		else {
			flags |= O_WRONLY;
		}
	}
	if (accessmode.find('w') != string::npos) {
		flags |= (O_TRUNC | O_CREAT);
	}
	else if (accessmode.find('a') != string::npos) {
		flags |= (O_APPEND | O_CREAT);
	}

	mode_t	mode = 0000664;

	// finally open the file.
	LOG_DEBUG(formatString("File::open(%s,%04o,%04o)", 
				(char*)anAddress.getAddress(), flags, mode));
	int		fd = ::open((char*)anAddress.getAddress(), flags, mode);
	if (fd < 0) {
		LOG_ERROR_STR("Can not open file: " << anAddress.getAddress());
		return (-1);
	}

	setHandle(fd);
	itsAddress = anAddress;

	return (fd);
}

//
// read (buffer, nrBytes, timeout)
//
int File::read(void*	buffer, size_t	nrBytes)
{
	// socket must be open ofcourse
	if (!getHandle()) {
		LOG_ERROR("File::read: socket not opened yet.");
		return (-1);
	}

	// check arguments
	ASSERTSTR (buffer, "File::read:null buffer");
	if (!nrBytes)  {
		return (nrBytes);
	}

	return (::read(getHandle(), buffer, nrBytes));
}

//
// write (buffer, nrBytes, timeout)
//
// Returnvalue: < 0					error occured
//				>= 0 != nrBytes		intr. occured or socket is nonblocking
//				>= 0 == nrBytes		everthing went OK.
//
int File::write(const void*	buffer, size_t	nrBytes)
{
	// socket must be open ofcourse
	if (!getHandle()) {
		LOG_ERROR("File::write: socket not opened yet.");
		return (-1);
	}

	// check arguments
	ASSERTSTR (buffer, "File::write:null buffer");
	if (!nrBytes)  {
		return (nrBytes);
	}

	return (::write(getHandle(), buffer, nrBytes));
}



  } // namespace LACE
} // namespace LOFAR
