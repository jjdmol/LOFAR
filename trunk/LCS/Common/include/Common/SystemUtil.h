//# SystemUtil.h: useful system utilities like copy files
//#
//# Copyright (C) 2002-2003
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

#ifndef LOFAR_COMMON_SYSTEMUTIL_H
#define LOFAR_COMMON_SYSTEMUTIL_H

// \file
// Useful system utilities like copy files

//# Includes
#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

// Copies a localfile the another machine. Function uses 'scp' and does some checks
// on the result of the copy action. Returns 0 on succes or the errornumber of the
// 'system' call on failure.
int remoteCopy (const string& localFile, 
			    const string& remoteHost, 
			    const string& remoteFile);

// Copies a remote file the the local machine. Function uses 'scp' and does some checks
// on the result of the copy action. Returns 0 on succes or the errornumber of the
// 'system' call on failure.
int copyFromRemote(const string& remoteHost, 
				   const string& remoteFile,
				   const string& localFile);

// Constructs a temporately filename that is guaranteed to be uniq. The user may pass
// his own mask for the filename. The mask should contain "XXXXXX" as a placeholder
// for the unique ID.
string getTempFileName(const string&	format="");

// Returns the hostname of this machine. The user can specify if he wants to short name
// or the full hostname containing the domainname also.
string myHostname(bool	giveFullName);

// Returns the IPaddress of this machine. If something goes wrong 0 is returned.
uint32 myIPV4Address();

// Return the full path to the current executable. An empty string is
// returned, if the full path cannot be determined.
string getExecutablePath();

// Return non-directory portion of a pathname. This implementation closely
// follows the description in the POSIX standard, IEEE Std 1003.1.

string basename(string path, string suffix="");

// Return the directory portion of a pathname. This implementation closely
// follows the description in the POSIX standard, IEEE Std 1003.1.
string dirname(string path);

} // namespace LOFAR

#endif
