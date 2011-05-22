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
#include <Common/LofarLogger.h>
#include <Common/Exception.h>
#include <Common/OpenMP.h>
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
#include <casa/OS/Timer.h>
#include <casa/sstream.h>
#define DORES True

using namespace casa;

namespace LOFAR { //# NAMESPACE CASA - BEGIN

  LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize, 
                                 CountedPtr<VisibilityResamplerBase>&,
                                 String iconvType, Float padding,
                                 Bool usezero, Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize), tilesize(itilesize),
  gridder(0), isTiled(False), convType(iconvType),
  maxAbsData(0.0), centerLoc(IPosition(4,0)), offsetLoc(IPosition(4,0)),
  usezero_p(usezero), noPadding_p(False), usePut2_p(False), 
  machineName_p("LofarFTMachine")

{
//   LOG_INFO ("LofarFTMachine::LofarFTMachine" << 1.0);
//   logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  useDoubleGrid_p=useDoublePrec;  
  canComputeResiduals_p=DORES;
}

  LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize, 
		   CountedPtr<VisibilityResamplerBase>&, String iconvType,
                                 const MeasurementSet& ms, Int nwPlanes,
		   MPosition mLocation, Float padding, Bool usezero, 
		   Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False), 
  usePut2_p(False), machineName_p("LofarFTMachine"), itsMS(ms), itsNWPlanes(nwPlanes), itsConvFunc(0)
{
  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mLocation_p=mLocation;
  tangentSpecified_p=False;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;

  // Create as many resamplers as there are possible threads.
  ///  visResamplers_p.resize (OpenMP::maxThreads());
}

LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize, 
		 CountedPtr<VisibilityResamplerBase>&, 
		 String iconvType,
		 MDirection mTangent, Float padding, Bool usezero, Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False), 
  usePut2_p(False), machineName_p("LofarFTMachine")
{
  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mTangent_p=mTangent;
  tangentSpecified_p=True;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;
}

LofarFTMachine::LofarFTMachine(Long icachesize, Int itilesize, 
		 CountedPtr<VisibilityResamplerBase>&, 
		 String iconvType, MPosition mLocation, MDirection mTangent, Float padding,
		 Bool usezero, Bool useDoublePrec)
: FTMachine(), padding_p(padding), imageCache(0), cachesize(icachesize),
  tilesize(itilesize), gridder(0), isTiled(False), convType(iconvType), maxAbsData(0.0), centerLoc(IPosition(4,0)),
  offsetLoc(IPosition(4,0)), usezero_p(usezero), noPadding_p(False), 
  usePut2_p(False),machineName_p("LofarFTMachine")
{
  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  mLocation_p=mLocation;
  mTangent_p=mTangent;
  tangentSpecified_p=True;
  useDoubleGrid_p=useDoublePrec;
  canComputeResiduals_p=DORES;
}

LofarFTMachine::LofarFTMachine(const RecordInterface& stateRec)
  : FTMachine()
{
  // Construct from the input state record
  logIO() << LogOrigin("LofarFTMachine", "LofarFTMachine(RecordInterface)")  << LogIO::NORMAL;
  logIO() << "You are using a non-standard FTMachine" << LogIO::WARN << LogIO::POST;
  String error;
  if (!fromRecord(error, stateRec)) 
    throw (AipsError("Failed to create gridder: " + error));
  canComputeResiduals_p=DORES;
}

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
  }
  return *this;
};

//----------------------------------------------------------------------
  LofarFTMachine::LofarFTMachine(const LofarFTMachine& other) : FTMachine(), machineName_p("LofarFTMachine")
  {
    //  visResampler_p.init(useDoubleGrid_p);
    operator=(other);
  }

//----------------------------------------------------------------------
//  CountedPtr<LofarFTMachine> LofarFTMachine::clone()
  LofarFTMachine* LofarFTMachine::clone()
  {
    LofarFTMachine* newftm = new LofarFTMachine(*this);
    return newftm;
  }

//----------------------------------------------------------------------
void LofarFTMachine::init() {

  logIO() << LogOrigin("LofarFTMachine", "init")  << LogIO::NORMAL;
  canComputeResiduals_p = DORES;
  ok();

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

  sumWeight.resize(npol, nchan);

  uvScale.resize(2);
  uvScale(0)=(Float(nx)*image->coordinates().increment()(0)); 
  uvScale(1)=(Float(ny)*image->coordinates().increment()(1)); 
  uvOffset.resize(2);
  uvOffset(0)=nx/2;
  uvOffset(1)=ny/2;

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
    
  itsWMax=2000.;// Set WMax
  itsConvFunc = new LofarConvolutionFunction(image->shape(),
                                             image->coordinates().directionCoordinate (image->coordinates().findCoordinate(Coordinate::DIRECTION)),
                                             itsMS, itsNWPlanes, itsWMax, 8);

  // Set up image cache needed for gridding. For BOX-car convolution
  // we can use non-overlapped tiles. Otherwise we need to use
  // overlapped tiles and additive gridding so that only increments
  // to a tile are written.

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
}

