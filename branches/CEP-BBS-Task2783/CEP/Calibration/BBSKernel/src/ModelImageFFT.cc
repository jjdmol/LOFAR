//# ModelImageFFT.cc: 
//#
//#
//# Copyright (C) 2012
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
//# $Id: ModelImageFFT.cc 20029 2012-02-20 15:50:23Z duscha $

#include <lofar_config.h>

#include <casa/Arrays/Matrix.h>

#include <synthesis/MeasurementComponents/FTMachine.h>
#include <LofarFT/LofarFTMachine.h>
#include <LofarFT/LofarCFStore.h>
//#include <LofarFT/LofarVisibilityResamplerBase.h>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelImageFFT.h>

#define DORES True                // why on earth is this done in LofarFTMachine?

using namespace std;
using namespace casa;

using namespace LOFAR;
using namespace BBS;

//*********************************************
//
// Constructors and destructor
//
//*********************************************

ModelImageFft::ModelImageFft( const casa::String &name,
                              const casa::MDirection phasedir,
                              double wmax, 
                              unsigned int nwplanes, 
                              bool aprojection)
{
//  ImageInterface<Complex>& iimage;    // load image from disk
  
  setDefaults();
  setWmax(wmax);
  if(nwplanes==1)
  {
    setWprojection(false);
    setNwplanes(1);
  }
  else
  {
    setWprojection(true);
    setWmax(wmax);
    setNwplanes(nwplanes);
  }
  setPhaseDir(phasedir);
  setAprojection(aprojection);
}

ModelImageFft::~ModelImageFft(void)
{
  // currently do nothing
  // TODO: Release memory of LatticeCache?
}

//*********************************************
//
// Setter functions for options
//
//*********************************************

void ModelImageFft::setDefaults()
{
    // http://www.lofar.org/operations/doku.php?id=engineering:software:tools:cimagerdoc:gridder&s[]=maxsupport
    itsOptions.ConvType="SF";                   // convolution type?
    itsOptions.Wprojection=False;               // don't use w-projection
    itsOptions.Aprojection=False;               // don't use w-projection
    itsOptions.Nwplanes=1;                      // number of w-projection planes (default: 1)
//    itsOptions.Mlocation;                     // location of observation, how to get it?
    itsOptions.padding=2;                       // padding used in gridding?
    itsOptions.wmax=75;                         // maximum w value to use?
//    itsBeamPath=getenv("LOFARROOT").append("/share");     // default path to beam model
    itsOptions.verbose=0;                       // verbosity level
    itsOptions.maxSupport=256;                  // maximum support
    itsOptions.oversample=8;                    // FFT oversampling
    itsOptions.imageName="";                    // name of ModelImage

    Matrix<Bool> muellerMask(4,4);

    itsOptions.gridMuellerMask=muellerMask;
    itsOptions.degridMuellerMask=muellerMask;
    itsOptions.NThread=1;                       // number of threads used
}

void ModelImageFft::setConvType(const casa::String type)
{
  if(type=="SF" || type=="BOX")
  {
    itsOptions.ConvType=type;
  }
  else
  {
    THROW(BBSKernelException, "Convolution type " << type << " is unknown.");
  }
}

void ModelImageFft::setWprojection(bool mode)
{
  itsOptions.Wprojection=mode;
}

void ModelImageFft::setAprojection(bool mode)
{
  itsOptions.Aprojection=mode;
}

void ModelImageFft::setNwplanes(casa::uInt nwplanes)
{
  itsOptions.Nwplanes=nwplanes;
}

void ModelImageFft::setMlocation(casa::MPosition &location)
{
  itsOptions.Mlocation=location;
}

void ModelImageFft::setPhaseDir(const casa::MDirection &phasedir)
{
  itsOptions.PhaseDir=phasedir;
}

void ModelImageFft::setPadding(casa::Float padding)
{
  if(padding < 0)
  {
    itsOptions.padding=padding;
  }
  else
  {
    THROW(BBSKernelException, "Padding < 0.");
  }
}

void ModelImageFft::setWmax(double wmax)
{
  itsOptions.wmax=wmax;
}

void ModelImageFft::setVerbose(casa::uInt verbose)
{
  itsOptions.verbose=verbose;
}

void ModelImageFft::setMaxSupport(casa::Int maxsupport)
{
  itsOptions.maxSupport=maxsupport;
}

void ModelImageFft::setOversample(casa::uInt oversample)
{
  itsOptions.oversample=oversample;
}

void ModelImageFft::setImageName(const casa::String &imagename)
{
  itsOptions.imageName=imagename;
}

