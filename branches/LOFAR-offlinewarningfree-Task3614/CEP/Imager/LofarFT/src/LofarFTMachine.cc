//# LofarFTMachine.cc: Gridder for LOFAR data correcting for DD effects
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
// #include <Common/OpenMP.h>
// #include <omp.h>

#include <msvis/MSVis/VisibilityIterator.h>
#include <casa/Quanta/UnitMap.h>
#include <casa/Quanta/UnitVal.h>
#include <measures/Measures/Stokes.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/Projection.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/FFTServer.h>
#include <LofarFT/LofarFTMachine.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <synthesis/MeasurementComponents/Utils.h>
#include <LofarFT/LofarVisResampler.h>
#include <synthesis/MeasurementComponents/CFStore.h>
#include <LofarFT/LofarVBStore.h>
#include <scimath/Mathematics/RigidVector.h>
#include <msvis/MSVis/StokesVector.h>
#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisSet.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/PagedImage.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/MaskedArray.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/MatrixIter.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <measures/Measures/UVWMachine.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LCBox.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/LatticeFFT.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/Lattices/LatticeStepper.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <casa/Utilities/CompositeNumber.h>
#include <casa/OS/PrecTimer.h>
#include <casa/sstream.h>
#include <casa/OS/HostInfo.h>
#include <casa/BasicMath/Random.h>
#define DORES True


using namespace casa;

namespace LOFAR { //# NAMESPACE CASA - BEGIN

//  LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize,
//                                 CountedPtr<VisibilityResamplerBase>&,
//                                 String iconvType, Float padding,
//                                 Bool usezero, Bool useDoublePrec)
//: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize), tilesize(itilesize),
//  gridder(0), isTiled(False), convType(iconvType),
//  maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
//  usezero_p(usezero), noPadding_p(False), usePut2_p(False),
//  machineName_p("LofarFTMachine")

//{
////   LOG_INFO ("LofarFTMachine::LofarFTMachine" << 1.0);
////   logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
//  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
//  useDoubleGrid_p=useDoublePrec;
//  canComputeResiduals_p=DORES;
//}

LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize,
                               CountedPtr<VisibilityResamplerBase>&,
                               String iconvType,
                               const MeasurementSet& ms, Int nwPlanes,
                               MPosition mLocation, Float padding, Bool usezero,
                               Bool useDoublePrec, double wmax,
                               Int verbose,
                               Int maxsupport, Int oversample,
                               const String& imgName,
                               const Matrix<bool>& gridMuellerMask,
                               const Matrix<bool>& degridMuellerMask,
			       Double RefFreq,
			       Bool Use_Linear_Interp_Gridder, 
			       Bool Use_EJones, 
			       int StepApplyElement, 
			       Double PBCut, 
			       Bool PredictFT, 
			       String PsfOnDisk, 
			       Bool UseMasksDegrid,
			       Bool reallyDoPSF, 
                               const Record& parameters
                              )//, 
			       //Double FillFactor)
  : FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
    tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType),
    maxAbsData(0.0), centerLoc(IPosition(4,0)),
    offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False),
    usePut2_p(False), machineName_p("LofarFTMachine"), itsMS(ms),
    itsNWPlanes(nwPlanes), itsWMax(wmax), itsConvFunc(0),
    itsVerbose(verbose),
    itsMaxSupport(maxsupport), itsOversample(oversample), itsImgName(imgName),
    itsGridMuellerMask(gridMuellerMask),
    itsDegridMuellerMask(degridMuellerMask),
    itsGriddingTime(0), itsDegriddingTime(0), itsCFTime(0), itsParameters(parameters)
{
  cout << "=======LofarFTMachine====================================" << endl;
  cout << itsParameters << endl;
  cout << "=========================================================" << endl;
  
  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mLocation_p=mLocation;
  tangentSpecified_p=False;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;
  itsNThread = OpenMP::maxThreads();
  AlwaysAssert (itsNThread>0, AipsError);
  itsGriddedData.resize (itsNThread);
  itsGriddedData2.resize (itsNThread);
  itsSumPB.resize (itsNThread);
  itsSumCFWeight.resize (itsNThread);
  itsSumWeight.resize (itsNThread);
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
  itsPredictFT=PredictFT;
  if(itsStepApplyElement>0){its_Apply_Element=true;}

  if(its_Use_Linear_Interp_Gridder){
    cout<<"Gridding using oversampling of 1 only"<<endl;
    itsOversample=1;
  };
  //cout<<"FTMahin: itsRefFreq "<<itsRefFreq<<endl;

  ROMSSpWindowColumns window(ms.spectralWindow());
  itsListFreq.resize(window.nrow());
  for(uInt i=0; i<window.nrow();++i){
    itsListFreq[i]=window.refFrequency()(i);
    cout<<"SPW"<<i<<", freq="<<itsListFreq[i]<<endl;
  };
  its_Already_Initialized=false;
}

//LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize,
//		 CountedPtr<VisibilityResamplerBase>&,
//		 String iconvType,
//		 MDirection mTangent, Float padding, Bool usezero, Bool useDoublePrec)
//: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
//  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
//  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False),
//  usePut2_p(False), machineName_p("LofarFTMachine")
//{
//  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
//  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
//  mTangent_p=mTangent;
//  tangentSpecified_p=True;
//  useDoubleGrid_p=useDoublePrec;
//  canComputeResiduals_p=DORES;
//}

//LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize,
//		 CountedPtr<VisibilityResamplerBase>&,
//		 String iconvType, MPosition mLocation, MDirection mTangent, Float padding,
//		 Bool usezero, Bool useDoublePrec)
//: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
//  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
//  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False),
//  usePut2_p(False),machineName_p("LofarFTMachine")
//{
//  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
//  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
//  mLocation_p=mLocation;
//  mTangent_p=mTangent;
//  tangentSpecified_p=True;
//  useDoubleGrid_p=useDoublePrec;
//  canComputeResiduals_p=DORES;
//}

//LofarFTMachine::LofarFTMachine(const RecordInterface& stateRec)
//  : FTMachine()
//{
//  // Construct from the input state record
//  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine(RecordInterface)")  << LogIO::NORMAL;
//  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
//  String error;
//  if (!fromRecord(error, stateRec))
//    throw (AipsError("Failed to create gridder: " + error));
//  canComputeResiduals_p=DORES;
//}