// This is nasty, we should use CountedPointers here.
LofarFTMachine::~LofarFTMachine() {
  if(imageCache) delete imageCache; imageCache=0;
  //if(arrayLattice) delete arrayLattice; arrayLattice=0;
  if(gridder) delete gridder; gridder=0;
  delete itsConvFunc;
}

// Initialize for a transform from the Sky domain. This means that
// we grid-correct, and FFT the image

void LofarFTMachine::initializeToVis(ImageInterface<Complex>& iimage,
                                     const VisBuffer& vb)
{
  image=&iimage;

  ok();

  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);

  visResamplers_p.init(useDoubleGrid_p);
  visResamplers_p.setMaps(chanMap, polMap);

  // Need to reset nx, ny for padding
  // Padding is possible only for non-tiled processing
  

  // If we are memory-based then read the image in and create an
  // ArrayLattice otherwise just use the PagedImage
  if(isTiled) {
    lattice=CountedPtr<Lattice<Complex> >(image, False);
  }
  else {
     IPosition gridShape(4, nx, ny, npol, nchan);
     griddedData.resize(gridShape);
     //griddedData can be a reference of image data...if not using model col
     //hence using an undocumented feature of resize that if 
     //the size is the same as old data it is not changed.
     //if(!usePut2_p) griddedData.set(0);
     griddedData.set(Complex(0.0));

     IPosition stride(4, 1);
     IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
     IPosition trc(blc+image->shape()-stride);

     IPosition start(4, 0);
     griddedData(blc, trc) = image->getSlice(start, image->shape());

     //if(arrayLattice) delete arrayLattice; arrayLattice=0;
     arrayLattice = new ArrayLattice<Complex>(griddedData);
     lattice=arrayLattice;
  }

  //AlwaysAssert(lattice, AipsError);

  logIO() << LogIO::DEBUGGING
	  << "Starting grid correction and FFT of image" << LogIO::POST;

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
  
    // Now do the FFT2D in place
    LatticeFFT::cfft2d(*lattice);
    
    logIO() << LogIO::DEBUGGING
	    << "Finished grid correction and FFT of image" << LogIO::POST;
    
}




void LofarFTMachine::finalizeToVis()
{
  if(isTiled) {

    logIO() << LogOrigin("LofarFTMachine", "finalizeToVis")  << LogIO::NORMAL;

    AlwaysAssert(imageCache, AipsError);
    AlwaysAssert(image, AipsError);
    ostringstream o;
    imageCache->flush();
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }
}


// Initialize the FFT to the Sky. Here we have to setup and initialize the
// grid. 
void LofarFTMachine::initializeToSky(ImageInterface<Complex>& iimage,
			     Matrix<Float>& weight, const VisBuffer& vb)
{
  // image always points to the image
  image=&iimage;

  init();

  // Initialize the maps for polarization and channel. These maps
  // translate visibility indices into image indices
  initMaps(vb);

  visResamplers_p.init(useDoubleGrid_p);
  visResamplers_p.setMaps(chanMap, polMap);

  sumWeight=0.0;
  weight.resize(sumWeight.shape());
  weight=0.0;

  // Initialize for in memory or to disk gridding. lattice will
  // point to the appropriate Lattice, either the ArrayLattice for
  // in memory gridding or to the image for to disk gridding.
  if(isTiled) {
    imageCache->flush();
    image->set(Complex(0.0));
    lattice=CountedPtr<Lattice<Complex> >(image, False);
  }
  else {
    IPosition gridShape(4, nx, ny, npol, nchan);
    griddedData.resize(gridShape);
    griddedData=Complex(0.0);
    if(useDoubleGrid_p){
      griddedData2.resize(gridShape);
      griddedData2=DComplex(0.0);
    }
    //iimage.get(griddedData, False);
    //if(arrayLattice) delete arrayLattice; arrayLattice=0;
    arrayLattice = new ArrayLattice<Complex>(griddedData);
    lattice=arrayLattice;
  }
  // if(useDoubleGrid_p) visResampler_p->initializePutBuffers(griddedData2, sumWeight);
  // else                visResampler_p->initializePutBuffers(griddedData, sumWeight);
  if(useDoubleGrid_p) visResamplers_p.initializeToSky(griddedData2, sumWeight);
  else                visResamplers_p.initializeToSky(griddedData, sumWeight);
  //AlwaysAssert(lattice, AipsError);
}



