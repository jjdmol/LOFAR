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

#include <AWImager2/FTMachine.h>
#include <AWImager2/CFStore.h>
#include <AWImager2/ConvolutionFunction.h>
#include <AWImager2/VisBuffer.h>
#include <AWImager2/VBStore.h>

#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/MaskedArray.h>
#include <casacore/casa/Arrays/ArrayLogical.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/Slicer.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Arrays/Cube.h>
#include <casacore/casa/Arrays/MatrixIter.h>
#include <casacore/casa/BasicSL/Constants.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Containers/Block.h>
#include <casacore/casa/Containers/Record.h>
#include <casacore/casa/Exceptions/Error.h>
#include <casacore/casa/OS/PrecTimer.h>
#include <casacore/casa/OS/DynLib.h>
#include <casacore/casa/Quanta/UnitMap.h>
#include <casacore/casa/Quanta/UnitVal.h>
#include <casacore/casa/Utilities/Assert.h>
#include <casacore/casa/Utilities/CompositeNumber.h>
#include <casacore/casa/sstream.h>

#include <casacore/coordinates/Coordinates/CoordinateSystem.h>
#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/coordinates/Coordinates/StokesCoordinate.h>
#include <casacore/coordinates/Coordinates/Projection.h>
#include <casacore/images/Images/ImageInterface.h>
#include <casacore/images/Images/PagedImage.h>
#include <casacore/lattices/Lattices/ArrayLattice.h>
#include <casacore/lattices/Lattices/SubLattice.h>
#include <casacore/lattices/LRegions/LCBox.h>
#include <casacore/lattices/Lattices/LatticeCache.h>
#include <casacore/lattices/LatticeMath/LatticeFFT.h>
#include <casacore/lattices/Lattices/LatticeIterator.h>
#include <casacore/lattices/Lattices/LatticeStepper.h>
#include <casacore/measures/Measures/UVWMachine.h>
#include <casacore/measures/Measures/Stokes.h>
#include <casacore/ms/MeasurementSets/MSColumns.h>
#include <casacore/scimath/Mathematics/FFTServer.h>
#include <casacore/scimath/Mathematics/RigidVector.h>
#include <casacore/scimath/Mathematics/ConvolveGridder.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>

#include "helper_functions.tcc"

#define DORES True

using namespace casa;

