//# FTMachine.cc: Gridder for LOFAR data correcting for DD effects
//#
//# Copyright (C) 2011
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
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/OpenMP.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/PrecTimer.h>
#include <casa/OS/DynLib.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/sstream.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <measures/Measures/UVWMachine.h>
#include <measures/Measures/Stokes.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisSet.h>

#include <LofarFT/FTMachine.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/ConvolutionFunction.h>
#include <LofarFT/VisResampler.h>
#include <LofarFT/VBStore.h>

#define DORES True

using namespace casa;

// Force instantiation of Singleton FTMachineFactory before any multi threading starts
namespace
{
  void* dummy = (void*) &LOFAR::LofarFT::FTMachineFactory::instance();
}


namespace LOFAR {
namespace LofarFT {

FTMachine::FTMachine(
  const MeasurementSet& ms, 
//   Int nwPlanes,
//   MPosition mLocation, 
//   Float padding,
//   Bool useDoublePrec, 
  const Record& parameters)  
  : casa::FTMachine(), 
   // Aliases for data members of casa::FTMachine
    itsImage(casa::FTMachine::image),
    itsNX(casa::FTMachine::nx),
    itsNY(casa::FTMachine::ny),
    itsNPol(casa::FTMachine::npol),
    itsNChan(casa::FTMachine::nchan),
    itsUseDoubleGrid(casa::FTMachine::useDoubleGrid_p),
    itsChanMap(casa::FTMachine::chanMap),
    itsPolMap(casa::FTMachine::polMap),
    
