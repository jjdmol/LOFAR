//# SourceDBBlob.cc: Class for a Blob file holding sources and their parameters
//#
//# Copyright (C) 2012
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

#include <lofar_config.h>
#include <ParmDB/SourceDBBlob.h>
#include <ParmDB/ParmMap.h>
#include <Common/Exception.h>

#include <casa/BasicSL/String.h>
#include <casa/Utilities/Sort.h>
#include <iostream>

using namespace casa;
using namespace std;

namespace LOFAR {
namespace BBS {

  SourceDBBlob::SourceDBBlob (const ParmDBMeta& pdm, bool forceNew)
    : SourceDBRep (pdm, forceNew),
      itsCanWrite (true),
      itsEndPos   (0)
  {
    if (!forceNew) {
      // First open as readonly to see if it exists.
      itsFile.open (pdm.getTableName().c_str(), ios::in | ios::binary);
      if (!itsFile) {
        forceNew = true;
      } else {
        // Get eof position.
        itsFile.seekg (0, ios::end);
        itsEndPos = itsFile.tellg();
        itsFile.close();
        // See if it can be opened as read/write.
        // If not, open it again as readonly.
        itsFile.open (pdm.getTableName().c_str(),
                      ios::in | ios::out | ios::binary);
        if (!itsFile) {
          itsFile.open (pdm.getTableName().c_str(), ios::in | ios::binary);
          itsCanWrite = false;
        }
        ASSERT (itsFile);
      }
    }
    if (forceNew) {
      itsFile.open (pdm.getTableName().c_str(),
                    ios::out | ios::in | ios::trunc | ios::binary);
      ASSERTSTR (itsFile, "SourceDB blob file " + pdm.getTableName() +
                 " cannot be created");
    }
    itsBufIn   = boost::shared_ptr<BlobIBufStream>(new BlobIBufStream(itsFile));
    itsBufOut  = boost::shared_ptr<BlobOBufStream>(new BlobOBufStream(itsFile));
    itsBlobIn  = boost::shared_ptr<BlobIStream>(new BlobIStream(*itsBufIn));
    itsBlobOut = boost::shared_ptr<BlobOStream>(new BlobOStream(*itsBufOut));
  }

  SourceDBBlob::~SourceDBBlob()
  {}

  void SourceDBBlob::lock (bool)
  {}

  void SourceDBBlob::unlock()
  {}

  void SourceDBBlob::clearTables()
  {
    // Recreate the file.
  }

  void SourceDBBlob::checkDuplicates()
  {
  }

  vector<string> SourceDBBlob::findDuplicatePatches()
  {
    return vector<string>();
  }

  vector<string> SourceDBBlob::findDuplicateSources()
  {
    return vector<string>();
  }

  bool SourceDBBlob::patchExists (const string&)
  {
    return false;
  }

  bool SourceDBBlob::sourceExists (const string&)
  {
    return false;
  }

  uint SourceDBBlob::addPatch (const string& patchName, int catType,
                               double apparentBrightness,
                               double ra, double dec,
                               bool)
  {
    ASSERTSTR (itsCanWrite, "SourceDBBlob: file is not writable");
    itsFile.seekp (0, ios::end);
    int16 cType = catType;
    itsBlobOut->putStart ("patch", 1);
    *itsBlobOut << patchName << cType << apparentBrightness << ra << dec;
    itsBlobOut->putEnd();
    itsEndPos = itsFile.tellp();
    return 0;
  }

  void SourceDBBlob::skipPatch()
  {
    string patchName;
    int16 cType;
    double apparentBrightness, ra, dec;
    itsBlobIn->getStart ("patch");
    *itsBlobIn >> patchName >> cType >> apparentBrightness >> ra >> dec;
    itsBlobIn->getEnd();
  }

  void SourceDBBlob::addSource (const SourceInfo& sourceInfo,
                                const string& patchName,
                                const ParmMap& defaultParameters,
                                double ra, double dec,
                                bool)
  {
    ASSERTSTR (itsCanWrite, "SourceDBBlob: file is not writable");
    itsFile.seekp (0, ios::end);
    SourceData src(sourceInfo, patchName, ra, dec);
    src.setParms (defaultParameters);
    src.writeSource (*itsBlobOut);
    itsEndPos = itsFile.tellp();
  }

  void SourceDBBlob::addSource (const SourceData& source, bool)
  {
    source.writeSource (*itsBlobOut);
  }

  void SourceDBBlob::addSource (const SourceInfo& sourceInfo,
                                const string& patchName,
                                int catType,
                                double apparentBrightness,
                                const ParmMap& defaultParameters,
                                double ra, double dec,
                                bool check)
  {
    addPatch (patchName, catType, apparentBrightness, ra, dec, check);
    addSource (sourceInfo, patchName, defaultParameters, ra, dec, check);
  }

  void SourceDBBlob::deleteSources (const string&)
  {
    THROW (Exception, "SourceDBBlob::deleteSources not possible");
  }

