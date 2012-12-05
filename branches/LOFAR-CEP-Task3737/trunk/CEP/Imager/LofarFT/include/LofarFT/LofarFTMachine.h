//# LofarFTMachine.h: Definition for LofarFTMachine
//# Copyright (C) 1996,1997,1998,1999,2000,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef LOFARFT_LOFARFTMACHINE_H
#define LOFARFT_LOFARFTMACHINE_H

#include <synthesis/MeasurementComponents/FTMachine.h>
#include <casa/OS/File.h>
#include <casa/OS/PrecTimer.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <LofarFT/LofarCFStore.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/FFTServer.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <images/Images/ImageInterface.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <casa/OS/Timer.h>

#include <fftw3.h>
#include <scimath/Mathematics/FFTServer.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/BasicSL/Complex.h>
#include <casa/OS/Timer.h>

#include <Common/OpenMP.h>

using namespace casa;

namespace LOFAR {

// <summary>  An FTMachine for Gridded Fourier transforms </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" tests="" demos="">

// <prerequisite>
//   <li> <linkto class=FTMachine>FTMachine</linkto> module
//   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
//   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
// </prerequisite>
//
// <etymology>
// FTMachine is a Machine for Fourier Transforms. LofarFTMachine does
// Grid-based Fourier transforms.
// </etymology>
//
// <synopsis>
// The <linkto class=SkyEquation>SkyEquation</linkto> needs to be able
// to perform Fourier transforms on visibility data. LofarFTMachine
// allows efficient Fourier Transform processing using a
// <linkto class=VisBuffer>VisBuffer</linkto> which encapsulates
// a chunk of visibility (typically all baselines for one time)
// together with all the information needed for processing
// (e.g. UVW coordinates).
//
// Gridding and degridding in LofarFTMachine are performed using a
// novel sort-less algorithm. In this approach, the gridded plane is
// divided into small patches, a cache of which is maintained in memory
// using a general-purpose <linkto class=LatticeCache>LatticeCache</linkto> class. As the (time-sorted)
// visibility data move around slowly in the Fourier plane, patches are
// swapped in and out as necessary. Thus, optimally, one would keep at
// least one patch per baseline.
//
// A grid cache is defined on construction. If the gridded uv plane is smaller
// than this, it is kept entirely in memory and all gridding and
// degridding is done entirely in memory. Otherwise a cache of tiles is
// kept an paged in and out as necessary. Optimally the cache should be
// big enough to hold all polarizations and frequencies for all
// baselines. The paging rate will then be small. As the cache size is
// reduced below this critical value, paging increases. The algorithm will
// work for only one patch but it will be very slow!
//
// This scheme works well for arrays having a moderate number of
// antennas since the saving in space goes as the ratio of
// baselines to image size. For the ATCA, VLBA and WSRT, this ratio is
// quite favorable. For the VLA, one requires images of greater than
// about 200 pixels on a side to make it worthwhile.
//
// The FFT step is done plane by plane for images having less than
// 1024 * 1024 pixels on each plane, and line by line otherwise.
//
// The gridding and degridding steps are implemented in Fortran
// for speed. In gridding, the visibilities are added onto the
// grid points in the neighborhood using a weighting function.
// In degridding, the value is derived by a weight summ of the
// same points, using the same weighting function.
// </synopsis>
//
// <example>
// See the example for <linkto class=SkyModel>SkyModel</linkto>.
// </example>
//
// <motivation>
// Define an interface to allow efficient processing of chunks of
// visibility data
// </motivation>
//
// <todo asof="97/10/01">
// <ul> Deal with large VLA spectral line case
// </todo>

class LofarFTMachine : public casa::FTMachine {
public:

  // Constructor: cachesize is the size of the cache in words
  // (e.g. a few million is a good number), tilesize is the
  // size of the tile used in gridding (cannot be less than
  // 12, 16 works in most cases), and convType is the type of
  // gridding used (SF is prolate spheriodal wavefunction,
  // and BOX is plain box-car summation). mLocation is
  // the position to be used in some phase rotations. If
  // mTangent is specified then the uvw rotation is done for
  // that location iso the image center.
  // <group>
//  LofarFTMachine(Long cachesize, Int tilesize, CountedPtr<VisibilityResamplerBase>& visResampler,
//	  String convType="SF", Float padding=1.0, Bool usezero=True, Bool useDoublePrec=False);
  LofarFTMachine(Long cachesize, Int tilesize,  CountedPtr<VisibilityResamplerBase>& visResampler, String convType, const MeasurementSet& ms,
                 Int nwPlanes,
                 MPosition mLocation, Float padding, Bool usezero,
                 Bool useDoublePrec, double wmax,
                 Int verbose,
                 Int maxsupport,
                 Int oversample,
                 const String& imageName,
                 const Matrix<Bool>& gridMuellerMask,
                 const Matrix<Bool>& degridMuellerMask,
		 Double RefFreq,
		 Bool Use_Linear_Interp_Gridder,
		 Bool Use_EJones,
		 int StepApplyElement,
		 int ApplyBeamCode,
		 Double PBCut,
		 Bool PredictFT,
		 String PsfOnDisk,
		 Bool UseMasksDegrid,
		 Bool ReallyDoPSF,
		 Double UVmin,
		 Double UVmax,
		 Bool MakeDirtyCorr,
                 const casa::Record& parameters
                );//,
		 //Double FillFactor);
//  LofarFTMachine(Long cachesize, Int tilesize,  CountedPtr<VisibilityResamplerBase>& visResampler,String convType,
//	 MDirection mTangent, Float padding=1.0, Bool usezero=True,
//	 Bool useDoublePrec=False);
//  LofarFTMachine(Long cachesize, Int tilesize,  CountedPtr<VisibilityResamplerBase>& visResampler,String convType,
//	 MPosition mLocation, MDirection mTangent, Float passing=1.0,
//	 Bool usezero=True, Bool useDoublePrec=False);
  // </group>

