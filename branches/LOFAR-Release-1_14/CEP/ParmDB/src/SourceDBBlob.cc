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
#include <iostream>

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

  void SourceDBBlob::addSource (const SourceInfo& sourceInfo,
                                const string& patchName,
                                int,
                                double,
                                const ParmMap& defaultParameters,
                                double ra, double dec,
                                bool check)
  {
    addSource (sourceInfo, patchName, defaultParameters,
               ra, dec, check);
  }

  void SourceDBBlob::deleteSources (const string&)
  {
    THROW (Exception, "SourceDBBlob::deleteSources not possible yet");
  }

  vector<string> SourceDBBlob::getPatches (int, const string&,
                                           double,
                                           double)
  {
    THROW (Exception, "SourceDBBlob::getPatches not implemented");
    return vector<string>();
  }

  vector<PatchInfo> SourceDBBlob::getPatchInfo (int,
                                                const string&,
                                                double,
                                                double)
  {
    THROW (Exception, "SourceDBBlob::getPatchInfo not implemented");
    return vector<PatchInfo>();
  }

  vector<SourceInfo> SourceDBBlob::getPatchSources (const string&)
  {
    THROW (Exception, "SourceDBBlob::getPatchSources not implemented");
    return vector<SourceInfo>();
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

} // namespace BBS
} // namespace LOFAR
