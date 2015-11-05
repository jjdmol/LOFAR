//# FileLocator.cc: Tries to locate a file in an earlier defined path.
//#
//# Copyright (C) 2006
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
#include <Common/LofarTypes.h>
#include <Common/LofarLogger.h>
#include <Common/FileLocator.h>
#include <Common/SystemUtil.h>
#include <cstdlib>
#include <cstring>

namespace LOFAR {

//
// rstripSlashes() : strip trailing slashes.
//
void rstripSlashes(string& path)
{
	string::size_type pos = path.find_last_not_of('/');
	if(pos != string::npos) {
		path.erase(pos + 1);
	}
}

//
// FileLocator()
//
FileLocator::FileLocator()
{
	addPathAtBack(resolveInput(BASE_SEARCH_DIR));
	string p = dirname(getExecutablePath());
	if (!p.empty()) {
		addPathAtBack(p + ":" + dirname(p));
	}
}

//
// FileLocator(aPath)
//
// Construct a FileLocator with a predefined path-chain.
//
FileLocator::FileLocator (const string&	aPath)
{
	addPathAtBack(resolveInput(aPath));
}

//
// ~FileLocator()
//
FileLocator::~FileLocator()
{}

//
// addPathAtBack(aPath): bool
//
// Adds the given pah(chain) at the back of the search path
//
void 	FileLocator::addPathAtBack  (const string& aPath)
{
	string	thePath=resolveInput(aPath);
	string::size_type	start  = 0;
	string::size_type	end;
	string::size_type	sepPos;
	bool	last = false;
	do {
		// search forwards through the new chain
		if ((sepPos = thePath.find(":", start)) == string::npos) {
			last = true;
			end = thePath.size();
		}
		else {
			end = sepPos;
		}
		// besure path does not end in a '/'
		rstripSlashes(thePath);
		// add path to list.
		itsPaths.push_back(thePath.substr(start, end-start));
		start = sepPos+1;
	} while (!last);
}

//
// addPathAtFront(aPath): bool
//
// Adds the given pah(chain) at the front of the search path
//
void 	FileLocator::addPathAtFront (const string& aPath)
{
	string	thePath=resolveInput(aPath);
	string::size_type	end = thePath.size()-1;
	string::size_type	start;
	string::size_type	sepPos;
	bool	last = false;
	do {
		// search backwards in the newchain for ':'
		if ((sepPos = thePath.rfind(":", end)) == string::npos) {
			last = true;
			start = string::npos;
		}
		else {
			start = sepPos;
		}
		// be sure path does not end in a '/'
		rstripSlashes(thePath);
		// add path to list.
		itsPaths.push_front(thePath.substr(start+1, end-start));
		end = sepPos-1;
	} while (!last);
}

//
// getPath(): string
//
// Returns the complete search path.
//
string	FileLocator::getPath		()
{
	iterator	iter     = itsPaths.begin();
	iterator	chainEnd = itsPaths.end();
	string		result;

	// no path set? return empty result.
	if (iter == chainEnd) {
		return (result);
	}

	// append all paths in the chain
	while (iter != chainEnd) {
		result.append(*iter + ":");
		++iter;
	}
	
	// remove last path separator
	result.erase(result.size() - 1, 1);

	return (result);
}

//
// hasPath(aPath): bool
//
// Checks if given path is in path-chain.
//
bool 	FileLocator::hasPath		(const string& aPath)
{
	iterator	iter     = itsPaths.begin();
	iterator	chainEnd = itsPaths.end();
	string		path2find(resolveInput(aPath));
	
	// Paths are stored without an trailing /, remove it from argument.
	rstripSlashes(path2find);

	while (iter != chainEnd) {
		if (*iter == path2find) {
			return (true);
		}
		if (!itsSubdir.empty()) {
			if ((*iter)+"/"+itsSubdir == path2find) {
				return(true);
			}
		}
		++iter;
	}

	// not found
	return (false);
}

//
// removePath(aPath): bool
//
// Removes the given path from the chain.
//
void	FileLocator::removePath  (const string& aPath)
{
	string		path2remove(resolveInput(aPath));
	
	// Paths are stored without an trailing /, remove it from argument.
	rstripSlashes(path2remove);

	itsPaths.remove(path2remove);
}

//
// locate(aFile): string
//
// Tries to find the file in the current searchpath. 
// Return full filename if found, or else an empty string.
//
string	FileLocator::locate		(const string& aFile)
{
	// return immediately if aFile is empty
	if(aFile.empty()) return string();
		// DILEMMA: the filelocator is often used to locate the log_prop file.
		//			using LOG_xxx here will result in an errormessage in that case.
		// SOLUTION: in global-init.cxx a variable 'initialized' is used in l4cp
		//			 to keep the state of the log-package. Make this var accessable.
//		LOG_DEBUG_STR ("Filename contains a /, returning inputname : " << aFile);

	struct stat		fileStat;
	int 			result;
	
	// If aFile contains an absolute path, simply test for its existence.
	if(aFile[0] == '/') {
		result = stat(aFile.c_str(), &fileStat);
		return (result == 0 ? aFile : string());
	}
	// Otherwise, search the path chain
	else {
		iterator	iter     = itsPaths.begin();
		iterator	chainEnd = itsPaths.end();
		while (iter != chainEnd) {
			// when itsSubdir is filled each test much be performed also with subdir
			for (int test = 0; test <= (itsSubdir.empty() ? 0 : 1); test++) {
				string			fullname;
				fullname = *iter + (*iter != "/" ? "/" : "");
				if (test == 0) {	// basedir?
					fullname += aFile;
				}
				else {				// test subdir
					fullname += itsSubdir + "/" + aFile;
				}
				result = stat(fullname.c_str(), &fileStat);
				if (result == 0) { // found?
					return (fullname);
				}
			}
			++iter;
		}
		// not found, return empty string.
		// See DILEMMA.
//		LOG_DEBUG_STR ("Filename not found in " << getPath() << 
//					   ", returning empty string");
		return string();
	}
}

//
// addSubdir(aSubdir)
//
void FileLocator::setSubdir(string	aSubdir)
{
	rstripSlashes(aSubdir);
	itsSubdir = aSubdir;
}



// [private]
// resolveInput (string)
//
// Resolve any environment variables in the inputstring
//
string FileLocator::resolveInput(const string&	input)
{
	// search for $
	string::size_type	dollarPos = input.find("$",0);
	if (dollarPos == string::npos) {		// no $ found?
		return (input);						// return original
	}

	LOG_TRACE_VAR_STR("resolving: " << input);

	// supported syntax: ${abc}|$abc[:|/xyz]
	// e.g. /dddd/ee:${abc}/bin:/ffff/gg
	// when $ is detected search for / and :
	//  /   :    action
	//	-   -    resolve $..input.size
	//  -   >0   resolve $..:
	//   [/>:]   resolve $..:
	//  >0  -    resolve $../ + add /..input.size
	//   [/<:]   resolve $../ + add /..:
	string::size_type		startPos = 0;
	string		result;
	do {
		result   = input.substr(startPos, dollarPos);	// add part till $
		startPos = dollarPos+1;
		string::size_type	slashPos = input.find("/", dollarPos);
		string::size_type	colonPos = input.find(":", dollarPos);
		string::size_type	endPos;					// end of path at : or EOS
		if (colonPos == string::npos) {
			endPos = input.size();
		}
		else {
			endPos = colonPos;
		}

		if ((slashPos == string::npos) || (slashPos > colonPos)) {
			// colon or EOS limits the $var
			char*	valPtr = 
						getenv(input.substr(startPos, endPos-startPos).c_str());
			if (!valPtr) {
				LOG_WARN_STR("Environment variable " << \
						input.substr(startPos, endPos-startPos) <<  \
						" can not be solved, excluding it from search path!");
			}
			else {
				result.append(valPtr);
				result.append(":");
			}
		}
		else {
			// slash limits the $var
			char*	valPtr = 
					getenv(input.substr(startPos, slashPos-startPos).c_str());
			if (!valPtr) {
				LOG_WARN_STR("Environment variable " <<
						input.substr(startPos, endPos-startPos) <<
						" can not be solved, excluding it from search path!");
			}
			else {
				// $var ends in / (e.g $abc/bin), resolved path may not contain :
				ASSERTSTR(strchr(valPtr, ':') == 0,
					"Environment variable " << 
					input.substr(startPos, slashPos-startPos) << 
					" may not contain a ':' --> " << valPtr);
				
				result.append(valPtr);
				result.append(input.substr(slashPos, endPos-slashPos));
				result.append(":");
			}
		}
		startPos = endPos+1;
	} while ((dollarPos = input.find("$", startPos)) != string::npos);

	// copy last part of input
	if (startPos < input.size()) {
		result.append(input.substr(startPos));
		result.append(":");
	}

	LOG_TRACE_VAR_STR("resolved: " << result.substr(0, result.size()-1));

	return (result.substr(0, result.size()-1));	// skip last colon
}


} // namespace LOFAR