  // ================================================  
//     itsPadding(padding), 
    itsVisResampler(new VisResampler()),
    itsMaxAbsData(0.0), 
    itsCenterLoc(IPosition(4,0)),
    itsOffsetLoc(IPosition(4,0)), 
    itsNoPadding(True),
    itsParameters(parameters), 
    itsMS(ms),
    itsNWPlanes(/*nwPlanes*/), 
    itsWMax(parameters.asDouble("wmax")), 
    itsConvFunc(), 
    itsVerbose(parameters.asInt("verbose")),
    itsMaxSupport(parameters.asInt("maxsupport")), 
    itsOversample(parameters.asInt("oversample")), 
    itsImageName(parameters.asString("imagename")),
    itsGridMuellerMask(parameters.asArrayBool("mueller.grid")),
    itsDegridMuellerMask(parameters.asArrayBool("mueller.degrid")),
    itsGriddingTime(0),   // counters to measure time spend per operation (Gridding, Degridding, and Convolution Function computation)
    itsDegriddingTime(0), //
    itsCFTime(0)          //
    
//     itsMLocation(mLocation),
//     itsTangentSpecified(False),
//     itsUseDoubleGrid(useDoublePrec),
//     itsCanComputeResiduals(DORES)
{
}

//----------------------------------------------------------------------
  FTMachine::FTMachine(const FTMachine& other) : 
    casa::FTMachine(),
    itsImage(casa::FTMachine::image), // Aliases for data members of casa::FTMachine
    itsNX(casa::FTMachine::nx),
    itsNY(casa::FTMachine::ny),
    itsNPol(casa::FTMachine::npol),
    itsNChan(casa::FTMachine::nchan),
    itsUseDoubleGrid(casa::FTMachine::useDoubleGrid_p),
    itsChanMap(casa::FTMachine::chanMap),
    itsPolMap(casa::FTMachine::polMap)
  {
    operator=(other);
  }

//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Assignment operator 
//
// Called by the copy constructor which is called by cloneFTM 
// 
// The assignment operator copies all members that are set to a nonzero value by the normal constructor 
// members that are set by the init method, called by InitializeToSky and InitializeToVis are set to zero
//

FTMachine& FTMachine::operator=(const FTMachine& other)
{
  if (this!=&other) 
  {
    //Do the base parameters
    casa::FTMachine::operator=(other);

    //private params
    itsUVScale.resize();
    itsUVOffset.resize();
    itsUVScale = other.itsUVScale;
    itsUVOffset = other.itsUVOffset;
    itsLattice = 0;
//     itsArrayLattice = 0;
    itsMaxAbsData = other.itsMaxAbsData;
    itsCenterLoc = other.itsCenterLoc;
    itsOffsetLoc = other.itsOffsetLoc;
    itsPadding = other.itsPadding;
    itsNoPadding = other.itsNoPadding;
    itsMS = other.itsMS;
    itsNWPlanes = other.itsNWPlanes;
    itsWMax = other.itsWMax;
    itsConvFunc = other.itsConvFunc;
    itsConjCFMap = other.itsConjCFMap;
    itsCFMap = other.itsCFMap;
    itsVerbose = other.itsVerbose;
    itsMaxSupport = other.itsMaxSupport;
    itsOversample = other.itsOversample;
    itsImageName = other.itsImageName;
    itsGridMuellerMask = other.itsGridMuellerMask;
    itsDegridMuellerMask = other.itsDegridMuellerMask;
    itsGriddingTime = other.itsGriddingTime;
    itsDegriddingTime = other.itsDegriddingTime;
    itsCFTime = other.itsCFTime;
  }
  return *this;
}

void FTMachine::init() {

  logIO() << LogOrigin("LofarFTMachine", "init")  << LogIO::NORMAL;
  
  ASSERT(itsImage);

  if (itsNoPadding)
  {
    itsNX    = itsImage->shape()(0);
    itsNY    = itsImage->shape()(1);
  }
  else
  {
    CompositeNumber cn(uInt(itsImage->shape()(0) * 2));
    itsNX    = cn.nextLargerEven(Int(itsPadding * Float(itsImage->shape()(0)) - 0.5));
    itsNY    = cn.nextLargerEven(Int(itsPadding * Float(itsImage->shape()(1)) - 0.5));
  }
  itsNPol  = image->shape()(2);
  itsNChan = image->shape()(3);

  itsUVScale.resize(3);
  itsUVScale = 0.0;
  itsUVScale(0) = Float(itsNX) * itsImage->coordinates().increment()(0);
  itsUVScale(1) = Float(itsNY) * itsImage->coordinates().increment()(1);
  itsUVScale(2) = Float(1)*abs(itsImage->coordinates().increment()(0));

  itsUVOffset.resize(3);
  itsUVOffset(0) = itsNX/2;
  itsUVOffset(1) = itsNY/2;
  itsUVOffset(2) = 0;

  itsPaddedShape = itsImage->shape();
  itsPaddedShape(0) = itsNX;
  itsPaddedShape(1) = itsNY;
  
  if (itsVerbose > 0) {
    cout << "Original shape " << itsImage->shape()(0) << ","
         << itsImage->shape()(1) << endl;
    cout << "Padded shape " << itsPaddedShape(0) << ","
         << itsPaddedShape(1) << endl;
  }
  
  itsConvFunc = new ConvolutionFunction(
    itsPaddedShape,
    itsImage->coordinates().directionCoordinate (itsImage->coordinates().findCoordinate(Coordinate::DIRECTION)),
    itsMS, 
    itsNWPlanes, 
    itsWMax,
    itsOversample, 
    itsVerbose, 
    itsMaxSupport,
    itsImageName,
    itsParameters);
}

FTMachine::~FTMachine()
{
}

const Matrix<Float>& FTMachine::getAveragePB() const
{
  // Read average beam from disk if not present.
  if (itsAvgPB.empty()) {
    PagedImage<Float> pim(itsImageName + ".avgpb");
    Array<Float> arr = pim.get();
    itsAvgPB.reference (arr.nonDegenerate(2));
  }
  return itsAvgPB;
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image
void FTMachine::initializeToVis(
  ImageInterface<Complex>& iimage,
  const VisBuffer& vb)
{
  if (itsVerbose > 0) {
    cout<<"---------------------------> initializeToVis"<<endl;
  }
  itsImage = &iimage;

  ok();
  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);

  itsVisResampler->init(itsUseDoubleGrid);
  itsVisResampler->setMaps(itsChanMap, itsPolMap);
  itsVisResampler->setCFMaps(itsCFMap, itsConjCFMap);

  // Need to reset nx, ny for padding

  IPosition gridShape(4, itsNX, itsNY, itsNPol, itsNChan);
  // Size and initialize as many grid buffers as needed per itsNGrid.
  // Note that except the first itsGriddedData buffers are assigned later.
  itsGriddedData[0].resize (gridShape);
  itsGriddedData[0] = Complex();
  for (int i=0; i<itsNGrid; ++i) 
  {
    itsSumPB[i].resize (itsPaddedShape[0], itsPaddedShape[1]);
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize(itsNPol, itsNChan);
    itsSumWeight[i] = 0.;
  }

  IPosition stride(4, 1);
  IPosition blc(
    4, 
    (itsNX - itsImage->shape()(0) + (itsNX % 2 == 0)) / 2,
    (itsNY - itsImage->shape()(1) + (itsNY % 2 == 0)) / 2,
    0, 
    0);
  IPosition trc(blc + itsImage->shape() - stride);
  
  if (itsVerbose > 0) 
  {
    cout<<"LofarFTMachine::initializeToVis === blc,trc,nx,ny,image->shape()"
        << blc << " " << trc << " " << nx << " " << ny << " " << itsImage->shape() << endl;
  }
  
  IPosition start(4, 0);
  itsGriddedData[0](blc, trc) = image->getSlice(start, image->shape());
  
  itsLattice = new ArrayLattice<Complex>(itsGriddedData[0]);

  logIO() << LogIO::DEBUGGING << "Starting grid correction and FFT of image" << LogIO::POST;

  const Matrix<Float>& data = getAveragePB();
  
  IPosition pos(4, itsLattice->shape()[0], itsLattice->shape()[1], 1, 1);
  IPosition pos2(2, itsLattice->shape()[0], itsLattice->shape()[1]);
  pos[2]=0.;
  pos[3]=0.;
  pos2[2]=0.;
  pos2[3]=0.;
  Int offset_pad(floor(data.shape()[0] - itsLattice->shape()[0])/2.);
  
  for(Int k = 0; k < itsLattice->shape()[2]; ++k)
  {
    for(Int i = 0; i < itsLattice->shape()[0]; ++i)
    {
      for(Int j = 0; j < itsLattice->shape()[0]; ++j)
      {
        pos[0] = i;
        pos[1] = j;
        pos[2] = k;
        pos2[0] = i + offset_pad;
        pos2[1] = j + offset_pad;
        Complex pixel(itsLattice->getAt(pos));
        double fact(1.);

        fact /= sqrt(data(pos2));
        pixel *= Complex(fact);
        
        itsLattice->putAt(pixel,pos);
      }
    }
  }

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // Now do the FFT2D in place
  LatticeFFT::cfft2d(*itsLattice);

  logIO() << LogIO::DEBUGGING
          << "Finished grid correction and FFT of image" << LogIO::POST;
}

void FTMachine::finalizeToVis()
{
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToVis"<<endl;
  }
}

// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid.
void FTMachine::initializeToSky(
  ImageInterface<Complex>& iimage,
  Matrix<Float>& weight, 
  const VisBuffer& vb)
{
  // image always points to the image
  itsImage = &iimage;
  if (itsVerbose > 0) {
    cout<<"---------------------------> initializeToSky"<<endl;
  }
  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);

