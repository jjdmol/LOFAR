//# combineFacets.cc: combine the faceted images made in mwimager-dd
//#
//# Copyright (C) 2009
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
#include <images/Images/PagedImage.h>
#include <images/Images/HDF5Image.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace LOFAR;
using namespace casa;
using namespace std;

string facetName (const string& imageName, int ix, int iy)
{
  ostringstream ostr;
  ostr << imageName << "-facet-" << setw(3) << setfill('0') << ix
       << '-' << setw(3) << setfill('0') << iy;
  return ostr.str();
}


void combine (const string& imageName, bool useHDF5, 
              double ra, double dec, const vector<int>& npix,
              const vector<int>& nfacet)
{
  if (useHDF5  &&  !HDF5Object::hasHDF5Support()) {
    useHDF5 = False;
  }
  ASSERT (npix.size() == 2);
  ASSERT (nfacet.size() == 2);
  // Open first image and get its coordinates.
  PagedImage<Float> img(facetName(imageName, 0, 0));
  IPosition imgShape = img.shape();
  imgShape[0] = nfacet[0]*npix[0];
  imgShape[1] = nfacet[1]*npix[1];
  CoordinateSystem csys (img.coordinates());
  // Coordinates of output image only changes in RA/DEC.
  ASSERT (csys.type(0) == Coordinate::DIRECTION);
  DirectionCoordinate coord = csys.directionCoordinate(0);
  Vector<double> cellSize = coord.increment();
  Vector<double> refPix(2);
  Vector<double> refPos(2);
  refPos[0] = ra;
  refPos[1] = dec;
  refPix[0] = imgShape[0]/2;
  refPix[1] = imgShape[1]/2;
  coord.setReferenceValue (refPos);
  coord.setReferencePixel (refPix);
  csys.replaceCoordinate (coord, 0);
  // Now create the new image.
  ImageInterface<Float>* newImg;
  if (useHDF5) {
    newImg = new HDF5Image<Float>  (imgShape, csys, imageName);
  } else {
    newImg = new PagedImage<Float> (imgShape, csys, imageName);
  }
  // Determine which part of the facets to take.
  IPosition facetShape = img.shape();
  int xpadding = (facetShape[0] - npix[0]) / 2;
  int ypadding = (facetShape[1] - npix[1]) / 2;
  Slicer slicer (IPosition(2, xpadding, ypadding),
                 IPosition(2, npix[0], npix[1]));
  // Put the data of each facet into the image.
  IPosition where(imgShape.size(), 0);
  for (int iy=0; iy<nfacet[1]; ++iy) {
    for (int ix=0; ix<nfacet[0]; ++ix) {
      PagedImage<Float> facet(facetName(imageName, ix, iy));
      newImg->putSlice (facet.getSlice(slicer), where);
      where[0] += npix[0];
    }
    where[0] = 0;
    where[1] += npix[1];
  }
  // Cleanup.
  delete newImg;
}

int main (int argc, char* argv[])
{
  if (argc < 3) {
    cerr << "Run as:   combineFacets parsetname imagename" << endl;
    return 1;
  }
  try {
    ParameterSet ps(argv[1]);
    combine (argv[2],
             ps.getBool  ("useHDF5", False),
             ps.getDouble("ra"),
             ps.getDouble("dec"),
             ps.getIntVector ("npix"),
             ps.getIntVector ("nfacet"));
  } catch (std::exception& x) {
    cerr << "Unepected exception: " << x.what() << endl;
    return 0;
  }
  return 0;
}
