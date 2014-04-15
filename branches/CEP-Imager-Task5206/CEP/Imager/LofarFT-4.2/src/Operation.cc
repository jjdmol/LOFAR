//# OperationClean.cc:
//#
//# Copyright (C) 2014
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
//# $Id: $

#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <LofarFT/Operation.h>

using namespace casa;

namespace LOFAR {
namespace LofarFT {
  
Operation::Operation()
{
  itsInputParSet.create( "ms", "", "Name of input MeasurementSet","string");
  itsInputParSet.create ("operation", "image", "", "string");
  
  itsInputParSet.create ("verbose", "0",
    "0=some output, 1=more output, 2=even more output",
    "int");
   
  itsInputParSet.create ("ApplyBeamCode", "0",
                  "Ask developers.",
                  "int");
  itsInputParSet.create ("UseMasks", "true",
                  "When the element beam is applied (ApplyElement), the addictional step of convolving the grid can be made more efficient by computing masks. If true, it will create a directory in which it stores the masks.",
                  "bool");
  itsInputParSet.create ("UVmin", "0",
                  "Minimum UV distance (klambda)",
                  "Double");
  itsInputParSet.create ("UVmax", "1000",
                  "Maximum UV distance (klambda)",
                  "Double");
  itsInputParSet.create ("MakeDirtyCorr", "false",
                  "Image plane correction.",
                  "bool");
  itsInputParSet.create ("UseWSplit", "true",
                  "W split.",
                  "bool");
  itsInputParSet.create ("SpheSupport", "15",
                  "Spheroidal/Aterm Support.",
                  "Double");
  itsInputParSet.create ("t0", "-1",
                  "tmin in minutes since beginning.",
                  "Double");
  itsInputParSet.create ("t1", "-1",
                  "tmax in minutes since beginning.",
                  "Double");
  itsInputParSet.create ("SingleGridMode", "true",
                  "If set to true, then the FTMachine uses only one grid.",
                  "bool");
}

void Operation::readArguments (int argc, char const* const* argv) 
{
  itsInputParSet.readArguments(argc, argv);
};

// Show the help info.
void Operation::showHelp (ostream& os, const string& name) 
{
  itsInputParSet.showHelp(os, name);
};

void Operation::setVersion (const string& version) 
{
  itsInputParSet.setVersion(version);
};

void Operation::run()
{
  itsMSName = itsInputParSet.getString("ms");
  itsParameters.define("verbose", itsInputParSet.getInt("verbose"));
  itsMS = MeasurementSet(itsMSName, Table::Update);
  itsImager = new Imager(itsMS, itsParameters);
}


Quantity Operation::readQuantity (const String& in)
{
  Quantity res;
  if (!Quantity::read(res, in)) {
    throw AipsError (in + " is an illegal quantity");
  }
  return res;
}

MDirection Operation::readDirection (const String& in)
{
  Vector<String> vals = stringToVector(in);
  if (vals.size() > 3) {
    throw AipsError ("MDirection value " + in + " is invalid;"
                     " up to 3 values can be given");
  }
  MDirection::Types tp;
  if (! MDirection::getType (tp, vals[0])) {
    throw AipsError(vals[0] + " is an invalid MDirection type");
  }
  Quantity v0(0, "deg");
  Quantity v1(90, "deg");     // same default as in measures.g
  if (vals.size() > 1  &&  !vals[1].empty()) {
    v0 = readQuantity(vals[1]);
  }
  if (vals.size() > 2  &&  !vals[2].empty()) {
    v1 = readQuantity(vals[2]);
  }
  return MDirection(v0, v1, tp);
}

void Operation::readFilter (const String& filter,
                 Quantity& bmajor, Quantity& bminor, Quantity& bpa)
{
  if (filter.empty()) {
    return;
  }
  Vector<String> strs = stringToVector(filter);
  if (strs.size() != 3) {
    throw AipsError("Specify gaussian tapering filter as bmajor,bminor,bpa");
  }
  if (! strs[0].empty()) {
    bmajor = readQuantity (strs[0]);
  }
  if (! strs[1].empty()) {
    bminor = readQuantity (strs[1]);
  }
  if (! strs[2].empty()) {
    bpa = readQuantity (strs[2]);
  }
}


} //# namespace LofarFT

} //# namespace LOFAR