void setGridMuellerMask(casa::Matrix<Bool> muellerMask)
{
  if(muellerMask.nrow() != 4 && muellerMask.ncolumn() != 4)
  {
    setGridMuellerMask(muellerMask);
  }
  else
  {
    THROW(BBSKernelException, "gridMuellerMask has wrong dimensions " << muellerMask.nrow() << "," 
          << muellerMask.ncolumn());
  }
}

void setDegridMuellerMask(casa::Matrix<Bool> muellerMask)
{
  if(muellerMask.nrow() != 4 && muellerMask.ncolumn() != 4)
  {
    setDegridMuellerMask(muellerMask);
  }
  else
  {
    setDegridMuellerMask(muellerMask);
  }
}

void ModelImageFft::setNThread(unsigned int nthread)
{
  itsOptions.NThread=nthread;
}


//*********************************************
//
// LofarFtMachine functions
//
//*********************************************

void ModelImageFft::computeConvolutionFunctions()
{
  itsConvolutionFunctions.resize(itsOptions.Nwplanes);  // do we really have a conv function per w-plane?
  for(int i=0; i<itsOptions.Nwplanes; i++)
  {
    itsConvolutionFunctions[i]; // call LofarConvolutionFunction constructor here
  }
  /*
  // Constructor (needs MS, so not much use)
  LofarConvolutionFunction(const IPosition& shape,
                           const DirectionCoordinate& coordinates,
                           const MeasurementSet& ms,
                           uInt nW, double Wmax,
                           uInt oversample,
                           Int verbose,
                           Int maxsupport,
                           const String& imgName,
                           Bool Use_EJones,
                           Bool Apply_Element,
                           const casa::Record& parameters
                          );

    LofarCFStore makeConvolutionFunction(uInt stationA, uInt stationB,
                                         Double time, Double w,
                                         const Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         Matrix<Complex>& Stack_PB_CF,
                                         double& sum_weight_square,
					                               uInt spw, Int TaylorTerm, double RefFreq);

    // Compute and store W-terms and A-terms in the fourier domain
    void store_all_W_images();
    */
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image
void ModelImageFft::initializeToVis(ImageInterface<Complex>& iimage)
{
  if (itsOptions.verbose > 0) {
    cout<<"---------------------------> initializeToVis"<<endl;
  }
  image=&iimage;

//  ok();   // don't do this
  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
//  initMaps(vb);
  initMaps();

  visResamplers_p.init(useDoubleGrid_p);
  visResamplers_p.setMaps(chanMap_p, polMap_p);
  visResamplers_p.setCFMaps(CFMap_p, ConjCFMap_p);

  // Need to reset nx, ny for padding
  // Padding is possible only for non-tiled processing

  // If we are memory-based then read the image in and create an
  // ArrayLattice otherwise just use the PagedImage
//  AlwaysAssert (!isTiled, AipsError);

  //  cout<<"LofarFTMachine::initializeToVis === is_NOT_Tiled!"<<endl;
  //cout << "npol="<<npol<<endl;
  IPosition gridShape(4, nx, ny, npol, nchan);
  // Size and initialize the grid buffer per thread.
  // Note the other itsGriddedData buffers are assigned later.
  itsGriddedData[0].resize (gridShape);
  itsGriddedData[0] = Complex();
  /* // Don't need beam stuff
  for (int i=0; i<itsOptions.NThread; ++i) {
    itsSumPB[i].resize (padded_shape[0], padded_shape[1]);
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize(npol, nchan);
    itsSumWeight[i] = 0.;
  }
  */

  //griddedData can be a reference of image data...if not using model col
  //hence using an undocumented feature of resize that if
  //the size is the same as old data it is not changed.
  //if(!usePut2_p) griddedData.set(0);

  IPosition stride(4, 1);
  IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
  IPosition trc(blc+image->shape()-stride);
  if (getVerbose() > 0) {
    cout<<"LofarFTMachine::initializeToVis === blc,trc,nx,ny,image->shape()"
        <<blc<<" "<<trc<<" "<<nx<<" "<<ny<<" "<<image->shape()<<endl;
  }
  IPosition start(4, 0);
  itsGriddedData[0](blc, trc) = image->getSlice(start, image->shape());
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  //======================CHANGED
  arrayLattice = new ArrayLattice<Complex>(itsGriddedData[0]);
  // Array<Complex> result(IPosition(4, nx, ny, npol, nchan),0.);
  // griddedData=result;
  // arrayLattice = new ArrayLattice<Complex>(griddedData);
  //======================END CHANGED
  lattice=arrayLattice;

  //AlwaysAssert(lattice, AipsError);
//  logIO() << LogIO::DEBUGGING
//	  << "Starting grid correction and FFT of image" << LogIO::POST;

  LOG_INFO_STR("Starting grid correction and FFT of image");

  //==========================
  // Cyr: I have commeneted that part which does the spheroidal correction of the clean components in the image plane.
  // We do this based on our estimate of the spheroidal function, stored in an image
  // Do the Grid-correction.
    // {
    //   Vector<Complex> correction(nx);
    //   correction=Complex(1.0, 0.0);
    //   // Do the Grid-correction
    //   IPosition cursorShape(4, nx, 1, 1, 1);
    //   IPosition axisPath(4, 0, 1, 2, 3);
    //   LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
    //   LatticeIterator<Complex> lix(*lattice, lsx);
    //   for(lix.reset();!lix.atEnd();lix++) {
    //     gridder->correctX1D(correction, lix.position()(1));
    // 	lix.rwVectorCursor()/=correction;
    //   }
    // }

}

void ModelImageFft::init() 
{

//  logIO() << LogOrigin("LofarFTMachine", "init")  << LogIO::NORMAL;
  LOG_INFO_STR("LofarFTMachine init()");
  canComputeResiduals_p = DORES;
//  ok();   // we don't do this

  // We are padding.
  if(!noPadding_p){
    CompositeNumber cn(uInt(image->shape()(0)*2));
    nx    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(0))-0.5));
    ny    = cn.nextLargerEven(Int(padding_p*Float(image->shape()(1))-0.5));
  }
  else{
    nx    = image->shape()(0);
    ny    = image->shape()(1);
  }
  npol  = image->shape()(2);
  nchan = image->shape()(3);
    // }

  uvScale.resize(3);
  uvScale=0.0;
  uvScale(0)=Float(nx)*image->coordinates().increment()(0);
  uvScale(1)=Float(ny)*image->coordinates().increment()(1);
  uvScale(2)=Float(1)*abs(image->coordinates().increment()(0));

  uvOffset.resize(3);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;
  uvOffset(2)=0;

  // Now set up the gridder. The possibilities are BOX and SF
  if(gridder) delete gridder; gridder=0;
  gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						 uvScale, uvOffset,
						 itsOptions.ConvType);

  // Setup the CFStore object to carry relavent info. of the Conv. Func.
  cfs_p.xSupport = gridder->cSupport();
  cfs_p.ySupport = gridder->cSupport();
  cfs_p.sampling.resize(2);
  cfs_p.sampling = gridder->cSampling();
  if (cfs_p.rdata.null())
    cfs_p.rdata = new Array<Double>(gridder->cFunction());
  // else
  //   (*cfs_p.rdata) = gridder->cFunction();

  padded_shape = image->shape();
  padded_shape(0) = nx;
  padded_shape(1) = ny;
  if (itsOptions.verbose > 0) {
    cout << "Original shape " << image->shape()(0) << ","
         << image->shape()(1) << endl;
    cout << "Padded shape " << padded_shape(0) << ","
         << padded_shape(1) << endl;
  }
  assert(padded_shape(0)!=image->shape()(0));
  
  // How to do this without the MS? using modified ModelImageConvolutionFunction class
  /*
  itsConvFunc = new LofarConvolutionFunction( padded_shape,
                                              image->coordinates().directionCoordinate (image->coordinates().findCoordinate(Coordinate::DIRECTION)),
                                              itsMS, itsNWPlanes, itsWMax,
                                              itsOversample, itsBeamPath,
					                                    itsVerbose, itsMaxSupport,
                                              itsImgName);

    // Compute the convolution function for all channel, for the polarisations
    // specified in the Mueller_mask matrix
    // Also specify weither to compute the Mueller matrix for the forward or
    // the backward step. A dirty way to calculate the average beam has been
    // implemented, by specifying the beam correcting to the given baseline
    // and timeslot.
    // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]
    LofarCFStore makeConvolutionFunction(uInt stationA, uInt stationB,
                                         Double time, Double w,
                                         const Matrix<bool>& Mask_Mueller,
                                         bool degridding_step,
                                         double Append_average_PB_CF,
                                         Matrix<Complex>& Stack_PB_CF,
                                         double& sum_weight_square);
  */
}


void ModelImageFft::fftImage()
{
    LatticeFFT::cfft2d(*lattice);       // Now do the FFT2D in place
}

// LOFAR specific rewritten function initialize polarization (polmap)
// and channel maps (chanmap)
//
//void ModelImageFft::initMaps(const casa::VisBuffer &)
void ModelImageFft::initMaps(void)
{

}