  itsVisResampler->init(itsUseDoubleGrid);
  itsVisResampler->setMaps(itsChanMap, itsPolMap);
  itsVisResampler->setCFMaps(itsCFMap, itsConjCFMap);

  // Initialize for in memory or to disk gridding. lattice will
  // point to the appropriate Lattice, either the ArrayLattice for
  // in memory gridding or to the image for to disk gridding.
  IPosition gridShape(4, itsNX, itsNY, itsNPol, itsNChan);
  // Size and initialize the grid buffers
  for (int i = 0; i < itsNGrid; ++i) 
  {
    itsGriddedData[i].resize (gridShape);
    itsGriddedData[i] = Complex();
    itsSumPB[i].resize (itsPaddedShape[0], itsPaddedShape[1]);
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize (itsNPol, itsNChan);
    itsSumWeight[i] = 0.;
  }
  
  // This is strange, weight is passed here by reference and then reshaped and filled with zeros.
  // What is the purpose ?
  weight.resize(itsSumWeight[0].shape());
  weight=0.0;

  itsLattice = new ArrayLattice<Complex>(itsGriddedData[0]);
}



void FTMachine::finalizeToSky()
{
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if (itsVerbose > 0) 
  {
    cout<<"---------------------------> finalizeToSky"<<endl;
  }

  
//   
//   Store Grids to disk for debugging purpose
//  
//   uInt nx(itsGriddedData[0].shape()[0]);
//   uInt ny(itsGriddedData[0].shape()[1]);
//   uInt npol(itsGriddedData[0].shape()[2]);
//   IPosition shapecube(3, nx, ny, npol);
//   for (int ii = 0; ii<itsNGrid; ++ii) 
//   {
//     Cube<Complex> tempimage(shapecube, 0.);
//     for(Int k = 0 ; k < npol; ++k)
//     {
//       for(uInt i = 0; i < nx; ++i)
//       {
//         for(uInt j = 0; j < nx; ++j)
//         {
//           IPosition pos(4, i, j, k, 0);
//           Complex pixel(itsGriddedData[ii](pos));
//           tempimage(i,j,k) = pixel;
//         }
//       }
//     }
//     store(tempimage,"Grid"+String::toString(ii)+".img");
//   }
//   Cube<Complex> tempimage(shapecube,0.);
//   for(Int k = 0; k < npol; ++k)
//   {
//     for(uInt i = 0; i < nx; ++i)
//     {
//       for(uInt j = 0; j < nx;+ +j)
//       {
//         IPosition pos(4,i,j,k,0);
//         Complex pixel(itsGriddedData[0](pos));
//         tempimage(i,j,k) = pixel;
//       }
//     }
//   }
//   store(tempimage,"Grid00.img");

  // Add all buffers into the first one.
  for (int i=1; i<itsNGrid; ++i) 
  {
    itsGriddedData[0] += itsGriddedData[i];
    itsSumWeight[0]   += itsSumWeight[i];
    itsSumCFWeight[0] += itsSumCFWeight[i];
    itsSumPB[0]       += itsSumPB[i];
  }

  
}


// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& FTMachine::getImage(Matrix<Float>& weights, Bool normalize)
{
  AlwaysAssert(itsImage, AipsError);
  logIO() << LogOrigin("LofarFTMachine", "getImage") << LogIO::NORMAL;

  if (itsVerbose > 0) 
  {
    cout<<"GETIMAGE"<<endl;
  }
  
  itsAvgPB.reference (itsConvFunc->compute_avg_pb(itsSumPB[0], itsSumCFWeight[0]));

  weights.resize(itsSumWeight[0].shape());

  convertArray(weights, itsSumWeight[0]);
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(normalize&&max(weights)==0.0) 
  {
    logIO() << LogIO::SEVERE << "No useful data in LofarFTMachine: weights all zero"
            << LogIO::POST;
    return *itsImage;
  }

  const IPosition latticeShape = itsLattice->shape();

  logIO() << LogIO::DEBUGGING
          << "Starting FFT and scaling of image" << LogIO::POST;

  if (itsUseDoubleGrid) 
  {
    ArrayLattice<DComplex> darrayLattice(itsGriddedData2[0]);
    LatticeFFT::cfft2d(darrayLattice,False);
    convertArray(itsGriddedData[0], itsGriddedData2[0]);
  }
  else 
  {
    LatticeFFT::cfft2d(*itsLattice, False);
  }

  if (itsVerbose > 0) 
  {
    cout<<"POLMAP:::::::  "<< itsPolMap << endl;
    cout<<"POLMAP:::::::  "<< itsCFMap << endl;
  }
  
  Int inx = itsLattice->shape()(0);
  Int iny = itsLattice->shape()(1);
  Vector<Complex> correction(inx);
  correction=Complex(1.0, 0.0);
  // Do the Grid-correction
  IPosition cursorShape(4, inx, 1, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(itsLattice->shape(), cursorShape, axisPath);
  LatticeIterator<Complex> lix(*itsLattice, lsx);
  for (lix.reset(); !lix.atEnd(); lix++) 
  {
    Int pol=lix.position()(2);
    Int chan=lix.position()(3);
    if (weights(pol, chan) != 0.0) 
    {
      if (normalize) 
      {
        Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
        lix.rwCursor()*=rnorm;
      }
      else 
      {
        Complex rnorm(Float(inx)*Float(iny));
        lix.rwCursor()*=rnorm;
      }
    }
    else {
      lix.woCursor()=0.0;
    }
  }
  
  IPosition pos(4, itsLattice->shape()[0], itsLattice->shape()[1], 1, 1);
  uInt shapeout(floor(itsLattice->shape()[0]/itsPadding));
  uInt istart(floor((itsLattice->shape()[0] - shapeout)/2.));
  Cube<Complex> tempimage(IPosition(3, shapeout, shapeout, itsLattice->shape()[2]));

  pos[3] = 0.;
  for(Int k=0; k < itsLattice->shape()[2]; ++k)
  {
    for(uInt i = 0; i < shapeout; ++i)
    {
      for(uInt j = 0; j < shapeout; ++j)
      {
        pos[0] = i + istart;
        pos[1] = j + istart;
        pos[2] = k;
        Complex pixel(itsLattice->getAt(pos));
        pixel /= sqrt(itsAvgPB(i + istart, j + istart));
        itsLattice->putAt(pixel,pos);
      }
    }
  }

  IPosition blc(
    4, 
    (itsNX - itsImage->shape()(0) + (itsNX % 2 == 0)) / 2, 
    (itsNY - itsImage->shape()(1) + (itsNY % 2 == 0)) / 2,
    0,
    0);
  IPosition stride(4, 1);
  IPosition trc( blc + itsImage->shape() - stride);
  // Do the copy
  IPosition start(4, 0);
  itsImage->put(itsGriddedData[0](blc, trc));
  
  return *image;
}

// Get weight image
void FTMachine::getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights)
{

  logIO() << LogOrigin("FTMachine", "getWeightImage") << LogIO::NORMAL;

  weights.resize(itsSumWeight[0].shape());
  convertArray(weights, itsSumWeight[0]);

  const IPosition latticeShape = weightImage.shape();

  Int nx = latticeShape(0);
  Int ny = latticeShape(1);

  IPosition loc(2, 0);
  IPosition cursorShape(4, nx, ny, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(latticeShape, cursorShape, axisPath);
  LatticeIterator<Float> lix(weightImage, lsx);
  for(lix.reset();!lix.atEnd();lix++) 
  {
    Int pol=lix.position()(2);
    Int chan=lix.position()(3);
    lix.rwCursor() = weights(pol,chan);
  }
}

void FTMachine::ok() {
  AlwaysAssert(image, AipsError);
}

// Make a plain straightforward honest-to-God image. This returns
// a complex image, without conversion to Stokes. The representation
// is that required for the visibilities.
//----------------------------------------------------------------------
void FTMachine::makeImage(
  FTMachine::Type type,
  ROVisibilityIterator& vi,
  ImageInterface<Complex>& theImage,
  Matrix<Float>& weight) 
{
  logIO() << LogOrigin("LofarFTMachine", "makeImage") << LogIO::NORMAL;

  if(type==FTMachine::COVERAGE) {
    logIO() << "Type COVERAGE not defined for Fourier transforms" << LogIO::EXCEPTION;
  }

  // Loop over all visibilities and pixels
  VisBuffer vb(vi);

  // Initialize put (i.e. transform to Sky) for this model
  vi.origin();
// 
// /home/vdtol/lofar-casapy-4.1/src/LOFAR/CEP/Imager/LofarFT/src/LofarFTMachine.cc:1502: error: incomplete type ‘casa::SkyModel’ used in nested name specifier
// /home/vdtol/lofar-casapy-4.1/src/LOFAR/CEP/Imager/LofarFT/src/LofarFTMachine.cc:1505: error: incomplete type ‘casa::SkyModel’ used in nested name specifier
  if(vb.polFrame()==MSIter::Linear) {
//     StokesImageUtil::changeCStokesRep(theImage, SkyModel::LINEAR);
  }
  else {
//     StokesImageUtil::changeCStokesRep(theImage, SkyModel::CIRCULAR);
  }

  initializeToSky(theImage, weight, vb);
  cout << "itsSumPB.size() " << itsSumPB.size() << endl;
  cout << "itsSumPB[0].shape() " << itsSumPB[0].shape() << endl;
  

  // Loop over the visibilities, putting VisBuffers
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
  {
    for (vi.origin(); vi.more(); vi++) 
    {
      switch(type) 
      {
      case FTMachine::RESIDUAL:
        if (itsVerbose > 0) cout<<"FTMachine::RESIDUAL"<<endl;
        vb.visCube()=vb.correctedVisCube();
        vb.visCube()-=vb.modelVisCube();
        put(vb, -1, False);
        break;
      case FTMachine::MODEL:
        if (itsVerbose > 0) cout<<"FTMachine::MODEL"<<endl;
        vb.visCube()=vb.modelVisCube();
        put(vb, -1, False);
        break;
      case FTMachine::CORRECTED:
        if (itsVerbose > 0) cout<<"FTMachine::CORRECTED"<<endl;
        vb.visCube()=vb.correctedVisCube();
        put(vb, -1, False);
        break;
      case FTMachine::PSF:
        if (itsVerbose > 0) cout<<"FTMachine::PSF"<<endl;
        vb.visCube()=Complex(1.0,0.0);
        put(vb, -1, True);
        break;
      case FTMachine::OBSERVED:
      default:
        if (itsVerbose > 0) cout<<"FTMachine::OBSERVED"<<endl;
        put(vb, -1, False);
        break;
      }
    }
  }
  finalizeToSky();
  // Normalize by dividing out weights, etc.
  getImage(weight, True);
}

void FTMachine::ComputeResiduals(VisBuffer&vb, Bool useCorrected)
{
  VBStore vbs;
  vbs.nRow_p = vb.nRow();
  vbs.beginRow_p = 0;
  vbs.endRow_p = vbs.nRow_p;
  vbs.modelCube_p.reference(vb.modelVisCube());
  if (useCorrected) 
  {
    vbs.correctedCube_p.reference(vb.correctedVisCube());
  }
  else
  {
    vbs.visCube_p.reference(vb.visCube());
  }
  vbs.useCorrected_p = useCorrected;
  itsVisResampler->lofarComputeResiduals(vbs);
}

void FTMachine::makeSensitivityImage(
  const VisBuffer& vb,
  const ImageInterface<Complex>& imageTemplate,
  ImageInterface<Float>& sensitivityImage)
{
  throw AipsError("Not implemented");
//     if (convFuncCtor_p->makeAverageResponse(vb, imageTemplate, sensitivityImage))
//   {
//     cfCache_p->flush(sensitivityImage,sensitivityPatternQualifierStr_p);
//   }

}

//
//---------------------------------------------------------------
//
void FTMachine::normalizeAvgPB(
  ImageInterface<Complex>& inImage,
  ImageInterface<Float>& outImage)
{
  throw AipsError("Not implemented");
  
//   LogIO log_l(LogOrigin("LofarFTMachine", "normalizeAvgPB"));
//   if (pbNormalized_p) return;
//   IPosition inShape(inImage.shape()),ndx(4,0,0,0,0);
//   Vector<Complex> peak(inShape(2));
// 
//   outImage.resize(inShape);
//   outImage.setCoordinateInfo(inImage.coordinates());
// 
//   Bool isRefIn;
//   Array<Complex> inBuf;
//   Array<Float> outBuf;
// 
//   isRefIn  = inImage.get(inBuf);
//   log_l << "Normalizing the average PBs to unity"
//         << LogIO::NORMAL << LogIO::POST;
//   //
//   // Normalize each plane of the inImage separately to unity.
//   //
//   Complex inMax = max(inBuf);
//   if (abs(inMax)-1.0 > 1E-3)
//     {
//       for(ndx(3)=0;ndx(3)<inShape(3);ndx(3)++)
//         for(ndx(2)=0;ndx(2)<inShape(2);ndx(2)++)
//           {
//             peak(ndx(2)) = 0;
//             for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
//               for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
//                 if (abs(inBuf(ndx)) > peak(ndx(2)))
//                   peak(ndx(2)) = inBuf(ndx);
// 
//             for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
//               for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
//                 //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
//                 inBuf(ndx) /= peak(ndx(2));
//           }
//       if (isRefIn) inImage.put(inBuf);
//     }
// 
//   ndx=0;
//   for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
//     for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
//       {
//         IPosition plane1(ndx);
//         plane1=ndx;
//         plane1(2)=1; // The other poln. plane
//         //	  avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
//         outBuf(ndx) = sqrt(real(inBuf(ndx) * inBuf(plane1)));
//       }
//   //
//   // Rather convoluted way of copying Pol. plane-0 to Pol. plane-1!!!
//   //
//   for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
//     for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
//       {
//         IPosition plane1(ndx);
//         plane1=ndx;
//         plane1(2)=1; // The other poln. plane
//         outBuf(plane1) = outBuf(ndx);
//       }
// 
//   pbNormalized_p = True;
}
//
//---------------------------------------------------------------
//
void FTMachine::normalizeAvgPB()
{
  throw AipsError("Not implemented");
//   LogIO log_l(LogOrigin("LofarFTMachine", "normalizeAvgPB"));
//   if (pbNormalized_p) return;
//   Bool isRefF;
//   Array<Float> avgPBBuf;
//   isRefF=avgPB_p->get(avgPBBuf);
//   //    Float pbMax = max(avgPBBuf);
//     {
//       pbPeaks.resize(avgPB_p->shape()(2),True);
//       // if (makingPSF) pbPeaks = 1.0;
//       // else pbPeaks /= (Float)noOfPASteps;
//       pbPeaks = 1.0;
//       log_l << "Normalizing the average PBs to " << 1.0
//             << LogIO::NORMAL << LogIO::POST;
// 
//       IPosition avgPBShape(avgPB_p->shape()),ndx(4,0,0,0,0);
//       Vector<Float> peak(avgPBShape(2));
// 
// 
//       Float pbMax = max(avgPBBuf);
//       if (fabs(pbMax-1.0) > 1E-3)
//         {
//           //	    avgPBBuf = avgPBBuf/noOfPASteps;
//           for(ndx(3)=0;ndx(3)<avgPBShape(3);ndx(3)++)
//             for(ndx(2)=0;ndx(2)<avgPBShape(2);ndx(2)++)
//               {
//                 peak(ndx(2)) = 0;
//                 for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
//                   for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
//                     if (abs(avgPBBuf(ndx)) > peak(ndx(2)))
//                       peak(ndx(2)) = avgPBBuf(ndx);
// 
//                 for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
//                   for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
//                     //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
//                     avgPBBuf(ndx) /= peak(ndx(2));
//               }
//           if (isRefF) avgPB_p->put(avgPBBuf);
//         }
// 
//       ndx=0;
//       for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
//         for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
//           {
//             IPosition plane1(ndx);
//             plane1=ndx;
//             plane1(2)=1; // The other poln. plane
//             avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
//             //	      avgPBBuf(ndx) = (avgPBBuf(ndx) * avgPBBuf(plane1));
//           }
//       for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
//         for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
//           {
//             IPosition plane1(ndx);
//             plane1=ndx;
//             plane1(2)=1; // The other poln. plane
//             avgPBBuf(plane1) = avgPBBuf(ndx);
//           }
//     }
//     pbNormalized_p = True;
}

void FTMachine::normalizeImage(
  Lattice<Complex>& skyImage,
  const Matrix<Double>& sumOfWts,
  Lattice<Float>& sensitivityImage,
  Lattice<Complex>& sensitivitySqImage,
  Bool fftNorm)
{
  //
  // Apply the gridding correction
  //
  if (itsVerbose > 0) 
  {
    cout<<"LofarFTMachine::normalizeImage"<<endl;
  }
  
  Int inx = skyImage.shape()(0);
  Int iny = skyImage.shape()(1);
  Vector<Complex> correction(inx);

  Vector<Float> sincConv(nx);
  Float centerX=nx/2;
  for (Int ix=0;ix<nx;ix++)
  {
    Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
    if (ix==centerX) 
    {
      sincConv(ix)=1.0;
    }
    else
    {
      sincConv(ix)=sin(x)/x;
    }
  }
  
  IPosition cursorShape(4, inx, 1, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(skyImage.shape(), cursorShape, axisPath);
  LatticeIterator<Complex> lix(skyImage, lsx);

  LatticeStepper lavgpb(sensitivityImage.shape(),cursorShape,axisPath);
  LatticeIterator<Float> liavgpb(sensitivityImage, lavgpb);
  LatticeStepper lavgpbSq(sensitivitySqImage.shape(),cursorShape,axisPath);
  LatticeIterator<Complex> liavgpbSq(sensitivitySqImage, lavgpbSq);

  for(lix.reset(),liavgpb.reset(),liavgpbSq.reset();
      !lix.atEnd();
      lix++,liavgpb++,liavgpbSq++)
  {
    Int pol=lix.position()(2);
    Int chan=lix.position()(3);

    if(sumOfWts(pol, chan)>0.0)
    {
      Int iy=lix.position()(1);
//       itsGridder->correctX1D(correction,iy);

      Vector<Complex> PBCorrection(liavgpb.rwVectorCursor().shape());
      Vector<Float> avgPBVec(liavgpb.rwVectorCursor().shape());
      Vector<Complex> avgPBSqVec(liavgpbSq.rwVectorCursor().shape());

      avgPBSqVec= liavgpbSq.rwVectorCursor();
      avgPBVec = liavgpb.rwVectorCursor();

      for(int i=0;i<PBCorrection.shape();i++)
      {
        PBCorrection(i)=(avgPBSqVec(i)/avgPBVec(i));///(sincConv(i)*sincConv(iy));
        if ((abs(avgPBVec(i))) >= pbLimit_p)
        {
          lix.rwVectorCursor()(i) /= PBCorrection(i);
        }
      }

      if(fftNorm)
      {
        Complex rnorm(Float(inx)*Float(iny)/sumOfWts(pol,chan));
        lix.rwCursor()*=rnorm;
      }
      else
      {
        Complex rnorm(Float(inx)*Float(iny));
        lix.rwCursor()*=rnorm;
      }
    }
    else
    {
      lix.woCursor()=0.0;
    }
  }
}
//
//---------------------------------------------------------------
//
void FTMachine::normalizeImage(
  Lattice<Complex>& skyImage,
  const Matrix<Double>& sumOfWts,
  Lattice<Float>& sensitivityImage,
  Bool fftNorm)
{
  //
  // Apply the gridding correction
  //
  if (itsVerbose > 0) {
    cout<<"LofarFTMachine::normalizeImage<"<<endl;
  }
  Int inx = skyImage.shape()(0);
  Int iny = skyImage.shape()(1);
  Vector<Complex> correction(inx);

  Vector<Float> sincConv(nx);
  Float centerX=nx/2;
  for (Int ix=0;ix<nx;ix++)
    {
      Float x=C::pi*Float(ix-centerX)/(Float(nx)*Float(convSampling));
      if(ix==centerX) sincConv(ix)=1.0;
      else 	    sincConv(ix)=sin(x)/x;
    }

  IPosition cursorShape(4, inx, 1, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(skyImage.shape(), cursorShape, axisPath);
  //    LatticeIterator<Complex> lix(skyImage, lsx);
  LatticeIterator<Complex> lix(skyImage, lsx);

  LatticeStepper lavgpb(sensitivityImage.shape(),cursorShape,axisPath);
  // Array<Float> senArray;sensitivityImage.get(senArray,True);
  // ArrayLattice<Float> senLat(senArray,True);
  //    LatticeIterator<Float> liavgpb(senLat, lavgpb);
  LatticeIterator<Float> liavgpb(sensitivityImage, lavgpb);

  for(lix.reset(),liavgpb.reset();
      !lix.atEnd();
      lix++,liavgpb++)
    {
      Int pol=lix.position()(2);
      Int chan=lix.position()(3);

      if(sumOfWts(pol, chan)>0.0)
        {
          Int iy=lix.position()(1);
//           itsGridder->correctX1D(correction,iy);

          Vector<Float> avgPBVec(liavgpb.rwVectorCursor().shape());

          avgPBVec = liavgpb.rwVectorCursor();

          for(int i=0;i<avgPBVec.shape();i++)
            {
              //
              // This with the PS functions
              //
              // PBCorrection(i)=FUNC(avgPBVec(i))*sincConv(i)*sincConv(iy);
              // if ((abs(PBCorrection(i)*correction(i))) >= pbLimit_p)
              // 	lix.rwVectorCursor()(i) /= PBCorrection(i)*correction(i);
              // else if (!makingPSF)
              // 	lix.rwVectorCursor()(i) /= correction(i)*sincConv(i)*sincConv(iy);
              //
              // This without the PS functions
              //
              //                Float tt=sqrt(avgPBVec(i))/avgPBVec(i);
              Float tt = pbFunc(avgPBVec(i),pbLimit_p);
                //		PBCorrection(i)=pbFunc(avgPBVec(i),pbLimit_p)*sincConv(i)*sincConv(iy);
                //                lix.rwVectorCursor()(i) /= PBCorrection(i);
              //                lix.rwVectorCursor()(i) *= tt;

              lix.rwVectorCursor()(i) /= tt;
              // if ((abs(tt) >= pbLimit_p))
              //   lix.rwVectorCursor()(i) /= tt;
              // else if (!makingPSF)
              //   lix.rwVectorCursor()(i) /= sincConv(i)*sincConv(iy);
            }

          if(fftNorm)
            {
              Complex rnorm(Float(inx)*Float(iny)/sumOfWts(pol,chan));
              lix.rwCursor()*=rnorm;
            }
          else
            {
              Complex rnorm(Float(inx)*Float(iny));
              lix.rwCursor()*=rnorm;
            }
        }
      else
        lix.woCursor()=0.0;
    }
}


void FTMachine::makeCFPolMap(const VisBuffer& vb, const Vector<Int>& locCfStokes,
                                Vector<Int>& polM)
{
  LogIO log_l(LogOrigin("LofarFTMachine", "findPointingOffsets"));
  Vector<Int> msStokes = vb.corrType();
  Int nPol = msStokes.nelements();
  polM.resize(polMap.shape());
  polM = -1;

  for(Int i=0;i<nPol;i++)
    for(uInt j=0;j<locCfStokes.nelements();j++)
      if (locCfStokes(j) == msStokes(i))
          {polM(i) = j;break;}
}
//
//---------------------------------------------------------------
//
// Given a polMap (mapping of which Visibility polarization is
// gridded onto which grid plane), make a map of the conjugate
// planes of the grid E.g, for Stokes-I and -V imaging, the two
// planes of the uv-grid are [LL,RR].  For input VisBuffer
// visibilites in order [RR,RL,LR,LL], polMap = [1,-1,-1,0].  The
// conjugate map will be [0,-1,-1,1].
//
void FTMachine::makeConjPolMap(const VisBuffer& vb,
                                    const Vector<Int> cfPolMap,
                                    Vector<Int>& conjPolMap)
{
  LogIO log_l(LogOrigin("LofarFTMachine", "makConjPolMap"));
  //
  // All the Natak (Drama) below with slicers etc. is to extract the
  // Poln. info. for the first IF only (not much "information
  // hiding" for the code to slice arrays in a general fashion).
  //
  // Extract the shape of the array to be sliced.
  //
  Array<Int> stokesForAllIFs = vb.msColumns().polarization().corrType().getColumn();
  IPosition stokesShape(stokesForAllIFs.shape());
  IPosition firstIFStart(stokesShape),firstIFLength(stokesShape);
  //
  // Set up the start and length IPositions to extract only the
  // first column of the array.  The following is required since the
  // array could have only one column as well.
  //
  firstIFStart(0)=0;firstIFLength(0)=stokesShape(0);
  for(uInt i=1;i<stokesShape.nelements();i++) {firstIFStart(i)=0;firstIFLength(i)=1;}
  //
  // Construct the slicer and produce the slice.  .nonDegenerate
  // required to ensure the result of slice is a pure vector.
  //
  Vector<Int> visStokes = stokesForAllIFs(Slicer(firstIFStart,firstIFLength)).nonDegenerate();

  conjPolMap = cfPolMap;

  Int i,j,N = cfPolMap.nelements();
  
  for(i=0;i<N;i++)
    if (cfPolMap[i] > -1)
      {
      if      (visStokes[i] == Stokes::XX)
        {
          conjPolMap[i]=-1;
          for(j=0;j<N;j++) if (visStokes[j] == Stokes::YY) break;
          conjPolMap[i]=cfPolMap[j];
        }
      else if (visStokes[i] == Stokes::YY)
        {
          conjPolMap[i]=-1;
          for(j=0;j<N;j++) if (visStokes[j] == Stokes::XX) break;
          conjPolMap[i]=cfPolMap[j];
        }
      else if (visStokes[i] == Stokes::YX)
        {
          conjPolMap[i]=-1;
          for(j=0;j<N;j++) if (visStokes[j] == Stokes::XY) break;
          conjPolMap[i]=cfPolMap[j];
        }
      else if (visStokes[i] == Stokes::XY)
        {
          conjPolMap[i]=-1;
          for(j=0;j<N;j++) if (visStokes[j] == Stokes::YX) break;
          conjPolMap[i]=cfPolMap[j];
        }
      }
}

void FTMachine::showTimings (ostream& os, double duration) const
{
  // The total time is the real elapsed time.
  // The cf and (de)gridding time is the sum of all threads, so scale
  // them back to real time.
//   double total = itsCFTime + itsGriddingTime + itsDegriddingTime;
//   double scale = 1;
//   if (total > 0) 
//   {
//     scale = itsTotalTimer.getReal() / total;
//   }
//   itsConvFunc->showTimings (os, duration, itsCFTime*scale);
//   if (itsGriddingTime > 0) 
//   {
//     os << "  gridding          ";
//     ConvolutionFunction::showPerc1 (os, itsGriddingTime*scale,
//                                           duration);
//     os << endl;
//   }
//   if (itsDegriddingTime > 0) 
//   {
//     os << "  degridding        ";
//     ConvolutionFunction::showPerc1 (os, itsDegriddingTime*scale,
//                                           duration);
//     os << endl;
//   }
}

} // end namespace LofarFT
} // end namespace LOFAR
