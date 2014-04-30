//# LofarFTMachineSplitBeam.cc: Gridder for LOFAR data correcting for DD effects
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

#include <LofarFT/FTMachineSplitBeam.h>

#include <time.h>

#include <Common/OpenMP.h>
#include <omp.h>

#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicMath/Random.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/PrecTimer.h>
#include <casa/OS/HostInfo.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <casa/sstream.h>
#include <casa/Utilities/Assert.h>
#include <casa/Utilities/CompositeNumber.h>

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
#include <measures/Measures/Stokes.h>
#include <measures/Measures/UVWMachine.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/RigidVector.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/CFStore.h>
#include <synthesis/MSVis/VisibilityIterator.h>
#include <synthesis/MSVis/StokesVector.h>
#include <synthesis/TransformMachines/StokesImageUtil.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/MSVis/VisSet.h>

#include <LofarFT/FFTCMatrix.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarVBStore.h>

#define DORES True

using namespace casa;

namespace 
{
  void sumGridsOMP(casa::Array<Complex>& grid, const vector< casa::Array<Complex> >& gridToAdd0 );
  void sumGridsOMP(casa::Array<DComplex>& grid, const vector< casa::Array<DComplex> >& gridToAdd0 );
  void sumGridsOMP(casa::Array<Complex>& grid, const casa::Array<Complex>& gridToAdd);
  void sumGridsOMP(casa::Array<DComplex>& grid, const casa::Array<DComplex>& gridToAdd);
}

namespace LOFAR { //# NAMESPACE LOFAR - BEGIN

FTMachineWsplit::FTMachineWsplit(
  Long icachesize, 
  Int itilesize,
  CountedPtr<VisibilityResamplerBase>&,
  String iconvType,
  const MeasurementSet& ms, 
  Int nwPlanes,
  MPosition mLocation, 
  Float padding, 
  Bool usezero,
  Bool useDoublePrec, 
  double wmax,
  Int verbose,
  Int maxsupport, 
  Int oversample,
  const String& imgName,
  const Matrix<bool>& gridMuellerMask,
  const Matrix<bool>& degridMuellerMask,
  Double refFreq,
  Bool useLinearInterpGridder, 
  Bool useEJones, 
  int stepApplyElement, 
  int applyBeamCode,
  Double pbCut, 
  Bool predictFT, 
  String psfOnDisk, 
  Bool useMasksDegrid,
  Bool reallyDoPSF,
  Double uvMin,
  Double uvMax,
  Bool makeDirtyCorr,
  const Record& parameters,
  LOFAR::ParameterSet& parset)//,
  : FTMachine(), 
    padding_p(padding), 
    imageCache(0), 
    cachesize(icachesize),
    tilesize(itilesize), 
    gridder(0), 
    isTiled(False), 
    convType(iconvType),
    maxAbsData(0.0), 
    centerLoc(IPosition(4,0)),
    offsetLoc(IPosition(4,0)), 
    usezero_p(usezero), 
    noPadding_p(False),
    usePut2_p(False), 
    machineName_p("LofarFTMachineWsplit"), 
    itsParameters(parameters), 
    itsMS(ms),
    itsNWPlanes(nwPlanes), 
    itsWMax(wmax), 
    itsConvFunc(0),
    itsVerbose(verbose),
    itsMaxSupport(maxsupport), 
    itsOversample(oversample), 
    itsImgName(imgName),
    itsGridMuellerMask(gridMuellerMask),
    itsDegridMuellerMask(degridMuellerMask),
    itsGriddingTime(0), 
    itsDegriddingTime(0), 
    itsCFTime(0), 
    thisterm_p(0)
{
  cout << "=======LofarFTMachineWSplit==============================" << endl;
  cout << itsParameters << endl;
  cout << "=========================================================" << endl;

  logIO() << LogOrigin(this-name(), this-name())  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mLocation_p=mLocation;
  tangentSpecified_p = False;
  useDoubleGrid_p = useDoublePrec;
  canComputeResiduals_p=DORES;
  itsNThread = OpenMP::maxThreads();
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedData=0;//->resize (itsNThread);
  itsGriddedData2=0;//.resize (itsNThread);
  itsSumPB.resize (itsNThread);
  itsSumCFWeight.resize (itsNThread);
  itsSumWeight.resize (itsNThread);
  itsDonePB=false;
  itsStackMuellerNew.resize(itsNThread);
  itsNumCycle=0;
  itsMakeDirtyCorr=MakeDirtyCorr;
  itsTimeWindow=itsParameters.asDouble("timewindow");
  its_t0 = -1.;
  its_tSel0 = itsParameters.asDouble("t0");
  its_tSel1 = itsParameters.asDouble("t1");

  itsRefFreq=RefFreq;
  itsNamePsfOnDisk=PsfOnDisk;
  its_Use_Linear_Interp_Gridder=Use_Linear_Interp_Gridder;
  its_Use_EJones=Use_EJones;
  its_UseMasksDegrid=UseMasksDegrid;
  its_PBCut=PBCut;
  its_reallyDoPSF=reallyDoPSF;
  //its_FillFactor=FillFactor;
  itsStepApplyElement=StepApplyElement;
  its_Apply_Element=false;
  its_ApplyBeamCode=ApplyBeamCode;
  itsPredictFT=PredictFT;
  itsUVmin=UVmin;
  itsUVmax=UVmax;
  itsSeconds.mark();
  itsSupport_Speroidal = FFTCMatrix::optimalOddFFTSize (itsParameters.asDouble("SpheSupport"));
  
  its_tot_time_grid=0.;
  its_tot_time_cf=0.;
  its_tot_time_w=0.;
  its_tot_time_el=0.;
  its_tot_time_tot=0;


  if(itsStepApplyElement>0){its_Apply_Element=true;}

  // if(its_Use_Linear_Interp_Gridder){
  //   cout<<"Gridding using oversampling of 1 only"<<endl;
  //   itsOversample=1;
  // };
  //cout<<"FTMahin: itsRefFreq "<<itsRefFreq<<endl;

  ROMSSpWindowColumns window(ms.spectralWindow());
  itsListFreq.resize(window.nrow());
  for(uInt i=0; i<window.nrow();++i){
    itsListFreq[i]=window.refFrequency()(i);
    cout<<"SPW="<<i<<", freq="<<itsListFreq[i]<<endl;
    if (itsVerbose > 0) {
      for(uInt j=0; j<(window.chanFreq()(i)).shape()[0];++j){
        cout<<"chan"<<(window.chanFreq()(i))[j]<<endl;
      }
    }
  }
  its_Already_Initialized=false;
}


//----------------------------------------------------------------------
FTMachineWsplit& FTMachineWsplit::operator=(const FTMachineWsplit& other)
{
  if(this!=&other) {
    //Do the base parameters
    FTMachine::operator=(other);

    //private params
    imageCache=other.imageCache;
    cachesize=other.cachesize;
    tilesize=other.tilesize;
    convType=other.convType;
    uvScale.resize();
    uvOffset.resize();
    uvScale=other.uvScale;
    uvOffset=other.uvOffset;
    if(other.gridder==0)
    {
      gridder=0;
    }
    else
    {
      gridder = new ConvolveGridder<Double, Complex>(
        IPosition(2, nx, ny),
        uvScale, 
        uvOffset,
        convType);
    }
    isTiled=other.isTiled;
    //lattice=other.lattice;
    lattice=0;
    tilesize=other.tilesize;
    arrayLattice=0;
    maxAbsData=other.maxAbsData;
    centerLoc=other.centerLoc;
    offsetLoc=other.offsetLoc;
    padding_p=other.padding_p;
    usezero_p=other.usezero_p;
    noPadding_p=other.noPadding_p;
    itsMS = other.itsMS;
    itsNWPlanes = other.itsNWPlanes;
    itsWMax = other.itsWMax;
    itsConvFunc = other.itsConvFunc;
    itsRefFreq=other.itsRefFreq;
    itsTaylorTerm = other.itsTaylorTerm;
    itsUseLinearInterpGridder= other.its_Use_Linear_Interp_Gridder;
    itsUse_EJones = other.its_Use_EJones;
    itsUseMasksDegrid = other.its_UseMasksDegrid;
    itsApply_Element = other.its_Apply_Element;
    itsApplyBeamCode = other.its_ApplyBeamCode;
    itsStepApplyElement = other.itsStepApplyElement;
    itsInitialized= other.its_Already_Initialized;
    itsReallyDoPSF = other.its_reallyDoPSF;
    its_PBCut= other.its_PBCut;
    itsStackMuellerNew=other.itsStackMuellerNew;
    itsDonePB=other.itsDonePB;
    itsUVmin=other.itsUVmin;
    itsUVmax=other.itsUVmax;
    itsListFreq=other.itsListFreq;
    itsNumCycle=other.itsNumCycle;
    itsMakeDirtyCorr=other.its_makeDirtyCorr;
    its_t0=other.its_t0;
    its_tSel0=other.its_tSel0;
    its_tSel1=other.its_tSel1;
    its_tot_time_grid =other.its_tot_time_grid ;
    its_tot_time_cf   =other.its_tot_time_cf   ;
    its_tot_time_w    =other.its_tot_time_w    ;
    its_tot_time_el   =other.its_tot_time_el   ;
    its_tot_time_tot  =other.its_tot_time_tot ;

    its_TimeWindow=other.its_TimeWindow;

    ConjCFMap_p = other.ConjCFMap_p;
    CFMap_p = other.CFMap_p;
    itsNThread = other.itsNThread;
    itsGriddedData=other.itsGriddedData;
    itsGriddedData2=other.itsGriddedData2;
    itsSumPB.resize (itsNThread);
    itsSumCFWeight.resize (itsNThread);
    itsSumWeight.resize (itsNThread);
    itsVerbose = other.itsVerbose;
    itsMaxSupport = other.itsMaxSupport;
    itsOversample = other.itsOversample;
    itsPredictFT = other.itsPredictFT;
    itsImgName = other.itsImgName;
    itsGridMuellerMask = other.itsGridMuellerMask;
    itsDegridMuellerMask = other.itsDegridMuellerMask;
    itsGriddingTime = other.itsGriddingTime;
    itsDegriddingTime = other.itsDegriddingTime;
    itsCFTime = other.itsCFTime;
    itsParameters = other.itsParameters;
    itsSupportSperoidal = other.itsSupportSperoidal;
  }
  return *this;
}

//----------------------------------------------------------------------
  FTMachineWsplit::FTMachineWsplit(const FTMachineWsplit& other) : FTMachine(), machineName_p("LofarFTMachineWsplit")
  {
    operator=(other);
  }