namespace LOFAR {
namespace LofarFT {

FTMachine::FTMachine(
  const MeasurementSet& ms, 
  const ParameterSet& parset)
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
    itsPadding(parset.getDouble("gridding.padding", 1.0)),
    itsMaxAbsData(0.0), 
    itsCenterLoc(IPosition(4,0)),
    itsOffsetLoc(IPosition(4,0)),
    itsParset(parset),
    itsMS(ms),
    itsNWPlanes(100 /*nwPlanes*/), 
    itsWMax(parset.getDouble("data.wmax", 10000.0)),
    itsOversample(parset.getInt("gridding.oversample", 9)),
    itsVerbose(parset.getInt("verbose",0)),
    itsMaxSupport(parset.getInt("gridding.maxsupport", 1024)),
    itsImageName(parset.getString("output.imagename", "")),
    itsGriddingTime(0),   // counters to measure time spend per operation (Gridding, Degridding, and Convolution Function computation)
    itsDegriddingTime(0), //
    itsCFTime(0),          //
    itsGriddedDataDomain(UV),
    itsAveragePB()
{
  itsConvFunc = new ConvolutionFunction(
    itsMS, 
    itsWMax,
    itsOversample, 
    itsVerbose, 
    itsMaxSupport,
    itsParset);
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
  
  LOG_DEBUG_STR("Original shape " << image.shape()(0) << "," << image.shape()(1));
  LOG_DEBUG_STR("Padded shape " << itsPaddedShape(0) << ","  << itsPaddedShape(1));
  
  itsConvFunc->init(
    itsPaddedShape,
    image.coordinates().directionCoordinate (image.coordinates().findCoordinate(Coordinate::DIRECTION)),
    itsImageName);
}

FTMachine::~FTMachine()
{
}

Matrix<Float> FTMachine::getAveragePB()
{
  if (itsAveragePB.empty()) {
    
    IPosition blc(
      2, 
      (itsPaddedNX - itsNX + (itsPaddedNX % 2 == 0)) / 2,
      (itsPaddedNY - itsNY + (itsPaddedNY % 2 == 0)) / 2);
    IPosition shape(2, itsNX, itsNY);
    Slicer slicer(blc, shape);
    
    itsAveragePB.reference(itsConvFunc->getAveragePB()(slicer));

    // Make it persistent.
    store(itsAveragePB, itsImageName + ".avgpb");
  }
  return itsAveragePB;
}

Matrix<Float> FTMachine::getSpheroidal()
{
  if (itsSpheroidal.empty()) {
    
    IPosition blc(
      2, 
      (itsPaddedNX - itsNX + (itsPaddedNX % 2 == 0)) / 2,
      (itsPaddedNY - itsNY + (itsPaddedNY % 2 == 0)) / 2);
    IPosition shape(2, itsNX, itsNY);
    Slicer slicer(blc, shape);
    
    itsSpheroidal.reference(itsConvFunc->getSpheroidal()(slicer));

  }
  return itsSpheroidal;
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
  
  itsNormalizeModel = normalize;

//   itsVisResampler->init(itsUseDoubleGrid);
}


void FTMachine::finalizeToVis()
{
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
    IPosition shape(4, itsNX, itsNY, itsNPol, itsNChan);
    
    CountedPtr<ImageInterface<Complex> > complex_model_subimage = new SubImage<Complex>(*itsComplexModelImages[model], Slicer(blc, shape), True);
    
    // convert float IQUV model image to complex image
    StokesImageUtil::From(*complex_model_subimage, *itsModelImages[model]);
    
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
    
    itsModelGrids[model].reference(itsComplexModelImages[model]->get());
    
    Complex* ptr = itsModelGrids[model].data();
    
    FFTCMatrix fft;
    for (int ii = 0; ii<itsModelGrids[model].shape()(3); ii++)
    {
      for (int jj = 0; jj<itsModelGrids[model].shape()(2); jj++)
      {
        fft.forward (itsPaddedNX, ptr, OpenMP::maxThreads());
        ptr += itsPaddedNX*itsPaddedNY;
      }
    }
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
    itsComplexImages[i]->get(itsGriddedData[i]);
    
    itsGriddedData[i] = Complex();
    itsSumPB[i].resize (itsPaddedShape[0], itsPaddedShape[1]);
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize (itsNPol, itsNChan);
    itsSumWeight[i] = 0.;
  }
}

void FTMachine::residual(
    VisBuffer& vb, 
    casa::Int row, 
    casa::FTMachine::Type type)
{
  // First put the negated value of the data column in the modelviscube of vb
  switch(type) 
  {
  case FTMachine::OBSERVED:
    vb.setModelVisCube(-vb.visCube());
    break;
  case FTMachine::CORRECTED:
    vb.setModelVisCube(-vb.correctedVisCube());
    break;
  case FTMachine::MODEL:
    vb.setModelVisCube(-vb.modelVisCube());
    break;
  default:
    throw Exception("Invalid value for argument type in call of FTMachine::residual");
  }
  
  // get adds the predicted visibilies to the modelviscube
  get(vb, row);
  // Now negate the modelviscube so that its value is datacolumn - predicted visibilities
  vb.setModelVisCube(-vb.modelVisCube());
  // Put the residual visibilities onto the grid
  put(vb, row, False, FTMachine::MODEL);
}

void FTMachine::get(casa::VisBuffer& vb, Int row) 
{
  // do not call this FTMachine with a casa::VisBuffer 
  // only use the get method which accepts a LOFAR VisBuffer
  throw Exception("Not implemented.");
//   put( *static_cast<VisBuffer*>(&vb), row);
}

void FTMachine::put(const casa::VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type) 
{
  // do not call this FTMachine with a casa::VisBuffer 
  // only use the put method which accepts a LOFAR VisBuffer
  throw Exception("Not implemented.");
//   put( *static_cast<const VisBuffer*>(&vb), row, dopsf, type);
}

// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
void FTMachine::getImages(Matrix<Float>& weights, Bool normalize_image)
{
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
        Array<Complex> slice(itsGriddedData[i](start,end));
        slice /= Complex(itsSumWeight[i](IPosition(2,pol,chan)));
      }
    }
    
    ArrayLattice<Complex> lattice(itsGriddedData[i]);
    if (itsGriddedDataDomain == UV)
    {
      LatticeFFT::cfft2d(lattice, True);
    }
    
    IPosition blc(
      4, 
      (itsPaddedNX - itsImages[i]->shape()(0) + (itsPaddedNX % 2 == 0)) / 2, 
      (itsPaddedNY - itsImages[i]->shape()(1) + (itsPaddedNY % 2 == 0)) / 2,
      0,
      0);
    
    IPosition shape(4, itsNX, itsNY, itsNPol, itsNChan);

    
    LOG_DEBUG_STR("Get subimage...");    
    CountedPtr<ImageInterface<Complex> > complex_subimage = new SubImage<Complex>(*itsComplexImages[i], Slicer(blc, shape), True);
    LOG_DEBUG_STR("done.");    

    normalize(*complex_subimage, True, True);

    LOG_DEBUG_STR("Converting to Stokes...");    
    StokesImageUtil::To(*itsImages[i], *complex_subimage);
    LOG_DEBUG_STR("done.");    
  }
}  

