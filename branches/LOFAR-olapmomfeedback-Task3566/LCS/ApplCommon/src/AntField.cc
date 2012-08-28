//#  AntField.cc: Class to manage an antenna field description.
//#
//#  Copyright (C) 2009
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
#include <Common/LofarLocators.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_fstream.h>
#include <Common/StringUtil.h>
#include <Common/StreamUtil.h>
#include <Common/SystemUtil.h>
#include <ApplCommon/AntField.h>

namespace LOFAR {
  // AntField(fileName)
  // The structure of the file is as follows:
  //
  // NORMAL-VECTOR <field>
  // 3 [ x y z ]
  //
  // ROTATION-MATRIX <field>
  // 3 x 3 [ ... ]
  //
  // <field>
  // 3 [ x y z ]
  // Nant x Npol x 3 [ ... ]
  //
  // optionally followed by more fields.
  //
  // The info is stored in itsxxxAntPos[ant,pol,xyz] and in
  // itsxxxRCUPos[rcu,xyz] because some programs are antenna based,
  // while others are rcu based.
  //
  AntField::AntField(const string& filename, bool mustExist)
  {
    // Locate the file (only done if no absolute path given).
    ConfigLocator cl;
    string   fullFilename(cl.locate(filename));
    ifstream inputStream (fullFilename.c_str());
    if (mustExist) {
      ASSERTSTR(inputStream.good(), "File :" << fullFilename
                << " (extracted from: " << filename
                << ") cannot be opened succesfully.");
    }

    // Reserve space for expected info.
    itsAntPos.resize         (MAX_FIELDS);
    itsFieldCentres.resize   (MAX_FIELDS);
    itsNormVectors.resize    (MAX_FIELDS);
    itsRotationMatrix.resize (MAX_FIELDS);
    itsRCULengths.resize     (MAX_FIELDS);

    // Fill in zeroes if the file does not exist.
    if (inputStream.good()) {
      readFile (inputStream, fullFilename);
      inputStream.close();
      LOG_INFO_STR("Antenna position file " << fullFilename
                   << " read in succesfully");
    
    } else {
      LOG_ERROR_STR("Antenna position file " << fullFilename
                    << " not found; using zeroes instead");
      setZeroes (filename);
    }

    // Finally construct the HBA0 and HBA1 info if their centre positions
    // are given.
    if (! getShape(itsFieldCentres[HBA0_IDX]).empty()) {
      LOG_DEBUG("Constructing HBA0 and HBA1 from HBA information");
      makeSubField (HBA0_IDX);
      makeSubField (HBA1_IDX);
    }

  }

  AntField::~AntField()
  {}

