//# SystemUtil.cc: Utility functions
//#
//# Copyright (C) 2002-2007
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
//#
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Common/SystemUtil.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <climits>

#if defined(__APPLE__)
# include <mach-o/dyld.h>
#endif

#if !defined(USE_NOSOCKETS)
// netdb is not available on Cray XT machines with Catamount.
#include <netdb.h>
#endif

namespace LOFAR {

//
// remoteCopy(localFile, remoteHost, remoteFile)
//
int remoteCopy (const	string& localFile, 
			    const	string& remoteHost, 
			    const	string& remoteFile,
				int		timeoutSec)
{
	string		tmpResultFile(getTempFileName());
	string		command;
  
	if (!timeoutSec) {
		// -B: batch mode; -q: no progress bar
		command = formatString("scp -Bq %s %s:%s 1>&2 2>%s", localFile.c_str(),
									remoteHost.c_str(), remoteFile.c_str(),
									tmpResultFile.c_str());
	}
	else {
		command = formatString("scp -Bq -o ConnectTimeout=%d %s %s:%s 1>&2 2>%s &", timeoutSec,
									localFile.c_str(),
									remoteHost.c_str(), remoteFile.c_str(),
									tmpResultFile.c_str());
	}

	// execute the command.
	int error = system(command.c_str());
	LOG_DEBUG(formatString("copy command: %s",command.c_str()));

	if (error == 0) {			
		LOG_INFO(formatString("Successfully copied %s to %s:%s",
						localFile.c_str(),remoteHost.c_str(),remoteFile.c_str()));
	}
	else {
		// an error occured, try to reconstruct the message
		char 	outputLine[200];
		string 	outputString;
		FILE* 	f = fopen(tmpResultFile.c_str(),"rt");	// open file with errormsg
		if (f == NULL) {						// oops, problems opening the file
			LOG_ERROR(
				formatString("Unable to remote copy %s to %s:%s: reason unknown",
				localFile.c_str(),remoteHost.c_str(),remoteFile.c_str()));
		}
		else {
			// construct the error message
			while(!feof(f)) {
				if(!fgets(outputLine,sizeof outputLine,f))
                  break;
				if(!feof(f)) {
					outputString+=string(outputLine);
				}
			}
			fclose(f);
			LOG_ERROR(formatString("Unable to remote copy %s to %s:%s: %s",
						localFile.c_str(),remoteHost.c_str(),remoteFile.c_str(),
						outputString.c_str()));
		}
	}

	// remove the temporarely file.
	remove(tmpResultFile.c_str());

	return (error);
}

//
// copyFromRemote(remoteHost, remoteFile, localFile)
//
int copyFromRemote(const string& remoteHost, 
				   const string& remoteFile,
				   const string& localFile)
{
	string		tmpResultFile(getTempFileName());
  
	// -B: batch mode; -q: no progress bar
	string command(formatString("scp -Bq %s:%s %s 1>&2 2>%s", 
									remoteHost.c_str(), remoteFile.c_str(), localFile.c_str(),
									tmpResultFile.c_str()));
	// execute the command.
	int error = system(command.c_str());
	LOG_DEBUG(formatString("copy command: %s",command.c_str()));

	if(error == 0) {			
		LOG_INFO(formatString("Successfully copied %s:%s to %s",
							  remoteHost.c_str(),remoteFile.c_str(),localFile.c_str()));
	}
	else {
		// an error occured, try to reconstruct the message
		char 	outputLine[200];
		string 	outputString;
		FILE* 	f = fopen(tmpResultFile.c_str(),"rt");	// open file with errormsg
		if (f == NULL) {						// oops, problems opening the file
			LOG_ERROR(
				formatString("Unable to remote copy %s:%s to %s: reason unknown",
							 remoteHost.c_str(),remoteFile.c_str(),localFile.c_str()));
		}
		else {
			// construct the error message
			while(!feof(f)) {
				if(!fgets(outputLine,sizeof outputLine,f))
                  break;
				if(!feof(f)) {
					outputString+=string(outputLine);
				}
			}
			fclose(f);
			LOG_ERROR(formatString("Unable to remote copy %s:%s to %s: %s",
								   remoteHost.c_str(),remoteFile.c_str(),localFile.c_str(),
								   outputString.c_str()));
		}
	}

	// remove the temporarely file.
	remove(tmpResultFile.c_str());

	return (error);
}

//
// getTempFileName([format])
//
string getTempFileName(const string&	format)
{
	char tempFileName [256];

	if (format.find("XXXXXX", 0) != string::npos) {	// did user specify mask?
		strncpy (tempFileName, format.c_str(), sizeof tempFileName);		// use user-mask
	}
	else {
		strncpy (tempFileName, "/tmp/LOFAR_XXXXXX", sizeof tempFileName); // use default mask
	}

    // note that if we actually cut off the mask, it will likely be invalid because
    // it has to end in XXXXXX
    tempFileName[sizeof tempFileName - 1] = 0;

	if (!mkstemp(tempFileName)) {					// let OS invent the name
      if(errno == EINVAL)
        LOG_ERROR(formatString("Invalid temporary file-name mask %s (specified %s)",tempFileName,format.c_str()));
      else
        LOG_ERROR(formatString("Could not create temporary file with mask %s (specified %s)",tempFileName,format.c_str()));

      return "";
    }  

	return string(tempFileName);
}

//
// myHostname(giveFullname)
//
string myHostname(bool	giveFullName)
{
	char	fullhostname[300];
	if (gethostname(fullhostname, sizeof fullhostname) != 0) {
		return ("localhost");
	}

        // result of gethostname is not guaranteed to be null-terminated!
        fullhostname[sizeof fullhostname - 1] = 0;

	if (!giveFullName) {
		char*	dot = strchr(fullhostname, '.');
		if (dot) {
			*dot='\0';
		}
	}

	return (fullhostname);
}

//
// myIPaddress
//
uint32 myIPV4Address()
{
#if defined USE_NOSOCKETS
	LOG_ERROR ("Function myIPV4Address not available.");
	return (0);
#else
	struct addrinfo*	hostEnt;
	struct addrinfo	    hints;
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_INET; // IPv4

	if (getaddrinfo(myHostname(false).c_str(),NULL,&hints,&hostEnt) != 0) {
		return (0);
	}

	// copy info to save place.
	uint32	address;
	bcopy (&reinterpret_cast<struct sockaddr_in *>(hostEnt->ai_addr)->sin_addr, &address, sizeof address);

    freeaddrinfo (hostEnt);

	return (address);
#endif
}

//
// getExecutablePath -- see http://stackoverflow.com/questions/1023306
//
string getExecutablePath()
{
  string path;
  char buf[PATH_MAX+1];
#if defined(__APPLE__)
  uint32_t size = sizeof(buf);
  if(_NSGetExecutablePath(buf, &size) == 0) {
    path = buf;
    if(realpath(path.c_str(), buf)) {
      path = buf;
    }
  }
#elif defined(__linux__)
  ssize_t size = readlink("/proc/self/exe", buf, PATH_MAX);
  if(size != -1) {
    buf[size] = '\0';
    path = buf;
  }
#elif defined(__sun__)
  if(realpath(getexecname(), buf)) {
    path = buf;
  }
#endif
  return path;
}


//
// basename -- this implementation closely follows the description in the POSIX
// standard, IEEE Std 1003.1.
//
string basename(string path, const string& suffix)
{
  // If path is empty, return an empty string
  if(path.empty()) return path;

  // If path contains only slash characters, return a single slash character.
  string::size_type pos = path.find_last_not_of('/');
  if(pos == string::npos) return "/";

  // Remove any trailing slash characters in path.
  path.erase(pos+1);

  // If there are any slash characters remaining in path, remove the prefix of
  // path up to and including the last slash.
  pos = path.find_last_of('/');
  if(pos != string::npos) path.erase(0,pos+1);

  // If suffix string is not empty, is not identical to the characters
  // remaining in path, and is identical to a suffix of the characters
  // remaining in path, then suffix will be removed from path.
  if(!suffix.empty() && suffix != path) {
    pos = path.find(suffix, path.size() - suffix.size());
    if(pos != string::npos) path.erase(pos);
  }

  // Return the resulting path.
  return path;
}

//
// basename
//
string basename(const char* path, const char* suffix)
{
  return basename(string(path), suffix);
}


//
// dirname -- this implementation closely follows the description in the POSIX
// standard, IEEE Std 1003.1.
//
string dirname(string path)
{
  // If path is empty, return a single period character.
  if(path.empty()) return ".";

  // If path contains only slash characters, return a single slash character.
  string::size_type pos = path.find_last_not_of('/');
  if(pos == string::npos) return "/";

  // Remove any trailing slash characters in path.
  path.erase(pos+1);

  // If there are no slash characters remaining in path, return a single
  // period character.
  pos = path.find_last_of('/');
  if(pos == string::npos) return ".";
 
  // Remove any trailing non-slash characters in path.
  path.erase(pos+1);

  // If the remaining path contains only slash characters, return a single
  // slash character.
  pos = path.find_last_not_of('/');
  if(pos == string::npos) return "/";

  // Remove trailing slash characters in path, if any.
  path.erase(pos+1);

  // If the remaining path is empty, set path to a single slash character.
  if(path.empty()) path = "/";

  // Return the resulting path.
  return path;
}


} // namespace LOFAR