void FTMachine::normalize(ImageInterface<Complex> &image, Bool do_beam, Bool do_spheroidal)
{
  
  Array<Float> beam;
  
  casa::Vector<casa::Double> x;

  if (do_spheroidal)
  {
    x.resize(itsNX);
    double halfsize = floor(itsNX / 2.0);
    double paddedhalfsize = floor(itsPaddedNX / 2.0);
    for (casa::uInt i=0; i<itsNX; ++i) 
    {
      x[i] = spheroidal(abs(i - halfsize) / paddedhalfsize);
    }
  }
  
  if (do_beam)
  {
    beam.reference(getAveragePB());
  }

  IPosition slice_shape(4, itsNX, itsNY, 1, 1);

  // Iterate over channels and polarizations

  int i_max = image.shape()[3];
  int j_max = image.shape()[2];
  int k_max = image.shape()[1];
  int l_max = image.shape()[0];
  
  #pragma omp parallel for collapse(4)
  for(Int i = 0; i < i_max; ++i)
  {
    for(Int j = 0; j < j_max; ++j)
    {
      for(Int k = 0; k < k_max; ++k)
      {
        for(Int l = 0; l < l_max; ++l)
        {
          IPosition pos(4,l,k,j,i);
          
          Complex v = image.getAt(pos);
          Float f = 1.0;
          if (do_spheroidal) f *= x[l] * x[k];
          if (do_beam) f *= beam(IPosition(2,l,k));
          if (f>1.0e-5)
          {
            image.putAt(v/f, pos);
          }
          else
          {
            image.putAt(0.0, pos);
          }
        }
      }
    }
  }
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
  ImageInterface<Float>& theImage,
  Matrix<Float>& weight) 
{
  logIO() << LogOrigin("LofarFTMachine", "makeImage") << LogIO::NORMAL;

  if(type==FTMachine::COVERAGE) {
    logIO() << "Type COVERAGE not defined for Fourier transforms" << LogIO::EXCEPTION;
  }

  // Loop over all visibilities and pixels
  VisBuffer vb(*static_cast<VisibilityIterator*>(&vi));

  
  
  // Initialize put (i.e. transform to Sky) for this model
  PtrBlock<ImageInterface<Float> * > images(1);
  images[0] = &theImage;
  
  initializeToSky(images, false);
  
  vi.origin();

  // Loop over the visibilities, putting VisBuffers
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) 
  {
    for (vi.origin(); vi.more(); vi++) 
    {
      switch(type) 
      {
      case FTMachine::RESIDUAL:
        residual(vb, -1);
        break;
      case FTMachine::PSF:
        put(vb, -1, True);
        break;
      default:
        put(vb, -1, False);
        break;
      }
    }
  }
  finalizeToSky();
  getImages(weight, True);
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
  
  switch (itsConvFunc->image_polarization())
  {
    case ConvolutionFunction::Polarization::STOKES :
      stokes(0) = Stokes::I;
      stokes(1) = Stokes::Q;
      stokes(2) = Stokes::U;
      stokes(3) = Stokes::V;
      break;
    case ConvolutionFunction::Polarization::CIRCULAR :
      stokes(0) = Stokes::RR;
      stokes(1) = Stokes::RL;
      stokes(2) = Stokes::LR;
      stokes(3) = Stokes::LL;
      break;
    case ConvolutionFunction::Polarization::LINEAR :
      stokes(0) = Stokes::XX;
      stokes(1) = Stokes::XY;
      stokes(2) = Stokes::YX;
      stokes(3) = Stokes::YY;
      break;
  }
  cout << endl;

  return StokesCoordinate(stokes);
}  

ImageInterface<Complex>& FTMachine::getImage(Matrix<Float>&, Bool)
{
  throw Exception("Not implemented");
}

void FTMachine::ComputeResiduals(
    casa::VisBuffer&, 
    casa::Bool)
{
  throw Exception("Not implemented");
}

void FTMachine::initializeToVis(casa::ImageInterface<casa::Complex>&, const casa::VisBuffer&)
{
  throw Exception("Not implemented");
}

void FTMachine::initializeToSky(casa::ImageInterface<casa::Complex>&, casa::Matrix<casa::Float>&, const casa::VisBuffer&)
{
  throw Exception("Not implemented");
}


} // end namespace LofarFT
} // end namespace LOFAR
