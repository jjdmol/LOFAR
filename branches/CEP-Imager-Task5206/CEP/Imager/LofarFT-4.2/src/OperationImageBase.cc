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
#include <LofarFT/OperationImageBase.h>


using namespace casa;


namespace LOFAR {
namespace LofarFT {

OperationImageBase::OperationImageBase(ParameterSet& parset): Operation(parset), OperationParamFTMachine(parset)
{}



void OperationImageBase::run()
{
  Operation::run();
  OperationParamFTMachine::run();
  
  Quantity qcellsize = readQuantity (itsParset.getString("cellsize","1arcsec"));
  MDirection phaseCenter;
  Bool doShift = False;
  Int fieldid = 0;
  Int nfacet = 0;

  MSSpWindowColumns window(itsMS.spectralWindow());
  Vector<Int> wind(window.nrow());
  for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};

  
  itsImager->defineImage (
    itsParset.getInt("npix",256),        // nx
    itsParset.getInt("npix",256),        // ny
    qcellsize,                           // cellx
    qcellsize,                           // celly
    String("I"),                         // stokes
    phaseCenter,                         // phaseCenter
    doShift  ?  -1 : fieldid,            // fieldid
    itsParset.getString("mode","mfs"),   // mode
    itsParset.getInt("img_nchan",1),     // nchan
    itsParset.getInt("img_chanstart",0), // start
    itsParset.getInt("img_chanstep",1),  // step
    MFrequency(),                        // mFreqstart
    MRadialVelocity(),                   // mStart
    Quantity(1,"km/s"),                  // qstep, Def=1 km/s
    wind,//spwid,                        // spectralwindowids
    nfacet);                             // facets
}

void OperationImageBase::init()
{
}

void OperationImageBase::makeEmpty (const String& imgName, Int fieldid)
{
  CoordinateSystem coords;
  AlwaysAssert (itsImager->imagecoordinates(coords), AipsError);
  String name(imgName);
  itsImager->makeEmptyImage(coords, name, fieldid);
  itsImager->unlock();
}

void OperationImageBase::showHelp (ostream& os, const string& name)
{
  //TODO
};


} //# namespace LofarFT
} //# namespace LOFAR
