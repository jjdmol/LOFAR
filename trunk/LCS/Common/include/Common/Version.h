//#  Version.h: Helper class for version info
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_COMMON_VERSION_H
#define LOFAR_COMMON_VERSION_H

#include <string>
#include <vector>
#include <iosfwd>

// Class to get info like version and revision of the package and the
// packages it uses.

namespace LOFAR {

  class Version
  {
  public:
    // Define the function to get the version info of a package.
    typedef Version GetInfoFunc();
    // Define the struct to hold the level and getInfo function of a package.
    typedef std::pair<int, GetInfoFunc*> PkgInfo;

    // Construct a Version object from the relevant package info.
    Version (const std::string& packageName,
	     const std::string& version,
	     const std::string& confVersion,
	     const std::string& revision,
	     const std::string& packageRevision,
	     const std::string& nrChangedFiles,
	     const std::string& buildTime,
	     const std::string& buildMachine);

    // Get the package name.
    const std::string& packageName() const
      { return itsPackageName; }
    // Get the package version (as in repository)
    const std::string& version() const
      { return itsVersion; }
    // Get the package version as given in configure.in.
    const std::string& confVersion() const
      { return itsConfVersion; }
    // Get the global revision number in repository.
    const std::string& revision() const
      { return itsRevision; }
    // Ge the revision number of the package, thus the last revision something
    // was checked in for this package version.
    const std::string& packageRevision() const
      { return itsPackageRevision; }
    // Get the number of files in the ackage whch wre different from the
    // repository when building the package.
    const std::string& nrChangedFiles() const
      { return itsNrChangedFiles; }
    // Get the date/time the package was built (i.e. configured).
    const std::string& buildTime() const
      { return itsBuildTime; }
    // Get the name of the machine on which the package was built.
    const std::string& buildMachine() const
      { return itsBuildMachine; }

    // The package name can be changed.
    void setPackageName (const std::string& name)
      { itsPackageName = name; }

    // Get the package T and all packages it uses.
    template<typename T>
    static std::vector<PkgInfo> getPackages()
    {
      std::vector<PkgInfo> vec;
      T::fillInfo (0, vec);
      return vec;
    }

    // Get all packages (and their versions) used by package T
    // (including the package itself).
    template<typename T>
    static std::vector<std::string> usedPackages()
      { return doUsedPackages (getPackages<T>()); }

    // Show the package or application name and the versions of the packages
    // being used.
    // Possible types are:
    // - tree:  show used packages as a tree (giving dependencies).
    // - full:  show each package fully (but only once)
    // - top:   show the top package only
    // - other: show each package briefly (name, version, revision, nrchanged)
    // Use like:
    // @code
    //  Version::show<BlobVersion> ("", "applname", std::cout);
    // @endcode
    template<typename T> static void show (const std::string& type,
					   const std::string& applName,
					   std::ostream& os)
    {
      if (type == "top") {
	Version vers = T::getInfo();
	vers.setPackageName (applName);
	vers.showAll (std::string(), true, os);
      } else {
	doShow (type, applName, os, getPackages<T>());
      }
    }

    // Show all info. It is indented as given.
    // A warning is printed if sameRev is false, which indicates that some
    // package used have a different revision number.
    void showAll (const std::string& indent,
		  bool sameRev, std::ostream& os) const;

    // Show brief info. It is indented as given.
    void showBrief (const std::string& indent,
		    std::ostream& os) const;

  private:
    // Flatten the given vector and retrieve package name and version.
    static std::vector<std::string> doUsedPackages
    (const std::vector<PkgInfo>& vec);

    // Show the info as needed.
    static void doShow (const std::string& type,
			const std::string& applName,
			std::ostream& os,
			const std::vector<PkgInfo>&);

    // Show the info as a tree of all packages used.
    static void doShowTree (const Version& versRoot,
			    std::ostream& os,
			    const std::vector<PkgInfo>& vec);

    // Flatten the given vector into the result.
    // It removes all duplicate entries and keeps the one with the highest
    // level. The result is ordered by level.
    static std::vector<PkgInfo> flatten (const std::vector<PkgInfo>& vec);

    //# Data members.
    std::string itsPackageName;
    std::string itsVersion;
    std::string itsConfVersion;
    std::string itsRevision;
    std::string itsPackageRevision;
    std::string itsNrChangedFiles;
    std::string itsBuildTime;
    std::string itsBuildMachine;
  };

}

#endif
