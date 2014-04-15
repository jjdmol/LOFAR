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

#include <LofarFT/FTMachine.h>
#include <LofarFT/CFStore.h>
#include <LofarFT/ConvolutionFunction.h>
#include <LofarFT/VisBuffer.h>
#include <LofarFT/VBStore.h>

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

#define DORES True

using namespace casa;

namespace LOFAR {
namespace LofarFT {

FTMachine::FTMachine(
  const MeasurementSet& ms, 
  const Record& parameters)  
  : casa::FTMachine(), 
   // Aliases for data members of casa::FTMachine
//     itsImage(0),
//     itsNX(0),
//     itsNY(0),
//     itsNPol(0),
//     itsNChan(0),
    itsUseDoubleGrid(False),
//     itsChanMap(casa::FTMachine::chanMap),
//     itsPolMap(casa::FTMachine::polMap),
    
  // ================================================  
    itsPadding(parameters.asDouble("padding")), 
    itsMaxAbsData(0.0), 
    itsCenterLoc(IPosition(4,0)),
    itsOffsetLoc(IPosition(4,0)), 
    itsParameters(parameters), 
    itsMS(ms),
    itsNWPlanes(100 /*nwPlanes*/), 
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
    itsParameters = other.itsParameters;
    itsVisResampler = other.itsVisResampler;
    itsNGrid = other.itsNGrid;
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

void FTMachine::init(const ImageInterface<Float> &image) {

  logIO() << LogOrigin("LofarFTMachine", "init")  << LogIO::NORMAL;
  
  itsNX    = image.shape()(0);
  itsNY    = image.shape()(1);

  CompositeNumber cn(max(itsNX, itsNY) * 2); // Create a list of composite numbers
  itsPaddedNX = cn.nextLargerEven(Int(itsPadding * Float(itsNX) - 0.5));
  itsPaddedNY = cn.nextLargerEven(Int(itsPadding * Float(itsNY) - 0.5));

  
  itsNPol  = 4; // image.shape()(2);
  itsNChan = image.shape()(3);

  itsUVScale.resize(3);
  itsUVScale = 0.0;
  itsUVScale(0) = Float(itsPaddedNX) * image.coordinates().increment()(0); 
  itsUVScale(1) = Float(itsPaddedNY) * image.coordinates().increment()(1);
  itsUVScale(2) = Float(1)*abs(image.coordinates().increment()(0));

  itsUVOffset.resize(3);
  itsUVOffset(0) = itsPaddedNX/2;
  itsUVOffset(1) = itsPaddedNY/2;
  itsUVOffset(2) = 0;

  itsPaddedShape = image.shape();
  itsPaddedShape(0) = itsPaddedNX;
  itsPaddedShape(1) = itsPaddedNY;
  
  if (itsVerbose > 0) {
    cout << "Original shape " << image.shape()(0) << ","
         << image.shape()(1) << endl;
    cout << "Padded shape " << itsPaddedShape(0) << ","
         << itsPaddedShape(1) << endl;
  }
  
  itsConvFunc = new ConvolutionFunction(
    itsPaddedShape,
    image.coordinates().directionCoordinate (image.coordinates().findCoordinate(Coordinate::DIRECTION)),
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
    
    // get the 'average beam' from disk
    PagedImage<Float> pim(itsImageName + ".avgpb");
    Array<Float> arr = pim.get();
    
    // get the spheroid
    IPosition start(2,0,0);
    IPosition length(2, itsNX-1, itsNY-1);
    Array<Float> spheroid = itsConvFunc->getSpheroidCut()(start,length);
    
// TODO: sqrt and spheroid should not be here, but image on disk is (average beam * spheroid)^2
    itsAvgPB = sqrt(arr.nonDegenerate(2))/spheroid;
    
  }
  return itsAvgPB;
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image

void FTMachine::initializeToVis(
  casa::PtrBlock<casa::ImageInterface<casa::Float>* > &model_images,
  casa::Bool normalize)
{

  itsModelImages = model_images;
  
  // Pass the first model image to init
  // Initializes image dimensions etc
  // Creates the convolution function generator
  init(*model_images[0]);
  
  // Create complex model grid
  // Does normalization to true sky brightness, padding, and fft
  initialize_model_grids(normalize);

//   itsVisResampler->init(itsUseDoubleGrid);
}


void FTMachine::finalizeToVis()
{
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToVis"<<endl;
  }
  // destroy model grids
  finalize_model_grids();
}


// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid.

void FTMachine::initializeToSky(
  casa::PtrBlock<casa::ImageInterface<casa::Float> * > &images,
  casa::Bool doPSF)
{
  itsImages = images;
  
  init(*images[0]);
  
  // initialize_grids uses itsImages to set up complex grids
  initialize_grids();
  
  
}

void FTMachine::finalizeToSky()
{
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToSky" << endl;
  }
}


// *****************************************************************
void FTMachine::initializeResidual(
  casa::PtrBlock<casa::ImageInterface<casa::Float>* > model_images,
  casa::PtrBlock<casa::ImageInterface<casa::Float>* > images, 
  casa::Bool normalize)
{
  itsModelImages = model_images;
  itsImages = images;
  init(*model_images[0]);
  initialize_model_grids(normalize);
  initialize_grids();
}

// ***********************************************************
void FTMachine::finalizeResidual()
{
  finalize_model_grids();
}

  
void FTMachine::initialize_model_grids(Bool normalize_model)
{
    
  uInt nmodels = itsModelImages.nelements();
  
  //TODO: ask FTMachine subclass through virtual function
  itsNGrid = nmodels;

  itsComplexModelImages.resize(nmodels);
  itsModelGrids.resize(nmodels);
  
  CoordinateSystem coords = itsModelImages[0]->coordinates();

  IPosition gridShape(4, itsPaddedNX, itsPaddedNY, itsNPol, itsNChan);
  
  StokesCoordinate stokes_coordinate = get_stokes_coordinates();
  
  Int stokes_index = coords.findCoordinate(Coordinate::STOKES);
  coords.replaceCoordinate(stokes_coordinate, stokes_index);
  
  for (uInt model = 0; model<nmodels; model++)
  {
    // create complex model images
    // Force in memory, allow 1e6 MB memory usage
    itsComplexModelImages[model] = new TempImage<Complex> (gridShape, coords, 1e6); 


    // Create a writable subimage
    // The selection is the non-padded part inside the padded image
    
    IPosition blc(
      4, 
      (itsPaddedNX - itsModelImages[model]->shape()(0) + (itsPaddedNX % 2 == 0)) / 2,
      (itsPaddedNY - itsModelImages[model]->shape()(1) + (itsPaddedNY % 2 == 0)) / 2,
      0, 
      0);
    IPosition trc(4, itsNX, itsNY, itsNPol, itsNChan);
    
    cout << itsComplexModelImages[model]->shape() << " " << blc << " " << trc << endl;
    CountedPtr<ImageInterface<Complex> > complex_model_subimage = new SubImage<Complex>(*itsComplexModelImages[model], Slicer(blc, trc), True);
    
    // convert float IQUV model image to complex image
    StokesImageUtil::From(*complex_model_subimage, *itsModelImages[model]);
    
    cout << complex_model_subimage->shape() << " " << IPosition(4,itsNX, itsNY, 4, itsNChan) << endl;
//     Array<Complex> slice;
//     slice = complex_model_subimage->getSlice(IPosition(4,0,0,0,0), IPosition(4,itsNX, itsNY, 4, itsNChan) );
//     convertArray(slice, itsModelImages[model]->get());
//     slice = complex_model_subimage->getSlice(IPosition(4,0,0,3,0), IPosition(4,itsNX, itsNY, 4, itsNChan) );
//     convertArray(slice, itsModelImages[model]->get());
    
    // normalize complex images 
    // divide out spheroidal
    // go to true flux if needed
    
    normalize(*complex_model_subimage, normalize_model, True);

//     PagedImage<Float> modelimage("model.img");
//     StokesImageUtil::From(*complex_model_subimage, modelimage);
//     normalize(*complex_model_subimage, False, True);
    
    
    // Now do the FFT2D in place
    // LatticeFFT::cfft2d takes the 2D FFT over the first two dimensions, and iterates over the rest
    LatticeFFT::cfft2d(*itsComplexModelImages[model]);
    
    itsModelGrids[model].reference(itsComplexModelImages[model]->get());
    cout << "**** " << abs(itsModelGrids[model](IPosition(4, 400,400,0,0))) << " ****" << endl;
  }
  
}


void FTMachine::finalize_model_grids()
{
  //TODO: destroy model grids
}

void FTMachine::initialize_grids()
{  
  
// TODO:   itsNGrid = ngrid();
  itsNGrid = itsImages.nelements();
  cout << "itsNGrid: " << itsNGrid << endl;
  
  Matrix<Float> weight;
  itsComplexImages.resize (itsNGrid);
  itsGriddedData.resize (itsNGrid);
  itsSumWeight.resize (itsNGrid);
  itsSumCFWeight.resize (itsNGrid);
  itsSumPB.resize (itsNGrid);
  
  IPosition gridShape(4, itsPaddedNX, itsPaddedNY, itsNPol, itsNChan);

  CoordinateSystem coords = itsImages[0]->coordinates();
  Int stokes_index = coords.findCoordinate(Coordinate::STOKES);
  StokesCoordinate stokes_coordinate = get_stokes_coordinates();
  coords.replaceCoordinate(stokes_coordinate, stokes_index);
  
  for (int i = 0; i < itsNGrid; ++i) 
  {
    // create complex images
    // Force in memory, allow 1e6 MB memory usage
    itsComplexImages[i] = new TempImage<Complex> (gridShape, coords, 1e6); 
    if (itsComplexImages[i]->get(itsGriddedData[i]))
    {
      cout << "OK, it is a reference." << endl;
    }
    else
    {
      cout << "THIS IS NOT OK, not a reference." << endl;
    }
    
    itsGriddedData[i] = Complex();
    itsSumPB[i].resize (itsPaddedShape[0], itsPaddedShape[1]);
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    cout << "itsNPol: " << itsNPol << endl;
    itsSumWeight[i].resize (itsNPol, itsNChan);
    itsSumWeight[i] = 0.;
  }
}

// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
void FTMachine::getImages(Matrix<Float>& weights, Bool normalize_image)
{
  cout << "FTMachineSimpleWB::getImage" << endl;
  
  logIO() << LogOrigin("FTMachine", "getImage") << LogIO::NORMAL;

  // force computation of average primary beam
  // the result is stored on disk
  // and later fetched by getAveragePB
  itsConvFunc->compute_avg_pb(itsSumPB[0], itsSumCFWeight[0]);
  
  for(Int i=0; i<itsImages.nelements(); i++)
  {
    
    IPosition start(4,0);
    IPosition end = itsGriddedData[i].shape() - 1; // end index is inclusive, need to subtract 1
    
    for(Int chan=0; chan < itsNChan; chan++)
    {
      start(3) = end(3) = chan;
      for(Int pol=0; pol < itsNPol; pol++)
      {
        start(2) = end(2) = pol;
        cout << start << " " << end << " " << itsGriddedData[i]. shape() << endl;
        Array<Complex> slice(itsGriddedData[i](start,end));
        slice /= Complex(itsSumWeight[i](IPosition(2,pol,chan)));
      }
    }
    
    ArrayLattice<Complex> lattice(itsGriddedData[i]);
    LatticeFFT::cfft2d(lattice, True);
    
    cout << "itsSumWeight[" << i << "]: " << itsSumWeight[i] << endl;
    
    IPosition blc(
      4, 
      (itsPaddedNX - itsImages[i]->shape()(0) + (itsPaddedNX % 2 == 0)) / 2, 
      (itsPaddedNY - itsImages[i]->shape()(1) + (itsPaddedNY % 2 == 0)) / 2,
      0,
      0);
    
    IPosition shape(4, itsNX, itsNY, itsNPol, itsNChan);

    cout << blc << endl;
    cout << shape << endl;
    cout << itsComplexImages[i]->shape() << endl;
    
    CountedPtr<ImageInterface<Complex> > complex_subimage = new SubImage<Complex>(*itsComplexImages[i], Slicer(blc, shape), True);

    cout << "isWritable: " << complex_subimage->isWritable() << endl;
    
    normalize(*complex_subimage, True, True);

    StokesImageUtil::To(*itsImages[i], *complex_subimage);
    
  }
}  

void FTMachine::normalize(ImageInterface<Complex> &image, Bool do_beam, Bool do_spheroidal)
{
  LatticeExprNode factor(1.0);
  
  if (do_spheroidal)
  {
    IPosition start(2,0,0);
    IPosition length(2, itsNX-1, itsNY-1);
    factor = factor * ArrayLattice<Float>(itsConvFunc->getSpheroidCut()(start,length));
  }
  
  if (do_beam)
  {
    factor = factor * ArrayLattice<Float>(getAveragePB()); 
  }

  Array<Complex> slice;
  IPosition start(4,0);
  IPosition slice_shape = image.shape();
  slice_shape[2] = 1;
  slice_shape[3] = 1;

  // Iterate over channels and polarizations

  for(Int i = 0; i < image.shape()[3]; ++i)
  {
    start[3] = i;
    for(Int j = 0; j < image.shape()[2]; ++j)
    {
      start[2] = j;
      image.getSlice(slice, start, slice_shape, True);
      ArrayLattice<Complex> lattice(slice);
      cout << lattice.shape() << " " << factor.shape() << endl;
      lattice.copyData(LatticeExpr<Complex>(iif(factor < 1e-2, 0.0, lattice / factor)));
    }
  }
}
  
//   itsAvgPB.reference (itsConvFunc->compute_avg_pb(itsSumPB[0], itsSumCFWeight[0]));
// 
//   weights.resize(itsSumWeight[0].shape());
// 
//   convertArray(weights, itsSumWeight[0]);
//   // If the weights are all zero then we cannot normalize
//   // otherwise we don't care.
//   if(normalize&&max(weights)==0.0) 
//   {
//     logIO() << LogIO::SEVERE << "No useful data in LofarFTMachine: weights all zero"
//             << LogIO::POST;
//     return *itsImage;
//   }
// 
//   const IPosition latticeShape = itsLattice->shape();
// 
//   logIO() << LogIO::DEBUGGING
//           << "Starting FFT and scaling of image" << LogIO::POST;
// 
//   if (itsUseDoubleGrid) 
//   {
//     ArrayLattice<DComplex> darrayLattice(itsGriddedData2[0]);
//     LatticeFFT::cfft2d(darrayLattice,False);
//     convertArray(itsGriddedData[0], itsGriddedData2[0]);
//   }
//   else 
//   {
//     LatticeFFT::cfft2d(*itsLattice, False);
//   }
// 
//   if (itsVerbose > 0) 
//   {
//     cout<<"POLMAP:::::::  "<< itsPolMap << endl;
//     cout<<"POLMAP:::::::  "<< itsCFMap << endl;
//   }
//   
//   Int inx = itsLattice->shape()(0);
//   Int iny = itsLattice->shape()(1);
//   Vector<Complex> correction(inx);
//   correction=Complex(1.0, 0.0);
//   // Do the Grid-correction
//   IPosition cursorShape(4, inx, 1, 1, 1);
//   IPosition axisPath(4, 0, 1, 2, 3);
//   LatticeStepper lsx(itsLattice->shape(), cursorShape, axisPath);
//   LatticeIterator<Complex> lix(*itsLattice, lsx);
//   for (lix.reset(); !lix.atEnd(); lix++) 
//   {
//     Int pol=lix.position()(2);
//     Int chan=lix.position()(3);
//     if (weights(pol, chan) != 0.0) 
//     {
//       if (normalize) 
//       {
//         Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
//         lix.rwCursor()*=rnorm;
//       }
//       else 
//       {
//         Complex rnorm(Float(inx)*Float(iny));
//         lix.rwCursor()*=rnorm;
//       }
//     }
//     else {
//       lix.woCursor()=0.0;
//     }
//   }
//   
//   IPosition pos(4, itsLattice->shape()[0], itsLattice->shape()[1], 1, 1);
//   uInt shapeout(floor(itsLattice->shape()[0]/itsPadding));
//   uInt istart(floor((itsLattice->shape()[0] - shapeout)/2.));
//   Cube<Complex> tempimage(IPosition(3, shapeout, shapeout, itsLattice->shape()[2]));
// 
//   pos[3] = 0.;
//   for(Int k=0; k < itsLattice->shape()[2]; ++k)
//   {
//     for(uInt i = 0; i < shapeout; ++i)
//     {
//       for(uInt j = 0; j < shapeout; ++j)
//       {
//         pos[0] = i + istart;
//         pos[1] = j + istart;
//         pos[2] = k;
//         Complex pixel(itsLattice->getAt(pos));
//         pixel /= sqrt(itsAvgPB(i + istart, j + istart));
//         itsLattice->putAt(pixel,pos);
//       }
//     }
//   }
// 


// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& FTMachine::getImage(Matrix<Float>& weights, Bool normalize)
{
//   AlwaysAssert(itsImage, AipsError);
//   logIO() << LogOrigin("FTMachine", "getImage") << LogIO::NORMAL;
// 
//   if (itsVerbose > 0) 
//   {
//     cout<<"GETIMAGE"<<endl;
//   }
//   
//   itsAvgPB.reference (itsConvFunc->compute_avg_pb(itsSumPB[0], itsSumCFWeight[0]));
// 
//   weights.resize(itsSumWeight[0].shape());
// 
//   convertArray(weights, itsSumWeight[0]);
//   // If the weights are all zero then we cannot normalize
//   // otherwise we don't care.
//   if(normalize&&max(weights)==0.0) 
//   {
//     logIO() << LogIO::SEVERE << "No useful data in LofarFTMachine: weights all zero"
//             << LogIO::POST;
//     return *itsImage;
//   }
// 
//   const IPosition latticeShape = itsLattice->shape();
// 
//   logIO() << LogIO::DEBUGGING
//           << "Starting FFT and scaling of image" << LogIO::POST;
// 
//   if (itsUseDoubleGrid) 
//   {
//     ArrayLattice<DComplex> darrayLattice(itsGriddedData2[0]);
//     LatticeFFT::cfft2d(darrayLattice,False);
//     convertArray(itsGriddedData[0], itsGriddedData2[0]);
//   }
//   else 
//   {
//     LatticeFFT::cfft2d(*itsLattice, False);
//   }
// 
//   if (itsVerbose > 0) 
//   {
//     cout<<"POLMAP:::::::  "<< itsPolMap << endl;
//     cout<<"POLMAP:::::::  "<< itsCFMap << endl;
//   }
//   
//   Int inx = itsLattice->shape()(0);
//   Int iny = itsLattice->shape()(1);
//   Vector<Complex> correction(inx);
//   correction=Complex(1.0, 0.0);
//   // Do the Grid-correction
//   IPosition cursorShape(4, inx, 1, 1, 1);
//   IPosition axisPath(4, 0, 1, 2, 3);
//   LatticeStepper lsx(itsLattice->shape(), cursorShape, axisPath);
//   LatticeIterator<Complex> lix(*itsLattice, lsx);
//   for (lix.reset(); !lix.atEnd(); lix++) 
//   {
//     Int pol=lix.position()(2);
//     Int chan=lix.position()(3);
//     if (weights(pol, chan) != 0.0) 
//     {
//       if (normalize) 
//       {
//         Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
//         lix.rwCursor()*=rnorm;
//       }
//       else 
//       {
//         Complex rnorm(Float(inx)*Float(iny));
//         lix.rwCursor()*=rnorm;
//       }
//     }
//     else {
//       lix.woCursor()=0.0;
//     }
//   }
//   
//   IPosition pos(4, itsLattice->shape()[0], itsLattice->shape()[1], 1, 1);
//   uInt shapeout(floor(itsLattice->shape()[0]/itsPadding));
//   uInt istart(floor((itsLattice->shape()[0] - shapeout)/2.));
//   Cube<Complex> tempimage(IPosition(3, shapeout, shapeout, itsLattice->shape()[2]));
// 
//   pos[3] = 0.;
//   for(Int k=0; k < itsLattice->shape()[2]; ++k)
//   {
//     for(uInt i = 0; i < shapeout; ++i)
//     {
//       for(uInt j = 0; j < shapeout; ++j)
//       {
//         pos[0] = i + istart;
//         pos[1] = j + istart;
//         pos[2] = k;
//         Complex pixel(itsLattice->getAt(pos));
//         pixel /= sqrt(itsAvgPB(i + istart, j + istart));
//         itsLattice->putAt(pixel,pos);
//       }
//     }
//   }
// 
//   IPosition blc(
//     4, 
//     (itsNX - itsImage->shape()(0) + (itsNX % 2 == 0)) / 2, 
//     (itsNY - itsImage->shape()(1) + (itsNY % 2 == 0)) / 2,
//     0,
//     0);
//   IPosition stride(4, 1);
//   IPosition trc( blc + itsImage->shape() - stride);
//   // Do the copy
//   IPosition start(4, 0);
//   itsImage->put(itsGriddedData[0](blc, trc));
//   
//   return *image;
}


// Get weight image
void FTMachine::getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights)
{
  throw AipsError("LOFAR::LofarFT::FTMachine::getWeightImage not implemented");
}