  void AntField::readFile (istream& inputStream, const string& fullFilename)
  {
    string fieldName;
    string inputLine;
    int    fieldIndex;
    // Read file and skip lines that start with '#' or are empty.
    // A line starting with ] is also empty.
    while (getline(inputStream, inputLine)) {
      if (inputLine.empty() || inputLine[0] == '\0' || inputLine[0] == ']' ||
          inputLine[0] == '#') {
        continue;
      }

      // Get the field name (is last word).
      vector<string> fields = StringUtil::split(inputLine, ' ');
      fieldName = fields[fields.size()-1];
      fieldIndex = name2Index(fieldName);
      ASSERTSTR(fieldIndex >= 0,
                "Only LBA, HBA, HBA0, and HBA1 allowed for antenna field "
                "(not '" << fieldName << "')");
      if (fields.size() == 2) {
        // NORMAL_VECTOR or ROTATION_MATRIX
        if (fields[0].find("NORMAL_VECTOR") != string::npos) {
          AFArray& nVect = itsNormVectors[fieldIndex];
          readBlitzArray<1> (nVect, inputStream);
          ASSERTSTR(getShape(nVect)[0]==3,
                    "NORM_VECTOR of field '" << fieldName
                    << " in " << fullFilename
                    << "' should be a 1 dimensional array with 3 values, not "
                    << getShape(nVect));
          LOG_DEBUG_STR(fieldName << " NORM_VECTOR is " << getData(nVect));
          continue;
        } else if (fields[0].find("ROTATION_MATRIX") != string::npos) {
          AFArray& rMat = itsRotationMatrix[fieldIndex];
          readBlitzArray<2> (rMat, inputStream);
          ASSERTSTR(getShape(rMat)[0]==3 && getShape(rMat)[1]==3,
                    "ROTATION_MATRIX of field '" << fieldName
                    << " in " << fullFilename
                    << "' should be a 2 dimensional array with 3x3 values, not "
                    << getShape(rMat));
          LOG_DEBUG_STR(fieldName << " ROTATION_MATRIX is " << getData(rMat));
          continue;
        } else {
          LOG_ERROR_STR("Unknown keyword '" << fields[0] << "' in "
                        << fullFilename);
        }
      } else { // expect centre position
        AFArray& centrePos = itsFieldCentres[fieldIndex];
        readBlitzArray<1> (centrePos, inputStream);
        ASSERTSTR(getShape(centrePos)[0]==3,
                  "Center position of field '" << fieldName
                  << " in " << fullFilename
                  << "' should be a 1 dimensional array with 3 values, not "
                  << getShape(centrePos));
        LOG_DEBUG_STR(fieldName << " Center is  at"
                      << getData(centrePos));
      }

      // TODO?
      // Allow HBA0 and HBA1 to be defined in core stations in stead of HBA ???
      // It complicates reading in the file, but is more intuitive for the user.
      // For now we require that LBA and HBA must be defined always.
      
      // Positions are not given for the HBA subfields.
      if (fieldName != "LBA"  &&  fieldName != "HBA") {
        continue;
      }

      // Read positions.
      AFArray& antennaPos = itsAntPos[fieldIndex];
      readBlitzArray<3> (antennaPos, inputStream);
      ASSERTSTR(getShape(antennaPos)[0] <= int(MAX_ANTENNAS)  &&
                getShape(antennaPos)[1] == int(N_POL)  &&
                getShape(antennaPos)[2] == 3,
                "Expected an array of size NrAntennas x nrPol x 3"
                " for field " << fieldName << " in " << fullFilename);
      LOG_DEBUG_STR(fieldName << " dimensions is " << getShape(antennaPos));

      // Calculate length of RCU vectors.
      makeRCULen (fieldIndex);
    } // while not EOF
    ASSERTSTR(!getShape(itsAntPos[LBA_IDX]).empty() &&
              !getShape(itsAntPos[HBA_IDX]).empty(),
              "File " << fullFilename <<
              "should contain definitions for both LBA and HBA antennafields");
    ASSERTSTR(getShape(itsFieldCentres[HBA0_IDX]).empty() ==
              getShape(itsFieldCentres[HBA1_IDX]).empty(),
              "File " << fullFilename <<
              "should contain definitions for both HBA0 and HBA1 or none");
  }

  void AntField::setZeroes (const string& fileName)
  {
    // Determine statiuon type (core, remote, or other).
    string bname(basename(fileName));
    string stype(bname.substr(0,2));
    int nlba = 96;
    int nhba = 96;
    if (stype == "CS"  ||  stype == "RS") {
      nhba = 48;
    }
    initArray (itsAntPos[LBA_IDX], nlba, 2, 3);
    initArray (itsAntPos[HBA_IDX], nhba, 2, 3);
    if (stype == "CS") {
      initArray (itsAntPos[HBA0_IDX], 24, 2, 3);
      initArray (itsAntPos[HBA1_IDX], 24, 2, 3);
    }
    for (int i=0; i<MAX_FIELDS; ++i) {
      if (stype == "CS"  ||  i < HBA0_IDX) {
        initArray (itsFieldCentres[i], 3);
        initArray (itsNormVectors[i], 3);
        initArray (itsRotationMatrix[i], 3, 3);
        makeRCULen (i);
      }
    }
  }