  // Construct from a Record containing the LofarFTMachine state
//  LofarFTMachine(const RecordInterface& stateRec);

  // Copy constructor
  LofarFTMachine(const LofarFTMachine &other);

  // Assignment operator
  LofarFTMachine &operator=(const LofarFTMachine &other);

  // Empty constructor. Allows the copy constructor of derived classes to create
  // an empty ftmachine and then call the assigment operator to fill it.
  LofarFTMachine() {};

  // Clone
  LofarFTMachine* clone() const;


  ~LofarFTMachine();

  // Show the relative timings of the various steps.
  void showTimings (std::ostream&, double duration) const;

  // Initialize transform to Visibility plane using the image
  // as a template. The image is loaded and Fourier transformed.
  void initializeToVis(ImageInterface<Complex>& image,
		       const VisBuffer& vb);

  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void finalizeToVis();
  void getSplitWplanes(VisBuffer& vb, Int row);
  void getTraditional(VisBuffer& vb, Int row);

  void putSplitWplanesOverlap(const VisBuffer& vb, Int row, Bool dopsf,
			      FTMachine::Type type);
  void putSplitWplanes(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type);
  void putTraditional(const VisBuffer& vb, Int row, Bool dopsf,
                         FTMachine::Type type);

  // Initialize transform to Sky plane: initializes the image
  void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
		       const VisBuffer& vb);


  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. DOES NOT
  // DO THE FINAL TRANSFORM!
  using casa::FTMachine::finalizeToSky;
  void finalizeToSky();


  // Get actual coherence from grid by degridding
  void get(VisBuffer& vb, Int row=-1);


  // Put coherence to grid by gridding.
  using casa::FTMachine::put;
  void put(const VisBuffer& vb, Int row=-1, Bool dopsf=False,
           FTMachine::Type type=FTMachine::OBSERVED);

  mutable Matrix<Float> itsAvgPB;
  Bool its_Use_Linear_Interp_Gridder;
  Bool its_UseWSplit;

  // Make the entire image
  using casa::FTMachine::makeImage;
  void makeImage(FTMachine::Type type,
		 VisSet& vs,
		 ImageInterface<Complex>& image,
		 Matrix<Float>& weight);

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);

  // Get the average primary beam.
  virtual const Matrix<Float>& getAveragePB() const;

  // Get the spheroidal cut.
  const Matrix<Float>& getSpheroidCut() const
    { return itsConvFunc->getSpheroidCut(); }

  Matrix<Float> getAverageResponse() const;

  ///  virtual void normalizeImage(Lattice<Complex>& skyImage,
  ///			      const Matrix<Double>& sumOfWts,
  ///			      Lattice<Float>& sensitivityImage,
  ///			      Bool fftNorm)
  ///    {throw(AipsError("LofarFTMachine::normalizeImage() called"));}

  void normalizeAvgPB();
  void normalizeAvgPB(ImageInterface<Complex>& inImage,
                      ImageInterface<Float>& outImage);
    //
    // Make a sensitivity image (sensitivityImage), given the gridded
    // weights (wtImage).  These are related to each other by a
    // Fourier transform and normalization by the sum-of-weights
    // (sumWt) and normalization by the product of the 2D FFT size
    // along each axis.  If doFFTNorm=False, normalization by the FFT
    // size is not done.  If sumWt is not provided, normalization by
    // the sum of weights is also not done.
    //



    virtual void makeSensitivityImage(Lattice<Complex>&,
				      ImageInterface<Float>&,
				      const Matrix<Float>& =Matrix<Float>(),
				      const Bool& =True) {}
    virtual void makeSensitivityImage(const VisBuffer& vb, const ImageInterface<Complex>& imageTemplate,
				      ImageInterface<Float>& sensitivityImage);

    inline virtual Float pbFunc(const Float& a, const Float& limit)
    {if (abs(a) >= limit) return (a);else return 1.0;};
    inline virtual Complex pbFunc(const Complex& a, const Float& limit)
    {if (abs(a)>=limit) return (a); else return Complex(1.0,0.0);};
    //
    // Given the sky image (Fourier transform of the visibilities),
    // sum of weights and the sensitivity image, this method replaces
    // the skyImage with the normalized image of the sky.
    //
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Double>& sumOfWts,
				Lattice<Float>& sensitivityImage,
				Bool fftNorm=True);
    virtual void normalizeImage(Lattice<Complex>& skyImage,
				const Matrix<Double>& sumOfWts,
				Lattice<Float>& sensitivityImage,
				Lattice<Complex>& sensitivitySqImage,
				Bool fftNorm=True);

    virtual ImageInterface<Float>& getSensitivityImage() {return *avgPB_p;}
    virtual Matrix<Double>& getSumOfWeights() {return sumWeight;};
    virtual Matrix<Double>& getSumOfCFWeights() {return sumCFWeight;};

  // Get the final weights image
  void getWeightImage(ImageInterface<Float>&, Matrix<Float>&);

  // Save and restore the LofarFTMachine to and from a record
  virtual Bool toRecord(String& error, RecordInterface& outRec,
			Bool withImage=False);
  virtual Bool fromRecord(String& error, const RecordInterface& inRec);

  // Can this FTMachine be represented by Fourier convolutions?
  virtual Bool isFourier() {return True;}

  virtual void setNoPadding(Bool nopad){noPadding_p=nopad;};

  virtual String name();
  //virtual void setMiscInfo(const Int qualifier){(void)qualifier;};

  //Cyr: The FTMachine has got to know the order of the Taylor term
  virtual void setMiscInfo(const Int qualifier){thisterm_p=qualifier;};
  virtual void ComputeResiduals(VisBuffer&vb, Bool useCorrected);


  void makeConjPolMap(const VisBuffer& vb, const Vector<Int> cfPolMap, Vector<Int>& conjPolMap);
  //    Vector<Int> makeConjPolMap(const VisBuffer& vb);
  void makeCFPolMap(const VisBuffer& vb, const Vector<Int>& cfstokes, Vector<Int>& polM);

  String itsNamePsfOnDisk;
  vector< vector< vector < Matrix<Complex> > > > itsStackMuellerNew;

  void setPsfOnDisk(String NamePsf){itsNamePsfOnDisk=NamePsf;}
  virtual String GiveNamePsfOnDisk(){return itsNamePsfOnDisk;}

    // Arrays for non-tiled gridding (one per thread).

  void initGridThreads(vector< Array<Complex> >&  otherGriddedData, vector< Array<DComplex> >&  otherGriddedData2)
  {

    itsGriddedData=&otherGriddedData;
    itsGriddedData2=&otherGriddedData2;
    if(itsParameters.asBool("SingleGridMode")==false){
      (*itsGriddedData).resize (itsNThread);
      (*itsGriddedData2).resize (itsNThread);
    } else {
      (*itsGriddedData).resize (1);
      (*itsGriddedData2).resize (1);
    }

    /* itsGriddedData.resize(otherGriddedData.size()); */
    /* for(uInt i=0; i<itsGriddedData.size()){ */
    /*   itsGriddedData[i].reference(otherGriddedData[i]); */
    /* } */
  }


  Matrix<Bool> itsMaskGridCS;
  Matrix<Bool> GiveMaskGrid(){
    return itsMaskGridCS;
  }

