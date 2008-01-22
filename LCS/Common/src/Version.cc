//#  Version.cc: Helper class for version info
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

#include <Common/Version.h>
#include <ostream>
#include <map>

namespace LOFAR {

  Version::Version (const std::string& packageName,
		    const std::string& version,
		    const std::string& confVersion,
		    const std::string& revision,
		    const std::string& packageRevision,
		    const std::string& nrChangedFiles,
		    const std::string& buildTime,
		    const std::string& buildMachine)
    : itsPackageName     (packageName),
      itsVersion         (version),
      itsConfVersion     (confVersion),
      itsRevision        (revision),
      itsPackageRevision (packageRevision),
      itsNrChangedFiles  (nrChangedFiles),
      itsBuildTime       (buildTime),
      itsBuildMachine    (buildMachine)
  {}

  void Version::doShow (const std::string& type,
			const std::string& applName,
			std::ostream& os,
			const std::vector<PkgInfo>& vec)
  {
    if (vec.empty()) {
      return;
    } 
    // Get main info.
    // Set name to given name.
    Version versRoot = vec[0].second();
    versRoot.setPackageName (applName);
    std::string mainRevision = versRoot.revision();
    // Print tree if required.
    if (type == "tree") {
      doShowTree (versRoot, os, vec);
      return;
    }
    bool full = (type == "full");
    // Only keep a single package in the vector at the highest level.
    // Map package to vector index.
    bool sameRev = true;
    int maxLevel = 0;
    std::vector<PkgInfo> vflat;
    std::map<GetInfoFunc*, int> m;
    for (std::vector<PkgInfo>::const_iterator iter = vec.begin();
	 iter != vec.end();
	 ++iter) {
      int level = iter->first;
      GetInfoFunc* func = iter->second;
      std::map<GetInfoFunc*,int>::iterator mfnd = m.find(func);
      if (mfnd == m.end()) {
	m[func] = vflat.size();
	vflat.push_back (*iter);
	sameRev = sameRev && (func().revision() == mainRevision);
      } else {
	int index = mfnd->second;
	if (level > vflat[index].first) {
	  vflat[index].first= level;
	}
      }
      if (level > maxLevel) {
	maxLevel = level;
      }
    }
    versRoot.showAll (std::string(), sameRev, os);
    os << " packages: " << std::endl;
    for (int level=1; level<=maxLevel; ++level) {
      for (std::vector<PkgInfo>::const_iterator viter = vflat.begin();
	   viter != vflat.end();
	   ++viter) {
	if (viter->first == level) {
	  Version vers = viter->second();
	  if (full) {
	    vers.showAll ("  ", true, os);
	  } else {
	    vers.showBrief ("  ", os);
	  }
	}
      }
    }
  }

  void Version::showAll (const std::string& indent,
			 bool sameRev, std::ostream& os) const
  { 
    os << indent << packageName() << ": version = " << version();
    if (version() != confVersion()) {
      os << " (in configure.in: " << confVersion() << ')';
    }
    os << std::endl;
    os << indent << " overall revision  = " << revision();
    if (!sameRev) os << " (note: used packages have different revision)";
    os << std::endl;
    os << indent << " package revision  = " << packageRevision()
       << " (last change in package)" << std::endl;
    os << indent << " built on " << buildMachine() << " at "
       << buildTime() << std::endl;
    if (nrChangedFiles() != "0") {
      os << indent << "  " << nrChangedFiles()
	 << " files were different from the repository" << std::endl;
    }
    os << std::endl;
  }

  void Version::showBrief (const std::string& indent,
			   std::ostream& os) const
  {
    os << indent << packageName();
    if (version() != "trunk") {
      os << '-' << version();
    }
    os << " (rev." << revision() << ')';
    if (nrChangedFiles() != "0") {
      os << ' ' << nrChangedFiles()
	 << " changed files" << std::endl;
    }
    os << std::endl;
  }

  void Version::doShowTree (const Version& versRoot,
			    std::ostream& os,
			    const std::vector<PkgInfo>& vec)
  {
    versRoot.showAll ("", true, os);
    std::vector<PkgInfo>::const_iterator viter = vec.begin();
    // Skip first one if name is the same as applName in versRoot.
    if (viter->second().packageName() == versRoot.packageName()) {
      ++viter;
    }
    for (; viter != vec.end(); ++viter) {
      // Blanks according to level.
      for (int i=0; i<viter->first; ++i) {
	os << ' ';
      }
      Version vers = viter->second();
      os << "  " << vers.packageName();
      if (vers.version() != "trunk") {
	os << '-' << vers.version();
      }
      os << std::endl;
    }
  }

}