//----------------------------------------------------------------------
LofarFTMachine& LofarFTMachine::operator=(const LofarFTMachine& other)
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
      gridder=0;
    else{
      gridder = new ConvolveGridder<Double, Complex>(IPosition(2, nx, ny),
						     uvScale, uvOffset,
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
    //cyrr: mfs
    itsRefFreq=other.itsRefFreq;
    thisterm_p=other.thisterm_p;
    its_Use_Linear_Interp_Gridder= other.its_Use_Linear_Interp_Gridder;
    its_Use_EJones= other.its_Use_EJones;
    its_UseMasksDegrid=other.its_UseMasksDegrid;
    its_Apply_Element= other.its_Apply_Element;
    itsStepApplyElement=other.itsStepApplyElement;
    its_Already_Initialized= other.its_Already_Initialized;
    its_reallyDoPSF = other.its_reallyDoPSF;
    its_PBCut= other.its_PBCut;
    //its_FillFactor=other.its_FillFactor;
     //cyrr: mfs

    ConjCFMap_p = other.ConjCFMap_p;
    CFMap_p = other.CFMap_p;
    itsNThread = other.itsNThread;
    itsGriddedData.resize (itsNThread);
    itsGriddedData2.resize (itsNThread);
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
  }
  return *this;
}

//----------------------------------------------------------------------
  LofarFTMachine::LofarFTMachine(const LofarFTMachine& other) : FTMachine(), machineName_p("LofarFTMachine")
  {
    //  visResampler_p.init(useDoubleGrid_p);
    operator=(other);
  }

//----------------------------------------------------------------------
//  CountedPtr<LofarFTMachine> LofarFTMachine::clone() const
  LofarFTMachine* LofarFTMachine::clone() const
  {
    LofarFTMachine* newftm = new LofarFTMachine(*this);
    return newftm;
  }