protected:
  vector< Array<Complex> >*  itsGriddedData;
  vector< Array<DComplex> >*  itsGriddedData2;
  // Padding in FFT
  Float padding_p;
  Int thisterm_p;
  Double itsRefFreq;
  Bool itsPredictFT;
  double its_tot_time_grid;
  double its_tot_time_cf;
  double its_tot_time_w;
  double its_tot_time_el;
  double its_tot_time_tot;

  Int itsTotalStepsGrid;
  Int itsTotalStepsDeGrid;
  Bool itsMasksGridAllDone;
  Bool itsMasksAllDone;
  Bool itsAllAtermsDone;
  Bool its_UseMasksDegrid;
  Bool its_SingleGridMode;
  Bool its_makeDirtyCorr;
  uInt its_NGrids;

  Timer itsSeconds;
  //Float its_FillFactor;
  // Get the appropriate data pointer
  Array<Complex>* getDataPointer(const IPosition&, Bool);

  void ok();

  void init();

  // Is this record on Grid? check both ends. This assumes that the
  // ends bracket the middle
  Bool recordOnGrid(const VisBuffer& vb, Int rownr) const;

  // Image cache
  LatticeCache<Complex> * imageCache;

  // Sizes
  Long cachesize;
  Int  tilesize;

  // Gridder
  ConvolveGridder<Double, Complex>* gridder;

  //Sum Grids
  template <class T>
  void SumGridsOMP(Array<T>& grid, const Array<T>& GridToAdd){
    int y,ch,pol;
    int GridSize(grid.shape()[0]);
    int NPol(grid.shape()[2]);
    int NChan(grid.shape()[3]);
    T* gridPtr;
    const T* GridToAddPtr;

#pragma omp parallel for private(y,ch,pol,gridPtr,GridToAddPtr)
    for(int x=0 ; x<grid.shape()[0] ; ++x){
      for(ch=0 ; ch<NChan ; ++ch){
	for(pol=0 ; pol<NPol ; ++pol){
	  gridPtr = grid.data() + ch*NPol*GridSize*GridSize + pol*GridSize*GridSize+x*GridSize;
	  GridToAddPtr = GridToAdd.data() + ch*NPol*GridSize*GridSize + pol*GridSize*GridSize+x*GridSize;
	  for(y=0 ; y<grid.shape()[1] ; ++y){
	    (*gridPtr++) += *GridToAddPtr++;
	    //gridPtr++;
	    //GridToAddPtr++;
	  }
	}
      }
    }

  }

  template <class T>
  void SumGridsOMP(Array<T>& grid, const vector< Array<T> >& GridToAdd0 ){

    for(uInt vv=0; vv<GridToAdd0.size();vv++){
      Array<T> GridToAdd(GridToAdd0[vv]);
      int y,ch,pol;
      int GridSize(grid.shape()[0]);
      int NPol(grid.shape()[2]);
      int NChan(grid.shape()[3]);
      T* gridPtr;
      const T* GridToAddPtr;

#pragma omp parallel for private(y,ch,pol,gridPtr,GridToAddPtr)
      for(int x=0 ; x<grid.shape()[0] ; ++x){
	for(ch=0 ; ch<NChan ; ++ch){
	  for(pol=0 ; pol<NPol ; ++pol){
	    gridPtr = grid.data() + ch*NPol*GridSize*GridSize + pol*GridSize*GridSize+x*GridSize;
	    GridToAddPtr = GridToAdd.data() + ch*NPol*GridSize*GridSize + pol*GridSize*GridSize+x*GridSize;
	    for(y=0 ; y<grid.shape()[1] ; ++y){
	      (*gridPtr++) += *GridToAddPtr++;
	      //gridPtr++;
	      //GridToAddPtr++;
	    }
	  }
	}
      }
    }

  }





  // Is this tiled?
  Bool isTiled;

  // Array lattice
  CountedPtr<Lattice<Complex> > arrayLattice;

  // Lattice. For non-tiled gridding, this will point to arrayLattice,
  //  whereas for tiled gridding, this points to the image
  CountedPtr<Lattice<Complex> > lattice;

  String convType;

  Float maxAbsData;

  // Useful IPositions
  IPosition centerLoc, offsetLoc;

  // Image Scaling and offset
  Vector<Double> uvScale, uvOffset;


  Array<Complex> its_stacked_GriddedData;
  Array<DComplex> its_stacked_GriddedData2;
  uInt itsNumCycle;


  //vector< Array<DComplex> > itsGriddedData2;
  vector< Matrix<Complex> > itsSumPB;
  vector< Matrix<Double> >  itsSumWeight;
  vector< double > itsSumCFWeight;


  ///Array<Complex>  griddedData;
  ///Array<DComplex> griddedData2;
  ///Matrix<Complex> itsSumPB;
  ///double itsSumWeight;

  Int priorCacheSize;

  // Grid/degrid zero spacing points?

  Bool usezero_p;

  //force no padding
  Bool noPadding_p;

  //Check if using put that avoids non-necessary reads
  Bool usePut2_p;

  //machine name
  String machineName_p;

  // Shape of the padded image
  IPosition padded_shape;

  Int convSampling;
    Float pbLimit_p;
    Int sensitivityPatternQualifier_p;
    String sensitivityPatternQualifierStr_p;
    Vector<Float> pbPeaks;
    Bool pbNormalized_p;
    // The average PB for sky image normalization
    //
    CountedPtr<ImageInterface<Float> > avgPB_p;
    CountedPtr<ImageInterface<Complex> > avgPBSq_p;

  LofarVisResampler visResamplers_p;

  casa::Record       itsParameters;
  casa::MeasurementSet itsMS;
  Int itsNWPlanes;
  double itsWMax;
  Double its_PBCut;
  int itsNThread;
  Bool its_Use_EJones;
  Double its_TimeWindow;
  //ofstream outFile;
  Bool its_Apply_Element;
  int its_ApplyBeamCode;
  Bool its_Already_Initialized;
  Bool                its_reallyDoPSF;
  Bool itsDonePB;
  Double itsUVmin;
  Double itsUVmax;
  CountedPtr<LofarConvolutionFunction> itsConvFunc;
  Vector<Int> ConjCFMap_p, CFMap_p;
  int itsVerbose;
  int itsMaxSupport;
  Int itsOversample;
  Vector< Double >    itsListFreq;
  String itsImgName;
  Matrix<Bool> itsGridMuellerMask;
  Matrix<Bool> itsDegridMuellerMask;
  double itsGriddingTime;
  double itsDegriddingTime;
  double itsCFTime;
  PrecTimer itsTotalTimer;
  PrecTimer itsCyrilTimer;

  double itsNextApplyTime;
  int itsCounterTimes;
  int itsStepApplyElement;
  double itsTStartObs;
  double itsDeltaTime;
  Array<Complex> itsTmpStackedGriddedData;
  Array<Complex> itsGridToDegrid;

  Vector<uInt> blIndex;
  vector<int> blStart, blEnd;
  Vector<Int> ant1;
  Vector<Int> ant2;

  void make_mapping(const VisBuffer& vb)
  {
  ant1 = vb.antenna1();
  ant2 = vb.antenna2();
    // Determine the baselines in the VisBuffer.
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
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
	double w0=abs(vb.uvw()[blIndex[lastIndex]](2));
	double w1=abs(vb.uvw()[blIndex[i-1]](2));
	double wMaxbl=std::max(w0,w1);
	if (wMaxbl <= itsWMax) {
	  //if (abs(Wmean) <= itsWMax) {
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
    double w0=abs(vb.uvw()[blIndex[lastIndex]](2));
    double w1=abs(vb.uvw()[blIndex[blnr.size()-1]](2));
    double wMaxbl=std::max(w0,w1);
    //if (abs(Wmean) <= itsWMax) {
    if (wMaxbl <= itsWMax) {
	//    if (abs(Wmean) <= itsWMax) {
      //if (itsVerbose > 1) {
	cout<<"...using w="<<Wmean<<endl;
	//}
      blStart.push_back (lastIndex);
      blEnd.push_back (blnr.size()-1);
    }
  }

  }

  vector<vector<uInt> > make_mapping_time(const VisBuffer& vb, uInt spw)
  {
    // Determine the baselines in the VisBuffer.
  ant1.assign(vb.antenna1());
  ant2.assign(vb.antenna2());
  const Vector<Double>& times = vb.timeCentroid();

  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.

  Float dtime(its_TimeWindow);
  vector<uInt> MapChunck;
  vector<vector<uInt> > Map;
  Double time0(times[0]);
  uInt bl_now(blnr[blIndex[0]]);
  for(uInt RowNum=0; RowNum<blIndex.size();++RowNum){
    uInt irow=blIndex[RowNum];

    Double timeRow(times[irow]);

    double u=vb.uvw()[irow](0);
    double v=vb.uvw()[irow](1);
    double w=vb.uvw()[irow](2);
    double uvdistance=(0.001)*sqrt(u*u+v*v)/(2.99e8/itsListFreq[spw]);
    Bool cond0((uvdistance>itsUVmin)&(uvdistance<itsUVmax));
    Bool cond1(abs(w)<itsWMax);
    if(!(cond0&cond1)){continue;}

    if(((timeRow-time0)>dtime)|(blnr[irow]!=bl_now))
      {
	time0=timeRow;
	Map.push_back(MapChunck);
	MapChunck.resize(0);
	bl_now=blnr[irow];
      }
    MapChunck.push_back(irow);
    }
  Map.push_back(MapChunck);

  /* cout.precision(20); */
  /* for(uInt i=0; i<Map.size();++i) */
  /*   { */
  /*     for(uInt j=0; j<Map[i].size();++j) */
  /* 	{ */
  /* 	  uInt irow=Map[i][j]; */
  /* 	  cout<<i<<" "<<j<<" A="<<ant1[irow]<<","<<ant2[irow]<<" w="<<vb.uvw()[irow](2)<<" t="<<times[irow]<<endl; */
  /* 	} */
  /*   } */

  return Map;

  }

  vector<Int> WIndexMap;
  vector<uInt> TimesMap;
  //uInt itsSelAnt0;
  //uInt itsSelAnt1;
  Double its_t0;
  Double its_tSel0;
  Double its_tSel1;

  vector<vector<vector<uInt> > > make_mapping_time_W(const VisBuffer& vb, uInt spw)
  {
    // Determine the baselines in the VisBuffer.
  ant1.assign(vb.antenna1());
  ant2.assign(vb.antenna2());
  const Vector<Double>& times = vb.timeCentroid();
  if(its_t0<0.){its_t0=times[0];}
  WIndexMap.resize(0);

  int nrant = 1 + max(max(ant1), max(ant2));
  Vector<Int> WPCoord;
  WPCoord.resize(ant1.size());
  for(uInt irow=0;irow<WPCoord.size();++irow){
    WPCoord[irow]=itsConvFunc->GiveWindexIncludeNegative(vb.uvw()[irow](2),spw);
  }
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(ant2+nrant*ant1+nrant*nrant*(WPCoord+itsNWPlanes));
  //blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.

  Float dtime(its_TimeWindow);
  vector<uInt> MapChunck;
  vector<vector<uInt> > MapW;
  vector<vector<vector<uInt> > > Map;
  Double time0(-1.);//times[blIndex[0]]);
  uInt bl_now;//blnr[blIndex[0]]);
  Int iwcoord;//=WPCoord[blIndex[0]]-itsNWPlanes;

  for(uInt RowNum=0; RowNum<blIndex.size();++RowNum){
    uInt irow=blIndex[RowNum];
    //cout<<ant1[irow]<<" "<<ant2[irow]<<" "<<times[irow]<<" "<<WPCoord[irow]<<endl;

    double u=vb.uvw()[irow](0);
    double v=vb.uvw()[irow](1);
    double w=vb.uvw()[irow](2);
    double uvdistance=(0.001)*sqrt(u*u+v*v)/(2.99e8/itsListFreq[spw]);
    Bool cond0(((uvdistance>itsUVmin)&(uvdistance<itsUVmax)));
    Bool cond1(abs(w)<itsWMax);
    //Bool cond2(((ant1[irow]==8)&(ant2[irow]==0)));
    //if
    //Bool cond2(((ant1[irow]==7)&(ant2[irow]==1)));
    Bool cond2(((ant1[irow]==5)&(ant2[irow]==40)));
    //Bool cond2((ant1[irow]==7));
    //Bool cond2((ant2[irow]==0));
    Double timeRow(times[irow]);
    Bool cond3((timeRow-its_t0)/60.>its_tSel0);
    Bool cond4((timeRow-its_t0)/60.<its_tSel1);
    Bool cond34(cond3&cond4);
    if(its_tSel0==-1.){cond34=true;}
    //if(!(cond0&cond1&cond2&cond34)){continue;}
    if(!(cond0&cond1&cond34)){continue;}

    if(time0==-1.){
      time0=timeRow;
      bl_now=blnr[irow];
      iwcoord=WPCoord[irow];
    }

    if(((timeRow-time0)>dtime)|(blnr[irow]!=bl_now))
      {
	time0=timeRow;
	MapW.push_back(MapChunck);
	MapChunck.resize(0);
	bl_now=blnr[irow];
      }
    if(WPCoord[irow]!=iwcoord){
      Map.push_back(MapW);
      MapW.resize(0);
      WIndexMap.push_back(iwcoord);
      iwcoord=WPCoord[irow];
    }

    MapChunck.push_back(irow);

    }
  MapW.push_back(MapChunck);
  WIndexMap.push_back(iwcoord);
  Map.push_back(MapW);


  /* for(uInt i=0; i<Map.size();++i) */
  /*   { */
  /*     for(uInt j=0; j<Map[i].size();++j) */
  /* 	{ */
  /* 	  for(uInt k=0; k<Map[i][j].size();++k) */
  /* 	    { */
  /* 	      uInt irow=Map[i][j][k]; */
  /* 	      cout<<i<<" "<<j<<" "<<k<<" A="<<ant1[irow]<<","<<ant2[irow]<<" w="<<vb.uvw()[irow](2)<<" windex="<<WIndexMap[i]<<" t="<<times[irow]<<endl; */
  /* 	    } */
  /* 	} */
  /*   } */

  /* for(uInt i=0; i<WIndexMap.size();++i) */
  /*   { */
  /*     cout<<" windex="<<WIndexMap[i]<<endl; */
  /*   } */

  return Map;



     /* } */
     /*  else { */
     /* 	Float dtime(its_TimeWindow); */
     /* 	Double time0(times[blIndex[blStart[i]]]); */

     /* 	vector<uInt> RowChunckStart; */
     /* 	vector<uInt> RowChunckEnd; */
     /* 	vector<vector< Float > > WsChunck; */
     /* 	vector< Float >          WChunck; */
     /* 	vector<Float> WCFforChunck; */
     /* 	Float wmin(1.e6); */
     /* 	Float wmax(-1.e6); */
     /* 	uInt NRow(blEnd[i]-blStart[i]+1); */
     /* 	Int NpixMax=0; */
     /* 	uInt WindexLast=itsConvFunc->GiveWindex(vbs.uvw()(2,blIndex[blStart[i]]),spw); */
     /* 	uInt Windex; */
     /* 	RowChunckStart.push_back(blStart[i]); */
     /* 	for(uInt Row=0; Row<NRow; ++Row){ */
     /* 	  uInt irow(blIndex[blStart[i]+Row]); */
     /* 	  Double timeRow(times[irow]); */
     /* 	  Int Npix1=itsConvFunc->GiveWSupport(vbs.uvw()(2,irow),spw); */
     /* 	  NpixMax=std::max(NpixMax,Npix1); */
     /* 	  Float w(vbs.uvw()(2,irow)); */
     /* 	  Windex=itsConvFunc->GiveWindex(vbs.uvw()(2,irow),spw); */

     /* 	  //if(WindexLast!=Windex) */
     /* 	  if((timeRow-time0)>dtime)//((WindexLast!=Windex)| */
     /* 	    { */
     /* 	      time0=timeRow; */
     /* 	      RowChunckEnd.push_back(blStart[i]+Row-1); */
     /* 	      WsChunck.push_back(WChunck); */
     /* 	      WChunck.resize(0); */
     /* 	      WCFforChunck.push_back((itsConvFunc->wScale()).center(WindexLast)); */
     /* 	      wmin=1.e6; */
     /* 	      wmax=-1.e6; */
     /* 	      if(Row!=(NRow-1)){ */
     /* 		RowChunckStart.push_back(blStart[i]+Row); */
     /* 	      } */
     /* 	      WindexLast=Windex; */
     /* 	    } */

     /* 	  WChunck.push_back(w); */

     /* 	} */
     /* 	WsChunck.push_back(WChunck); */
     /* 	RowChunckEnd.push_back(blEnd[i]); */
     /* 	WCFforChunck.push_back((itsConvFunc->wScale()).center(Windex)); */
     /* 	uInt irow(blIndex[blEnd[i]]); */
     /* 	Int Npix1=itsConvFunc->GiveWSupport(vbs.uvw()(2,irow),spw); */
     /* 	NpixMax=std::max(NpixMax,Npix1); */
     /* 	// for(uInt chunk=0; chunk<RowChunckStart.size();++chunk){ */
     /* 	//   cout<<NRow<<" bl: "<<i<<" || Start("<<RowChunckStart.size()<<"): "<<RowChunckStart[chunk]<<" , End("<<RowChunckEnd.size()<<"): "<<RowChunckEnd[chunk] */
     /* 	//       <<" w="<< 0.5*(vbs.uvw()(2,blIndex[RowChunckEnd[chunk]])+vbs.uvw()(2,blIndex[RowChunckStart[chunk]])) */
     /* 	//       <<" size="<< WsChunck[chunk].size()<<" wCF="<< WCFforChunck[chunk]<<endl; */

     /* 	//   // for(uInt iii=0; iii< WsChunck[chunk].size();++iii){ */
     /* 	//   //   cout<<WsChunck[chunk][iii]<<" "<<vbs.uvw()(2,blIndex[RowChunckEnd[chunk]]<<endl; */
     /* 	//   // } */

     /* 	// } */



     /* 	for(uInt chunk=0; chunk<RowChunckStart.size();++chunk){ */
     /* 	  Float WmeanChunk(0.5*(vbs.uvw()(2,blIndex[RowChunckEnd[chunk]])+vbs.uvw()(2,blIndex[RowChunckStart[chunk]]))); */
     /* 	  cout<<times[blIndex[RowChunckEnd[chunk]]]-times[blIndex[RowChunckStart[chunk]]]<<" "<<WmeanChunk<<endl; */
     /* 	  cfStore=  itsConvFunc->makeConvolutionFunction (ant1[ist], ant2[ist], timeChunk,//MaxTime, */
     /* 							  WmeanChunk, */
     /* 							//vbs.uvw()(2,blIndex[RowChunckEnd[chunk]]), */
     /* 							itsDegridMuellerMask, */
     /* 							true, */
     /* 							0.0, */
     /* 							itsSumPB[threadNum], */
     /* 							itsSumCFWeight[threadNum] */
     /* 							,spw,thisterm_p,itsRefFreq, */
     /* 							itsStackMuellerNew[threadNum], */
     /* 							  0);//NpixMax */
     /* 	//visResamplers_p.lofarGridToData_interp(vbs, itsGridToDegrid, */
     /* 	//				       blIndex, RowChunckStart[chunk], RowChunckEnd[chunk], cfStore, WsChunck[chunk], */
     /* 	//				       itsConvFunc->wStep(), WCFforChunck[chunk], itsConvFunc->wCorrGridder()); */
     /* 	visResamplers_p.lofarGridToData(vbs, itsGridToDegrid, */
     /* 					       blIndex, RowChunckStart[chunk], RowChunckEnd[chunk], cfStore); */
     /* 	} */

     /*  } */

  }

  double  itsSupport_Speroidal;

  vector<vector<vector<vector<uInt> > > > make_mapping_time_W_grid(const VisBuffer& vb, uInt spw)
  {
    // Determine the baselines in the VisBuffer.
  ant1.assign(vb.antenna1());
  ant2.assign(vb.antenna2());
  const Vector<Double>& times = vb.timeCentroid();
  if(its_t0<0.){its_t0=times[0];}
  WIndexMap.resize(0);
  Double recipWvl = itsListFreq[spw] / 2.99e8;

  int nrant = 1 + max(max(ant1), max(ant2));
  Vector<Int> WPCoord;
  WPCoord.resize(ant1.size());
  for(uInt irow=0;irow<WPCoord.size();++irow){
    WPCoord[irow]=itsConvFunc->GiveWindexIncludeNegative(vb.uvw()[irow](2),spw);
  }
  // Sort on baseline (use a baseline nr which is faster to sort).
  Vector<Int> blnr(ant2+nrant*ant1+nrant*nrant*(WPCoord+itsNWPlanes));
  //blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  GenSortIndirect<Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.

  Float dtime(its_TimeWindow);
  vector<uInt> MapChunck;
  vector<vector<uInt> > MapW;
  vector<vector<vector<uInt> > > Map;

  vector<Int > xminBL;
  vector<vector<Int> > xminW;
  vector<Int > xmaxBL;
  vector<vector<Int> > xmaxW;
  vector<Int > yminBL;
  vector<vector<Int> > yminW;
  vector<Int > ymaxBL;
  vector<vector<Int> > ymaxW;

  Double time0(-1.);//times[blIndex[0]]);
  uInt bl_now;//blnr[blIndex[0]]);
  Int iwcoord;//=WPCoord[blIndex[0]]-itsNWPlanes;

  float scaling(2.);
  float support((itsSupport_Speroidal-1)/2+1);
  Int xmin=1e20;
  Int xmax=-1e20;
  Int ymin=1e20;
  Int ymax=-1e20;

  for(uInt RowNum=0; RowNum<blIndex.size();++RowNum){
    uInt irow=blIndex[RowNum];

    double u=vb.uvw()[irow](0);
    double v=vb.uvw()[irow](1);
    double w=vb.uvw()[irow](2);
    double uvdistance=(0.001)*sqrt(u*u+v*v)/(2.99e8/itsListFreq[spw]);
    Bool cond0(((uvdistance>itsUVmin)&(uvdistance<itsUVmax)));
    Bool cond1(abs(w)<itsWMax);
    Bool cond2(((ant1[irow]==5)&(ant2[irow]==40)));
    Double timeRow(times[irow]);
    Bool cond3((timeRow-its_t0)/60.>its_tSel0);
    Bool cond4((timeRow-its_t0)/60.<its_tSel1);
    Bool cond34(cond3&cond4);
    if(its_tSel0==-1.){cond34=true;}
    if(!(cond0&cond1&cond34)){continue;}
    //if(!(cond0&cond1&cond34&cond2)){continue;}

    if(time0==-1.){
      time0=timeRow;
      bl_now=blnr[irow];
      iwcoord=WPCoord[irow];
    }

    if(((timeRow-time0)>dtime)|(blnr[irow]!=bl_now))
      {
	time0=timeRow;
	MapW.push_back(MapChunck);
	MapChunck.resize(0);

	xminBL.push_back(xmin);
	xmaxBL.push_back(xmax);
	yminBL.push_back(ymin);
	ymaxBL.push_back(ymax);

	xmin=1e20;
	xmax=-1e20;
	ymin=1e20;
	ymax=-1e20;

	bl_now=blnr[irow];
      }
    if(WPCoord[irow]!=iwcoord){
      Map.push_back(MapW);
      MapW.resize(0);

      xminW.push_back(xminBL);
      xminBL.resize(0);
      xmaxW.push_back(xmaxBL);
      xmaxBL.resize(0);
      yminW.push_back(yminBL);
      yminBL.resize(0);
      ymaxW.push_back(ymaxBL);
      ymaxBL.resize(0);

      WIndexMap.push_back(iwcoord);
      iwcoord=WPCoord[irow];
    }

    MapChunck.push_back(irow);

    Int xrow = int(u * uvScale(0) * recipWvl + uvOffset(0));
    Int yrow = int(v * uvScale(1) * recipWvl + uvOffset(1));
    if(xrow-support<xmin){xmin=xrow-support;};
    if(xrow+support>xmax){xmax=xrow+support;};
    if(yrow-support<ymin){ymin=yrow-support;};
    if(yrow+support>ymax){ymax=yrow+support;};

    }
  MapW.push_back(MapChunck);
  Map.push_back(MapW);
  xminBL.push_back(xmin);
  xmaxBL.push_back(xmax);
  yminBL.push_back(ymin);
  ymaxBL.push_back(ymax);
  xminW.push_back(xminBL);
  xmaxW.push_back(xmaxBL);
  yminW.push_back(yminBL);
  ymaxW.push_back(ymaxBL);

  WIndexMap.push_back(iwcoord);

  /* for(uInt i=0; i<Map.size();++i) */
  /*   { */
  /*     for(uInt j=0; j<Map[i].size();++j) */
  /* 	{ */

  /* 	  for(uInt k=0; k<Map[i][j].size();++k) */
  /* 	    { */
  /* 	      uInt irow=Map[i][j][k]; */
  /* 	      cout<<"iw="<<i<<" ibl="<<j<<" imap="<<k<<" A="<<ant1[irow]<<","<<ant2[irow]<<" w="<<vb.uvw()[irow](2)<<" windex="<<WIndexMap[i]<<" t="<<times[irow]<<endl; */
  /* 	      double u=vb.uvw()[irow](0); */
  /* 	      double v=vb.uvw()[irow](1); */
  /* 	      Int xrow=int(float(u)*scaling); */
  /* 	      Int yrow=int(float(v)*scaling); */
  /* 	      cout<<"   "<<xminW[i][j]<<" ("<<xrow-support<<")"<<endl; */
  /* 	      cout<<"   "<<xmaxW[i][j]<<" ("<<xrow+support<<")"<<endl; */
  /* 	      cout<<"   "<<yminW[i][j]<<" ("<<yrow-support<<")"<<endl; */
  /* 	      cout<<"   "<<ymaxW[i][j]<<" ("<<yrow+support<<")"<<endl; */
  /* 	      cout<<" "<<endl; */
  /* 	    } */
  /* 	} */
  /*   } */

  //  ofstream outFile("output_grids.txt");

  vector<uInt> MapChunckOut;
  vector<vector<uInt> > MapWGridOut;
  vector<vector<vector<uInt> > > MapWOut;
  vector<vector<vector<vector<uInt> > > > MapOut;

  vector<IPosition > posBlock;

  for(uInt i=0; i<Map.size();++i)
    {
      MapWGridOut.resize(0);
      MapWOut.resize(0);
      Vector<Bool> done;
      done.resize(Map[i].size());
      done=false;
      Bool alldone(false);
      Bool cond_xmin,cond_xmax,cond_ymin,cond_ymax;
      uInt iblock(0);
      //MapWGridOut.push_back(Map[i][0]);

      posBlock.resize(0);
      IPosition pos(2,1,1);
      //pos(0)=i;
      //pos(1)=0;
      //posBlock.push_back(pos);

      //cout<<"  plane w="<<i<<" nbl_blocks="<< Map[i].size()<<endl;

      while(!alldone){

	for(uInt j=0; j<Map[i].size();++j)
	  {
	    // Find if baseline j has overlap with the current grid
	    if(done[j]==true){continue;}
	    Bool NoOverlap(true);
	    for(uInt jj=0; jj<MapWGridOut.size();++jj)
	      {
		cond_xmin=xminW[i][j]<=xmaxW[posBlock[jj](0)][posBlock[jj](1)];
		cond_xmax=xmaxW[i][j]>=xminW[posBlock[jj](0)][posBlock[jj](1)];
		cond_ymin=yminW[i][j]<=ymaxW[posBlock[jj](0)][posBlock[jj](1)];
		cond_ymax=ymaxW[i][j]>=yminW[posBlock[jj](0)][posBlock[jj](1)];
		Bool condIsOverlap(cond_xmin&&cond_xmax&&cond_ymin&&cond_ymax);
		if(condIsOverlap){
		  NoOverlap=false;
		  break;
		}
	      }
	    if(NoOverlap){
	      MapWGridOut.push_back(Map[i][j]);
	      done[j]=true;
	      pos(0)=i;
	      pos(1)=j;
	      posBlock.push_back(pos);
	    }
	  }

	alldone=true;
	for(uInt j=0; j<done.size();++j)
	  {
	    if(done[j]==false){alldone=false;break;}
	  }

	/* for(uInt iii=0; iii<MapWGridOut.size();++iii){ */
	/*   uInt icoord(posBlock[iii](0)); */
	/*   uInt jcoord(posBlock[iii](1)); */
	/*   outFile<<"   "<<i<<" "<<iblock<<" "<<xminW[icoord][jcoord]<<" "<<xmaxW[icoord][jcoord]<<" "<<yminW[icoord][jcoord]<<" "<<ymaxW[icoord][jcoord]<<endl; */
	/* } */

	posBlock.resize(0);
	MapWOut.push_back(MapWGridOut);
	MapWGridOut.resize(0);
	iblock+=1;


      }
      MapOut.push_back(MapWOut);
      MapWOut.resize(0);

    }

  /* for(uInt i=0; i<MapOut.size();++i){ */
  /*   for(uInt j=0; j<MapOut[i].size();++j){ */
  /*     uInt icoord(posBlock[iii](0)); */
  /*     uInt jcoord(posBlock[iii](1)); */
  /*     outFile<<"   "<<i<<" "<<iblock<<" "<<xminW[icoord][jcoord]<<" "<<xmaxW[icoord][jcoord]<<" "<<yminW[icoord][jcoord]<<" "<<ymaxW[icoord][jcoord]<<endl; */
  /*   } */
  /* } */

  return MapOut;

  }


  ///////////////////////////////////////

  FFTCMatrix  FFTMachine;

  void dofft(Matrix<Complex>& arr, bool direction)
{
  int sz(arr.nrow());
  int nthreads(OpenMP::maxThreads());

  if(direction==true)
  {
    FFTMachine.normalized_forward(arr.nrow(),arr.data(),nthreads, FFTW_MEASURE);
  }

  if(direction==false)
  {
    FFTMachine.normalized_backward(arr.nrow(),arr.data(),nthreads, FFTW_MEASURE);
  }

}

  ///////////////////////////////////////
  vector<FFTCMatrix>  VecFFTMachine;
  void dofftVec(Matrix<Complex>& arr, bool direction, int nth=0, int pol=0)
{
  int sz(arr.nrow());
  int nthreads(OpenMP::maxThreads());
  if(nth!=0){nthreads=nth;}

  if(direction==true)
  {
    VecFFTMachine[pol].normalized_forward(arr.nrow(),arr.data(),nthreads, FFTW_MEASURE);
  }

  if(direction==false)
  {
    VecFFTMachine[pol].normalized_backward(arr.nrow(),arr.data(),nthreads, FFTW_MEASURE);
  }

}




  ////////////////////////////////////////

  template <class T>
  void store (const DirectionCoordinate &dir, const Matrix<T> &data,
              const string &name)
  {
    //cout<<"Saving... "<<name<<endl;
    Vector<Int> stokes(1);
    stokes(0) = Stokes::I;
    CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
  }

  template <class T>
  void store(const Matrix<T> &data, const string &name)
  {
    Matrix<Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
    Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
    Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
    DirectionCoordinate dir(MDirection::J2000, Projection(Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const Cube<T> &data, const string &name)
  {
    Matrix<Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
    Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
    Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
    DirectionCoordinate dir(MDirection::J2000, Projection(Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const DirectionCoordinate &dir, const Cube<T> &data,
             const string &name)
  {
    AlwaysAssert(data.shape()(2) == 4, SynthesisError);
    //cout<<"Saving... "<<name<<endl;
    Vector<Int> stokes(4);
    stokes(0) = Stokes::XX;
    stokes(1) = Stokes::XY;
    stokes(2) = Stokes::YX;
    stokes(3) = Stokes::YY;
    CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    PagedImage<T>
      im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)),
         csys, name);
    im.putSlice(data, IPosition(4, 0, 0, 0, 0));
  }
      /* template <class T> */
      /*   void store(const Cube<T> &data, const string &name) */
      /*   { */

      /*     CoordinateSystem csys; */
      /*     Matrix<Double> xform(2, 2); */
      /*     xform = 0.0; */
      /*     xform.diagonal() = 1.0; */
      /*     Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad"); */
      /*     Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad"); */
      /*     Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad"); */
      /*     csys.addCoordinate(DirectionCoordinate(MDirection::J2000, Projection(Projection::SIN), */
      /*                        refLatLon, refLatLon, incLon, incLat, */
      /*                        xform, data.shape()(0) / 2, data.shape()(1) / 2)); */

      /*     Vector<Int> stokes(4); */
      /*     stokes(0) = Stokes::XX; */
      /*     stokes(1) = Stokes::XY; */
      /*     stokes(2) = Stokes::YX; */
      /*     stokes(3) = Stokes::YY; */
      /*     csys.addCoordinate(StokesCoordinate(stokes)); */
      /*     csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6)); */

      /*     PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)), csys, name); */
      /*     im.putSlice(data, IPosition(4, 0, 0, 0, 0)); */
      /*   } */


};

} //# end namespace

#endif
