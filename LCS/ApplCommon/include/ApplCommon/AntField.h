//#  AntField.h: Class to manage an antenna field description.
//#
//#  Copyright (C) 2010
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

#ifndef LOFAR_APPLCOMMON_ANTFIELD_H
#define LOFAR_APPLCOMMON_ANTFIELD_H

// \file AntField.h
// Class to manage the antenna subsets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/LofarLogger.h>

//# Avoid 'using namespace' in headerfiles

namespace LOFAR {

  // \addtogroup ApplCommon
  // @{

  //# --- Forward Declarations ---
  //# classes mentioned as parameter or returntype without virtual functions.


  // This class represents an antenna array. The LOFAR remote station will
  // initially have two antenna arrays. One for the low-band antennae,
  // and one for the high-band antennae.
  // Alternative configurations of the antennas are described in AntennaSets.

  class AntField
  {
  public:
    // Define array as pair of shape and data.
    typedef pair<vector<size_t>, vector<double> > AFArray;

    // Read the AntennaField file.
    // If <src>mustExist=true</src> an exception is thrown if the file does
    // not exist. Otherwise an error message is logged and zero data is filled
    // in for the various arrays.
    explicit AntField (const string& filename, bool mustExist=true);

    ~AntField();

    // all about the bits
    // [antNr, pol, xyz]
    const AFArray& AntPos(const string& fieldName) const
      { return (itsAntPos[name2Index(fieldName)]); }

    // [xyz]
    const AFArray& Centre(const string& fieldName) const
      { return (itsFieldCentres[name2Index(fieldName)]); }

    // [rcu]
    const AFArray& RCULengths(const string& fieldName) const
      { return (itsRCULengths[name2Index(fieldName)]); }

    // [xyz]
    const AFArray& normVector(const string& fieldName) const
      { return (itsNormVectors[name2Index(fieldName)]); }

    // [3, 3]
    const AFArray& rotationMatrix(const string& fieldName) const
      { return (itsRotationMatrix[name2Index(fieldName)]); }


    static const int LBA_IDX    = 0;
    static const int HBA_IDX    = 1;
    static const int HBA0_IDX   = 2;
    static const int HBA1_IDX   = 3;
    static const int MAX_FIELDS = 4;

    AFArray& AntPos(int fieldIndex)
      { return (itsAntPos[fieldIndex]); }
    AFArray& Centre(int fieldIndex)
      { return (itsFieldCentres[fieldIndex]); }
    AFArray& RCULengths(int fieldIndex)
      { return (itsRCULengths[fieldIndex]); }
    AFArray& normVector(int fieldIndex)
      { return (itsNormVectors[fieldIndex]); }
    AFArray& rotationMatrix(int fieldIndex)
      { return (itsRotationMatrix[fieldIndex]); }

    int nrAnts(const string& fieldName) const;

    bool isAntField(const string& name) const
      { return (name2Index(name) >= 0); }

    int ringNr(const string& fieldName) const
      { return ((fieldName == "HBA1") ? 1 : 0); }

    // Helper functions to get shape and data.
    static vector<size_t>& getShape (AFArray& array)
      { return array.first; }
    static const vector<size_t>& getShape (const AFArray& array)
      { return array.first; }
    static vector<double>& getData (AFArray& array)
      { return array.second; }
    static const vector<double>& getData (const AFArray& array)
      { return array.second; }

    // Translate name of antennaField to index in array vectors.
    int name2Index(const string& fieldName) const;

    // Get max nr of fields
    int maxFields() const;

    // Helper function to read an array in Blitz format.
    template<int NDIM>
    static void readBlitzArray(AntField::AFArray& array, istream& is)
    {
      char sep;
      vector<size_t>& shape = AntField::getShape(array);
      vector<double>& data  = AntField::getData(array);
      shape.resize (NDIM);
      // Read the extent vector (shape): this is separated by 'x's, e.g.
      //   3 x 4 x 5
      size_t sz = 1;
      for (uint i=0; i<NDIM; ++i) {
        if (i > 0) {
          is >> sep;
          ASSERTSTR(sep == 'x', "Format error while scanning input array"
                    << endl << " (expected 'x' between array extents)");
        }
        is >> shape[i];
        ASSERTSTR(!is.bad(), "Premature end of input while scanning array");
        sz *= shape[i];
      }
      is >> sep;
      ASSERTSTR(sep == '[', "Format error while scanning input array"
                << endl << " (expected '[' before beginning of array data)");
      data.resize (sz);
      for (vector<double>::iterator iter=data.begin();
           iter!=data.end(); ++iter) {
        ASSERTSTR(!is.bad(), "Premature end of input while scanning array");
        is >> *iter;
      }
      is >> sep;
      ASSERTSTR(sep == ']', "Format error while scanning input array"
                << endl << " (expected ']' after end of array data)");
    }


  private:
    // Copying is not allowed.
    AntField(const AntField& that);
    AntField& operator=(const AntField& that);

    // Read the arrays from the file.
    void readFile (istream& inputStream, const string& fullFilename);

    // Initialize all arrays to zero.
    void setZeroes (const std::string& fileName);

    // Initialize an array to the given shape and fill with zeroes.
    void initArray (AntField::AFArray& array, size_t n1);
    void initArray (AntField::AFArray& array, size_t n1, size_t n2);
    void initArray (AntField::AFArray& array, size_t n1, size_t n2, size_t n3);

    // Calculate the length of the RCU vectors.
    void makeRCULen (int fieldIndex);

    // Create info for HBA0 or HBA1 subfield.
    void makeSubField (int fieldIndex);

    //# --- Datamembers ---
    // Note: we use a vector<AFArray> so every array can have its own size.
    vector<AFArray> itsFieldCentres;	// [ (x,y,z) ]
    vector<AFArray> itsAntPos;		// [ antNr, pol, (x,y,z) ]
    vector<AFArray> itsNormVectors;	// [ (x,y,z) ]
    vector<AFArray> itsRotationMatrix;	// [ 3, 3 ]
    // During calculations we often need the length of the vectors.
    vector<AFArray> itsRCULengths;
  };

  // @}
} // namespace LOFAR

#endif
