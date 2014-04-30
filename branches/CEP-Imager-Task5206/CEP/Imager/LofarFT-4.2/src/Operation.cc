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
  
Operation::Operation(ParameterSet& parset):
    itsParset(parset)
{}

void Operation::init()
{
  itsMSName = itsParset.getString("ms");
  itsMS = MeasurementSet(itsMSName, Table::Update);
}

// Show the help info.
void Operation::showHelp (ostream& os, const string& name) 
{
  os<<
  "General parameters:"
  "  operation       : operation name                                "<<endl<<
  "                    (string,  no default   )                      "<<endl<<
  "  displayprogress : display progress                              "<<endl<<
  "                    (bool  ,  default false)                      "<<endl<<
  "  verbose         : verbosity level                               "<<endl<<
  "                    (int   ,  default 0    )                      "<<endl<<
  "  chunksize       : amount of data read at once                   "<<endl<<
  "                    (int   ,  default 0    )                      "<<endl;
};

void Operation::run()
{
  init();
  itsImager = new Imager(itsMS, itsParameters, itsParset);
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
