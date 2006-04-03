//#  FileLocator.cc: Tries to locate a file in an earlier defined path.
//#
//#  Copyright (C) 2006
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
#include <Common/LofarLogger.h>
#include <Common/FileLocator.h>

namespace LOFAR {

//
// FileLocator()
//
FileLocator::FileLocator()
{}

//
// FileLocator(aPath)
//
// Construct a FileLocator with a predefined path-chain.
//
FileLocator::FileLocator (const string&	aPath)
{
	addPathAtBack(aPath);
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
	uint32	start  = 0;
	uint32	end;
	uint32	sepPos;
	bool	last = false;
	do {
		// search forwards through the new chain
		if ((sepPos = aPath.find(":", start)) == string::npos) {
			last = true;
			end = aPath.size();
		}
		else {
			end = sepPos;
		}
		// besure path does not end in a '/'
		if (aPath[end-1] == '/') {
			end--;
		}
		// add path to list.
		itsPaths.push_back(aPath.substr(start, end-start));
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
	uint32	end = aPath.size()-1;
	int32	start;	// note: because rfind = uint32 and sepPos becomes < 0
	uint32	sepPos;
	bool	last = false;
	do {
		// search backwards in the newchain for ':'
		if ((sepPos = aPath.rfind(":", end)) == string::npos) {
			last = true;
			start = -1;
		}
		else {
			start = sepPos;
		}
		// be sure path does not end in a '/'
		if (aPath[end] == '/') {
			end--;
		}
		// add path to list.
		itsPaths.push_front(aPath.substr(start+1, end-start));
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
		result.append(*iter + "/:");
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
	string		path2find(aPath);
	
	// Paths are stored without an trailing /, remove it from argument.
	if (path2find[path2find.size()-1] == '/') {
		path2find.erase(path2find.size() - 1, 1);
	}

	while (iter != chainEnd) {
		if (*iter == path2find) {
			return (true);
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
	string		path2remove(aPath);
	
	// Paths are stored without an trailing /, remove it from argument.
	if (path2remove[path2remove.size()-1] == '/') {
		path2remove.erase(path2remove.size() - 1, 1);
	}

	itsPaths.remove(path2remove);
}

//
// locate(aFile): string
//
// Tries to find the file in the current searchpath. Returns the
// full filename is found or the original filename when not found.
//
string	FileLocator::locate		(const string& aFile)
{
	// if filename contains a '/', just return the name
	if (aFile.find("/", 0) != string::npos) {
		LOG_DEBUG_STR ("Filename contains a /, returning inputname : " << aFile);
		return (aFile);
	}

	// search the path chain
	iterator	iter     = itsPaths.begin();
	iterator	chainEnd = itsPaths.end();
	while (iter != chainEnd) {
		struct stat		fileStat;
		string			fullname = *iter + "/" + aFile;
		int result = stat(fullname.c_str(), &fileStat);
		if (result == 0) { // found?
			return (fullname);
		}
		++iter;
	}

	// not found, return original file
	LOG_DEBUG_STR ("Filename not found in " << getPath() << 
				   ", returning inputname : " << aFile);
	return (aFile);
}

} // namespace LOFAR
