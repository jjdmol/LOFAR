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

//#include <synthesis/MeasurementComponents/GridFT.h>       // GridFT machine for simple FT
#include <synthesis/MeasurementComponents/Utils.h>
#include <synthesis/MeasurementComponents/ComponentFTMachine.h>   // rotateUVW
#include <synthesis/MeasurementComponents/FTMachine.h>
#include <measures/Measures/UVWMachine.h>   // phase shift, rotate uvw etc.
#include <LofarFT/LofarFTMachine.h>
//#include <LofarFT/LofarVbStore.h>
//#include <LofarFT/LofarVisibilityResamplerBase.h>
#include <images/Images/PagedImage.h>
//#include <images/Images/ImageInterface.h>

#include <Common/OpenMP.h>

#include <Common/Exception.h>
#include <Common/LofarLogger.h>   // for ASSERT and ASSERTSTR?
#include <BBSKernel/Exceptions.h>
#include <BBSKernel/ModelImageConvolutionFunction.h>
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
                              bool aprojection,
                              uInt stationA, uInt stationB)
{
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

  setImageName(name);
  
  image=new PagedImage<Complex>(name);    // load image from disk

  initMaps();
//  initializeToVis();  // this now works directly on the image in the class attributes
}

ModelImageFft::~ModelImageFft(void)
{
  // currently do nothing
  // TODO: Release memory of image?
  delete image;
//  delete itsGridder;
}

// Only do GridFT without w-projection on the visibility grid
//
void ModelImageFft::getUVW( const Matrix<double>& uvw, const Vector<Int>& chanMap, size_t bufSize, 
                            DComplex* xx, DComplex* xy, DComplex* yx, DComplex* yy)
{
  ASSERT(bufSize == uvw.size()*chanMap.size());   // check buffer size

  // Create and fill VBStore object
  casa::VBStore vbs;

  // Set up VisResampler
  itsVisResampler=ModelImageVisibilityResampler::VisibilityResampler(itsConvFunc);
  initPolMap();     // initialize polarization map
  itsVisResampler.setMaps(chanMap, polMap);
  
  Matrix<Double>& sumwt;
  Array<DComplex> griddedData;  // Create array to hold correlation data

  // Loop over uvw timeslots
  for(unsigned int i=0; ;i++)
  {
    // resample baseline uvw from VisibitlityResampler::DataToGrid()
    DataToGrid(griddedData, vbs, sumwt, false);

    // Write 
  }
}