void LofarFTMachine::finalizeToSky()
{  
  //AlwaysAssert(lattice, AipsError);
  // Now we flush the cache and report statistics
  // For memory based, we don't write anything out yet.
  if(isTiled) {
    logIO() << LogOrigin("LofarFTMachine", "finalizeToSky")  << LogIO::NORMAL;

    AlwaysAssert(image, AipsError);
    AlwaysAssert(imageCache, AipsError);
    imageCache->flush();
    ostringstream o;
    imageCache->showCacheStatistics(o);
    logIO() << o.str() << LogIO::POST;
  }
  // if(useDoubleGrid_p) visResamplers_p[0].GatherGrids(griddedData2, sumWeight);
  // else                visResamplers_p[0].GatherGrids(griddedData, sumWeight);
  if(useDoubleGrid_p) visResamplers_p.finalizeToSky(griddedData2, sumWeight);
  else                visResamplers_p.finalizeToSky(griddedData, sumWeight);
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

  logIO() << LogOrigin("LofarFTMachine", "put") << 
     LogIO::NORMAL << "I am gridding " << vb.nRow() << " row(s)."  << LogIO::POST;

  gridOk(gridder->cSupport()(0));

  //Check if ms has changed then cache new spw and chan selection
  if(vb.newMS())   matchAllSpwChans(vb);
  
  //Here we redo the match or use previous match
  
  //Channel matching for the actual spectral window of buffer
  if(doConversion_p[vb.spectralWindow()])
    matchChannel(vb.spectralWindow(), vb);
  else
    {
      chanMap.resize();
      chanMap=multiChanMap_p[vb.spectralWindow()];
    }

  //No point in reading data if its not matching in frequency
  if(max(chanMap)==-1) return;

  const Matrix<Float> *imagingweight;
  imagingweight=&(vb.imagingWeight());
  
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
  }
   
  rotateUVW(uvw, dphase, vb);
  refocus(uvw, vb.antenna1(), vb.antenna2(), dphase, vb);

  // Set up VBStore object to point to the relevant info of the VB.
  LofarVBStore vbs;

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
	  cout<<"using w="<<Wmean<<endl;
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
      cout<<"...using w="<<Wmean<<endl;
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size()-1);
    }
  }
  // Determine the time center of this data chunk.
  const Vector<Double>& times = vb.time();
  double time = 0.5 * (times[times.size()-1] - times[0]);
  
  // Determine the terms of the Mueller matrix that should be calculated
  IPosition shape_data(2, 4,4);
  Matrix<bool> Mask_Mueller(shape_data,0.);
  for(uInt i=0; i<4; ++i){Mask_Mueller(i,i)=1;};

  vbs.nRow_p = vb.nRow();

  vbs.uvw_p.reference(uvw);
  vbs.imagingWeight_p.reference(elWeight);
  vbs.visCube_p.reference(data);
  vbs.freq_p.reference(interpVisFreq_p);
  vbs.rowFlag_p.reference(vb.flagRow());  

  // Really nice way of converting a Cube<Int> to Cube<Bool>.
  // However the VBS objects should ultimately be references
  // directly to bool cubes.
  //**************
  vbs.flagCube_p.resize(flags.shape());    vbs.flagCube_p = False; vbs.flagCube_p(flags!=0) = True;
  //  vbs.flagCube_p.reference(vb.flagCube());
  //**************

  visResamplers_p.setParams(uvScale,uvOffset,dphase);
  visResamplers_p.setMaps(chanMap, polMap);
    
    
  ///#pragma omp parallel
    {
      // Thread-private variables.
      // The for loop can be parallellized. This must be done dynamically,
      // because the execution times of iterations can vary.
  ///#pragma omp for schedule(dynamic)
      for (uint i=0; i<blStart.size(); ++i) {
        // NOTE: vbs assign below will not work if OpenMP is switched on.
        // Then need to pass in as function arguments.
        vbs.beginRow_p = blStart[i];
        vbs.endRow_p = blEnd[i];
        Int ist  = blIndex[blStart[i]];
        Int iend = blIndex[blEnd[i]];
        // Get the convolution function.
        LofarCFStore cfStore =
          itsConvFunc->makeConvolutionFunction (ant1[ist], ant2[ist], time,
                                    0.5*(vb.uvw()[ist](2) + vb.uvw()[iend](2)),
                                            Mask_Mueller);
        //Double or single precision gridding.
        if (useDoubleGrid_p) {
          visResamplers_p.lofarDataToGrid(griddedData2, vbs, blIndex, sumWeight, dopsf, cfStore);
        } else {
          visResamplers_p.lofarDataToGrid(griddedData, vbs, blIndex, sumWeight, dopsf, cfStore); 
        }
      }
    } // end omp parallel
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
  else 
    {
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
      
      visResamplers_p.setParams(uvScale,uvOffset,dphase);
      visResamplers_p.setMaps(chanMap, polMap);

      // De-gridding
      ////      visResamplers_p.GridToData(vbs, griddedData);
    }
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

  weights.resize(sumWeight.shape());

  convertArray(weights, sumWeight);
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
    if(useDoubleGrid_p)
      {
	ArrayLattice<DComplex> darrayLattice(griddedData2);
	LatticeFFT::cfft2d(darrayLattice,False);
	convertArray(griddedData, griddedData2);
	//Don't need the double-prec grid anymore...
	griddedData2.resize();
      }
    else
      LatticeFFT::cfft2d(*lattice,False);

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
	Int chan=lix.position()(3);
	if(weights(pol, chan)!=0.0) {
	  gridder->correctX1D(correction, lix.position()(1));
	  lix.rwVectorCursor()/=correction;
	  if(normalize) {
	    Complex rnorm(Float(inx)*Float(iny)/weights(pol,chan));
	    lix.rwCursor()*=rnorm;
	  }
	  else {
	    Complex rnorm(Float(inx)*Float(iny));
	    lix.rwCursor()*=rnorm;
	  }
	}
	else {
	  lix.woCursor()=0.0;
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
      image->put(griddedData(blc, trc));
    }
  }
    
  return *image;
}