void FTMachine::ok() {
//   AlwaysAssert(image, AipsError);
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
  VisBuffer vb(*static_cast<VisibilityIterator*>(&vi));

  
  
  // Initialize put (i.e. transform to Sky) for this model
  vi.origin();

//   cout << "Image shape: " << theImage.shape() << endl;
  CoordinateSystem coords = theImage.coordinates();
  Int stokesIndex = coords.findCoordinate(Coordinate::STOKES);
  AlwaysAssert(stokesIndex>-1, AipsError);
  StokesCoordinate stokesCoord = coords.stokesCoordinate(stokesIndex);
//   cout << "Image stokes:" << stokesCoord.stokes() << endl;
  theImage.putAt(Complex(1.0,0.0), IPosition(4,0,0,0,0));
//   cout << "Image data: " << theImage.getAt(IPosition(4,0,0,0,0)) << ", "
//        << theImage.getAt(IPosition(4,0,0,1,0))  << ", "
//        << theImage.getAt(IPosition(4,0,0,2,0))  << ", "
//        << theImage.getAt(IPosition(4,0,0,3,0)) << endl;
  
  if(vb.polFrame()==MSIter::Linear) {
    StokesImageUtil::changeCStokesRep(theImage, StokesImageUtil::LINEAR);
  }
  else {
    StokesImageUtil::changeCStokesRep(theImage, StokesImageUtil::CIRCULAR);
  }

  coords = theImage.coordinates();
  stokesIndex = coords.findCoordinate(Coordinate::STOKES);
  AlwaysAssert(stokesIndex>-1, AipsError);
  stokesCoord = coords.stokesCoordinate(stokesIndex);
//   cout << "Image stokes:" << stokesCoord.stokes() << endl;

//   cout << "Image data: " << theImage.getAt(IPosition(4,0,0,0,0)) << ", "
//        << theImage.getAt(IPosition(4,0,0,1,0))  << ", "
//        << theImage.getAt(IPosition(4,0,0,2,0))  << ", "
//        << theImage.getAt(IPosition(4,0,0,3,0)) << endl;

//   PtrBlock<ImageInterface images  
//   initializeToSky(theImage, weight, vb);
//   cout << "itsSumPB.size() " << itsSumPB.size() << endl;
//   cout << "itsSumPB[0].shape() " << itsSumPB[0].shape() << endl;
  

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

void FTMachine::ComputeResiduals(casa::VisBuffer&vb, Bool useCorrected)
{
  VBStore vbs;
  vbs.nRow(vb.nRow());
  vbs.beginRow(0);
  vbs.endRow(vbs.endRow());
  vbs.modelVisCube(vb.modelVisCube());
  if (useCorrected) 
  {
    vbs.visCube(vb.correctedVisCube());
  }
  else
  {
    vbs.visCube(vb.visCube());
  }
  itsVisResampler->ComputeResiduals(vbs);
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

StokesCoordinate FTMachine::get_stokes_coordinates()
{
  Vector<Int> stokes(4);
  
  cout << "Convolution function wants image polarization: ";
  switch (itsConvFunc->image_polarization())
  {
    case ConvolutionFunction::Polarization::STOKES :
      cout << "STOKES";
      stokes(0) = Stokes::I;
      stokes(1) = Stokes::Q;
      stokes(2) = Stokes::U;
      stokes(3) = Stokes::V;
      break;
    case ConvolutionFunction::Polarization::CIRCULAR :
      cout << "CIRCULAR";
      stokes(0) = Stokes::RR;
      stokes(1) = Stokes::RL;
      stokes(2) = Stokes::LR;
      stokes(3) = Stokes::LL;
      break;
    case ConvolutionFunction::Polarization::LINEAR :
      cout << "LINEAR";
      stokes(0) = Stokes::XX;
      stokes(1) = Stokes::XY;
      stokes(2) = Stokes::YX;
      stokes(3) = Stokes::YY;
      break;
  }
  cout << endl;

  return StokesCoordinate(stokes);
}  


} // end namespace LofarFT
} // end namespace LOFAR