  FTMachineWsplit* LofarFTMachineWsplit::clone() const
  {
    FTMachineWsplit* newftm = new FTMachineWsplit(*this);
    return newftm;
  }

//----------------------------------------------------------------------
void FTMachineWsplit::init() {

  logIO() << LogOrigin(this-name(), "init")  << LogIO::NORMAL;
  canComputeResiduals_p = DORES;
  ok();

  isTiled=False;
  nx = image->shape()(0);
  ny = image->shape()(1);
  if (!noPadding_p) {
    // Make sure padding is always even.
    nx += int((padding_p-1)*nx/2.)*2;
    ny += int((padding_p-1)*ny/2.)*2;
  }
  npol  = image->shape()(2);
  nchan = image->shape()(3);

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
  gridder = new ConvolveGridder<Double, Complex>(
    IPosition(2, nx, ny),
    uvScale, 
    uvOffset,
    convType);

  // Setup the CFStore object to carry relavent info. of the Conv. Func.
  cfs_p.xSupport = gridder->cSupport();
  cfs_p.ySupport = gridder->cSupport();
  cfs_p.sampling.resize(2);
  cfs_p.sampling = gridder->cSampling();
  if (cfs_p.rdata.null()) cfs_p.rdata = new Array<Double>(gridder->cFunction());

  padded_shape = image->shape();
  padded_shape(0) = nx;
  padded_shape(1) = ny;
  cout << "Original shape " << image->shape()(0) << ","
        << image->shape()(1) << endl;
  cout << "Padded shape " << padded_shape(0) << ","
        << padded_shape(1) << endl;
  fftw_init_threads();
  fftw_plan_with_nthreads(1);
  itsConvFunc = new ConvolutionFunction(
    padded_shape,
    image->shape(),
    image->coordinates().directionCoordinate 
    (image->coordinates().findCoordinate(Coordinate::DIRECTION)),
    itsMS, 
    itsNWPlanes, 
    itsWMax,
    itsOversample,
    itsVerbose, 
    itsMaxSupport,
    itsImgName+String::toString(thisterm_p),
    its_Use_EJones,
    its_Apply_Element,
    its_ApplyBeamCode,
    itsParameters,
    itsStackMuellerNew);
  
  itsNWPlanes = itsConvFunc->m_nWPlanes;

  VecFFTMachine.resize(image->shape()(2));
  Matrix<Complex> dummy(IPosition(2,nx,nx),0.);
  cout<<" initialise plans"<<endl;
  for(Int pol=0; pol<npol; ++pol)
  {
    VecFFTMachine[pol].normalized_forward(
      dummy.nrow(),
      dummy.data(),
      OpenMP::maxThreads()/npol,
      FFTW_MEASURE);
    VecFFTMachine[pol].normalized_backward(
      dummy.nrow(),
      dummy.data(),
      OpenMP::maxThreads()/npol , 
      FFTW_MEASURE);
  }
  cout<<" done..."<<endl;

  itsMasksGridAllDone = false;
  itsAllAtermsDone = false;
  // Set up image cache needed for gridding. For BOX-car convolution
  // we can use non-overlapped tiles. Otherwise we need to use
  // overlapped tiles and additive gridding so that only increments
  // to a tile are written.
  itsInitialized=true;

  if(imageCache) delete imageCache; imageCache=0;

  if(isTiled) 
  {
    Float tileOverlap=0.5;
    if(convType=="box") 
    {
      tileOverlap=0.0;
    }
    else 
    {
      tileOverlap=0.5;
      tilesize=max(12,tilesize);
    }
    IPosition tileShape=IPosition(4,tilesize,tilesize,npol,nchan);
    Vector<Float> tileOverlapVec(4);
    tileOverlapVec=0.0;
    tileOverlapVec(0)=tileOverlap;
    tileOverlapVec(1)=tileOverlap;
    Int tmpCacheVal=static_cast<Int>(cachesize);
    imageCache = new LatticeCache <Complex> (
      *image, tmpCacheVal, tileShape,
      tileOverlapVec,
      (tileOverlap>0.0));
  }
  itsCyrilTimer.start();
  itsTStartObs=1.e30;
  itsDeltaTime=0.;
  itsNextApplyTime=0.;;
  itsCounterTimes=0;
}

// This is nasty, we should use CountedPointers here.
FTMachineWsplit::~FTMachineWsplit()
{
  if(imageCache) delete imageCache; imageCache=0;
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  if(gridder) delete gridder; gridder=0;
//  delete itsConvFunc;
}

const Matrix<Float>& FTMachineWsplit::getAveragePB() const
{
  if (itsAvgPB.empty()) 
  {
    PagedImage<Float> pim(itsImgName+String::toString(thisterm_p) + ".avgpb");
    Array<Float> arr = pim.get();
    itsAvgPB.reference (arr.nonDegenerate(2));
  }
  return itsAvgPB;
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image
void FTMachineWsplit::initializeToVis(
  ImageInterface<Complex>& iimage,
  const VisBuffer& vb)
{
  image = &iimage;

  ok();
  if(!itsInitialized) init();
  
  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);

  visResamplers_p.init(useDoubleGrid_p);
  visResamplers_p.setMaps(chanMap, polMap);
  visResamplers_p.setCFMaps(CFMap_p, ConjCFMap_p);

  // Need to reset nx, ny for padding
  // Padding is possible only for non-tiled processing

  // If we are memory-based then read the image in and create an
  // ArrayLattice otherwise just use the PagedImage
  AlwaysAssert (!isTiled, AipsError);

  IPosition gridShape(4, nx, ny, npol, nchan);
  // Size and initialize the grid buffer per thread.
  // Note the other (*itsGriddedData) buffers are assigned later.
  // (*itsGriddedData)[0].resize (gridShape);
  // (*itsGriddedData)[0] = Complex();

  for (int i=0; i<itsNThread; ++i) 
  {
    itsSumPB[i].resize (int(itsSupport_Speroidal), int(itsSupport_Speroidal));
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize(npol, nchan);
    itsSumWeight[i] = 0.;
  }
  
  if(!useDoubleGrid_p)
  {
    for (uInt i=0; i<itsNThread; ++i) {
      (*itsGriddedData)[i].resize (gridShape);
      (*itsGriddedData)[i] = Complex();
    }
    its_stacked_GriddedData.resize (gridShape);
    its_stacked_GriddedData = Complex();
  } 
  else 
  {
    for (uInt i=0; i<itsNThread; ++i) 
    {
      (*itsGriddedData2)[i].resize (gridShape);
      (*itsGriddedData2)[i] = DComplex();
    }
    its_stacked_GriddedData2.resize (gridShape);
    its_stacked_GriddedData2 = DComplex();
  }

  itsCounterTimes=0;
  itsTStartObs=1.e30;
  itsDeltaTime=0.;
  itsTotalStepsGrid=0;
  itsTotalStepsDeGrid=0;

  //griddedData can be a reference of image data...if not using model col
  //hence using an undocumented feature of resize that if
  //the size is the same as old data it is not changed.
  //if(!usePut2_p) griddedData.set(0);

  IPosition stride(4, 1);
  IPosition blc(4, (nx-image->shape()(0) + (nx%2==0)) / 2, (ny-image->shape()(1) + (ny%2==0))/2, 0, 0);
  IPosition trc(blc+image->shape()-stride);
  if (itsVerbose > 0) 
  {
    cout<<"LofarFTMachineWsplit::initializeToVis === blc,trc,nx,ny,image->shape()"
        <<blc<<" "<<trc<<" "<<nx<<" "<<ny<<" "<<image->shape()<<endl;
  }
  IPosition start(4, 0);
  its_stacked_GriddedData(blc, trc) = image->getSlice(start, image->shape());

  arrayLattice = new ArrayLattice<Complex>(its_stacked_GriddedData);
  lattice=arrayLattice;


  logIO() << LogIO::DEBUGGING
	  << "Starting grid correction and FFT of image" << LogIO::POST;

  const Matrix<Float>& data = getAveragePB();


  uInt shapeout(floor(lattice->shape()[0]/padding_p));
  //uInt istart(floor((lattice->shape()[0]-shapeout)/2.));

  const Matrix<Float>& sphe = getSpheroidCut();

  uInt istart(floor((lattice->shape()[0]-shapeout)/2.));

  IPosition pos(4,1,1,1,1);
  double fact(1.);
  Complex pix;
  uInt jj;
  for(uInt ch=0; ch<its_stacked_GriddedData.shape()[3]; ++ch)
  {
    #pragma omp parallel
    #pragma omp for private(pos,fact,jj,pix) schedule(dynamic)
    for(uInt ii=0; ii<shapeout; ++ii)
    {
      pos[0]=ii+istart;
      for(jj=0; jj<shapeout; ++jj)
      {
        pos[1]=jj+istart;
        pos[3]=ch;
        Double prod(1.);
        if (sphe(pos)<its_PBCut) 
        {
          prod=0.;
        }
        else
        {
          if(!itsPredictFT)
          {
//             prod*=1./sqrt(data(istart+ii,istart+jj));
            prod/=sphe(istart+ii,istart+jj);
            if (itsApplyElement)
            {
              prod /= sphe(istart+ii,istart+jj);
            }
          }
          else 
          {
            prod /= sphe(istart+ii,istart+jj);
            if (itsApplyElement) 
            {
              prod /= sphe(istart+ii,istart+jj);
            }
          }
        }
        for(uInt pol=0; pol<its_stacked_GriddedData.shape()[2]; ++pol)
        {
          pos[2] = pol;
          pix = its_stacked_GriddedData(pos);
          pix *= Complex(prod);
          itsStackedGriddedData(pos) = pix;
        }
      }
    }
  }
  itsNumCycle+=1;
  logIO() << LogIO::DEBUGGING << "Finished grid correction and FFT of image" << LogIO::POST;
}

void FTMachineWsplit::finalizeToVis()
{
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToVis"<<endl;
  }
  cout<<"---------------------------> finalizeToVis"<<endl;
  itsAllAtermsDone = true;
}


// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid.
void FTMachineWsplit::initializeToSky(
  ImageInterface<Complex>& iimage,
  Matrix<Float>& weight, 
  const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;
  if(!itsAlreadyInitialized){init();};

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);

  visResamplers_p.init(useDoubleGrid_p);
  visResamplers_p.setMaps(chanMap, polMap);
  visResamplers_p.setCFMaps(CFMap_p, ConjCFMap_p);

  // Initialize for in memory or to disk gridding. lattice will
  // point to the appropriate Lattice, either the ArrayLattice for
  // in memory gridding or to the image for to disk gridding.
  AlwaysAssert (!isTiled, AipsError);
  IPosition gridShape(4, nx, ny, npol, nchan);
  // Size and initialize the grid buffer per thread.
  its_stacked_GriddedData.resize (gridShape);
  its_stacked_GriddedData = Complex();

  if(!useDoubleGrid_p)
  {
    for (uInt i=0; i<itsNThread; ++i) 
    {
      (*itsGriddedData)[i].resize (gridShape);
      (*itsGriddedData)[i] = Complex();
    }
  } 
  else 
  {
    for (uInt i=0; i<itsNThread; ++i) 
    {
      (*itsGriddedData2)[i].resize (gridShape);
      (*itsGriddedData2)[i] = DComplex();
    }
  }

  for (int i=0; i<itsNThread; ++i) 
  {
    itsSumPB[i].resize (int(itsSupport_Speroidal), int(itsSupport_Speroidal));
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize (npol, nchan);
    itsSumWeight[i] = 0.;
  }

  weight.resize(itsSumWeight[0].shape());
  weight=0.0;
  itsCounterTimes=0;
  itsTStartObs=1.e30;
  itsDeltaTime=0.;
  itsTotalStepsGrid=0;
  itsTotalStepsDeGrid=0;

  arrayLattice = new ArrayLattice<Complex>(its_stacked_GriddedData);
  lattice=arrayLattice;
}

void FTMachineWsplit::finalizeToSky()
{
  //AlwaysAssert(lattice, AipsError);
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToSky"<<endl;
  }

  itsMasksGridAllDone=true;
  itsAllAtermsDone=true;

  if(!its_Apply_Element){
    SumGridsOMP(its_stacked_GriddedData, (*itsGriddedData));
    for (uInt i=0; i<itsNThread; ++i) {
      (*itsGriddedData)[i]=Complex();
    }
  }


  for (int i=1; i<itsNThread; ++i) {
    //(*itsGriddedData)[0] += (*itsGriddedData)[i];
    itsSumWeight[0]   += itsSumWeight[i];
    itsSumCFWeight[0] += itsSumCFWeight[i];
    itsSumPB[0]       += itsSumPB[i];
  }
}

Array<Complex>* FTMachineWsplit::getDataPointer(
  const IPosition& centerLoc2D,
  Bool readonly) 
{
  Array<Complex>* result;
  // Is tiled: get tiles and set up offsets
  centerLoc(0)=centerLoc2D(0);
  centerLoc(1)=centerLoc2D(1);
  result=&imageCache->tile(offsetLoc,centerLoc, readonly);
  gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
  return result;
}


// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& LofarFTMachine::getImage(Matrix<Float>& weights, Bool normalize)
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(gridder, AipsError);
  AlwaysAssert(image, AipsError);
  logIO() << LogOrigin(this-name(), "getImage") << LogIO::NORMAL;

  //cout<<"weights.shape() "<<weights.shape()<<"  "<<sumWeight<<endl;

  weights.resize(itsSumWeight[0].shape());

  convertArray(weights, itsSumWeight[0]);
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(normalize&&max(weights)==0.0) {
    logIO() << LogIO::SEVERE << "No useful data in FTMachine: weights all zero"
	    << LogIO::POST;
  }
  else {

    const IPosition latticeShape = lattice->shape();

    logIO() << LogIO::DEBUGGING
	    << "Starting FFT and scaling of image" << LogIO::POST;



    // if(useDoubleGrid_p){
    //   convertArray(griddedData, griddedData2);
    //   //Don't need the double-prec grid anymore...
    //   griddedData2.resize();
    // }

    // x and y transforms
    //    LatticeFFT::cfft2d(*lattice,False);
    //
    // Retain the double precision grid for FFT as well.  Convert it
    // to single precision just after (since images are still single
    // precision).
    //


    IPosition gridMaskShape(2,its_stacked_GriddedData.shape()[0],its_stacked_GriddedData.shape()[1]);
    itsMaskGridCS.resize(gridMaskShape);
    itsMaskGridCS=false;
    IPosition posmask(4,1,1,1,1);
    posmask[2]=0;
    posmask[3]=0;
    for(uInt i=0;i<its_stacked_GriddedData.shape()[0];++i){
      for(uInt j=0;j<its_stacked_GriddedData.shape()[1];++j){
	posmask[0]=i;
	posmask[1]=j;
	if(its_stacked_GriddedData(posmask)!=Complex(0.)){itsMaskGridCS(i,j)=true;}
      }
    }

    // if(useDoubleGrid_p) {
    //   ArrayLattice<DComplex> darrayLattice(itsGriddedData2[0]);
    //   LatticeFFT::cfft2d(darrayLattice,False);
    //   convertArray((*itsGriddedData)[0], itsGriddedData2[0]);
    //   //Don't need the double-prec grid anymore...
    //   ///griddedData2.resize();
    // } else {
    //LatticeFFT::cfft2d(*lattice, False);
    // } 

#pragma omp parallel
  {
#pragma omp for
    for(Int pol=0; pol<npol; ++pol){
      Complex* ptr = its_stacked_GriddedData.data()+pol*nx*nx;
      Matrix<Complex> arr(IPosition(2,nx,nx), ptr, SHARE);
      dofftVec(arr,false,itsNThread/npol,pol);
    }
  }
  fftw_plan_with_nthreads(1);
  //   for(uInt pol=0; pol<npol; ++pol){
  //     Complex* ptr = its_stacked_GriddedData.data()+pol*nx*nx;
  //     Matrix<Complex> arr(IPosition(2,nx,nx), ptr, SHARE);
  //     dofft(arr,false);
  //   }
  // fftw_plan_with_nthreads(1);


  itsAvgPB.reference (itsConvFunc->Compute_avg_pb(itsSumPB[0], itsSumCFWeight[0]));

  // Pol correction
  if (its_Apply_Element&its_makeDirtyCorr){
    if(itsDonePB==false){
      itsConvFunc->Make_MuellerAvgPB(itsStackMuellerNew, itsSumCFWeight[0]);
    }
    its_stacked_GriddedData=itsConvFunc->Correct_CC(its_stacked_GriddedData);
  }
  // End Pol correction

  // arrayLattice = new ArrayLattice<Complex>(its_stacked_GriddedData);
  // lattice=arrayLattice;
  itsDonePB=true;


    if (itsVerbose > 0) {
      cout<<"POLMAP:::::::  "<<polMap<<endl;
      cout<<"POLMAP:::::::  "<<CFMap_p<<endl;
    }
    //cout<<"CFPOLMAP:::::::  "<<cfPolMap<<endl;
    //Int i,j,N = cfPolMap.nelements();
    //for(i=0;i<N;i++){
    //  if (cfPolMap[i] > -1){cout<<"cfPolMap[i]<<visStokes[i]"<<cfPolMap[i]<<" "<<visStokes[i]<<endl;};
    //};


    uInt shapeout(floor(lattice->shape()[0]/padding_p));
    uInt istart(floor((lattice->shape()[0]-shapeout)/2.));

    const Matrix<Float>& sphe = getSpheroidCut();

    IPosition pos(4,1,1,1,1);
    Complex pix;
    uInt jj;
      for(uInt ch=0; ch<its_stacked_GriddedData.shape()[3]; ++ch){
      #pragma omp parallel
    {
      #pragma omp for private(pos,pix,jj) schedule(dynamic)
      	for(uInt ii=0; ii<shapeout; ++ii){
      	  pos[0]=ii+istart;
      	  for(jj=0; jj<shapeout; ++jj){
      	    pos[1]=jj+istart;
	    pos[3]=ch;
	    Double prod(1.);
	    if((sphe(ii+istart,jj+istart)<its_PBCut)|(sqrt(itsAvgPB(ii+istart,jj+istart))<its_PBCut)){prod=0.;}
	    else{
	      prod/=sqrt(itsAvgPB(ii+istart,jj+istart));
	      if(its_Apply_Element){prod/=sphe(ii+istart,jj+istart);}
	      prod/=sphe(ii+istart,jj+istart);
	    }
	    
	    for(uInt pol=0; pol<its_stacked_GriddedData.shape()[2]; ++pol){
	      pos[2]=pol;
	      pix=its_stacked_GriddedData(pos);
	      pix*=prod;
	      its_stacked_GriddedData(pos)=pix;
	    }
	  }
      	}
    }
      }
      

    if(!isTiled) {
      // Check the section from the image BEFORE converting to a lattice
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      // Do the copy
      IPosition start(4, 0);
      image->put(its_stacked_GriddedData(blc, trc));
    }
  }

  //store(*image,"last.img");
  return *image;
}