bool ModelImageFft::isLinear() const
{
  if(itsOptions.polarization == LINEAR)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool ModelImageFft::isCircular() const
{
  if(itsOptions.polarization == CIRCULAR)
  {
    return true;
  }
  else
  {
    return false;
  }
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
    muellerMask[0]=1;
    muellerMask[1]=1;
    muellerMask[2]=1;
    muellerMask[3]=1;

    itsOptions.gridMuellerMask=muellerMask;
    itsOptions.degridMuellerMask=muellerMask;
    itsOptions.NThread=1;                       // number of threads used

    itsOptions.polarization=LINEAR;
//    itsOptions.linearPolarized=true;
//    itsOptions.circularPolarized=true;
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

void setGridMuellerMask(const casa::Matrix<Bool> &muellerMask)
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

void setDegridMuellerMask(const casa::Matrix<Bool> &muellerMask)
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

void ModelImageFft::setStoreConvFunctions(bool store)
{
  itsOptions.storeConvFunctions=store;
}

void ModelImageFft::setStations(uInt stationA, uInt stationB)
{
  itsOptions.stationA=stationA;
  itsOptions.stationB=stationB;
}

void ModelImageFft::setFrequencies(const Vector<Double> &frequencies)
{
  itsOptions.frequencies=frequencies;
}

void ModelImageFft::setPolarization(polType polarization)
{
  itsOptions.polarization=polarization;
}

//*********************************************
//
// LofarFtMachine functions re-innovated
//
//*********************************************

void ModelImageFft::computeConvolutionFunctions()
{
/*
  itsConvolutionFunctions.resize(itsOptions.Nwplanes);  // do we really have a conv function per w-plane?
  for(int i=0; i<itsOptions.Nwplanes; i++)
  {
    itsConvolutionFunctions[i]; // call LofarConvolutionFunction constructor here
  }
*/
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
//void ModelImageFft::initializeToVis(casa::ImageInterface<casa::Complex>& iimage)
/*
void ModelImageFft::initializeToVis()
{
  if (itsOptions.verbose > 0) {
    cout<<"---------------------------> initializeToVis"<<endl;
  }
//  image=&iimage;    // we have direct access to the class variable

//  ok();   // don't do this
  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
//  initMaps(vb);
  initMaps();

  // With W-projection or A-projection use visResampler
  if(wprojection()==true || aprojection()==true)
  {
    visResamplers_p.init(useDoubleGrid_p);
    visResamplers_p.setMaps(chanMap_p, polMap_p);
    visResamplers_p.setCFMaps(CFMap_p, ConjCFMap_p);
  }

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
  {
    Vector<Complex> correction(nx);
    correction=Complex(1.0, 0.0);
    // Do the Grid-correction
    IPosition cursorShape(4, nx, 1, 1, 1);
    IPosition axisPath(4, 0, 1, 2, 3);
    LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
    LatticeIterator<Complex> lix(*lattice, lsx);
    for(lix.reset();!lix.atEnd();lix++) {
      gridder->correctX1D(correction, lix.position()(1));
      lix.rwVectorCursor()/=correction;
    }
  }

  LOG_INFO_STR("Finished grid correction of image.");
}
*/

/* This is tailored to LOFAR::BBS::ConvolutionFunction
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
  
  // For (streaming) BBS predict we don't have a MS available
  if(wprojection() || aprojection())
  {
  }

  // If it was requested to save the convolution functions to disk, do it
  if(getStoreConvFunctions())
  {
    // save LofarConvolutionFunctions as paged images on disk
    itsConvFunc->store_all_W_images();
  }

  // Compute the convolution function for all channel, for the polarisations
  // specified in the Mueller_mask matrix
  // Also specify weither to compute the Mueller matrix for the forward or
  // the backward step. A dirty way to calculate the average beam has been
  // implemented, by specifying the beam correcting to the given baseline
  // and timeslot.
  // RETURNS in a LofarCFStore: result[channel][Mueller row][Mueller column]

  // TODO: How to deal with and without A-term here?
  if(itsOptions.Wprojection)
  {
    itsLofarConvFunc = new LOFAR::BBS::LofarConvolutionFunction(padded_shape,
                                              image->coordinates().directionCoordinate (image->coordinates().findCoordinate(Coordinate::DIRECTION)),
                                              itsOptions.refFrequency,
                                              itsOptions.Nwplanes,
                                              itsOptions.wmax,
                                              itsOptions.oversample,
                                              itsOptions.verbose,
                                              itsOptions.maxSupport,
                                              itsOptions.imageName);      
  }
  else if(itsOptions.Aprojection)
  {
  }
  else    // use normal GridFT function
  {
    GridFT *itsGridder=new GridFT( 4000000, 16, itsOptions.ConvType, itsOptions.PhaseDir, 
                                  itsOptions.padding, usezero_p, useDoubleGrid_p);
  }
}
*/

void ModelImageFft::initChannels()
{
    chanMap.resize(nvischan);
    chanMap.set(-1);
/*    
    Vector<Double> lsrFreq(0);
    Bool condoo=False;
    
    
    if(freqFrameValid_p){
      vb.lsrFrequency(spw, lsrFreq, condoo);
      doConversion_p[spw]=condoo;
    }
    else{
      lsrFreq=vb.frequency();
      doConversion_p[spw]=False;
    }
    if(lsrFreq.nelements() ==0){
      return False;
    }
    lsrFreq_p.resize(lsrFreq.nelements());
    lsrFreq_p=lsrFreq;
*/
    Vector<Double> c(1);
    c=0.0;
    Vector<Double> f(1);
    Int nFound=0;   
    

    //cout.precision(10);
    for (Int chan=0;chan<nvischan;chan++) 
    {
//      f(0)=lsrFreq[chan];
      if(spectralCoord_p.toPixel(c, f)) 
      {
        Int pixel=Int(floor(c(0)+0.5));  // round to chan freq at chan center 
        //cout << "spw " << spw << " f " << f(0) << " pixel "<< c(0) << "  " << pixel << endl;
        /////////////
        //c(0)=pixel;
        //spectralCoord_p.toWorld(f, c);
        // cout << "f1 " << f(0) << " pixel "<< c(0) << "  " << pixel << endl;
        ////////////////
        if(pixel>-1&&pixel<nchan)
        {
          chanMap(chan)=pixel;
          nFound++;
          if(nvischan>1&&(chan==0||chan==nvischan-1)) 
          {
            logIO() << LogIO::DEBUGGING
              << "Selected visibility channel : " << chan+1
              << " has frequency "
              <<  MFrequency(Quantity(f(0), "Hz")).get("GHz").getValue()
              << " GHz and maps to image pixel " << pixel+1 << LogIO::POST;
          }
        }
      }
    }
//    multiChanMap_p[spw].resize();
//    multiChanMap_p[spw]=chanMap;
}

void ModelImageFft::fftImage()
{
  LOG_INFO_STR("ModelImageFft::fftImage ffting image");
  LatticeFFT::cfft2d(*lattice);       // Now do the FFT2D in place
}

void storeFFTImage(void)
{
  LOG_INFO_STR("ModelImageFft::storeFFTImage storing ffted image");
}

// LOFAR specific rewritten function initialize polarization (polmap)
// and channel maps (chanmap)
//
//void ModelImageFft::initMaps(const casa::VisBuffer &)
void ModelImageFft::initMaps(void)
{
  LOG_INFO_STR("ModelImageFft::initMaps()");

  // Set the frame for the UVWMachine
  mFrame_p=MeasFrame(MEpoch(Quantity(getTime(), "s")), mLocation_p);
    
  // First get the CoordinateSystem for the image and then find
  // the DirectionCoordinate
  CoordinateSystem coords=image->coordinates();
  Int directionIndex=coords.findCoordinate(Coordinate::DIRECTION);
  AlwaysAssert(directionIndex>=0, AipsError);
  DirectionCoordinate directionCoord=coords.directionCoordinate(directionIndex);

  // get the first position of moving source
  if(fixMovingSource_p)
  {   
    //First convert to HA-DEC or AZEL for parallax correction
    MDirection::Ref outref1(MDirection::AZEL, mFrame_p);
    MDirection tmphadec=MDirection::Convert(movingDir_p, outref1)();
    MDirection::Ref outref(directionCoord.directionType(), mFrame_p);
    firstMovingDir_p=MDirection::Convert(tmphadec, outref)();    
  }

  // Decide if uvwrotation is not necessary, if phasecenter and
  // image center are with in one pixel distance; Save some 
  //  computation time especially for spectral cubes.
  {
    Vector<Double> equal= (mImage_p.getAngle()-
         getPhaseDir().getAngle()).getValue();
    if((abs(equal(0)) < abs(directionCoord.increment()(0))) 
        && (abs(equal(1)) < abs(directionCoord.increment()(1))))
    {
      doUVWRotation_p=False;
    }
    else
    {
      doUVWRotation_p=True;
    }
  }
  // Get the object distance in meters
  Record info(image->miscInfo());
  if(info.isDefined("distance"))
  {
    info.get("distance", distance_p);
    if(abs(distance_p)>0.0)
    {
      LOG_INFO_STR("Distance to object is set to " << distance_p/1000.0
      << "km: applying focus correction");
    }
  }
  
  // Set up the UVWMachine. 
  uvwMachine_p=new UVWMachine(mImage_p, getPhaseDir(), mFrame_p, False, True);
  AlwaysAssert(uvwMachine_p, AipsError);
  
// TODO: do we need this?
//    lastFieldId_p=vb.fieldId();
//    lastMSId_p=vb.msId();

  // Now we need MDirection of the image phase center. This is
  // what we define it to be. So we define it to be the
  // center pixel. So we have to do the conversion here.
  // This is independent of padding since we just want to know 
  // what the world coordinates are for the phase center
  // pixel
  {
    Vector<Double> pixelPhaseCenter(2);
    pixelPhaseCenter(0) = Double( image->shape()(0) / 2 );
    pixelPhaseCenter(1) = Double( image->shape()(1) / 2 );
    directionCoord.toWorld(mImage_p, pixelPhaseCenter);
  }

  // Set up maps
  Int spectralIndex=coords.findCoordinate(Coordinate::SPECTRAL);
  AlwaysAssert(spectralIndex>-1, AipsError);
  spectralCoord_p=coords.spectralCoordinate(spectralIndex);
  
  //Store the image/grid channels freq values
  {
    Int chanNumbre=image->shape()(3);
    Vector<Double> pixindex(chanNumbre);
    imageFreq_p.resize(chanNumbre);
    Vector<Double> tempStorFreq(chanNumbre);
    indgen(pixindex);
    //    pixindex=pixindex+1.0; 
    for (Int ll=0; ll< chanNumbre; ++ll)
    {
      if( !spectralCoord_p.toWorld(tempStorFreq(ll), pixindex(ll)))
      {
        logIO() << "Cannot get imageFreq " << LogIO::EXCEPTION;  
      }
    }
    convertArray(imageFreq_p,tempStorFreq);
  }
  
  //Destroy any conversion layer Freq coord if freqframe is not valid
  if(!freqFrameValid_p)
  {
    MFrequency::Types imageFreqType=spectralCoord_p.frequencySystem();
    spectralCoord_p.setFrequencySystem(imageFreqType);   
    spectralCoord_p.setReferenceConversion(imageFreqType, 
             MEpoch(Quantity(getTime(), "s")),
             mLocation_p,
             mImage_p);
  }
  
  // Channel map: do this properly by looking up the frequencies
  // If a visibility channel does not map onto an image
  // pixel then we set the corresponding chanMap to -1.
  // This means that put and get must always check for this
  // value (see e.g. GridFT)
  
  nvischan  = itsOptions.frequencies.size();
  interpVisFreq_p.resize();
  interpVisFreq_p=getFrequencies();
/*
  if(selectedSpw_p.nelements() < 1){
//    Vector<Int> myspw(1);
//      myspw[0]=vb.spectralWindow();
//      setSpw(myspw, freqFrameValid_p);
  }
*/
//    matchAllSpwChans(vb);
  /*
  chanMap.resize();
  
  //  cout << "VBSPW " << vb.spectralWindow() << "  " << multiChanMap_p[vb.spectralWindow()] << endl;
  chanMap=multiChanMap_p[vb.spectralWindow()];
  */
  
  initChannels();
  if(chanMap.nelements() == 0)
    chanMap=Vector<Int>(itsOptions.frequencies.size(), -1);
  {
    LOG_DEBUG_STR("Channel Map: " << chanMap);
  }
  // Should never get here
  if(max(chanMap)>=nchan||min(chanMap)<-1) {
    LOG_DEBUG_STR("Illegal Channel Map: " << chanMap);
  }

  multiChanMap_p[0]=chanMap;

  //  logIO() << LogOrigin("LofarFTMachine", "init")  << LogIO::NORMAL;
  LOG_INFO_STR("ModelImagefft::initMaps() done");
}

// Initialise polarization info
//
void ModelImageFft::initPolMap()
{
  // get correlation types from our own dummy function
  ;

/*
  void FTMachine::initPolInfo(const VisBuffer& vb)
  {
    //
    // Need to figure out where to compute the following arrays/ints
    // in the re-factored code.
    // ----------------------------------------------------------------
    {
      polInUse_p = 0;
      uInt N=0;
      for(uInt i=0;i<polMap.nelements();i++) if (polMap(i) > -1) polInUse_p++;
      cfStokes_p.resize(polInUse_p);
      for(uInt i=0;i<polMap.nelements();i++) 
	if (polMap(i) > -1) {cfStokes_p(N) = vb.corrType()(i);N++;}
    }
  }
*/


}

// Set up correlations vector depending on linear or circular polarization
//
Vector<Int> ModelImageFft::getCorrType()
{
  Vector<Int> corrType(4);

  if(isLinear())
  {
    corrType[0]=Stokes::XX;
    corrType[1]=Stokes::XY;
    corrType[2]=Stokes::YX;
    corrType[3]=Stokes::YY;
  }
  else
  {
    corrType[0]=Stokes::RR;
    corrType[1]=Stokes::RL;
    corrType[2]=Stokes::LR;
    corrType[3]=Stokes::LL;  
  }
  return corrType;
}

// Check that input model image has Jy/pixel flux
//
bool ModelImageFft::validModelImage(const casa::String &imageName, string &error)
{
  size_t pos=string::npos;
  bool valid=false;
  bool validName=false, validUnit=false;

  if((pos=imageName.find(".model")) != string::npos)   // Check filename extension
  {
    validName=true;
  }
  else if((pos=imageName.find(".img")) != string::npos)
  {
    error="Image filename ";
    error.append(imageName).append(" must have .model extension.");
    validName=false;    
  }
  else if((pos=imageName.find(".image")) != string::npos)
  {
    error="Image filename ";
    error.append(imageName).append(" must have .model extension.");
    validName=false;
  }
  else    // no extension is acceptable
  {
    validName=true;     // can we accept no extension?
  }

  // Look for Jy/beam entry in file table keywords "unit"
  Table image(imageName);                                       // open image-table as read-only
  const TableRecord &imageKeywords(image.keywordSet());
  RecordFieldId unitsId("units");
  string units=imageKeywords.asString(unitsId);
  if(units=="Jy/pixel")
  {
    validUnit=true;
  }
  else
  {
    error="Image ";
    error.append(imageName).append(" must have flux unit Jy/pixel.");
    validUnit=false;
  }

  if(validName && validUnit)    // Determine final validity
  {
    valid=true;
  }
  else
  {
    valid=false;
  }
  return valid;
}

// Get the patch direction, i.e. RA/Dec of the central image pixel
//
casa::MDirection ModelImageFft::getPatchDirection(const ImageInterface<Complex> &image)
{
  casa::IPosition imageShape;                             // shape of image
  casa::Vector<casa::Double> Pixel(2);                    // pixel coords vector of image centre
  casa::MDirection MDirWorld(casa::MDirection::J2000);   // astronomical direction in J2000
//  casa::PagedImage<casa::Float> image(patchName);         // open image
    
  imageShape=image.shape();                               // get centre pixel
  Pixel[0]=floor(imageShape[0]/2);
  Pixel[1]=floor(imageShape[1]/2);

  // Determine DirectionCoordinate
  casa::DirectionCoordinate dir(image.coordinates().directionCoordinate (image.coordinates().findCoordinate(casa::Coordinate::DIRECTION)));
  dir.toWorld(MDirWorld, Pixel);

  return MDirWorld;
}