//----------------------------------------------------------------------
void LofarFTMachine::init() {

  logIO() << LogOrigin("LofarFTMachine", "init")  << LogIO::NORMAL;
  canComputeResiduals_p = DORES;
  ok();
  //  cout<<"LofarFTMachine::init()" <<endl;

  /* hardwiring isTiled is False
  // Padding is possible only for non-tiled processing
  if((padding_p*padding_p*image->shape().product())>cachesize) {
    isTiled=True;
    nx    = image->shape()(0);
    ny    = image->shape()(1);
    npol  = image->shape()(2);
    nchan = image->shape()(3);
  }
  else {
  */
    // We are padding.

  //cout<<"padding_p!!!!! "<<padding_p<<endl;

  isTiled=False;
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
						 convType);

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
  if (itsVerbose > 0) {
    cout << "Original shape " << image->shape()(0) << ","
         << image->shape()(1) << endl;
    cout << "Padded shape " << padded_shape(0) << ","
         << padded_shape(1) << endl;
  }
  //assert(padded_shape(0)!=image->shape()(0));
  itsConvFunc = new LofarConvolutionFunction(padded_shape,
                                             image->coordinates().directionCoordinate (image->coordinates().findCoordinate(Coordinate::DIRECTION)),
                                             itsMS, itsNWPlanes, itsWMax,
                                             itsOversample,
                                             itsVerbose, itsMaxSupport,
                                             itsImgName+String::toString(thisterm_p),
					     its_Use_EJones,
					     its_Apply_Element,
                                             itsParameters);

  // Set up image cache needed for gridding. For BOX-car convolution
  // we can use non-overlapped tiles. Otherwise we need to use
  // overlapped tiles and additive gridding so that only increments
  // to a tile are written.
  its_Already_Initialized=true;

  if(imageCache) delete imageCache; imageCache=0;

  if(isTiled) {
    Float tileOverlap=0.5;
    if(convType=="box") {
      tileOverlap=0.0;
    }
    else {
      tileOverlap=0.5;
      tilesize=max(12,tilesize);
    }
    IPosition tileShape=IPosition(4,tilesize,tilesize,npol,nchan);
    Vector<Float> tileOverlapVec(4);
    tileOverlapVec=0.0;
    tileOverlapVec(0)=tileOverlap;
    tileOverlapVec(1)=tileOverlap;
    Int tmpCacheVal=static_cast<Int>(cachesize);
    imageCache=new LatticeCache <Complex> (*image, tmpCacheVal, tileShape,
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
LofarFTMachine::~LofarFTMachine()
{
  if(imageCache) delete imageCache; imageCache=0;
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  if(gridder) delete gridder; gridder=0;
//  delete itsConvFunc;
}

const Matrix<Float>& LofarFTMachine::getAveragePB() const
{
  //cout<<"return beam"<<endl;
  // Read average beam from disk if not present.
  if (itsAvgPB.empty()) {
    //cout<<"...read beam "<<itsImgName+String::toString(thisterm_p) + ".avgpb"<<endl;
    PagedImage<Float> pim(itsImgName+String::toString(thisterm_p) + ".avgpb");
    Array<Float> arr = pim.get();
    itsAvgPB.reference (arr.nonDegenerate(2));
  }
  return itsAvgPB;
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image
void LofarFTMachine::initializeToVis(ImageInterface<Complex>& iimage,
                                     const VisBuffer& vb)
{
  image=&iimage;

  ok();
  if(!its_Already_Initialized){init();};//init();

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

  //  cout<<"LofarFTMachine::initializeToVis === is_NOT_Tiled!"<<endl;
  //======================CHANGED
  //nx=640;
  //ny=640;
  //======================END CHANGED
  //cout << "npol="<<npol<<endl;
  IPosition gridShape(4, nx, ny, npol, nchan);
  // Size and initialize the grid buffer per thread.
  // Note the other itsGriddedData buffers are assigned later.
  itsGriddedData[0].resize (gridShape);
  itsGriddedData[0] = Complex();
  its_stacked_GriddedData.resize (gridShape);
  its_stacked_GriddedData = Complex();
  for (int i=0; i<itsNThread; ++i) {
    itsSumPB[i].resize (padded_shape[0], padded_shape[1]);
    itsSumPB[i] = Complex();
    itsSumCFWeight[i] = 0.;
    itsSumWeight[i].resize(npol, nchan);
    itsSumWeight[i] = 0.;
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
  IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
  IPosition trc(blc+image->shape()-stride);
  if (itsVerbose > 0) {
    cout<<"LofarFTMachine::initializeToVis === blc,trc,nx,ny,image->shape()"
        <<blc<<" "<<trc<<" "<<nx<<" "<<ny<<" "<<image->shape()<<endl;
  }
  IPosition start(4, 0);
  its_stacked_GriddedData(blc, trc) = image->getSlice(start, image->shape());
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  //======================CHANGED
  arrayLattice = new ArrayLattice<Complex>(its_stacked_GriddedData);
  // Array<Complex> result(IPosition(4, nx, ny, npol, nchan),0.);
  // griddedData=result;
  // arrayLattice = new ArrayLattice<Complex>(griddedData);
  //======================END CHANGED
  lattice=arrayLattice;

  //AlwaysAssert(lattice, AipsError);

  logIO() << LogIO::DEBUGGING
	  << "Starting grid correction and FFT of image" << LogIO::POST;

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

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Normalising clean components by the beam

    // const Matrix<Float>& datai = getSpheroidCut();
    const Matrix<Float>& data = getAveragePB();
    //    cout<<"tmp.shape() "<<data.shape()<<"  "<<lattice->shape()<<endl;
    IPosition pos(4,lattice->shape()[0],lattice->shape()[1],1,1);
    IPosition pos2(2,lattice->shape()[0],lattice->shape()[1]);
    pos[2]=0.;
    pos[3]=0.;
    pos2[2]=0.;
    pos2[3]=0.;
    Int offset_pad(floor(data.shape()[0]-lattice->shape()[0])/2.);

    //    cout<<"LofarFTMachine::initializeToVis lattice->shape() == "<<lattice->shape()<<endl;
    String nameii(itsImgName+String::toString(thisterm_p) + ".spheroid_cut_im");
    ostringstream nameiii(nameii);
    PagedImage<Float> tmpi(nameiii.str().c_str());
    Slicer slicei(IPosition(4,0,0,0,0), tmpi.shape(), IPosition(4,1,1,1,1));
    Array<Float> datai;
    tmpi.doGetSlice(datai, slicei);

    String nameii_element("Spheroid_cut_im_element.img");
    ostringstream nameiii_element(nameii_element);
    PagedImage<Float> tmpi_element(nameiii_element.str().c_str());
    Slicer slicei_element(IPosition(4,0,0,0,0), tmpi_element.shape(), IPosition(4,1,1,1,1));
    Array<Float> spheroidCutElement;
    tmpi_element.doGetSlice(spheroidCutElement, slicei_element);

    Complex ff;
    double I=100.;
    double Q=0.;
    double U=0.;
    double V=0.;

    double maxPB(0.);
    double minPB(1e10);
    for(uInt i=0;i<lattice->shape()[0];++i){
      for(uInt j=0;j<lattice->shape()[0];++j){
	double pixel(data(i+offset_pad,j+offset_pad));
	if(abs(pixel)>maxPB){maxPB=abs(pixel);};
	if(abs(pixel)<minPB){minPB=abs(pixel);};
      }
    }

    for(Int k=0;k<lattice->shape()[2];++k){
      ff=0.;
      //cout<<"k="<<k<<endl;
      if(k==0){ff=I+Q;}
      // if(k==1){ff=I-Q;}
      if(k==1){ff=Complex(U,0.)+Complex(0.,V);}
      if(k==2){ff=Complex(U,0.)-Complex(0.,V);}
      if(k==3){ff=I-Q;}
      for(Int i=0;i<lattice->shape()[0];++i){
	for(Int j=0;j<lattice->shape()[0];++j){
	  pos[0]=i;
	  pos[1]=j;
	  pos[2]=k;
	  pos2[0]=i+offset_pad;
	  pos2[1]=j+offset_pad;
	  Complex pixel(lattice->getAt(pos));
	  double fact(1.);

	  // pixel=0.;
	  // if((pos[0]==372.)&&(pos[1]==370.)){//319
	  //   pixel=ff;//*139./143;//-100.;
	  //   //if(datai(pos2)>1e-6){fact/=datai(pos2)*datai(pos2);};//*datai(pos2);};
	  //   //if(datai(pos2)>1e-6){fact*=sqrt(maxPB)/sqrt(data(pos2));};
	  //   fact*=sqrt(maxPB)/sqrt(data(pos2));
	  //   //if(data(pos2)>1e-6){fact/=sqrt(data(pos2));};//*datai(pos2);};
	  //   pixel*=Complex(fact);
	  // }

	  if(!itsPredictFT){
	    fact*=sqrt(maxPB)/sqrt(data(pos2));
	  } else {
	    fact/=datai(pos2); //*datai(pos2); 
	    if(its_Apply_Element){fact/=spheroidCutElement(pos2);}
	  }
	  pixel*=Complex(fact);

	  if((data(pos2)>=(minPB))&&(abs(pixel)>0.)){   // SvdT: Had to make comparison great _or equal_ because of fake PB consisting of all ones
	    lattice->putAt(pixel,pos);
	  };
	}
      }
    }

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Now do the FFT2D in place
    LatticeFFT::cfft2d(*lattice);

    if((!(itsConvFunc->itsFilledVectorMasks))&&(its_Apply_Element)){itsConvFunc->ReadMaskDegrid();}
    //if((!(itsConvFunc->VectorMaskIsFilled()))&&(its_Apply_Element)){itsConvFunc->ReadMaskDegrid();}

    logIO() << LogIO::DEBUGGING
	    << "Finished grid correction and FFT of image" << LogIO::POST;

    // for(uInt k=0;k<lattice->shape()[2];++k){
    //   for(uInt i=0;i<lattice->shape()[0];++i){
    // 	for(uInt j=0;j<lattice->shape()[0];++j){
    // 	  pos[0]=i;
    // 	  pos[1]=j;
    // 	  pos[2]=k;
    // 	  Complex pixel(lattice->getAt(pos));
    // 	  //cout<<"i,j,pixel value: "<<i<<" "<<j<<" "<<pixel<<endl;

    // 	};
    //   };
    // };
}




void LofarFTMachine::finalizeToVis()
{
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToVis"<<endl;
  }
}


// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid.
void LofarFTMachine::initializeToSky(ImageInterface<Complex>& iimage,
			     Matrix<Float>& weight, const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;
  //if (itsVerbose > 0) {
  //cout<<"---------------------------> initializeToSky"<<" its_Already_Initialized : "<<its_Already_Initialized<<endl;
  //}
    if(!its_Already_Initialized){init();};

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
  for (int i=0; i<itsNThread; ++i) {
    itsGriddedData[i].resize (gridShape);
    itsGriddedData[i] = Complex();
    itsSumPB[i].resize (padded_shape[0], padded_shape[1]);
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

  //iimage.get(griddedData, False);
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  arrayLattice = new ArrayLattice<Complex>(its_stacked_GriddedData);
  lattice=arrayLattice;
  // if(useDoubleGrid_p) visResampler_p->initializePutBuffers(griddedData2, sumWeight);
  // else                visResampler_p->initializePutBuffers(griddedData, sumWeight);
//// Are the following calls needed for LOFAR?
///  if(useDoubleGrid_p) visResamplers_p.initializeToSky(griddedData2, sumWeight);
///  else                visResamplers_p.initializeToSky(griddedData, sumWeight);
  //AlwaysAssert(lattice, AipsError);
}



void LofarFTMachine::finalizeToSky()
{
  //AlwaysAssert(lattice, AipsError);
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if (itsVerbose > 0) {
    cout<<"---------------------------> finalizeToSky"<<endl;
  }
  // DEBUG: Store the grid per thread
  // uInt nx(itsGriddedData[0].shape()[0]);
  // IPosition shapecube(3,nx,nx,4);
  // for (int ii=0; ii<itsNThread; ++ii) {
  //   Cube<Complex> tempimage(shapecube,0.);
  //   for(Int k=0;k<itsGriddedData[0].shape()[2];++k){
  //     for(uInt i=0;i<nx;++i){
  // 	for(uInt j=0;j<nx;++j){
  // 	  IPosition pos(4,i,j,k,0);
  // 	  Complex pixel(itsGriddedData[ii](pos));
  // 	  tempimage(i,j,k)=pixel;
  // 	}
  //     }
  //   }
  //   store(tempimage,"Grid"+String::toString(ii)+".img");
  // }

  // Add all buffers into the first one.

  // for(uInt channel=0;channel< its_stacked_GriddedData.shape()[3];++channel){
  //   for(uInt jj=0;jj<its_stacked_GriddedData.shape()[2];++jj){
  //     cout<<"Add all buffers into the first one. jj="<<jj<<endl;
  //     Matrix<Complex> plane_array_out  = its_stacked_GriddedData(Slicer(IPosition(4, 0, 0, jj, 0),
  // 									IPosition(4, nx, nx, 1, 1))).nonDegenerate();
  //     ArrayLattice<Complex> lattice(plane_array_out);
  //     LatticeFFT::cfft2d(lattice, true);
  //     plane_array_out/=static_cast<Float>(plane_array_out.shape()(0)*plane_array_out.shape()(1));
  //   }
  // }

  if(!its_Apply_Element){
    SumGridsOMP(its_stacked_GriddedData, itsGriddedData);
    for (int i=0; i<itsNThread; ++i) {
      itsGriddedData[i]=Complex();
    }
  }


  for (int i=1; i<itsNThread; ++i) {
    //itsGriddedData[0] += itsGriddedData[i];
    itsSumWeight[0]   += itsSumWeight[i];
    itsSumCFWeight[0] += itsSumCFWeight[i];
    itsSumPB[0]       += itsSumPB[i];
  }

  // Cube<Complex> tempimage(IPosition(3,nx,nx,4),0.);
  // for(Int k=0;k<4;++k){
  //   for(uInt i=0;i<nx;++i){
  //     for(uInt j=0;j<nx;++j){
  // 	IPosition pos(4,i,j,k,0);
  // 	Complex pixel(its_stacked_GriddedData(pos));
  // 	tempimage(i,j,k)=pixel;
  //     }
  //   }
  // }
  // store(tempimage,"Grid00.img");

  // if(useDoubleGrid_p) visResamplers_p[0].GatherGrids(griddedData2, sumWeight);
  // else                visResamplers_p[0].GatherGrids(griddedData, sumWeight);
//// Are the following calls needed for LOFAR?
///  if(useDoubleGrid_p) visResamplers_p.finalizeToSky(griddedData2, sumWeight);
///  else                visResamplers_p.finalizeToSky(griddedData, sumWeight);
}



Array<Complex>* LofarFTMachine::getDataPointer(const IPosition& centerLoc2D,
				       Bool readonly) {
  Array<Complex>* result;
  // Is tiled: get tiles and set up offsets
  centerLoc(0)=centerLoc2D(0);
  centerLoc(1)=centerLoc2D(1);
  result=&imageCache->tile(offsetLoc,centerLoc, readonly);
  gridder->setOffset(IPosition(2, offsetLoc(0), offsetLoc(1)));
  return result;
}

void LofarFTMachine::put(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type)
{

  itsCyrilTimer.stop();
  //PrecTimer TimerCyril;
  //TimerCyril.start();

  if (itsVerbose > 0) {
    logIO() << LogOrigin("LofarFTMachine", "put") << LogIO::NORMAL
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




  //cout<<"... Gridding Spectral Window:    "<<vb.spectralWindow()<<", with Taylor Term: "<< thisterm_p<<endl;

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


  // // const Vector<Double>& times = vb.timeCentroid();
  // // double time = 0.5 * (times[times.size()-1] + times[0]);
  // const Vector<Double>& freq = vb.lsrFrequency();
  // const Vector<Int>& obs = vb.observationId();
  // Vector<Double> lsrFreq(0);
  // Bool condoo=False;
  // vb.lsrFrequency(0, lsrFreq, condoo);
  // cout<<"mmm " <<lsrFreq<<" "<<condoo<<endl;
  // vb.lsrFrequency(1, lsrFreq, condoo);
  // cout<<"mmmmm " <<lsrFreq<<" "<<condoo<<endl;
  //const Vector<Double>& timess = vb.timeCentroid();

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

  // Determine the baselines in the VisBuffer.
  const Vector<Int>& ant1 = vb.antenna1();
  const Vector<Int>& ant2 = vb.antenna2();
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  Vector<uInt> blIndex;
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
  vector<int> blStart, blEnd;
  blStart.reserve (nrant*(nrant+1)/2);
  blEnd.reserve   (nrant*(nrant+1)/2);
  Int  lastbl     = -1;
  Int  lastIndex  = 0;
  bool usebl      = false;
  bool allFlagged = true;
  const Vector<Bool>& flagRow = vb.flagRow();
  for (uint i=0; i<blnr.size(); ++i) {
    Int inx = blIndex[i];
    Int bl = blnr[inx];
    if (bl != lastbl) {
      // New baseline. Write the previous end index if applicable.
      if (usebl  &&  !allFlagged) {
	double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[i-1]](2)));
	if (abs(Wmean) <= itsWMax) {
	  if (itsVerbose > 1) {
	    cout<<"using w="<<Wmean<<endl;
	  }
	  blStart.push_back (lastIndex);
	  blEnd.push_back (i-1);
	}
      }

      // Skip auto-correlations and high W-values.
      // All w values are close, so if first w is too high, skip baseline.
      usebl = false;

      if (ant1[inx] != ant2[inx]) {
	usebl = true;
      }
      lastbl=bl;
      lastIndex=i;
    }
    // Test if the row is flagged.
    if (! flagRow[inx]) {
      allFlagged = false;
    }
  }
  // Write the last end index if applicable.
  if (usebl  &&  !allFlagged) {
    double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[blnr.size()-1]](2)));
    if (abs(Wmean) <= itsWMax) {
      if (itsVerbose > 1) {
	cout<<"...using w="<<Wmean<<endl;
      }
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size()-1);
    }
  }
  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double time = 0.5 * (times[times.size()-1] + times[0]);

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
  itsConvFunc->computeAterm (time);
  CyrilTimer2Aterm.stop();
  double Taterm=CyrilTimer2Aterm.getReal();

  uInt Nchannels = vb.nChannel();

  itsTotalTimer.start();

  vector< Bool> done;
  done.resize(int(blStart.size()));
  for(int i=0; i<int(blStart.size()); ++i) {done[i]=false;};

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
  {
    // Thread-private variables.
    PrecTimer gridTimer;
    PrecTimer cfTimer;
    PrecTimer CyrilTimer;
    // The for loop can be parallellized. This must be done dynamically,
    // because the execution times of iterations can vary greatly.


#pragma omp for schedule(dynamic)
    for (int i=0; i<int(blStart.size()); ++i) {
      Int ist  = blIndex[blStart[i]];
      Int iend = blIndex[blEnd[i]];
      if(done[i]==true){continue;};
      //if(doagain>0){
	//cout<<"Doing again (doagain) baseline: A1="<<ant1[ist]<<", A2="<<ant2[ist]<<endl;
      //}

      try{

      // compute average weight for baseline for CF averaging
	double average_weight=0.;
      uInt Nvis=0;
      for(Int j=ist; j<iend; ++j){
        uInt row=blIndex[j];
        if(!vbs.rowFlag()[row]){
          Nvis+=1;
          for(uInt k=0; k<Nchannels; ++k) {
            average_weight=average_weight+vbs.imagingWeight()(k,row);
          }
        }
      }
      if(Nvis>0){
	average_weight=average_weight/Nvis;
      } else {average_weight=0.;}
      ///        itsSumWeight += average_weight * average_weight;
      if (itsVerbose > 1) {
        cout<<"average weights= "<<average_weight<<", Nvis="<<Nvis<<endl;
      }

      int threadNum = OpenMP::threadNum();

      // Get the convolution function.
      if (itsVerbose > 1) {
        cout.precision(20);
        cout<<"A1="<<ant1[ist]<<", A2="<<ant2[ist]<<", time="<<fixed<<time<<endl;
      }
      //#pragma omp critical(LofarFTMachine_makeConvolutionFunction)
      //{
      CyrilTimer2conv.start();
      cfTimer.start();
      Double Wmean=0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend));
      //cout<< Wmean<<endl;
      LofarCFStore cfStore =
        itsConvFunc->makeConvolutionFunction (ant1[ist], ant2[ist], time,
                                              Wmean,
                                              itsGridMuellerMask, false,
                                              average_weight,
                                              itsSumPB[threadNum],
                                              itsSumCFWeight[threadNum],
					      spw,thisterm_p,itsRefFreq
					      );
      


      //cfTimer.stop();
      CyrilTimer2conv.stop();

      Int nConvX = (*(cfStore.vdata))[0][0][0].shape()[0];
      //cout<<ant1[ist]<<" "<<ant2[ist]<<" " <<nConvX/5<<endl;
      //double cfstep=CyrilTimer2conv.getReal();
      CyrilTimer2grid.start();
      if (useDoubleGrid_p) {
        visResamplers_p.lofarDataToGrid(itsGriddedData2[threadNum], vbs, blIndex,
                                        blStart[i], blEnd[i],
                                        itsSumWeight[threadNum], dopsf, cfStore);
      } else {
        if (itsVerbose > 1) {
          cout<<"  gridding"<<" thread="<<threadNum<<'('<<itsNThread<<"), A1="<<ant1[ist]<<", A2="<<ant2[ist]<<", time=" <<time<<endl;
        }
        gridTimer.start();
	if(!its_Use_Linear_Interp_Gridder){
	  //cout<<"itsGriddedData[threadNum] "<<itsGriddedData[threadNum].shape()<<endl;
	  visResamplers_p.lofarDataToGrid
	    (itsGriddedData[threadNum], vbs, blIndex, blStart[i],
	     blEnd[i], itsSumWeight[threadNum], dopsf, cfStore);
	} else{
	  visResamplers_p.lofarDataToGrid_linear
	    (itsGriddedData[threadNum], vbs, blIndex, blStart[i],
	     blEnd[i], itsSumWeight[threadNum], dopsf, cfStore);

	};
	  gridTimer.stop();
      }
      CyrilTimer2grid.stop();
      //cout<<"Gridding calculation: "<<nConvX<<" "<<cfstep<<" "<<CyrilTimer2grid.getReal()<<endl;
      //CyrilTimer2grid.reset();
      //CyrilTimer2conv.reset();
      //CyrilTimer.reset();
      done[i]=true;
      } catch (std::bad_alloc &)
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

  //cout<<"time: "<<time<<" "<<itsStepApplyElement<<" "<<its_Apply_Element<<endl;
  CyrilTimer2elem.start();

  if(itsCounterTimes==(itsStepApplyElement-1)/2){itsNextApplyTime=time;}
  if(its_Apply_Element){
    if((itsCounterTimes==itsStepApplyElement-1)||(lastchunk)){
      Array<Complex> tmp_stacked_GriddedData;
      tmp_stacked_GriddedData.resize (itsGriddedData[0].shape());
      tmp_stacked_GriddedData = Complex();
      SumGridsOMP(tmp_stacked_GriddedData, itsGriddedData);
      //itsConvFunc->MakeMaskDegrid(tmp_stacked_GriddedData, itsTotalStepsGrid);
      Array<Complex> tmp_stacked_GriddedData_appliedelement=itsConvFunc->ApplyElementBeam2 (tmp_stacked_GriddedData, itsNextApplyTime, spw, itsGridMuellerMask, false);
      if(its_UseMasksDegrid){
	itsConvFunc->MakeMaskDegrid(tmp_stacked_GriddedData_appliedelement, itsTotalStepsGrid);
      }
      SumGridsOMP(its_stacked_GriddedData, tmp_stacked_GriddedData_appliedelement);
      CyrilTimer2elem.stop();
      itsCounterTimes=0;
      for (int i=0; i<itsNThread; ++i) {
	itsGriddedData[i]=Complex();
      }
      itsTotalStepsGrid+=1;
    } else {
      itsCounterTimes+=1;
    }
  }

  CyrilTimer2elem.stop();
  //cout<<"times: aterm:"<<Taterm<<", conv: "<<CyrilTimer2conv.getReal()<<", grid: "<<CyrilTimer2grid.getReal()<<", gridconv: "<<Tgridconv<<", sum: "<<CyrilTimer2elem.getReal()<<", other: "<<itsCyrilTimer.getReal()<<endl;
  //cout<<"times: conv:"<<CyrilTimer2conv.getReal()<<", grid:"<<CyrilTimer2grid.getReal()<<", element:"<<CyrilTimer2elem.getReal()<<endl;
  itsTotalTimer.stop();
  itsCyrilTimer.reset();
  itsCyrilTimer.start();
}


