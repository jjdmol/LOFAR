//# FileLocator.h: Tries to locate a file in an earlier defined path.
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

#ifndef LOFAR_COMMON_FILELOCATOR_H
#define LOFAR_COMMON_FILELOCATOR_H

// \file 
// Tries to locate a file in an earlier defined path.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_list.h>
#include <sys/stat.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// class_description
// FileLocator is a class that helps you to find a file that must be somewhere
// in an earlier defined path.
class FileLocator
{
public:
	#define		BASE_SEARCH_DIR		"/opt/lofar:/opt/lofar/share"

	typedef list<string>::iterator		iterator;

	// Create a FileLocator with a default search path. This path consists
	// of the directory containing the currently running executable and
	// its parent directory (or "." and ".." if that path cannot be
	// determined), plus the directories defined in BASE_SEARCH_DIR.
	FileLocator();
	// Create a FileLocator with a predefined path.
	explicit FileLocator (const string&	aPath);
	// Destructor is virtual to allow defining derived flavors like
	// ConfigLocator, LogLocator, etc with predefined paths.
	virtual  ~FileLocator();

	//# Functions for managing the search path
	// Add the given path(chain) at the end of the current chain.
	void 	addPathAtBack  (const string& aPath);
	// Add the given path(chain) at the beginning of the current chain.
	void 	addPathAtFront (const string& aPath);
	// Get the registered path-chain.
	string	getPath		   ();
	// Checks if the given path is in the registered chain.
	bool 	hasPath		   (const string& aPath);
	// Removes the given path when it is in the registered chain.
	void	removePath     (const string& aPath);

	//# Functions for managing subpath
	// Use given subdir when searching the files. First the 'basedir' is
	// tried, then the 'basedir'/'subdir' is tried.
	void			setSubdir  (const string&	aSubdir);
	// Clear the subdir
	inline void		clearSubdir();
	// Show the subdir
	inline string	getSubdir();

	//# Finally where it is all about.
	// Tries to locate the given filename. When the file is found in the
	// chain the full filename is returned. When the file can not be found
	// the returned string is equal to the passed argument.
	string	locate		   (const string& aFile);

private:
	// Copying is not allowed
	FileLocator(const FileLocator&	that);
	FileLocator& operator=(const FileLocator& that);

	// resolv environment variables
	string resolveInput(const string&	input);

	//# --- Datamembers ---
	list<string>		itsPaths;
	string				itsSubdir;
};

//# ---------- inline functions ----------
//
// addSubdir(aSubdir)
//
inline void FileLocator::clearSubdir()
{
	itsSubdir.clear();
}


//
// getSubdir(): subdir
//
inline string FileLocator::getSubdir()
{
	return (itsSubdir);
}


} // namespace LOFAR

#endif