  vector<string> SourceDBBlob::getPatches (int category, const string& pattern,
                                           double minBrightness,
                                           double maxBrightness)
  {
    // If not done yet, read all data from the file.
    readAll();
    Regex regex;
    if (pattern.size() > 0) {
      regex = Regex::fromPattern(pattern);
    }
    // Fill the patch names selecting only the required ones.
    vector<String> names;
    vector<Int> categories;
    vector<double> brightness;
    names.reserve (itsPatches.size());
    categories.reserve (itsPatches.size());
    brightness.reserve (itsPatches.size());
    for (map<string,PatchInfo>::const_iterator iter=itsPatches.begin();
         iter!=itsPatches.end(); ++iter) {
      if ((category < 0       ||  iter->second.getCategory() == category)  &&
          (minBrightness < 0  ||  iter->second.apparentBrightness() >= minBrightness)  &&
          (maxBrightness < 0  ||  iter->second.apparentBrightness() <= maxBrightness)  &&
          (pattern.size() == 0  ||  String(iter->first).matches (regex))) {
        names.push_back (iter->first);
        categories.push_back (iter->second.getCategory());
        brightness.push_back (iter->second.apparentBrightness());
      }
    }
    // Sort in order of category, brightness, name.
    Sort sort;
    sort.sortKey (&(categories[0]), TpInt);
    sort.sortKey (&(brightness[0]), TpDouble, 0, Sort::Descending);
    sort.sortKey (&(names[0]), TpString);
    Vector<uInt> index(names.size());
    sort.sort (index, names.size());
    vector<string> nmout;
    nmout.reserve (names.size());
    for (uint i=0; i<names.size(); ++i) {
      nmout.push_back (names[index[i]]);
    }
    return nmout;
  }

  vector<PatchInfo> SourceDBBlob::getPatchInfo (int category,
                                                const string& pattern,
                                                double minBrightness,
                                                double maxBrightness)
  {
    vector<string> names = getPatches(category, pattern, minBrightness,
                                      maxBrightness);
    vector<PatchInfo> info;
    info.reserve (names.size());
    for (vector<string>::const_iterator iter=names.begin();
         iter!=names.end(); ++iter) {
      info.push_back (itsPatches.find(*iter)->second);
    }
    return info;
  }

  vector<SourceInfo> SourceDBBlob::getPatchSources (const string& patchName)
  {
    vector<SourceInfo> info;
    map<string,vector<SourceData> >::const_iterator iter =
      itsSources.find(patchName);
    if (iter != itsSources.end()) {
      const vector<SourceData>& sources = iter->second;
      info.reserve (sources.size());
      for (vector<SourceData>::const_iterator srciter=sources.begin();
         srciter!=sources.end(); ++srciter) {
        info.push_back (srciter->getInfo());
      }
    }
    return info;
  }

  vector<SourceData> SourceDBBlob::getPatchSourceData (const string& patchName)
  {
    return itsSources[patchName];
  }

  SourceInfo SourceDBBlob::getSource (const string&)
  {
    THROW (Exception, "SourceDBBlob::getSource not implemented");
  }

  vector<SourceInfo> SourceDBBlob::getSources (const string&)
  {
    THROW (Exception, "SourceDBBlob::getSources not implemented");
    return vector<SourceInfo>();
  }

  bool SourceDBBlob::atEnd()
  {
    return itsFile.tellg() >= itsEndPos;
  }

  void SourceDBBlob::rewind()
  {
    itsFile.seekg (0, ios::beg);
  }

  void SourceDBBlob::getNextSource (SourceData& src)
  {
    while (true) {
      string type = itsBlobIn->getNextType();
      if (type == "source") {
        break;
      }
      // Skip the patch info.
      skipPatch();
    }
    src.readSource (*itsBlobIn);
  }

  PatchInfo SourceDBBlob::readPatch()
  {
    double ra, dec, brightness;
    string name;
    int16 catType;
    ASSERT (itsBlobIn->getStart ("patch") == 1);   // version must be 1
    *itsBlobIn >> name >> catType >> brightness >> ra >> dec;
    itsBlobIn->getEnd();
    return PatchInfo (name, ra, dec, catType, brightness);
  }

  void SourceDBBlob::readAll()
  {
    // Only read if not done yet.
    if (itsSources.size() > 0) {
      return;
    }
    // Keep current position.
    int64 pos = itsFile.tellg();
    // Read from beginning till end.
    rewind();
    while (itsFile.tellg() < itsEndPos) {
      if (itsBlobIn->getNextType() == "patch") {
        PatchInfo info (readPatch());
        itsPatches.insert (make_pair(info.getName(), info));
      } else {
        SourceData info;
        info.readSource (*itsBlobIn);
        itsSources[info.getPatchName()].push_back (info);
      }
    }
    // Reset original position.
    itsFile.seekp (pos);
  }

} // namespace BBS
} // namespace LOFAR