  void AntField::initArray (AntField::AFArray& array, size_t n1)
  {
    vector<size_t>& shape = AntField::getShape(array);
    vector<double>& data  = AntField::getData(array);
    shape.resize (1);
    shape[0] = n1;
    data.resize (n1);
    std::fill (data.begin(), data.end(), 0.);
  }

  void AntField::initArray (AntField::AFArray& array, size_t n1, size_t n2)
  {
    vector<size_t>& shape = AntField::getShape(array);
    vector<double>& data  = AntField::getData(array);
    shape.resize (2);
    shape[0] = n1;
    shape[1] = n2;
    data.resize (n1*n2);
    std::fill (data.begin(), data.end(), 0.);
  }

  void AntField::initArray (AntField::AFArray& array,
                            size_t n1, size_t n2, size_t n3)
  {
    vector<size_t>& shape = AntField::getShape(array);
    vector<double>& data  = AntField::getData(array);
    shape.resize (3);
    shape[0] = n1;
    shape[1] = n2;
    shape[2] = n3;
    data.resize (n1*n2*n3);
    std::fill (data.begin(), data.end(), 0.);
  }

  int AntField::name2Index(const string& fieldName) const
  {
    if (fieldName == "LBA")  return(LBA_IDX);
    if (fieldName == "HBA")  return(HBA_IDX);
    if (fieldName == "HBA0") return(HBA0_IDX);
    if (fieldName == "HBA1") return(HBA1_IDX);
    return (-1);
  }

  void AntField::makeRCULen (int fieldIndex)
  {
    // Calculate length of vectors.
    const AFArray& antennaPos = itsAntPos[fieldIndex];
    AFArray& rcuLen = itsRCULengths[fieldIndex];
    int nrrcu = getShape(antennaPos)[0] * N_POL;
    getShape(rcuLen).resize (1);
    getShape(rcuLen)[0] = nrrcu;
    getData(rcuLen).resize (nrrcu);
    for (int i=0; i<nrrcu; ++i) {
      const double* pos = &(getData(antennaPos)[i*3]);
      getData(rcuLen)[i] = sqrt(pos[0]*pos[0] + pos[1]*pos[1] + pos[2]*pos[2]);
    }
  }

  void AntField::makeSubField (int fieldIndex)
  {
    // Get the difference of the centres of subfield and full field.
    vector<double> diff(3);
    for (int i=0; i<3; ++i) {
      diff[i] = (getData(itsFieldCentres[fieldIndex])[i] -
                 getData(itsFieldCentres[HBA_IDX])[i]);
    }
    // Get the appropriate subset of the full antenna field.
    AFArray& fullAntPos = itsAntPos[HBA_IDX];
    int halfNrAnts = nrAnts("HBA") / 2;
    int halfSize   = getData(fullAntPos).size() / 2;
    AFArray& antPos = itsAntPos[fieldIndex];
    getShape(antPos) = getShape(fullAntPos);
    getShape(antPos)[0] = halfNrAnts;
    getData(antPos).resize (halfSize);
    double* to         = &(getData(antPos)[0]);
    const double* from = &(getData(fullAntPos)[0]);
    if (fieldIndex == HBA1_IDX) {
      from += halfSize;
    }
    for (int i=0; i<N_POL*halfNrAnts; ++i) {
      *to++ = *from++ - diff[0];
      *to++ = *from++ - diff[1];
      *to++ = *from++ - diff[2];
    }
    makeRCULen (fieldIndex);
  }

  int AntField::maxFields() const
  {
    return MAX_FIELDS;
  }

  int AntField::nrAnts(const string& fieldName) const
  {
    const vector<size_t>& shape = getShape(itsAntPos[name2Index(fieldName)]);
    return (shape.empty()  ?  0 : shape[0]);
  }

} // namespace LOFAR