// Get weight image
void LofarFTMachine::getWeightImage(ImageInterface<Float>& weightImage, Matrix<Float>& weights) 
{

  logIO() << LogOrigin("LofarFTMachine", "getWeightImage") << LogIO::NORMAL;

  weights.resize(sumWeight.shape());
  convertArray(weights,sumWeight);

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
  outRec.define("sumofweights", sumWeight);
  if(withImage && image){ 
    ImageInterface<Complex>& tempimage(*image);
    Record imageContainer;
    String error;
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
  inRec.get("sumofweights", sumWeight);
  if(inRec.nfields() > 12 ){
    Record imageAsRec=inRec.asRecord("image");
    if(!image) { 
      image= new TempImage<Complex>(); 
    };
    String error;
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
      griddedData.resize(gridShape);
      griddedData=Complex(0.0);
      IPosition blc(4, (nx-image->shape()(0)+(nx%2==0))/2, (ny-image->shape()(1)+(ny%2==0))/2, 0, 0);
      IPosition start(4, 0);
      IPosition stride(4, 1);
      IPosition trc(blc+image->shape()-stride);
      griddedData(blc, trc)=image->getSlice(start, image->shape());
      
      //if(arrayLattice) delete arrayLattice; arrayLattice=0;
      arrayLattice = new ArrayLattice<Complex>(griddedData);
      lattice=arrayLattice;
    }

    //AlwaysAssert(lattice, AipsError);
    AlwaysAssert(gridder, AipsError);
    AlwaysAssert(image, AipsError);
  };
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
	vb.visCube()=vb.correctedVisCube();
	vb.visCube()-=vb.modelVisCube();
        put(vb, -1, False);
        break;
      case FTMachine::MODEL:
	vb.visCube()=vb.modelVisCube();
        put(vb, -1, False);
        break;
      case FTMachine::CORRECTED:
	vb.visCube()=vb.correctedVisCube();
        put(vb, -1, False);
        break;
      case FTMachine::PSF:
	vb.visCube()=Complex(1.0,0.0);
        put(vb, -1, True);
        break;
      case FTMachine::OBSERVED:
      default:
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
  LofarVBStore vbs;
  vbs.nRow_p = vb.nRow();
  vbs.modelCube_p.reference(vb.modelVisCube());
  if (useCorrected) vbs.correctedCube_p.reference(vb.correctedVisCube());
  else vbs.visCube_p.reference(vb.visCube());
  vbs.useCorrected_p = useCorrected;
  visResamplers_p.lofarComputeResiduals(vbs);
}

} //# end namespace

