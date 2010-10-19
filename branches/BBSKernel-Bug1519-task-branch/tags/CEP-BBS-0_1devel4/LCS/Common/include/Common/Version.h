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
#include <Common/StreamUtil.h>

// Class to get the version (revision) of the package and the packages it uses.

namespace LOFAR {

  class Version
  {
  public:
    // Add package/version to vector if not existing yet.
    // It returns false if the version differs from the main version.
    static bool fillVersion (const std::string& package,
			     const std::string& version,
			     const std::string& versionSuffix,
			     const std::string& mainVersion,
			     std::vector<std::string>& v);

    // Show the package or application name and the versions of the packages
    // being used.
    // Use like:
    // @code
    //  Version::show<BlobVersion> ("applname", std::cout);
    // @endcode
    template<typename T> static void show (const std::string& name,
					   std::ostream& os)
    {
      std::vector<std::string> v;
      bool sameVers = T::fillVersion (T::getVersion(), v);
      os << name << ": main version = " << T::getVersion();
      if (!sameVers) os << '*';
      os << endl;
      os << "  packages: " << v << endl;
    }

    // Get the main version and the individual package versions being used.
    // It returns false if a package version differs from the main version.
    // Use like:
    // @code
    //  std::string mainVersion;
    //  std::vector<std::string> packageVersions;
    //  Version::get<BlobVersion> (mainVersion, packageVersions);
    // @endcode
    template<typename T> static bool get (std::string& mainVersion,
					  std::vector<std::string>& pkgVers)
    {
      mainVersion = T::getVersion();
      return T::fillVersion (mainVersion, pkgVers);
    }
  };

}

#endif