// Get weight image
void LofarFTMachine::getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights)
{

  logIO() << LogOrigin(this-name(), "getWeightImage") << LogIO::NORMAL;

  weights.resize(itsSumWeight[0].shape());
  convertArray(weights,itsSumWeight[0]);

  const IPosition latticeShape = weightImage.shape();

  Int nx=latticeShape(0);
  Int ny=latticeShape(1);

  IPosition loc(2, 0);
  IPosition cursorShape(4, nx, ny, 1, 1);
  IPosition axisPath(4, 0, 1, 2, 3);
  LatticeStepper lsx(latticeShape, cursorShape, axisPath);
  LatticeIterator<Float> lix(weightImage, lsx);
  for(lix.reset();!lix.atEnd();lix++) {
    Int pol=lix.position()(2);
    Int chan=lix.position()(3);
    lix.rwCursor()=weights(pol,chan);
  }
}

Bool LofarFTMachine::toRecord(String& error, RecordInterface& outRec,
			Bool withImage) {

  // Save the current LofarFTMachine object to an output state record
  Bool retval = True;

  Double cacheVal=(Double)cachesize;
  outRec.define("cache", cacheVal);
  outRec.define("tile", tilesize);
  outRec.define("gridfunction", convType);

  Vector<Double> phaseValue(2);
  String phaseUnit;
  phaseValue=mTangent_p.getAngle().getValue();
  phaseUnit= mTangent_p.getAngle().getUnit();
  outRec.define("phasevalue", phaseValue);
  outRec.define("phaseunit", phaseUnit);

  Vector<Double> dirValue(3);
  String dirUnit;
  dirValue=mLocation_p.get("m").getValue();
  dirUnit=mLocation_p.get("m").getUnit();
  outRec.define("dirvalue", dirValue);
  outRec.define("dirunit", dirUnit);

  outRec.define("padding", padding_p);
  outRec.define("maxdataval", maxAbsData);

  Vector<Int> center_loc(4), offset_loc(4);
  for (Int k=0; k<4 ; k++){
    center_loc(k)=centerLoc(k);
    offset_loc(k)=offsetLoc(k);
  }
  outRec.define("centerloc", center_loc);
  outRec.define("offsetloc", offset_loc);
  outRec.define("sumofweights", itsSumWeight[0]);
  if(withImage && image){
    ImageInterface<Complex>& tempimage(*image);
    Record imageContainer;
    retval = (retval || tempimage.toRecord(error, imageContainer));
    outRec.defineRecord("image", imageContainer);
  }
  return retval;
}

