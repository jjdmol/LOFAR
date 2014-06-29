//#  AntennaField.cc: one_line_description
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
//#  $Id: AntennaField.cc 15145 2010-03-05 10:35:46Z overeem $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <APL/APLCommon/AntennaField.h>
#include <Common/LofarLogger.h>

using namespace blitz;

namespace LOFAR {

//-------------------------- creation and destroy ---------------------------
static AntennaField* globalAntennaFieldInstance = 0;

AntennaField* globalAntennaField()
{
  if (globalAntennaFieldInstance == 0) {
    globalAntennaFieldInstance = new AntennaField("AntennaField.conf");
  }
  return (globalAntennaFieldInstance);
}

//
// AntennaField(fileName)
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
// The info is stored in itsxxxAntPos[ant,pol,xyz] and in itsxxxRCUPos[rcu,xyz] because
// some programs are antenna based while others are rcu based.
//
AntennaField::AntennaField(const string& filename)
  : itsAntField (filename)
{
  int maxFields = itsAntField.maxFields();
  // Reserve space for expected info.
  itsAntPos.resize         (maxFields);
  itsRCUPos.resize         (maxFields);
  itsFieldCentres.resize   (maxFields);
  itsNormVectors.resize    (maxFields);
  itsRotationMatrix.resize (maxFields);
  itsRCULengths.resize     (maxFields);
  // Create the blitz arrays for the various fields.
  for (int i=0; i<maxFields; ++i) {
    makeArray3d (itsAntField.AntPos(i),         itsAntPos[i]);
    makeArray1d (itsAntField.Centre(i),         itsFieldCentres[i]);
    makeArray1d (itsAntField.normVector(i),     itsNormVectors[i]);
    makeArray2d (itsAntField.rotationMatrix(i), itsRotationMatrix[i]);
    makeArray1d (itsAntField.RCULengths(i),     itsRCULengths[i]);
    // Create RCUPos from the AntPos (same data, different shape).
    makeRCUPos  (itsAntField.AntPos(i),         itsRCUPos[i]);
  }
}

//
// ~AntennaField()
//
AntennaField::~AntennaField()
{
}

void AntennaField::makeArray1d (AntField::AFArray& array,
                                blitz::Array<double,1>& out)
{
  if (! AntField::getShape(array).empty()) {
    TinyVector<int,1> shape;
    shape[0] = AntField::getShape(array)[0];
    double* dataPtr = &(AntField::getData(array)[0]);
    out.reference (blitz::Array<double,1>(dataPtr, shape,
                                          blitz::neverDeleteData));
  }
}

void AntennaField::makeArray2d (AntField::AFArray& array,
                                blitz::Array<double,2>& out)
{
  if (! AntField::getShape(array).empty()) {
    TinyVector<int,2> shape;
    shape[0] = AntField::getShape(array)[0];
    shape[1] = AntField::getShape(array)[1];
    double* dataPtr = &(AntField::getData(array)[0]);
    out.reference (blitz::Array<double,2>(dataPtr, shape,
                                          blitz::neverDeleteData));
  }
}

void AntennaField::makeArray3d (AntField::AFArray& array,
                                blitz::Array<double,3>& out)
{
  if (! AntField::getShape(array).empty()) {
    TinyVector<int,3> shape;
    shape[0] = AntField::getShape(array)[0];
    shape[1] = AntField::getShape(array)[1];
    shape[2] = AntField::getShape(array)[2];
    double* dataPtr = &(AntField::getData(array)[0]);
    out.reference (blitz::Array<double,3>(dataPtr, shape,
                                          blitz::neverDeleteData));
  }
}

void AntennaField::makeRCUPos (AntField::AFArray& array,
                               blitz::Array<double,2>& out)
{
  if (! AntField::getShape(array).empty()) {
    TinyVector<int,2> shape;
    shape[0] = AntField::getShape(array)[0] * AntField::getShape(array)[1];
    shape[1] = AntField::getShape(array)[2];
    double* dataPtr = &(AntField::getData(array)[0]);
    out.reference (blitz::Array<double,2>(dataPtr, shape,
                                          blitz::neverDeleteData));
  }
}

} // namespace LOFAR
