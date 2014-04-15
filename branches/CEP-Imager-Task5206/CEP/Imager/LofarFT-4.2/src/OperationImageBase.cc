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

OperationImageBase::OperationImageBase()
{
  itsInputParSet.create ("image", "", "Name of output image file (default is <msname-stokes-mode-nchan>.img)", "string");
  itsInputParSet.create ("fits", "no","Name of output image fits file ('no' means no fits file) empty is <imagename>.fits", "string");
  itsInputParSet.create ("hdf5", "no", "Name of output image HDF5 file ('no' means no HDF5 file) empty is <imagename>.hdf5", "string");
  itsInputParSet.create ("prior", "", "Name of prior image file (default is <imagename>.prior", "string");
  itsInputParSet.create ("data", "DATA", "Name of DATA column to use", "string");
  itsInputParSet.create ("mode", "mfs", "Imaging mode (mfs, channel, or velocity)", "string");
  itsInputParSet.create ("filter", "", "Apply gaussian tapering filter; specify as major,minor,pa", "string");
  itsInputParSet.create ("weight", "briggs",
                  "Weighting scheme (uniform, superuniform, natural, briggs (robust), briggsabs, or radial",
                  "string");
  itsInputParSet.create ("noise", "1.0",
                  "Noise (in Jy) for briggsabs weighting",
                  "float");
  itsInputParSet.create ("robust", "0.0",
                  "Robust parameter",
                  "float");
  itsInputParSet.create ("cachesize", "512",
                  "maximum size of gridding cache (in MBytes)",
                  "int");
  itsInputParSet.create ("nfacets", "1",
                  "number of facets in x or y",
                  "int");
  itsInputParSet.create ("npix", "256",
                  "number of image pixels in x and y direction",
                  "int");
  itsInputParSet.create ("cellsize", "1arcsec",
                  "pixel width in x and y direction",
                  "quantity string");
  itsInputParSet.create ("phasecenter", "",
                  "phase center to be used (e.g. 'j2000, 05h30m, -30.2deg')",
                  "direction string");
  itsInputParSet.create ("img_nchan", "1",
                  "number of frequency channels in image",
                  "int");
  itsInputParSet.create ("img_chanstart", "0",
                  "first frequency channel in image (0-relative)",
                  "int");
  itsInputParSet.create ("img_chanstep", "1",
                  "frequency channel step in image",
                  "int");
}



void OperationImageBase::run()
{
  Operation::run();
  OperationParamFTMachine::run();
  
  itsParameters.define("imagename", itsInputParSet.getString("image"));
  itsParameters.define("npix", itsInputParSet.getInt("npix"));
  itsParameters.define("cellsize", itsInputParSet.getString("cellsize"));
  itsParameters.define("mode", itsInputParSet.getString("mode"));
  
  itsParameters.define("img_nchan", itsInputParSet.getInt("img_nchan"));
  itsParameters.define("img_chanstart", itsInputParSet.getInt("img_chanstart"));
  itsParameters.define("img_chanstep", itsInputParSet.getInt("img_chanstep"));
  
  Quantity qcellsize = readQuantity (itsParameters.asString("cellsize"));
  MDirection phaseCenter;
  Bool doShift = False;
  Int fieldid = 0;
  Int nfacet = 0;

  MSSpWindowColumns window(itsMS.spectralWindow());
  Vector<Int> wind(window.nrow());
  for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};

  
  itsImager->defineImage (
    itsParameters.asInt("npix"),                       // nx
    itsParameters.asInt("npix"),                       // ny
    qcellsize,                    // cellx
    qcellsize,                    // celly
    String("I"),                       // stokes

    phaseCenter,                  // phaseCenter
    doShift  ?  -1 : fieldid,     // fieldid
    itsParameters.asString("mode"),                  // mode
    itsParameters.asInt("img_nchan"),                // nchan
    itsParameters.asInt("img_chanstart"),                 // start
    itsParameters.asInt("img_chanstep"),                 // step
    MFrequency(),                 // mFreqstart
    MRadialVelocity(),            // mStart
    Quantity(1,"km/s"),           // qstep, Def=1 km/s
    wind,//spwid,                 // spectralwindowids
    nfacet);                    // facets
  cout << "Hi, I am OperationImageBase::run" << endl;
}

void OperationImageBase::makeEmpty (const String& imgName, Int fieldid)
{
  CoordinateSystem coords;
  AlwaysAssert (itsImager->imagecoordinates(coords), AipsError);
  String name(imgName);
  itsImager->makeEmptyImage(coords, name, fieldid);
  itsImager->unlock();
}


} //# namespace LofarFT
} //# namespace LOFAR