// Degrid
void LofarFTMachine::get(VisBuffer& vb, Int row)
{
  if (itsVerbose > 0) {
    logIO() << LogOrigin("LofarFTMachine", "get") << LogIO::NORMAL
            << "I am degridding " << vb.nRow() << " row(s)."  << LogIO::POST;
    logIO() << LogIO::NORMAL << "Padding is " << padding_p  << LogIO::POST;
  }
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


  // Determine the baselines in the VisBuffer.
  const Vector<Int>& ant1 = vb.antenna1();
  const Vector<Int>& ant2 = vb.antenna2();
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  Vector<uInt> blIndex;
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
  vector<int> blStart, blEnd;
  blStart.reserve (nrant*(nrant+1)/2);
  blEnd.reserve   (nrant*(nrant+1)/2);
  Int  lastbl     = -1;
  Int  lastIndex  = 0;
  bool usebl      = false;
  bool allFlagged = true;
  const Vector<Bool>& flagRow = vb.flagRow();
  for (uint i=0; i<blnr.size(); ++i) 
  {
    Int inx = blIndex[i];
    Int bl = blnr[inx];
    if (bl != lastbl) 
    {
      // New baseline. Write the previous end index if applicable.
      if (usebl  &&  !allFlagged) 
      {
        double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[i-1]](2)));
        if (abs(Wmean) <= itsWMax) 
        {
          if (itsVerbose > 1) {
            cout<<"using w="<<Wmean<<endl;
          }
          blStart.push_back (lastIndex);
          blEnd.push_back (i-1);
        }
      }
      // Skip auto-correlations and high W-values.
      // All w values are close, so if first w is too high, skip baseline.
      usebl = false;

      if (ant1[inx] != ant2[inx]) {
        usebl = true;
      }
      lastbl=bl;
      lastIndex=i;
    }
    // Test if the row is flagged.
    if (! flagRow[inx]) {
      allFlagged = false;
    }
    
  }
  // Write the last end index if applicable.
  if (usebl  &&  !allFlagged) 
  {
    double Wmean(0.5*(vb.uvw()[blIndex[lastIndex]](2) + vb.uvw()[blIndex[blnr.size()-1]](2)));
    if (abs(Wmean) <= itsWMax) 
    {
      if (itsVerbose > 1) 
      {
        cout<<"...using w="<<Wmean<<endl;
      }
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size()-1);
    }
  }

  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.timeCentroid();
  double time = 0.5 * (times[times.size()-1] + times[0]);
  //ROVisIter& via(vb.iter());

  // First compute the A-terms for all stations (if needed).
  itsConvFunc->computeAterm (time);

  if(times[0]<itsTStartObs){itsTStartObs=times[0];}
  if(itsDeltaTime<(times[times.size()-1] - times[0])){itsDeltaTime=(times[times.size()-1] - times[0]);};
  if(itsDeltaTime<(times[times.size()-1] - times[0])){itsDeltaTime=(times[times.size()-1] - times[0]);};

  itsTotalTimer.start();

  vector< Bool> done;
  done.resize(int(blStart.size()));
  for(int i=0; i<int(blStart.size()); ++i) {done[i]=false;};

  Bool all_done(false);
  ///  Int Max_Num_Threads(itsNThread);
  ///  omp_set_num_threads(Max_Num_Threads);

  PrecTimer CyrilElement;
  CyrilElement.start();
  cout.precision(20);
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
    for (int i=0; i<int(blStart.size()); ++i) {
      // #pragma omp critical(LofarFTMachine_lofarGridToData)
      // {
      Int ist  = blIndex[blStart[i]];
      Int iend = blIndex[blEnd[i]];
      if(done[i]==true){continue;};
      try {
      int threadNum = OpenMP::threadNum();
      // Get the convolution function for degridding.
     if (itsVerbose > 1) {
        cout<<"ANTENNA "<<ant1[ist]<<" "<<ant2[ist]<<endl;
     }
      cfTimer.start();
      CyrilConv.start();

      LofarCFStore cfStore =
        itsConvFunc->makeConvolutionFunction (ant1[ist], ant2[ist], time,
                                              0.5*(vbs.uvw()(2,ist) + vbs.uvw()(2,iend)),
                                              itsDegridMuellerMask,
                                              true,
                                              0.0,
                                              itsSumPB[threadNum],
                                              itsSumCFWeight[threadNum]
					      ,spw,thisterm_p,itsRefFreq);
      cfTimer.stop();

      CyrilConv.stop();
      CyrilGrid.start();

      degridTimer.start();
      visResamplers_p.lofarGridToData(vbs, itsGridToDegrid,//its_stacked_GriddedData,//itsGriddedData[0],
                                      blIndex, blStart[i], blEnd[i], cfStore);
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



// Finalize the FFT to the Sky. Here we actually do the FFT and
// return the resulting image
ImageInterface<Complex>& LofarFTMachine::getImage(Matrix<Float>& weights, Bool normalize)
{
  //AlwaysAssert(lattice, AipsError);
  AlwaysAssert(gridder, AipsError);
  AlwaysAssert(image, AipsError);
  logIO() << LogOrigin("LofarFTMachine", "getImage") << LogIO::NORMAL;

  itsAvgPB.reference (itsConvFunc->Compute_avg_pb(itsSumPB[0], itsSumCFWeight[0]));

  //cout<<"weights.shape() "<<weights.shape()<<"  "<<sumWeight<<endl;

  weights.resize(itsSumWeight[0].shape());

  convertArray(weights, itsSumWeight[0]);
  // If the weights are all zero then we cannot normalize
  // otherwise we don't care.
  if(normalize&&max(weights)==0.0) {
    logIO() << LogIO::SEVERE << "No useful data in LofarFTMachine: weights all zero"
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
    if(useDoubleGrid_p) {
      ArrayLattice<DComplex> darrayLattice(itsGriddedData2[0]);
      LatticeFFT::cfft2d(darrayLattice,False);
      convertArray(itsGriddedData[0], itsGriddedData2[0]);
      //Don't need the double-prec grid anymore...
      ///griddedData2.resize();
    } else {
      LatticeFFT::cfft2d(*lattice, False);
    }

    if (itsVerbose > 0) {
      cout<<"POLMAP:::::::  "<<polMap<<endl;
      cout<<"POLMAP:::::::  "<<CFMap_p<<endl;
    }
    //cout<<"CFPOLMAP:::::::  "<<cfPolMap<<endl;
    //Int i,j,N = cfPolMap.nelements();
    //for(i=0;i<N;i++){
    //  if (cfPolMap[i] > -1){cout<<"cfPolMap[i]<<visStokes[i]"<<cfPolMap[i]<<" "<<visStokes[i]<<endl;};
    //};


    // Cyr: This does a normalisation by the number of pixel and spheroidal
    // function in the dirty image.
    // I have commented out the spheroidal normalisation (correctX1D part)
    {
      Int inx = lattice->shape()(0);
      Int iny = lattice->shape()(1);
      Vector<Complex> correction(inx);
      correction=Complex(1.0, 0.0);
      // Do the Grid-correction
      IPosition cursorShape(4, inx, 1, 1, 1);
      IPosition axisPath(4, 0, 1, 2, 3);
      LatticeStepper lsx(lattice->shape(), cursorShape, axisPath);
      LatticeIterator<Complex> lix(*lattice, lsx);
      for(lix.reset();!lix.atEnd();lix++) {
        Int pol=lix.position()(2);
	//cout<<"pol "<<pol<<endl;
        Int chan=lix.position()(3);
        if(weights(pol, chan)!=0.0) {
          //gridder->correctX1D(correction, lix.position()(1));
	  //cout<<"correction "<<correction<<endl;
          //lix.rwVectorCursor()/=correction;
          if(normalize) {
            Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
            lix.rwCursor()*=rnorm;
	    //cout<<"rnorm "<<rnorm<<endl;
          }
          else {
            Complex rnorm(Float(inx)*Float(iny));
            lix.rwCursor()*=rnorm;
	    //cout<<"rnorm "<<rnorm<<endl;
          }
        }
        else {
          lix.woCursor()=0.0;
        }
      }
    }

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // Normalising dirty image by the spheroidal function

    // String namei("sphe.img");
    // ostringstream name(namei);
    // PagedImage<Float> tmp(name.str().c_str());
    // Slicer slice(IPosition(4,0,0,0,0), tmp.shape(), IPosition(4,1,1,1,1));
    // Array<Float> data;
    // tmp.doGetSlice(data, slice);
    // IPosition posi(4,lattice->shape()[0],lattice->shape()[1],1,1);
    // IPosition posi2(4,lattice->shape()[0],lattice->shape()[1],1,1);
    // posi[2]=0.;
    // posi[3]=0.;
    // posi2[2]=0.;
    // posi2[3]=0.;
    // Int offset_pad(floor(data.shape()[0]-lattice->shape()[0])/2.);




    // for(uInt k=0;k<lattice->shape()[2];++k){
    //   for(uInt i=0;i<lattice->shape()[0];++i){
    // 	for(uInt j=0;j<lattice->shape()[0];++j){
    // 	  posi[0]=i;
    // 	  posi[1]=j;
    // 	  posi[2]=k;
    // 	  posi2[0]=i+offset_pad;
    // 	  posi2[1]=j+offset_pad;
    // 	  Complex pixel(lattice->getAt(posi));
    // 	  //pixel/=data(posi2);//*data(posi2);
    // 	  lattice->putAt(pixel,posi);
    // 	};
    //   };
    // };
    //====================================================================================================================
    //====================================================================================================================
    //====================================================================================================================
    // Cyr: Normalisation by the beam!!!!!
    //cout<<"lattice shape: "<<lattice->shape()<<endl;
    IPosition pos(4,lattice->shape()[0],lattice->shape()[1],1,1);
    uInt shapeout(floor(lattice->shape()[0]/padding_p));
    uInt istart(floor((lattice->shape()[0]-shapeout)/2.));
    Cube<Complex> tempimage(IPosition(3,shapeout,shapeout,lattice->shape()[2]));

    pos[3]=0.;
    double minPB(1e10);
    double maxPB(0.);
    for(uInt i=0;i<shapeout;++i){
      for(uInt j=0;j<shapeout;++j){
	double pixel(itsAvgPB(i+istart,j+istart));
	if(abs(pixel)>maxPB){maxPB=abs(pixel);};
	if(abs(pixel)<minPB){minPB=abs(pixel);};
      }
    }

    const Matrix<Float>& sphe = getSpheroidCut();

    //maxPB=1.;
    for(Int k=0;k<lattice->shape()[2];++k){
      for(uInt i=0;i<shapeout;++i){
    	for(uInt j=0;j<shapeout;++j){
    	  pos[0]=i+istart;
    	  pos[1]=j+istart;
    	  pos[2]=k;
    	  Complex pixel(lattice->getAt(pos));

    	  pixel*=sqrt(maxPB)/sqrt(itsAvgPB(i+istart,j+istart));


	  //if(itsAvgPB(pos)<1e-6*maxPB){pixel=0.;}
	  if((sqrt(itsAvgPB(pos))/sphe(pos)<its_PBCut)||(itsAvgPB(pos)<2.*minPB)){pixel=0.;}
    	  lattice->putAt(pixel,pos);
    	  tempimage(i,j,k)=pixel;///weights(0,0);
    	}
      }
    }

    // uInt count_cycle(0);
    // Bool written(false);

    // while(!written){
    //   Cube<Complex> tempimagePB(IPosition(3,shapeout,shapeout,lattice->shape()[2]));
    //   for(Int k=0;k<lattice->shape()[2];++k){
    // 	for(uInt i=0;i<shapeout;++i){
    // 	  for(uInt j=0;j<shapeout;++j){
    // 	    tempimagePB(i,j,k)=itsAvgPB(i,j);
    // 	  };
    // 	};
    //   };
    //   cout<<"count_cycle ======================= "<<count_cycle<<" "<<normalize<<endl;
    //   File myFile("Cube_dirty.img"+String::toString(count_cycle));
    //   if(!myFile.exists()){
    //   	written=true;
    //   	store(tempimage,"Cube_dirty.img"+String::toString(count_cycle));
    //   	store(tempimagePB,"Cube_dirty.img"+String::toString(count_cycle)+".pb");

    //   }
    //   else{
    //   	count_cycle++;
    //   };
    // };

    //====================================================================================================================
    //====================================================================================================================
    //====================================================================================================================



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

  logIO() << LogOrigin("LofarFTMachine", "getWeightImage") << LogIO::NORMAL;

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


  logIO() << LogOrigin("LofarFTMachine", "makeImage") << LogIO::NORMAL;

  if(type==FTMachine::COVERAGE) {
    logIO() << "Type COVERAGE not defined for Fourier transforms" << LogIO::EXCEPTION;
  }


  // Initialize the gradients
  ROVisIter& vi(vs.iter());

  // Loop over all visibilities and pixels
  VisBuffer vb(vi);

  // Initialize put (i.e. transform to Sky) for this model
  vi.origin();

  if(vb.polFrame()==MSIter::Linear) {
    StokesImageUtil::changeCStokesRep(theImage, SkyModel::LINEAR);
  }
  else {
    StokesImageUtil::changeCStokesRep(theImage, SkyModel::CIRCULAR);
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
    LogIO log_l(LogOrigin("LofarFTMachine", "normalizeAvgPB"));
    if (pbNormalized_p) return;
    IPosition inShape(inImage.shape()),ndx(4,0,0,0,0);
    Vector<Complex> peak(inShape(2));

    outImage.resize(inShape);
    outImage.setCoordinateInfo(inImage.coordinates());

    Bool isRefIn, isRefOut;
    Array<Complex> inBuf;
    Array<Float> outBuf;

    isRefIn  = inImage.get(inBuf);
    isRefOut = outImage.get(outBuf);
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
    LogIO log_l(LogOrigin("LofarFTMachine", "normalizeAvgPB"));
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



		// if (makingPSF)
		PBCorrection(i)=(avgPBSqVec(i)/avgPBVec(i));///(sincConv(i)*sincConv(iy));
		//		PBCorrection(i)=(avgPBSqVec(i));///(sincConv(i)*sincConv(iy));
		//		PBCorrection(i)=avgPBVec(i);///(sincConv(i)*sincConv(iy));

		// else
		//		PBCorrection(i)=(avgPBVec(i));//*sincConv(i)*sincConv(iy);
		//		if ((abs(avgPBSqVec(i))) >= pbLimit_p)
		if ((abs(avgPBVec(i))) >= pbLimit_p)
		  lix.rwVectorCursor()(i) /= PBCorrection(i);

		// if ((abs(PBCorrection(i))) >= pbLimit_p)
		//   lix.rwVectorCursor()(i) /= PBCorrection(i);
		// else if (!makingPSF)
		//   lix.rwVectorCursor()(i) /= sincConv(i)*sincConv(iy);


		// PBCorrection(i)=FUNC(avgPBVec(i)/avgPBSqVec(i))/(sincConv(i)*sincConv(iy));
		// lix.rwVectorCursor()(i) *= PBCorrection(i);

		// if ((abs(avgPBSqVec(i))) >= pbLimit_p)
		//   lix.rwVectorCursor()(i) *= PBCorrection(i);
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


  void LofarFTMachine::makeCFPolMap(const VisBuffer& vb, const Vector<Int>& locCfStokes,
				 Vector<Int>& polM)
  {
    LogIO log_l(LogOrigin("LofarFTMachine", "findPointingOffsets"));
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

} //# end namespace
