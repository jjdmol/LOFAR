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

String facetName (const String& imageName, const String& baseExt,
                  int ix, int iy)
{
  ostringstream ostr;
  ostr << imageName << '-' << setw(3) << setfill('0') << ix
       << '-' << setw(3) << setfill('0') << iy << baseExt;
  return ostr.str();
}


void combine (const String& msName, const String& imageExt, bool useHDF5, 
              double ra, double dec, const vector<int>& npix,
              const vector<int>& nfacet)
{
  if (useHDF5  &&  !HDF5Object::hasHDF5Support()) {
    useHDF5 = False;
  }
  ASSERT (npix.size() == 2);
  ASSERT (nfacet.size() == 2);
  // Get base name of images from the msName.
  Path msPath(msName);
  String dirNm = msPath.dirName();
  String baseNm = msPath.baseName();
  // Remove the file extension.
  String::size_type inx = baseNm.find('.');
  if (inx != String::npos) {
    baseNm = baseNm.before(inx);
  }
  // Append the possibly given image extension.
  baseNm += imageExt;
  // Get the file extension and remove from base name; default is .img.
  String baseExt = ".img";
  inx = baseNm.find('.');
  if (inx != String::npos) {
    baseExt = baseNm.from(inx);
    baseNm = baseNm.before(inx);
  }
  String imageName = dirNm + '/' + baseNm;
  // Open first image and get its coordinates.
  PagedImage<Float> img(facetName(imageName, baseExt, 0, 0));
  IPosition imgShape = img.shape();
  imgShape[0] = npix[0];
  imgShape[1] = npix[1];
  CoordinateSystem csys (img.coordinates());
  // Coordinates of output image only changes in RA/DEC.
  ASSERT (csys.type(0) == Coordinate::DIRECTION);
  DirectionCoordinate coord = csys.directionCoordinate(0);
  Vector<double> cellSize = coord.increment();
  Vector<double> refPix = coord.referencePixel();
  Vector<double> refPos = coord.referenceValue();
  cout << refPos[0] <<' '<<refPos[1]<<' '<<refPix[0]<<' '<<refPix[1]<<endl;
  refPos[0] = ra;
  refPos[1] = dec;
  refPix[0] = imgShape[0]/2;
  refPix[1] = imgShape[1]/2;
  cout << refPos[0] <<' '<<refPos[1]<<' '<<refPix[0]<<' '<<refPix[1]<<endl;
  coord.setReferenceValue (refPos);
  coord.setReferencePixel (refPix);
  csys.replaceCoordinate (coord, 0);
  // Now create the new image.
  ImageInterface<Float>* newImg;
  if (useHDF5) {
    newImg = new HDF5Image<Float>  (imgShape, csys, imageName+baseExt);
  } else {
    newImg = new PagedImage<Float> (imgShape, csys, imageName+baseExt);
  }
  // Get nr of pixels to take per facet.
  vector<int> npixf(npix);
  npixf[0] /= nfacet[0];
  npixf[1] /= nfacet[1];
  // Determine which part of the facets to take.
  IPosition facetShape = img.shape();
  IPosition stslice (facetShape.size(), 0);
  IPosition lenslice(facetShape);
  for (int i=0; i<2; ++i) {
    stslice[i] = (facetShape[i] - npixf[i]) / 2;
    lenslice[i] = npixf[i];
  }
  cout<<stslice<<lenslice<<facetShape<<imgShape<<endl;
  Slicer slicer (stslice, lenslice);
  // Put the data of each facet into the image.
  IPosition where(imgShape.size(), 0);
  for (int iy=0; iy<nfacet[1]; ++iy) {
    for (int ix=0; ix<nfacet[0]; ++ix) {
      PagedImage<Float> facet(facetName(imageName, baseExt, ix, iy));
      newImg->putSlice (facet.getSlice(slicer), where);
      where[0] += npixf[0];
    }
    where[0] = 0;
    where[1] += npixf[1];
  }
  // Cleanup.
  delete newImg;
}

int main (int argc, char* argv[])
{
  if (argc < 4) {
    cerr << "Run as:   combineFacets parsetname ms imageext" << endl;
    return 1;
  }
  try {
    ParameterSet ps(argv[1]);
    combine (argv[2],
             argv[3],
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