Bool LofarFTMachine::fromRecord(String& error, const RecordInterface& inRec)
{
  Bool retval = True;
  gridder=0; imageCache=0; lattice=0; arrayLattice=0;
  Double cacheVal;
  inRec.get("cache", cacheVal);
  cachesize=(Long)cacheVal;
  inRec.get("tile", tilesize);
  inRec.get("gridfunction", convType);

  Vector<Double> phaseValue(2);
  inRec.get("phasevalue",phaseValue);
  String phaseUnit;
  inRec.get("phaseunit",phaseUnit);
  Quantity val1(phaseValue(0), phaseUnit);
  Quantity val2(phaseValue(1), phaseUnit);
  MDirection phasecenter(val1, val2);

  mTangent_p=phasecenter;
  // This should be passed down too but the tangent plane is
  // expected to be specified in all meaningful cases.
  tangentSpecified_p=True;
  Vector<Double> dirValue(3);
  String dirUnit;
  inRec.get("dirvalue", dirValue);
  inRec.get("dirunit", dirUnit);
  MVPosition dummyMVPos(dirValue(0), dirValue(1), dirValue(2));
  MPosition mLocation(dummyMVPos, MPosition::ITRF);
  mLocation_p=mLocation;

  inRec.get("padding", padding_p);
  inRec.get("maxdataval", maxAbsData);

  Vector<Int> center_loc(4), offset_loc(4);
  inRec.get("centerloc", center_loc);
  inRec.get("offsetloc", offset_loc);
  uInt ndim4 = 4;
  centerLoc=IPosition(ndim4, center_loc(0), center_loc(1), center_loc(2),
		      center_loc(3));
  offsetLoc=IPosition(ndim4, offset_loc(0), offset_loc(1), offset_loc(2),
		      offset_loc(3));
  inRec.get("sumofweights", itsSumWeight[0]);
  if(inRec.nfields() > 12 ){
    Record imageAsRec=inRec.asRecord("image");
    if(!image) {
      image= new TempImage<Complex>();
    }
    retval = (retval || image->fromRecord(error, imageAsRec));

    // Might be changing the shape of sumWeight
    init();

    if(isTiled) {
      lattice=CountedPtr<Lattice<Complex> >(image, False);
    }
    else {
      // Make the grid the correct shape and turn it into an array lattice
      // Check the section from the image BEFORE converting to a lattice
      IPosition gridShape(4, nx, ny, npol, nchan);
      its_stacked_GriddedData.resize(gridShape);
      its_stacked_GriddedData=Complex(0.0);
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition start(4, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      its_stacked_GriddedData(blc, trc)=image->getSlice(start, image->shape());

      //if(arrayLattice) delete arrayLattice; arrayLattice=0;
      arrayLattice = new ArrayLattice<Complex>(its_stacked_GriddedData);
      lattice=arrayLattice;
    }

    //AlwaysAssert(lattice, AipsError);
    AlwaysAssert(gridder, AipsError);
    AlwaysAssert(image, AipsError);
  }
  return retval;
}

void LofarFTMachine::ok() {
  AlwaysAssert(image, AipsError);
}

// Make a plain straightforward honest-to-God image. This returns
// a complex image, without conversion to Stokes. The representation
// is that required for the visibilities.
//----------------------------------------------------------------------
void LofarFTMachine::makeImage(FTMachine::Type type,
		       VisSet& vs,
		       ImageInterface<Complex>& theImage,
		       Matrix<Float>& weight) {


  logIO() << LogOrigin(this-name(), "makeImage") << LogIO::NORMAL;

  if(type==FTMachine::COVERAGE) {
    logIO() << "Type COVERAGE not defined for Fourier transforms" << LogIO::EXCEPTION;
  }


  // Initialize the gradients
  ROVisIter& vi(vs.iter());

  // Loop over all visibilities and pixels
  VisBuffer vb(vi);

  // Initialize put (i.e. transform to Sky) for this model
  vi.origin();

  
// Temporarily disable because of
// /home/vdtol/lofar-casapy-4.1/src/LOFAR/CEP/Imager/LofarFT/src/LofarFTMachine.cc:2493: error: incomplete type ‘casa::SkyModel’ used in nested name specifier
// /home/vdtol/lofar-casapy-4.1/src/LOFAR/CEP/Imager/LofarFT/src/LofarFTMachine.cc:2496: error: incomplete type ‘casa::SkyModel’ used in nested name specifier
  
  if(vb.polFrame()==MSIter::Linear) {
//     StokesImageUtil::changeCStokesRep(theImage, SkyModel::LINEAR);  
  }
  else {
//     StokesImageUtil::changeCStokesRep(theImage, SkyModel::CIRCULAR);
  }

  initializeToSky(theImage,weight,vb);

  // Loop over the visibilities, putting VisBuffers
  for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
    for (vi.origin(); vi.more(); vi++) {

      switch(type) {
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

String LofarFTMachine::name(){

  return machineName_p;


}

void LofarFTMachine::ComputeResiduals(VisBuffer&vb, Bool useCorrected)
{

  //cout<<"LofarFTMachine::ComputeResiduals "<<vb.corrType()<<endl;
  LofarVBStore vbs;
  PrecTimer TimerResid;
  TimerResid.start();
  vbs.nRow_p = vb.nRow();
  vbs.beginRow_p = 0;
  vbs.endRow_p = vbs.nRow_p;
  vbs.modelCube_p.reference(vb.modelVisCube());
  if (useCorrected) vbs.correctedCube_p.reference(vb.correctedVisCube());
  else vbs.visCube_p.reference(vb.visCube());
  //  cout<<"BLA===="<<vb.visCube()<<"    "<<useCorrected<<endl;

  //for(uInt i=0;i<vbs.nRow_p;++i){cout<<"ROW "<<i<<" "<<vb.antenna1()(i)<<" "<<vb.antenna2()(i)<<endl;};

  vbs.useCorrected_p = useCorrected;
  visResamplers_p.lofarComputeResiduals(vbs);

  TimerResid.stop();
  //cout<<"Residuals: "<<TimerResid.getReal()<<endl;

  //  vb.correctedVisCube()=0.;//vb.modelVisCube();
}

  void LofarFTMachine::makeSensitivityImage(const VisBuffer& vb,
					 const ImageInterface<Complex>& imageTemplate,
					 ImageInterface<Float>& sensitivityImage)
  {
    cout<<"============================== makeSensitivityImage"<<endl;
    if (convFuncCtor_p->makeAverageResponse(vb, imageTemplate, sensitivityImage))
      cfCache_p->flush(sensitivityImage,sensitivityPatternQualifierStr_p);
  }
  //
  //---------------------------------------------------------------
  //
  void LofarFTMachine::normalizeAvgPB(ImageInterface<Complex>& inImage,
				   ImageInterface<Float>& outImage)
  {
    LogIO log_l(LogOrigin(this->name(), "normalizeAvgPB"));
    if (pbNormalized_p) return;
    IPosition inShape(inImage.shape()),ndx(4,0,0,0,0);
    Vector<Complex> peak(inShape(2));

    outImage.resize(inShape);
    outImage.setCoordinateInfo(inImage.coordinates());

    Bool isRefIn;
    Array<Complex> inBuf;
    Array<Float> outBuf;

    isRefIn  = inImage.get(inBuf);
    log_l << "Normalizing the average PBs to unity"
	  << LogIO::NORMAL << LogIO::POST;
    //
    // Normalize each plane of the inImage separately to unity.
    //
    Complex inMax = max(inBuf);
    if (abs(inMax)-1.0 > 1E-3)
      {
	for(ndx(3)=0;ndx(3)<inShape(3);ndx(3)++)
	  for(ndx(2)=0;ndx(2)<inShape(2);ndx(2)++)
	    {
	      peak(ndx(2)) = 0;
	      for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
		  if (abs(inBuf(ndx)) > peak(ndx(2)))
		    peak(ndx(2)) = inBuf(ndx);

	      for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
		for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
		  //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
		  inBuf(ndx) /= peak(ndx(2));
	    }
	if (isRefIn) inImage.put(inBuf);
      }

    ndx=0;
    for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
      for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
	{
	  IPosition plane1(ndx);
	  plane1=ndx;
	  plane1(2)=1; // The other poln. plane
	  //	  avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
	  outBuf(ndx) = sqrt(real(inBuf(ndx) * inBuf(plane1)));
	}
    //
    // Rather convoluted way of copying Pol. plane-0 to Pol. plane-1!!!
    //
    for(ndx(1)=0;ndx(1)<inShape(1);ndx(1)++)
      for(ndx(0)=0;ndx(0)<inShape(0);ndx(0)++)
	{
	  IPosition plane1(ndx);
	  plane1=ndx;
	  plane1(2)=1; // The other poln. plane
	  outBuf(plane1) = outBuf(ndx);
	}

    pbNormalized_p = True;
  }
  //
  //---------------------------------------------------------------
  //
  void LofarFTMachine::normalizeAvgPB()
  {
    LogIO log_l(LogOrigin(this-name(), "normalizeAvgPB"));
    if (pbNormalized_p) return;
    Bool isRefF;
    Array<Float> avgPBBuf;
    isRefF=avgPB_p->get(avgPBBuf);
    //    Float pbMax = max(avgPBBuf);
      {
	pbPeaks.resize(avgPB_p->shape()(2),True);
	// if (makingPSF) pbPeaks = 1.0;
	// else pbPeaks /= (Float)noOfPASteps;
	pbPeaks = 1.0;
	log_l << "Normalizing the average PBs to " << 1.0
	      << LogIO::NORMAL << LogIO::POST;

	IPosition avgPBShape(avgPB_p->shape()),ndx(4,0,0,0,0);
	Vector<Float> peak(avgPBShape(2));


	Float pbMax = max(avgPBBuf);
	if (fabs(pbMax-1.0) > 1E-3)
	  {
	    //	    avgPBBuf = avgPBBuf/noOfPASteps;
	    for(ndx(3)=0;ndx(3)<avgPBShape(3);ndx(3)++)
	      for(ndx(2)=0;ndx(2)<avgPBShape(2);ndx(2)++)
		{
		  peak(ndx(2)) = 0;
		  for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
		    for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
		      if (abs(avgPBBuf(ndx)) > peak(ndx(2)))
			peak(ndx(2)) = avgPBBuf(ndx);

		  for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
		    for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
		      //		      avgPBBuf(ndx) *= (pbPeaks(ndx(2))/peak(ndx(2)));
		      avgPBBuf(ndx) /= peak(ndx(2));
		}
	    if (isRefF) avgPB_p->put(avgPBBuf);
	  }

	ndx=0;
	for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
	  for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
	    {
	      IPosition plane1(ndx);
	      plane1=ndx;
	      plane1(2)=1; // The other poln. plane
	      avgPBBuf(ndx) = (avgPBBuf(ndx) + avgPBBuf(plane1))/2.0;
	      //	      avgPBBuf(ndx) = (avgPBBuf(ndx) * avgPBBuf(plane1));
	    }
	for(ndx(1)=0;ndx(1)<avgPBShape(1);ndx(1)++)
	  for(ndx(0)=0;ndx(0)<avgPBShape(0);ndx(0)++)
	    {
	      IPosition plane1(ndx);
	      plane1=ndx;
	      plane1(2)=1; // The other poln. plane
	      avgPBBuf(plane1) = avgPBBuf(ndx);
	    }
      }
      pbNormalized_p = True;
  }

  void LofarFTMachine::normalizeImage(Lattice<Complex>& skyImage,
				   const Matrix<Double>& sumOfWts,
				   Lattice<Float>& sensitivityImage,
				   Lattice<Complex>& sensitivitySqImage,
				   Bool fftNorm)
  {
    //
    // Apply the gridding correction
    //
    if (itsVerbose > 0) {
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
	if(ix==centerX) sincConv(ix)=1.0;
	else 	    sincConv(ix)=sin(x)/x;
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
	    gridder->correctX1D(correction,iy);

	    Vector<Complex> PBCorrection(liavgpb.rwVectorCursor().shape());
	    Vector<Float> avgPBVec(liavgpb.rwVectorCursor().shape());
	    Vector<Complex> avgPBSqVec(liavgpbSq.rwVectorCursor().shape());

	    avgPBSqVec= liavgpbSq.rwVectorCursor();
	    avgPBVec = liavgpb.rwVectorCursor();

	    for(int i=0;i<PBCorrection.shape();i++)
	      {
		// if (makingPSF)
		PBCorrection(i)=(avgPBSqVec(i)/avgPBVec(i));///(sincConv(i)*sincConv(iy));
		//		PBCorrection(i)=(avgPBSqVec(i));///(sincConv(i)*sincConv(iy));
		//		PBCorrection(i)=avgPBVec(i);///(sincConv(i)*sincConv(iy));

		// else
		//		PBCorrection(i)=(avgPBVec(i));//*sincConv(i)*sincConv(iy);
		//		if ((abs(avgPBSqVec(i))) >= pbLimit_p)
		if ((abs(avgPBVec(i))) >= pbLimit_p)
		  lix.rwVectorCursor()(i) /= PBCorrection(i);

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
  //
  //---------------------------------------------------------------
  //
  void LofarFTMachine::normalizeImage(Lattice<Complex>& skyImage,
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
	    gridder->correctX1D(correction,iy);

	    Vector<Float> avgPBVec(liavgpb.rwVectorCursor().shape());

	    avgPBVec = liavgpb.rwVectorCursor();

	    for(int i=0;i<avgPBVec.shape();i++)
	      {
		Float tt = pbFunc(avgPBVec(i),pbLimit_p);
                  //		PBCorrection(i)=pbFunc(avgPBVec(i),pbLimit_p)*sincConv(i)*sincConv(iy);
                  //                lix.rwVectorCursor()(i) /= PBCorrection(i);
		//                lix.rwVectorCursor()(i) *= tt;

		lix.rwVectorCursor()(i) /= tt;
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


  void LofarFTMachine::makeCFPolMap(const VisBuffer& vb, const Vector<Int>& locCfStokes,
				 Vector<Int>& polM)
  {
    LogIO log_l(LogOrigin(this->name(), "findPointingOffsets"));
    Vector<Int> msStokes = vb.corrType();
    //cout<<"LofarFTMachine findPointingOffsets "<< msStokes << " "<<locCfStokes<<endl;
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
  void LofarFTMachine::makeConjPolMap(const VisBuffer& vb,
				     const Vector<Int> cfPolMap,
				     Vector<Int>& conjPolMap)
  {
    LogIO log_l(LogOrigin(this->name(), "makConjPolMap"));
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

  void LofarFTMachine::showTimings (ostream& os, double duration) const
  {
    // The total time is the real elapsed time.
    // The cf and (de)gridding time is the sum of all threads, so scale
    // them back to real time.
    double total = itsCFTime + itsGriddingTime + itsDegriddingTime;
    double scale = 1;
    if (total > 0) {
      scale = itsTotalTimer.getReal() / total;
    }
    itsConvFunc->showTimings (os, duration, itsCFTime*scale);
    if (itsGriddingTime > 0) {
      os << "  gridding          ";
      LofarConvolutionFunction::showPerc1 (os, itsGriddingTime*scale,
                                           duration);
      os << endl;
    }
    if (itsDegriddingTime > 0) {
      os << "  degridding        ";
      LofarConvolutionFunction::showPerc1 (os, itsDegriddingTime*scale,
                                           duration);
      os << endl;
    }
  }

void FTMachineSplitBeam::put(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type)
{
  itsCyrilTimer.stop();

  if (itsVerbose > 0) 
  {
    logIO() << LogOrigin(this-name(), "put") << LogIO::NORMAL
            << "I am gridding " << vb.nRow() << " row(s)."  << LogIO::POST;
    logIO() << LogIO::NORMAL << "Padding is " << padding_p  << LogIO::POST;
  }


  gridOk(gridder->cSupport()(0));

  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())   matchAllSpwChans(vb);

  //Here we redo the match or use previous match

  //Channel matching for the actual spectral window of buffer
  if (doConversion_p[vb.spectralWindow()]) {
    matchChannel(vb.spectralWindow(), vb);
  } else {
    chanMap.resize();
    chanMap=multiChanMap_p[vb.spectralWindow()];
  }

  uInt spw(vb.spectralWindow());

  //No point in reading data if it's not matching in frequency
  if(max(chanMap)==-1) return;

  const Matrix<Float> *imagingweight;
  imagingweight=&(vb.imagingWeight());

  if(its_reallyDoPSF) {dopsf=true;}
  if(dopsf) {type=FTMachine::PSF;}
  Cube<Complex> data;
  //Fortran gridder need the flag as ints
  Cube<Int> flags;
  Matrix<Float> elWeight;
  interpolateFrequencyTogrid(vb, *imagingweight,data, flags, elWeight, type);


  Int startRow, endRow, nRow;
  if (row==-1) { nRow=vb.nRow(); startRow=0; endRow=nRow-1; }
  else         { nRow=1; startRow=row; endRow=row; }

  // Get the uvws in a form that Fortran can use and do that
  // necessary phase rotation. On a Pentium Pro 200 MHz
  // when null, this step takes about 50us per uvw point. This
  // is just barely noticeable for Stokes I continuum and
  // irrelevant for other cases.
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;

  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
    // cout << "freq  "<< freq[i]   << endl;
    // cout << "obsid "<< obs[i]    << vb.dataDescriptionId() <<endl;
    // cout << "times "<< timess[i] << endl;
  }

  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // Set up VBStore object to point to the relevant info of the VB.
  LofarVBStore vbs;
  makeCFPolMap(vb,cfStokes_p,CFMap_p);
  makeConjPolMap(vb,CFMap_p,ConjCFMap_p);

  //LofarFTMachine::make_mapping(vb);
  vector<vector<uInt> > MapBlTimes=LofarFTMachine::make_mapping_time(vb, spw);
  

  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double timeChunk = 0.5 * (times[times.size()-1] + times[0]);

  vbs.nRow_p = vb.nRow();
  vbs.uvw_p.reference(uvw);
  vbs.imagingWeight_p.reference(elWeight);
  vbs.visCube_p.reference(data);
  //  vbs.visCube_p.reference(vb.modelVisCube());
  vbs.freq_p.reference(interpVisFreq_p);
  vbs.rowFlag_p.reference(vb.flagRow());

  // Really nice way of converting a Cube<Int> to Cube<Bool>.
  // However the VBS objects should ultimately be references
  // directly to bool cubes.
  //**************


  vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;
  //  vbs.flagCube_p.reference(vb.flagCube());
  //**************

   // Determine the terms of the Mueller matrix that should be calculated
  visResamplers_p.setParams(uvScale,uvOffset,dphase);
  visResamplers_p.setMaps(chanMap, polMap);

  // First compute the A-terms for all stations (if needed).
  PrecTimer CyrilTimer2Aterm;
  CyrilTimer2Aterm.start();
  if(!itsAllAtermsDone){
    itsConvFunc->computeAterm (timeChunk);
    itsConvFunc->computeVecAterm(times[0],times[times.size()-1],its_TimeWindow);
  }
  CyrilTimer2Aterm.stop();
  double Taterm=CyrilTimer2Aterm.getReal();

  uInt Nchannels = vb.nChannel();

  itsTotalTimer.start();

  vector< Bool> done;
  done.resize(int(MapBlTimes.size()));
  for(int i=0; i<int(MapBlTimes.size()); ++i) {done[i]=false;};

  Bool all_done(false);
  Int doagain(0);

  ///  Int Max_Num_Threads(itsNThread);
  ///  omp_set_num_threads(Max_Num_Threads);


  //logIO() <<"============================== Gridding data " << LogIO::POST;
  //cout<<"... gridding with t= "<<time<<endl;
  PrecTimer CyrilTimer2grid;
  PrecTimer CyrilTimer2conv;
  PrecTimer CyrilTimer2gridconv;
  CyrilTimer2gridconv.start();
    //    CyrilTimer2conv.reset();

  while(!all_done){
    #pragma omp parallel
    // Thread-private variables.
    PrecTimer gridTimer;
    PrecTimer cfTimer;
    // The for loop can be parallellized. This must be done dynamically,
    // because the execution times of iterations can vary greatly.

    #pragma omp for schedule(dynamic)
    for (int i=0; i<int(MapBlTimes.size()); ++i) 
    {
      if((done[i]==true)|(MapBlTimes[i].size()==0)){continue;};
      Int ChunkSize(MapBlTimes[i].size());
      Int ist  = MapBlTimes[i][0];
      Int iend = MapBlTimes[i][ChunkSize-1];

      try
      {

        double average_weight=0.;
        if (itsDonePB==false)
        {
          uInt Nvis=0;
          for(Int j=0; j<ChunkSize; ++j)
          {
            uInt row=MapBlTimes[i][j];
            if(!vbs.rowFlag()[row])
            {
              Nvis+=1;
              for(uInt k=0; k<Nchannels; ++k) 
              {
                average_weight=average_weight+vbs.imagingWeight()(k,row);
              }
            }
          }
          if(Nvis>0)
          {
            average_weight=average_weight/Nvis;
          } 
          else 
          { 
            average_weight=0.;
           
          }
          if (itsVerbose > 1) 
          {
            cout<<"average weights= "<<average_weight<<", Nvis="<<Nvis<<endl;
          }
        }

        int threadNum = OpenMP::threadNum();

        // Get the convolution function.
        if (itsVerbose > 1) {
          cout.precision(20);
          cout<<"A1="<<ant1[ist]<<", A2="<<ant2[ist]<<", time="<<fixed<<timeChunk<<endl;
        }

        CyrilTimer2conv.start();
        cfTimer.start();
        Double Wmean=0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend));
        Double TimeMean=0.5*(times[ist] + times[iend]);

        Vector<uInt> ChanBlock;
        ChanBlock.resize(1);
        ChanBlock[0]=spw;

        LofarCFStore cfStore = itsConvFunc->makeConvolutionFunction (
          ant1[ist], 
          ant2[ist], 
          TimeMean,
          Wmean,
          itsGridMuellerMask, 
          false,
          average_weight,
          itsSumPB[threadNum],
          itsSumCFWeight[threadNum],
          ChanBlock,
          thisterm_p,
          itsRefFreq,
          itsStackMuellerNew[threadNum],
          0, 
          true);
      
        CyrilTimer2conv.stop();

        Int nConvX = (*(cfStore.vdata))[0][0][0].shape()[0];
        double cfstep=CyrilTimer2conv.getReal();
        CyrilTimer2grid.start();
        gridTimer.start();
        visResamplers_p.lofarDataToGrid_interp(
          (*itsGriddedData)[threadNum], 
          vbs,
          MapBlTimes[i],
          itsSumWeight[threadNum], 
          dopsf, 
          cfStore);
        gridTimer.stop();


        CyrilTimer2grid.stop();
        CyrilTimer2grid.reset();
        CyrilTimer2conv.reset();

        done[i]=true;
      } 
      catch (std::bad_alloc &)
      {
        cout<<"-----------------------------------------"<<endl;
        cout<<"!!!!!!! GRIDDING: Skipping baseline: "<<ant1[ist]<<" | "<<ant2[ist]<<endl;
        cout<<"memoryUsed() "<< HostInfo::memoryUsed()<< ", Free: "<<HostInfo::memoryFree()<<endl;
        cout<<"-----------------------------------------"<<endl;
      };
      // } // end omp critical
    } // end omp for

    double cftime = cfTimer.getReal();
#pragma omp atomic
    itsCFTime += cftime;
    double gtime = gridTimer.getReal();
#pragma omp atomic
    itsGriddingTime += gtime;
  } // end omp parallel

    all_done=true;
    int number_missed(0);
    for (int i=0; i<int(blStart.size()); ++i) {
      if(done[i]==false){all_done=false;number_missed+=1;};
    };
    if(all_done==false){
      //cout<<"================================"<<endl;
      //cout<<"Memory exception returned by "<<number_missed<<" threads"<<endl;
      //cout<<"Reducing number of threads to: "<<int(omp_get_num_threads()/2.)<<endl;
      //cout<<"================================"<<endl;
      doagain+=1;
      //omp_set_num_threads(int(omp_get_num_threads()/2.));
    };

  }//end While loop

  CyrilTimer2gridconv.stop();
  double Tgridconv=CyrilTimer2gridconv.getReal();

  PrecTimer CyrilTimer2elem;
  if(itsDeltaTime<(times[times.size()-1] - times[0])){itsDeltaTime=(times[times.size()-1] - times[0]);};
  Bool lastchunk(false);
  if((times[times.size()-1] - times[0])<0.95*itsDeltaTime){
    lastchunk=true;
    itsDeltaTime=0.;
  }

  //cout<<"time: "<<time<<" "<<itsStepApplyElement<<" "<<itsCounterTimes<<" "<<its_Apply_Element<<" "<<lastchunk<<" "<<itsDeltaTime<<endl;

  if(itsCounterTimes==(itsStepApplyElement-1)/2){itsNextApplyTime=timeChunk;}
  if(its_Apply_Element){
    if((itsCounterTimes==itsStepApplyElement-1)||(lastchunk)){
      CyrilTimer2elem.start();
      Array<Complex> tmp_stacked_GriddedData;
      tmp_stacked_GriddedData.resize ((*itsGriddedData)[0].shape());
      tmp_stacked_GriddedData = Complex();
      SumGridsOMP(tmp_stacked_GriddedData, (*itsGriddedData));
      //itsConvFunc->MakeMaskDegrid(tmp_stacked_GriddedData, itsTotalStepsGrid);
      Array<Complex> tmp_stacked_GriddedData_appliedelement=itsConvFunc->ApplyElementBeam2 (tmp_stacked_GriddedData, itsNextApplyTime, spw, itsGridMuellerMask, false);
      // if(its_UseMasksDegrid){
      // 	itsConvFunc->MakeMaskDegrid(tmp_stacked_GriddedData_appliedelement, itsTotalStepsGrid);
      // }
      SumGridsOMP(its_stacked_GriddedData, tmp_stacked_GriddedData_appliedelement);
      CyrilTimer2elem.stop();
      itsCounterTimes=0;
      for (uInt i=0; i<itsNThread; ++i) {
	(*itsGriddedData)[i]=Complex();
      }
      itsTotalStepsGrid+=1;
      CyrilTimer2elem.stop();
      //cout<<"============== element:"<<CyrilTimer2elem.getReal()<<endl;
      CyrilTimer2elem.reset();
    } else {
      itsCounterTimes+=1;
    }
  }

  //cout<<"times: aterm:"<<Taterm<<", conv: "<<CyrilTimer2conv.getReal()<<", grid: "<<CyrilTimer2grid.getReal()<<", gridconv: "<<Tgridconv<<", sum: "<<CyrilTimer2elem.getReal()<<", other: "<<itsCyrilTimer.getReal()<<endl;
  itsTotalTimer.stop();
  itsCyrilTimer.reset();
  itsCyrilTimer.start();
}


void LofarFTMachine::get(VisBuffer& vb, Int row)
{
  gridOk(gridder->cSupport()(0));
  // If row is -1 then we pass through all rows
  Int startRow, endRow, nRow;
  if (row < 0) { nRow=vb.nRow(); startRow=0; endRow=nRow-1;}
  else         { nRow=1; startRow=row; endRow=row; }

  // Get the uvws in a form that Fortran can use
  Matrix<Double> uvw(3, vb.uvw().nelements());  uvw=0.0;
  Vector<Double> dphase(vb.uvw().nelements());  dphase=0.0;
  //NEGATING to correct for an image inversion problem
  for (Int i=startRow;i<=endRow;i++) {
    for (Int idim=0;idim<2;idim++) uvw(idim,i)=-vb.uvw()(i)(idim);
    uvw(2,i)=vb.uvw()(i)(2);
  }
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())  matchAllSpwChans(vb);
  uInt spw(vb.spectralWindow());
  //cout<<"... De-Gridding Spectral Window: "<<vb.spectralWindow()<<", with Taylor Term: "<< thisterm_p<<endl;

  logIO() << LogOrigin(this-name(), "get") << LogIO::NORMAL
	  << "I am degridding " << vb.nRow() << " row(s)."  << LogIO::POST;


  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()])
    matchChannel(vb.spectralWindow(), vb);
  else
    {
      chanMap.resize();
      chanMap=multiChanMap_p[vb.spectralWindow()];
    }

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1)    return;

  Cube<Complex> data;
  Cube<Int> flags;
  getInterpolateArrays(vb, data, flags);

  // Apparently we don't support "tiled gridding" any more (good! :)).
  if(isTiled)
    throw(SynthesisFTMachineError("LofarFTMachine::get(): Internal error.  isTiled is True. "));

  LofarVBStore vbs;
  vbs.nRow_p = vb.nRow();
  vbs.beginRow_p = 0;
  vbs.endRow_p = vbs.nRow_p;

  vbs.uvw_p.reference(uvw);
  //    vbs.imagingWeight.reference(elWeight);
  vbs.visCube_p.reference(data);

  vbs.freq_p.reference(interpVisFreq_p);
  vbs.rowFlag_p.resize(0); vbs.rowFlag_p = vb.flagRow();
  if(!usezero_p)
    for (Int rownr=startRow; rownr<=endRow; rownr++)
      if(vb.antenna1()(rownr)==vb.antenna2()(rownr)) vbs.rowFlag_p(rownr)=True;

  // Really nice way of converting a Cube<Int> to Cube<Bool>.
  // However these should ultimately be references directly to bool
  // cubes.
  vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;
  //    vbs.rowFlag.resize(rowFlags.shape());  vbs.rowFlag  = False; vbs.rowFlag(rowFlags) = True;

  // Determine the terms of the Mueller matrix that should be calculated
  visResamplers_p.setParams(uvScale,uvOffset,dphase);
  visResamplers_p.setMaps(chanMap, polMap);

  //LofarFTMachine::make_mapping(vb);
  vector<vector<uInt> > MapBlTimes=LofarFTMachine::make_mapping_time(vb, spw);

  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double timeChunk = 0.5 * (times[times.size()-1] + times[0]);
  double MinTime = times[0];
  double MaxTime = times[times.size()-1];
  // if(its_Use_Linear_Interp_Gridder){
  //   itsConvFunc->computeAterm (MinTime);
  //   itsConvFunc->computeAterm (MaxTime);
  // } else {
  if(!itsAllAtermsDone){
    itsConvFunc->computeAterm (timeChunk);
    itsConvFunc->computeVecAterm(times[0],times[times.size()-1],its_TimeWindow);
  }

    //}

  //ROVisIter& via(vb.iter());

  // First compute the A-terms for all stations (if needed).

  if(times[0]<itsTStartObs){itsTStartObs=times[0];}
  if(itsDeltaTime<(times[times.size()-1] - times[0])){itsDeltaTime=(times[times.size()-1] - times[0]);};
  if(itsDeltaTime<(times[times.size()-1] - times[0])){itsDeltaTime=(times[times.size()-1] - times[0]);};

  itsTotalTimer.start();

  vector< Bool> done;
  done.resize(int(MapBlTimes.size()));
  for(int i=0; i<int(MapBlTimes.size()); ++i) {done[i]=false;};

  Bool all_done(false);
  ///  Int Max_Num_Threads(itsNThread);
  ///  omp_set_num_threads(Max_Num_Threads);

  PrecTimer CyrilElement;
  CyrilElement.start();
  //cout.precision(20);
  //cout<<" ======================= De-Grid ... time="<<time<<", at "<<itsCounterTimes<<endl;
  if(its_Apply_Element){
    //cout<<"itsCounterTimes= "<<itsCounterTimes<<endl;
    if(itsCounterTimes==0){
     double TimeElement(itsTStartObs+itsDeltaTime*itsStepApplyElement/2.);
     //cout<<"... Appying element with t="<<TimeElement<<", itsTStartObs="<<itsTStartObs<<", itsDeltaTime="<<itsDeltaTime<<endl;
     itsConvFunc->computeAterm(TimeElement);
     if(its_UseMasksDegrid){
       itsGridToDegrid.reference(itsConvFunc->ApplyElementBeam2(its_stacked_GriddedData, TimeElement, spw, itsGridMuellerMask, true, itsTotalStepsDeGrid));
     }else{
       itsGridToDegrid.reference(itsConvFunc->ApplyElementBeam2(its_stacked_GriddedData, TimeElement, spw, itsGridMuellerMask, true));
     }
     itsTotalStepsDeGrid+=1;
    }
    itsCounterTimes+=1;
    if(itsCounterTimes==itsStepApplyElement){
      itsTStartObs=1.e30;
      itsCounterTimes=0;
    }
    Bool lastchunk(false);
    if((times[times.size()-1] - times[0])<0.95*itsDeltaTime){
      //cout<<"Last Chunk Degrid!!!"<<endl;
      lastchunk=true;
      itsDeltaTime=0.;
      itsTStartObs=1e12;
    }


  } else{
    itsGridToDegrid.reference(its_stacked_GriddedData);
  }
  CyrilElement.stop();

  // arrayLattice = new ArrayLattice<Complex>(tmp_stacked_GriddedData2);
  // cout<<"LofarConvolutionFunction::ApplyElementBeam "<<"FFT the element corrected model image"<<endl;
  // lattice=arrayLattice;
  // LatticeFFT::cfft2d(*lattice);

  //logIO() <<"============================== De-Gridding data " << LogIO::POST;
  PrecTimer CyrilConv;
  PrecTimer CyrilGrid;


  while(!all_done){




#pragma omp parallel
  {
    // Thread-private variables.
    PrecTimer degridTimer;
    PrecTimer cfTimer;
    // The for loop can be parallellized. This must be done dynamically,
    // because the execution times of iterations can vary greatly.
    #pragma omp for schedule(dynamic)
    for (int i=0; i<int(MapBlTimes.size()); ++i) {
      if((done[i]==true)|(MapBlTimes[i].size()==0)){continue;};
      Int ChunkSize(MapBlTimes[i].size());
      Int ist  = MapBlTimes[i][0];
      Int iend = MapBlTimes[i][ChunkSize-1];

      try {
      int threadNum = OpenMP::threadNum();
      // Get the convolution function for degridding.
      if (itsVerbose > 1) {
	cout<<"ANTENNA "<<ant1[ist]<<" "<<ant2[ist]<<endl;
      }
      cfTimer.start();
      CyrilConv.start();
      Double TimeMean=0.5*(times[ist] + times[iend]);

      Vector<uInt> ChanBlock;
      ChanBlock.resize(1);
      ChanBlock[0]=spw;
      LofarCFStore cfStore;
      cfStore=  itsConvFunc->makeConvolutionFunction (ant1[ist], ant2[ist], TimeMean,
                                              0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend)),
                                              itsDegridMuellerMask,
                                              true,
                                              0.0,
                                              itsSumPB[threadNum],
                                              itsSumCFWeight[threadNum]
					      ,ChanBlock,thisterm_p,itsRefFreq,
					      itsStackMuellerNew[threadNum],
						      0, true);
	visResamplers_p.lofarGridToData_interp(vbs, itsGridToDegrid,//its_stacked_GriddedData,//(*itsGriddedData)[0],
					       MapBlTimes[i], cfStore);
      cfTimer.stop();

      CyrilConv.stop();
      CyrilGrid.start();

      degridTimer.start();
      CyrilGrid.stop();

      degridTimer.stop();
      done[i]=true;

      } catch (std::bad_alloc &)
	{
	  cout<<"-----------------------------------------"<<endl;
	  cout<<"!!!!!!! DE-GRIDDING: Skipping baseline: "<<ant1[ist]<<" | "<<ant2[ist]<<endl;
	  cout<<"memoryUsed() "<< HostInfo::memoryUsed()<< ", Free: "<<HostInfo::memoryFree()<<endl;
	  cout<<"-----------------------------------------"<<endl;
	}
      

    } // end omp for
    double cftime = cfTimer.getReal();
#pragma omp atomic
    itsCFTime += cftime;
    double gtime = degridTimer.getReal();
#pragma omp atomic
    itsGriddingTime += gtime;
  } // end omp parallel

    all_done=true;
    int number_missed(0);
    for (int i=0; i<int(blStart.size()); ++i) {
      //cout<<"done: "<<i<<" "<<done[i]<<endl;
      if(done[i]==false){all_done=false;number_missed+=1;};
    };
    if(all_done==false){
      //cout<<"================================"<<endl;
      //cout<<"Memory exception returned by "<<number_missed<<" threads"<<endl;
      //cout<<"Reducing number of threads to: "<<int(omp_get_num_threads()/2.)<<endl;
      //cout<<"================================"<<endl;
      //omp_set_num_threads(int(omp_get_num_threads()/2.));
    };

  }//end While loop

  //cout<<"Element: "<<CyrilElement.getReal()<<", Conv: "<<CyrilConv.getReal()<<", Grid: "<<CyrilGrid.getReal()<<endl;;



  itsTotalTimer.stop();
  interpolateFrequencyFromgrid(vb, data, FTMachine::MODEL);
}


