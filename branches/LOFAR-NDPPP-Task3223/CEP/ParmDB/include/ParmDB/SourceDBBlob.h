//# SourceDBBlob.h: Class for a Blob file holding sources and their parameters
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

// @file
// @brief Base class for a table holding sources and their parameters
// @author Ger van Diepen (diepen AT astron nl)

#ifndef LOFAR_PARMDB_SOURCEDBBLOB_H
#define LOFAR_PARMDB_SOURCEDBBLOB_H

//# Includes
#include <ParmDB/SourceDB.h>
#include <ParmDB/SourceInfo.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_set.h>
#include <fstream>
#include <Blob/BlobOBufStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>


namespace LOFAR {
namespace BBS {

  // @ingroup ParmDB
  // @{

  // @brief Class holding a  data of a source.
  class SourceData
  {
  public:
    SourceData();

    SourceData (const SourceInfo&, const string& patchName,
                double ra, double dec);

    const SourceInfo& getInfo() const
      { return itsInfo; }
    const string& getPatchName() const
      { return itsPatchName; }
    double getRa() const
      { return itsRa; }
    double getDec() const
      { return itsDec; }
    double getI() const
      { return itsI; }
    double getQ() const
      { return itsQ; }
    double getU() const
      { return itsU; }
    double getV() const
      { return itsV; }
    double getMajorAxis() const
      { return itsMajorAxis; }
    double getMinorAxis() const
      { return itsMinorAxis; }
    double getOrientation() const
      { return itsOrientation; }
    double getPolarizationAngle() const
      { return itsPolAngle; }
    double getPolarizedFraction() const
      { return itsPolFrac; }
    double getRotationMeasure() const
      { return itsRM; }
    const vector<double>& getSpectralIndex() const
      { return itsSpInx; }

    void setParms (const ParmMap& defaultParameters);

    void writeSource (BlobOStream&);
    void readSource (BlobIStream&);

  private:
    // Set a parameter.
    // If defined, its value is taken from the map.
    // Otherwise the default value is used.
    void setParm (const ParmMap& parms, const string& name,
                  double defValue, double& value);

    SourceInfo     itsInfo;
    string         itsPatchName;
    double         itsRa;
    double         itsDec;
    double         itsI;
    double         itsQ;
    double         itsU;
    double         itsV;
    double         itsMajorAxis;
    double         itsMinorAxis;
    double         itsOrientation;
    double         itsPolAngle;
    double         itsPolFrac;
    double         itsRM;
    vector<double> itsSpInx;
  };

  // @brief Class for a Blob file holding source parameters.
  class SourceDBBlob : public SourceDBRep
  {
  public:
    SourceDBBlob (const ParmDBMeta& pdm, bool forceNew);

    virtual ~SourceDBBlob();

    // Writelock and unlock the file.
    // It does not doanything.
    // <group>
    virtual void lock (bool lockForWrite);
    virtual void unlock();
    // </group>

    // Check for duplicate patches or sources.
    // An exception is thrown if that is the case.
    virtual void checkDuplicates();

    // Find non-unique patch names.
    virtual vector<string> findDuplicatePatches();

    // Find non-unique source names.
    virtual vector<string> findDuplicateSources();

    // Test if the patch already exists.
    virtual bool patchExists (const string& patchName);

    // Test if the source already exists.
    virtual bool sourceExists (const string& sourceName);

    // Add a patch and return its patchId.
    // Nomally ra and dec should be filled in, but for moving patches
    // (e.g. sun) this is not needed.
    // <br>Optionally it is checked if the patch already exists.
    virtual uint addPatch (const string& patchName, int catType,
                           double apparentBrightness,
                           double ra, double dec,
                           bool check);

    // Add a source to a patch.
    // Its ra and dec and default parameters will be stored as default
    // values in the associated ParmDB tables. The names of the parameters
    // will be preceeded by the source name and a colon.
    // The map should contain the parameters belonging to the source type.
    // Missing parameters will default to 0.
    // <br>Optionally it is checked if the patch already exists.
    virtual void addSource (const SourceInfo& sourceInfo,
                            const string& patchName,
                            const ParmMap& defaultParameters,
                            double ra, double dec,
                            bool check);

    // Add a source which forms a patch in itself (with the same name).
    // <br>Optionally it is checked if the patch or source already exists.
    virtual void addSource (const SourceInfo& sourceInfo,
                            int catType,
                            double apparentBrightness,
                            const ParmMap& defaultParameters,
                            double ra, double dec,
                            bool check);

    // Get patch names in order of category and decreasing apparent flux.
    // category < 0 means all categories.
    // A brightness < 0 means no test on brightness.
    virtual vector<string> getPatches (int category, const string& pattern,
                                       double minBrightness,
                                       double maxBrightness);

    // Get the info of all patches (name, ra, dec).
    virtual vector<PatchInfo> getPatchInfo (int category,
                                            const string& pattern,
                                            double minBrightness,
                                            double maxBrightness);

    // Get the sources belonging to the given patch.
    virtual vector<SourceInfo> getPatchSources (const string& patchName);

    // Get the source info of the given source.
    virtual SourceInfo getSource (const string& sourceName);

    // Get the info of all sources matching the given (filename like) pattern.
    virtual vector<SourceInfo> getSources (const string& pattern);

    // Delete the sources records matching the given (filename like) pattern.
    // This is not possible yet.
    virtual void deleteSources (const std::string& sourceNamePattern);

    // Clear file (i.e. remove everything).
    virtual void clearTables();

    // Get the next source from the table.
    // An exception is thrown if there are no more sources.
    void getNextSource (SourceData& src);

    // Tell if we are the end of the file.
    bool atEnd();

    // Reset to the beginning of the file.
    void rewind();

  private:
    // Skip the patch info in the file.
    void skipPatch();

    //# Data members
    std::fstream                      itsFile;
    boost::shared_ptr<BlobIBufStream> itsBufIn;
    boost::shared_ptr<BlobOBufStream> itsBufOut;
    boost::shared_ptr<BlobIStream>    itsBlobIn;
    boost::shared_ptr<BlobOStream>    itsBlobOut;
    bool  itsCanWrite;
    int64 itsEndPos;
  };

  // @}

} // namespace BBS
} // namespace LOFAR

#endif