void FTMachineWsplit::initGridThreads(
  vector< casa::Array<casa::Complex> >&  otherGriddedData, 
  vector< casa::Array<casa::DComplex> >&  otherGriddedData2)
{
  itsGriddedData=&otherGriddedData;
  itsGriddedData2=&otherGriddedData2;
  (*itsGriddedData).resize (itsNThread);
  (*itsGriddedData2).resize (itsNThread);
}

} //# end namespace LOFAR

namespace 
{

  template <class T>
  void sumGridsOMP(casa::Array<T>& grid, const vector< casa::Array<T> >& gridToAdd0 )
  {
    for(casa::uInt vv=0; vv<gridToAdd0.size(); vv++)
    {
      casa::Array<T> gridToAdd(gridToAdd0[vv]);
      int y,ch,pol;
      int gridSize(grid.shape()[0]);
      int nPol(grid.shape()[2]);
      int nChan(grid.shape()[3]);
      T* gridPtr;
      const T* gridToAddPtr;
      
#pragma omp parallel for private(y,ch,pol,gridPtr,gridToAddPtr)
      for(int x=0 ; x<grid.shape()[0] ; ++x)
      {
        for(ch=0 ; ch<nChan ; ++ch)
        {
          for(pol=0 ; pol<nPol ; ++pol)
          {
            gridPtr = grid.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
            gridToAddPtr = gridToAdd.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
            for(y=0 ; y<grid.shape()[1] ; ++y)
            {
               (*gridPtr++) += *gridToAddPtr++;
            }
          }
        }
      }
    }
  }
  
  //Sum Grids
  template <class T>
  void sumGridsOMP(casa::Array<T>& grid, const casa::Array<T>& gridToAdd){
    int y,ch,pol;
    int gridSize(grid.shape()[0]);
    int nPol(grid.shape()[2]);
    int nChan(grid.shape()[3]);
    T* gridPtr;
    const T* gridToAddPtr;
    
#pragma omp parallel for private(y,ch,pol,gridPtr,GridToAddPtr)
    for(int x=0 ; x<grid.shape()[0] ; ++x)
    {
      for(ch=0 ; ch<nChan ; ++ch)
      {
        for(pol=0 ; pol<nPol ; ++pol)
        {
          gridPtr = grid.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
          gridToAddPtr = gridToAdd.data() + ch*nPol*gridSize*gridSize + pol*gridSize*gridSize+x*gridSize;
          for(y=0 ; y<grid.shape()[1] ; ++y)
          {
            (*gridPtr++) += *gridToAddPtr++;
          }
        }
      }
    }
    
  }
  
} // end unnamed namespace

