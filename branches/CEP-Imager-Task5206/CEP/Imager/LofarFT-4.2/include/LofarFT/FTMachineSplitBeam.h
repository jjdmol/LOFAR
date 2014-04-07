//# FTMachineWsplit.h: Definition for FTMachineWsplit
//#
//# Copyright (C) 2013
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

#ifndef LOFARFT_LOFARFTMACHINEWSPLIT_H
#define LOFARFT_LOFARFTMACHINEWSPLIT_H

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Containers/Block.h>
#include <casa/OS/File.h>
#include <casa/OS/PrecTimer.h>
#include <casa/OS/Timer.h>

#include <images/Images/ImageInterface.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <scimath/Mathematics/FFTServer.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <synthesis/TransformMachines/FTMachine.h>

#include <LofarFT/VisResampler.h>
#include <LofarFT/ConvolutionFunction.h>
#include <LofarFT/CFStore.h>

namespace LOFAR {


class FTMachineWsplit : public casa::FTMachine {
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
//  FTMachineWsplit(casa::Long cachesize, casa::Int tilesize, casa::CountedPtr<VisibilityResamplerBase>& visResampler,
//	  casa::String convType="SF", casa::Float padding=1.0, casa::Bool usezero=casa::True, casa::Bool useDoublePrec=casa::False);
  FTMachineWsplit(
    casa::Long cachesize, 
    casa::Int tilesize,  
    casa::CountedPtr<casa::VisibilityResamplerBase>& visResampler, 
    casa::String convType, 
    const casa::MeasurementSet& ms,
    casa::Int nwPlanes,
    casa::MPosition mLocation, 
    casa::Float padding, 
    casa::Bool usezero,
    casa::Bool useDoublePrec, 
    double wmax,
    casa::Int verbose,
    casa::Int maxsupport,
    casa::Int oversample,
    const casa::String& imageName,
    const casa::Matrix<casa::Bool>& gridMuellerMask,
    const casa::Matrix<casa::Bool>& degridMuellerMask,
    casa::Double refFreq,
    casa::Bool use_Linear_Interp_Gridder,
    casa::Bool use_EJones,
    int stepApplyElement,
    int applyBeamCode,
    casa::Double pbCut,
    casa::Bool predictFT,
    casa::String psfOnDisk,
    casa::Bool useMasksDegrid,
    casa::Bool reallyDoPSF,
    casa::Double uvMin,
    casa::Double uvMax,
    casa::Bool makeDirtyCorr,
    const casa::Record& parameters);//,

  // Copy constructor
  FTMachineWsplit(const FTMachineWsplit &other);

  // Assignment operator
  FTMachineWsplit &operator=(const FTMachineWsplit &other);

  // Clone
  FTMachineWsplit* clone() const;


  ~FTMachineWsplit();
  
  virtual casa::String name() const { return "FTMachineWsplit";};

  // Show the relative timings of the various steps.
  void showTimings (
    std::ostream&, 
    double duration) const;

  // Initialize transform to Visibility plane using the image
  // as a template. The image is loaded and Fourier transformed.
  void initializeToVis(
    casa::ImageInterface<casa::Complex>& image,
    const casa::VisBuffer& vb);

  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void finalizeToVis();
  
  void getSplitWplanes(
    casa::VisBuffer& vb, 
    casa::Int row);
  
  void getTraditional(
    casa::VisBuffer& vb, 
    casa::Int row);
  
  void putSplitWplanesOverlap(
    const casa::VisBuffer& vb, 
    casa::Int row, 
    casa::Bool dopsf,
    FTMachine::Type type);
  
  void putSplitWplanes(
    const casa::VisBuffer& vb, 
    casa::Int row, 
    casa::Bool dopsf,
    FTMachine::Type type);
  
  void putTraditional(
    const casa::VisBuffer& vb, 
    casa::Int row, 
    casa::Bool dopsf,
    FTMachine::Type type);
  
  // Initialize transform to Sky plane: initializes the image
  void initializeToSky(
    casa::ImageInterface<casa::Complex>& image,  
    casa::Matrix<casa::Float>& weight,
    const casa::VisBuffer& vb);


  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. DOES NOT
  // DO THE FINAL TRANSFORM!
  void finalizeToSky();

  // Get actual coherence from grid by degridding
  void get(
    casa::VisBuffer& vb, 
    casa::Int row=-1);

  // Put coherence to grid by gridding.
  void put(
    const casa::VisBuffer& vb, 
    casa::Int row=-1, 
    casa::Bool dopsf=casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);

  
  // Make the entire image
  void makeImage(casa::FTMachine::Type type,
		 casa::VisSet& vs,
		 casa::ImageInterface<casa::Complex>& image,
		 casa::Matrix<casa::Float>& weight);

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  casa::ImageInterface<casa::Complex>& getImage(casa::Matrix<casa::Float>&, casa::Bool normalize=casa::True);

  // Get the average primary beam.
  virtual const casa::Matrix<casa::Float>& getAveragePB() const;

  // Get the spheroidal cut.
  const casa::Matrix<casa::Float>& getSpheroidCut() const
    { return itsConvFunc->getSpheroidCut(); }


  ///  virtual void normalizeImage(Lattice<casa::Complex>& skyImage,
  ///			      const casa::Matrix<casa::Double>& sumOfWts,
  ///			      Lattice<casa::Float>& sensitivityImage,
  ///			      casa::Bool fftNorm)
  ///    {throw(AipsError("FTMachineWsplit::normalizeImage() called"));}

  void normalizeAvgPB();
  void normalizeAvgPB(
    casa::ImageInterface<casa::Complex>& inImage,
    casa::ImageInterface<casa::Float>& outImage);
    //
    // Make a sensitivity image (sensitivityImage), given the gridded
    // weights (wtImage).  These are related to each other by a
    // Fourier transform and normalization by the sum-of-weights
    // (sumWt) and normalization by the product of the 2D FFT size
    // along each axis.  If doFFTNorm=casa::False, normalization by the FFT
    // size is not done.  If sumWt is not provided, normalization by
    // the sum of weights is also not done.
    //



  virtual void makeSensitivityImage(casa::Lattice<casa::Complex>&,
    casa::ImageInterface<casa::Float>&,
    const casa::Matrix<casa::Float>& =casa::Matrix<casa::Float>(),
    const casa::Bool& = casa::True) {}
      
  virtual void makeSensitivityImage(
    const casa::VisBuffer& vb, 
    const casa::ImageInterface<casa::Complex>& imageTemplate,
    casa::ImageInterface<casa::Float>& sensitivityImage);

  inline virtual casa::Float pbFunc(
    const casa::Float& a, 
    const casa::Float& limit)
  {if (abs(a) >= limit) return (a);else return 1.0;};
    
  inline virtual casa::Complex pbFunc(
    const casa::Complex& a, 
    const casa::Float& limit)
  {if (abs(a)>=limit) return (a); else return casa::Complex(1.0,0.0);};
    
  //
  // Given the sky image (Fourier transform of the visibilities),
  // sum of weights and the sensitivity image, this method replaces
  // the skyImage with the normalized image of the sky.
  //
  virtual void normalizeImage(
    casa::Lattice<casa::Complex>& skyImage,
    const casa::Matrix<casa::Double>& sumOfWts,
    casa::Lattice<casa::Float>& sensitivityImage,
    casa::Bool fftNorm=casa::True);
    
  virtual void normalizeImage(
    casa::Lattice<casa::Complex>& skyImage,
    const casa::Matrix<casa::Double>& sumOfWts,
    casa::Lattice<casa::Float>& sensitivityImage,
    casa::Lattice<casa::Complex>& sensitivitySqImage,
    casa::Bool fftNorm=casa::True);

  virtual casa::ImageInterface<casa::Float>& getSensitivityImage() {return *avgPB_p;}
  virtual casa::Matrix<casa::Double>& getSumOfWeights() {return sumWeight;};
  virtual casa::Matrix<casa::Double>& getSumOfCFWeights() {return sumCFWeight;};

  // Get the final weights image
  void getWeightImage(
    casa::ImageInterface<casa::Float>&, 
    casa::Matrix<casa::Float>&);

  // Save and restore the FTMachineWsplit to and from a record
  virtual casa::Bool toRecord(
    casa::String& error, 
    casa::RecordInterface& outRec,
    casa::Bool withImage=casa::False);
  
  virtual casa::Bool fromRecord(
    casa::String& error, 
    const casa::RecordInterface& inRec);

  // Can this FTMachine be represented by Fourier convolutions?
  virtual casa::Bool isFourier() {return casa::True;}

  virtual void setNoPadding(casa::Bool nopad){noPadding_p=nopad;};

  virtual casa::String name();

  //Cyr: The FTMachine has got to know the order of the Taylor term
  virtual void setMiscInfo(const casa::Int qualifier){thisterm_p=qualifier;};
  
  virtual void ComputeResiduals(
    casa::VisBuffer&vb, 
    casa::Bool useCorrected);


  void makeConjPolMap(
    const casa::VisBuffer& vb, 
    const casa::Vector<casa::Int> cfPolMap, 
    casa::Vector<casa::Int>& conjPolMap);

  void makeCFPolMap(
    const casa::VisBuffer& vb, 
    const casa::Vector<casa::Int>& cfstokes, 
    casa::Vector<casa::Int>& polM);

  void setPsfOnDisk(casa::String NamePsf){itsNamePsfOnDisk=NamePsf;}
  
  virtual casa::String giveNamePsfOnDisk(){return itsNamePsfOnDisk;}
  
    // Arrays for non-tiled gridding (one per thread).
  
  void initGridThreads(
    vector< casa::Array<casa::Complex> >&  otherGriddedData, 
    vector< casa::Array<casa::DComplex> >&  otherGriddedData2);

  casa::Matrix<casa::Bool> giveMaskGrid()
  {
    return itsMaskGridCS;
  }

protected:
  vector< casa::Array<casa::Complex> >*  itsGriddedData;
  vector< casa::Array<casa::DComplex> >*  itsGriddedData2;
  mutable casa::Matrix<casa::Float> itsAvgPB;
  // Padding in FFT
  casa::Float padding_p;
  casa::Int thisterm_p;
  casa::Double itsRefFreq;
  casa::Bool itsPredictFT;
  double its_tot_time_grid;
  double its_tot_time_cf;
  double its_tot_time_w;
  double its_tot_time_el;
  double its_tot_time_tot;

  casa::Matrix<casa::Bool> itsMaskGridCS;
  casa::String itsNamePsfOnDisk;
  vector< vector< vector < casa::Matrix<casa::Complex> > > > itsStackMuellerNew; 
  
  casa::Int itsTotalStepsGrid;
  casa::Int itsTotalStepsDeGrid;
  casa::Bool itsMasksGridAllDone;
  casa::Bool itsMasksAllDone;
  casa::Bool itsAllAtermsDone;
  casa::Bool its_UseMasksDegrid;
  casa::Bool its_SingleGridMode;
  casa::Bool its_makeDirtyCorr;
  casa::uInt its_NGrids;

  casa::Timer itsSeconds;
  //casa::Float its_FillFactor;
  // Get the appropriate data pointer
  casa::Array<casa::Complex>* getDataPointer(const casa::IPosition&, casa::Bool);

  void ok();

  void init();

  // Is this record on Grid? check both ends. This assumes that the
  // ends bracket the middle
  casa::Bool recordOnGrid(const casa::VisBuffer& vb, casa::Int rownr) const;

  // Image cache
  casa::LatticeCache<casa::Complex> * imageCache;

  // Sizes
  casa::Long cachesize;
  casa::Int  tilesize;

  // Gridder
  casa::ConvolveGridder<casa::Double, casa::Complex>* gridder;

  casa::Bool itsUseLinearInterpGridder;
  casa::Bool itsUseWSplit;


  // Is this tiled?
  casa::Bool isTiled;

  // Array lattice
  casa::CountedPtr<casa::Lattice<casa::Complex> > arrayLattice;

  // Lattice. For non-tiled gridding, this will point to arrayLattice,
  //  whereas for tiled gridding, this points to the image
  casa::CountedPtr<casa::Lattice<casa::Complex> > lattice;

  casa::String convType;

  casa::Float maxAbsData;

  // Useful IPositions
  casa::IPosition centerLoc, offsetLoc;

  // Image Scaling and offset
  casa::Vector<casa::Double> uvScale, uvOffset;

  
  casa::Array<casa::Complex> its_stacked_GriddedData;
  casa::Array<casa::DComplex> its_stacked_GriddedData2;
  casa::uInt itsNumCycle;
  

  //vector< casa::Array<casa::DComplex> > itsGriddedData2;
  vector< casa::Matrix<casa::Complex> > itsSumPB;
  vector< casa::Matrix<casa::Double> >  itsSumWeight;
  vector< double > itsSumCFWeight;


  ///casa::Array<casa::Complex>  griddedData;
  ///casa::Array<casa::DComplex> griddedData2;
  ///casa::Matrix<casa::Complex> itsSumPB;
  ///double itsSumWeight;

  casa::Int priorCacheSize;

  // Grid/degrid zero spacing points?

  casa::Bool usezero_p;

  //force no padding
  casa::Bool noPadding_p;

  //Check if using put that avoids non-necessary reads
  casa::Bool usePut2_p;

  //machine name
  casa::String machineName_p;

  // Shape of the padded image
  casa::IPosition padded_shape;

  casa::Int convSampling;
    casa::Float pbLimit_p;
    casa::Int sensitivityPatternQualifier_p;
    casa::String sensitivityPatternQualifierStr_p;
    casa::Vector<casa::Float> pbPeaks;
    casa::Bool pbNormalized_p;
    // The average PB for sky image normalization
    //
    casa::CountedPtr<casa::ImageInterface<casa::Float> > avgPB_p;
    casa::CountedPtr<casa::ImageInterface<casa::Complex> > avgPBSq_p;

  LofarVisResampler visResamplers_p;

  casa::Record       itsParameters;
  casa::MeasurementSet itsMS;
  casa::Int itsNWPlanes;
  double itsWMax;
  casa::Double its_PBCut;
  int itsNThread;
  casa::Bool its_Use_EJones;
  casa::Double its_TimeWindow;
  //ofstream outFile;
  casa::Bool its_Apply_Element;
  int its_ApplyBeamCode;
  casa::Bool its_Already_Initialized;
  casa::Bool                its_reallyDoPSF;
  casa::Bool itsDonePB;
  casa::Double itsUVmin;
  casa::Double itsUVmax;
  casa::CountedPtr<LofarConvolutionFunction> itsConvFunc;
  casa::Vector<casa::Int> ConjCFMap_p, CFMap_p;
  int itsVerbose;
  int itsMaxSupport;
  casa::Int itsOversample;
  casa::Vector< casa::Double >    itsListFreq;
  casa::String itsImgName;
  casa::Matrix<casa::Bool> itsGridMuellerMask;
  casa::Matrix<casa::Bool> itsDegridMuellerMask;
  double itsGriddingTime;
  double itsDegriddingTime;
  double itsCFTime;
  casa::PrecTimer itsTotalTimer;
  casa::PrecTimer itsCyrilTimer;

  double itsNextApplyTime;
  int itsCounterTimes;
  int itsStepApplyElement;
  double itsTStartObs;
  double itsDeltaTime;
  casa::Array<casa::Complex> itsTmpStackedGriddedData;
  casa::Array<casa::Complex> itsGridToDegrid;

  casa::Vector<casa::uInt> blIndex;
  vector<int> blStart, blEnd;
  casa::Vector<casa::Int> ant1;
  casa::Vector<casa::Int> ant2;

  void make_mapping(const casa::VisBuffer& vb)
  {
  ant1 = vb.antenna1();
  ant2 = vb.antenna2();
    // Determine the baselines in the VisBuffer.
  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  casa::Vector<casa::Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  casa::GenSortIndirect<casa::Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.
  blStart.reserve (nrant*(nrant+1)/2);
  blEnd.reserve   (nrant*(nrant+1)/2);
  casa::Int  lastbl     = -1;
  casa::Int  lastIndex  = 0;
  bool usebl      = false;
  bool allFlagged = true;
  const casa::Vector<casa::Bool>& flagRow = vb.flagRow();
  for (uint i=0; i<blnr.size(); ++i) {
    casa::Int inx = blIndex[i];
    casa::Int bl = blnr[inx];
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

  vector<vector<casa::uInt> > make_mapping_time(const casa::VisBuffer& vb, casa::uInt spw)
  {
    // Determine the baselines in the VisBuffer.
  ant1.assign(vb.antenna1());
  ant2.assign(vb.antenna2());
  const casa::Vector<casa::Double>& times = vb.timeCentroid();

  int nrant = 1 + max(max(ant1), max(ant2));
  // Sort on baseline (use a baseline nr which is faster to sort).
  casa::Vector<casa::Int> blnr(nrant*ant1);
  blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  casa::GenSortIndirect<casa::Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.

  casa::Float dtime(its_TimeWindow);
  vector<casa::uInt> MapChunck;
  vector<vector<casa::uInt> > Map;
  casa::Double time0(times[0]);
  casa::Int bl_now(blnr[blIndex[0]]);
  for(casa::uInt RowNum=0; RowNum<blIndex.size();++RowNum){
    casa::uInt irow=blIndex[RowNum];

    casa::Double timeRow(times[irow]);

    double u=vb.uvw()[irow](0);
    double v=vb.uvw()[irow](1);
    double w=vb.uvw()[irow](2);
    double uvdistance=(0.001)*sqrt(u*u+v*v)/(2.99e8/itsListFreq[spw]);
    casa::Bool cond0((uvdistance>itsUVmin)&(uvdistance<itsUVmax));
    casa::Bool cond1(abs(w)<itsWMax);
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

  return Map;
     
  }

  vector<casa::Int> WIndexMap;
  vector<casa::uInt> TimesMap;
  casa::Double its_t0;
  casa::Double its_tSel0;
  casa::Double its_tSel1;

  vector<vector<vector<casa::uInt> > > make_mapping_time_W(const casa::VisBuffer& vb, casa::uInt spw)
  {
    // Determine the baselines in the VisBuffer.
  ant1.assign(vb.antenna1());
  ant2.assign(vb.antenna2());
  const casa::Vector<casa::Double>& times = vb.timeCentroid();
  if(its_t0<0.){its_t0=times[0];}
  WIndexMap.resize(0);

  int nrant = 1 + max(max(ant1), max(ant2));
  casa::Vector<casa::Int> WPCoord;
  WPCoord.resize(ant1.size());
  for(casa::uInt irow=0;irow<WPCoord.size();++irow){
    WPCoord[irow]=itsConvFunc->GiveWindexIncludeNegative(vb.uvw()[irow](2),spw);
  }
  // Sort on baseline (use a baseline nr which is faster to sort).
  casa::Vector<casa::Int> blnr(ant2+nrant*ant1+nrant*nrant*(WPCoord+itsNWPlanes));
  //blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  casa::GenSortIndirect<casa::Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.

  casa::Float dtime(its_TimeWindow);
  vector<casa::uInt> MapChunck;
  vector<vector<casa::uInt> > MapW;
  vector<vector<vector<casa::uInt> > > Map;
  casa::Double time0(-1.);//times[blIndex[0]]);
  casa::Int bl_now;//blnr[blIndex[0]]);
  casa::Int iwcoord;//=WPCoord[blIndex[0]]-itsNWPlanes;

  for(casa::uInt RowNum=0; RowNum<blIndex.size();++RowNum){
    casa::uInt irow=blIndex[RowNum];
    //cout<<ant1[irow]<<" "<<ant2[irow]<<" "<<times[irow]<<" "<<WPCoord[irow]<<endl;
    
    double u=vb.uvw()[irow](0);
    double v=vb.uvw()[irow](1);
    double w=vb.uvw()[irow](2);
    double uvdistance=(0.001)*sqrt(u*u+v*v)/(2.99e8/itsListFreq[spw]);
    casa::Bool cond0(((uvdistance>itsUVmin)&(uvdistance<itsUVmax)));
    casa::Bool cond1(abs(w)<itsWMax);
    //casa::Bool cond2(((ant1[irow]==8)&(ant2[irow]==0)));
    //if 
    //casa::Bool cond2(((ant1[irow]==7)&(ant2[irow]==1)));
    casa::Bool cond2(((ant1[irow]==5)&(ant2[irow]==40)));
    //casa::Bool cond2((ant1[irow]==7));
    //casa::Bool cond2((ant2[irow]==0));
    casa::Double timeRow(times[irow]);
    casa::Bool cond3((timeRow-its_t0)/60.>its_tSel0);
    casa::Bool cond4((timeRow-its_t0)/60.<its_tSel1);
    casa::Bool cond34(cond3&cond4);
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


  /* for(casa::uInt i=0; i<Map.size();++i) */
  /*   { */
  /*     for(casa::uInt j=0; j<Map[i].size();++j) */
  /* 	{ */
  /* 	  for(casa::uInt k=0; k<Map[i][j].size();++k) */
  /* 	    { */
  /* 	      casa::uInt irow=Map[i][j][k]; */
  /* 	      cout<<i<<" "<<j<<" "<<k<<" A="<<ant1[irow]<<","<<ant2[irow]<<" w="<<vb.uvw()[irow](2)<<" windex="<<WIndexMap[i]<<" t="<<times[irow]<<endl; */
  /* 	    } */
  /* 	} */
  /*   } */

  /* for(casa::uInt i=0; i<WIndexMap.size();++i) */
  /*   { */
  /*     cout<<" windex="<<WIndexMap[i]<<endl; */
  /*   } */

  return Map;
     
  }

  double  itsSupport_Speroidal;

  vector<vector<vector<vector<casa::uInt> > > > make_mapping_time_W_grid(const casa::VisBuffer& vb, casa::uInt spw)
  {
    // Determine the baselines in the VisBuffer.
  ant1.assign(vb.antenna1());
  ant2.assign(vb.antenna2());
  const casa::Vector<casa::Double>& times = vb.timeCentroid();
  if(its_t0<0.){its_t0=times[0];}
  WIndexMap.resize(0);
  casa::Double recipWvl = itsListFreq[spw] / 2.99e8;

  int nrant = 1 + max(max(ant1), max(ant2));
  casa::Vector<casa::Int> WPCoord;
  WPCoord.resize(ant1.size());
  for(casa::uInt irow=0;irow<WPCoord.size();++irow){
    WPCoord[irow]=itsConvFunc->GiveWindexIncludeNegative(vb.uvw()[irow](2),spw);
  }
  // Sort on baseline (use a baseline nr which is faster to sort).
  casa::Vector<casa::Int> blnr(ant2+nrant*ant1+nrant*nrant*(WPCoord+itsNWPlanes));
  //blnr += ant2;  // This is faster than nrant*ant1+ant2 in a single line
  casa::GenSortIndirect<casa::Int>::sort (blIndex, blnr);
  // Now determine nr of unique baselines and their start index.

  casa::Float dtime(its_TimeWindow);
  vector<casa::uInt> MapChunck;
  vector<vector<casa::uInt> > MapW;
  vector<vector<vector<casa::uInt> > > Map;

  vector<casa::Int > xminBL;
  vector<vector<casa::Int> > xminW;
  vector<casa::Int > xmaxBL;
  vector<vector<casa::Int> > xmaxW;
  vector<casa::Int > yminBL;
  vector<vector<casa::Int> > yminW;
  vector<casa::Int > ymaxBL;
  vector<vector<casa::Int> > ymaxW;

  casa::Double time0(-1.);//times[blIndex[0]]);
  casa::Int bl_now;//blnr[blIndex[0]]);
  casa::Int iwcoord;//=WPCoord[blIndex[0]]-itsNWPlanes;

  float scaling(2.);
  float support((itsSupport_Speroidal-1)/2+1);
  casa::Int xmin=2147483647;
  casa::Int xmax=-2147483647;
  casa::Int ymin=2147483647;
  casa::Int ymax=-2147483647;

  for(casa::uInt RowNum=0; RowNum<blIndex.size();++RowNum){
    casa::uInt irow=blIndex[RowNum];
      
    double u=vb.uvw()[irow](0);
    double v=vb.uvw()[irow](1);
    double w=vb.uvw()[irow](2);
    double uvdistance=(0.001)*sqrt(u*u+v*v)/(2.99e8/itsListFreq[spw]);
    casa::Bool cond0(((uvdistance>itsUVmin)&(uvdistance<itsUVmax)));
    casa::Bool cond1(abs(w)<itsWMax);
    casa::Bool cond2(((ant1[irow]==5)&(ant2[irow]==40)));
    casa::Double timeRow(times[irow]);
    casa::Bool cond3((timeRow-its_t0)/60.>its_tSel0);
    casa::Bool cond4((timeRow-its_t0)/60.<its_tSel1);
    casa::Bool cond34(cond3&cond4);
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

	xmin=2147483647;
	xmax=-2147483647;
	ymin=2147483647;
	ymax=-2147483647;

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
    
    casa::Int xrow = int(u * uvScale(0) * recipWvl + uvOffset(0));
    casa::Int yrow = int(v * uvScale(1) * recipWvl + uvOffset(1));
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

  /* for(casa::uInt i=0; i<Map.size();++i) */
  /*   { */
  /*     for(casa::uInt j=0; j<Map[i].size();++j) */
  /* 	{ */
	  
  /* 	  for(casa::uInt k=0; k<Map[i][j].size();++k) */
  /* 	    { */
  /* 	      casa::uInt irow=Map[i][j][k]; */
  /* 	      cout<<"iw="<<i<<" ibl="<<j<<" imap="<<k<<" A="<<ant1[irow]<<","<<ant2[irow]<<" w="<<vb.uvw()[irow](2)<<" windex="<<WIndexMap[i]<<" t="<<times[irow]<<endl; */
  /* 	      double u=vb.uvw()[irow](0); */
  /* 	      double v=vb.uvw()[irow](1); */
  /* 	      casa::Int xrow=int(float(u)*scaling); */
  /* 	      casa::Int yrow=int(float(v)*scaling); */
  /* 	      cout<<"   "<<xminW[i][j]<<" ("<<xrow-support<<")"<<endl; */
  /* 	      cout<<"   "<<xmaxW[i][j]<<" ("<<xrow+support<<")"<<endl; */
  /* 	      cout<<"   "<<yminW[i][j]<<" ("<<yrow-support<<")"<<endl; */
  /* 	      cout<<"   "<<ymaxW[i][j]<<" ("<<yrow+support<<")"<<endl; */
  /* 	      cout<<" "<<endl; */
  /* 	    } */
  /* 	} */
  /*   } */

  //  ofstream outFile("output_grids.txt");

  vector<casa::uInt> MapChunckOut;
  vector<vector<casa::uInt> > MapWGridOut;
  vector<vector<vector<casa::uInt> > > MapWOut;
  vector<vector<vector<vector<casa::uInt> > > > MapOut;

  casa::vector<casa::IPosition > posBlock;

  for(casa::uInt i=0; i<Map.size();++i)
    {
      MapWGridOut.resize(0);
      MapWOut.resize(0);
      casa::Vector<casa::Bool> done;
      done.resize(Map[i].size());
      done=false;
      casa::Bool alldone(false);
      casa::Bool cond_xmin,cond_xmax,cond_ymin,cond_ymax;
      casa::uInt iblock(0);
      //MapWGridOut.push_back(Map[i][0]);

      posBlock.resize(0);
      casa::IPosition pos(2,1,1);
      //pos(0)=i;
      //pos(1)=0;
      //posBlock.push_back(pos);

      //cout<<"  plane w="<<i<<" nbl_blocks="<< Map[i].size()<<endl;

      while(!alldone){
	
	for(casa::uInt j=0; j<Map[i].size();++j)
	  {
	    // Find if baseline j has overlap with the current grid
	    if(done[j]==true){continue;}
	    casa::Bool NoOverlap(true);
	    for(casa::uInt jj=0; jj<MapWGridOut.size();++jj)
	      {
		cond_xmin=xminW[i][j]<=xmaxW[posBlock[jj](0)][posBlock[jj](1)];
		cond_xmax=xmaxW[i][j]>=xminW[posBlock[jj](0)][posBlock[jj](1)];
		cond_ymin=yminW[i][j]<=ymaxW[posBlock[jj](0)][posBlock[jj](1)]; 
		cond_ymax=ymaxW[i][j]>=yminW[posBlock[jj](0)][posBlock[jj](1)];
		casa::Bool condIsOverlap(cond_xmin&&cond_xmax&&cond_ymin&&cond_ymax);
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
	for(casa::uInt j=0; j<done.size();++j)
	  {
	    if(done[j]==false){alldone=false;break;}
	  }

	/* for(casa::uInt iii=0; iii<MapWGridOut.size();++iii){ */
	/*   casa::uInt icoord(posBlock[iii](0)); */
	/*   casa::uInt jcoord(posBlock[iii](1)); */
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

  /* for(casa::uInt i=0; i<MapOut.size();++i){ */
  /*   for(casa::uInt j=0; j<MapOut[i].size();++j){ */
  /*     casa::uInt icoord(posBlock[iii](0)); */
  /*     casa::uInt jcoord(posBlock[iii](1)); */
  /*     outFile<<"   "<<i<<" "<<iblock<<" "<<xminW[icoord][jcoord]<<" "<<xmaxW[icoord][jcoord]<<" "<<yminW[icoord][jcoord]<<" "<<ymaxW[icoord][jcoord]<<endl; */
  /*   } */
  /* } */

  return MapOut;

  }


  ///////////////////////////////////////

  FFTCMatrix  FFTMachine;

  void dofft(casa::Matrix<casa::Complex>& arr, bool direction)
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
  void dofftVec(casa::Matrix<casa::Complex>& arr, bool direction, int nth=0, int pol=0)
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
  void store (const casa::DirectionCoordinate &dir, const casa::Matrix<T> &data,
              const string &name)
  {
    //cout<<"Saving... "<<name<<endl;
    casa::Vector<casa::Int> stokes(1);
    stokes(0) = casa::Stokes::I;
    casa::CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(casa::StokesCoordinate(stokes));
    csys.addCoordinate(casa::SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    casa::PagedImage<T> im(casa::TiledShape(casa::IPosition(4, data.shape()(0), data.shape()(1), 1, 1)), csys, name);
    im.putSlice(data, casa::IPosition(4, 0, 0, 0, 0));
  }

  template <class T>
  void store(const casa::Matrix<T> &data, const string &name)
  {
    casa::Matrix<casa::Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    casa::Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> refLatLon(45.0 * casa::C::pi / 180.0, "rad");
    casa::DirectionCoordinate dir(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const casa::Cube<T> &data, const string &name)
  {
    casa::Matrix<casa::Double> xform(2, 2);
    xform = 0.0;
    xform.diagonal() = 1.0;
    casa::Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * casa::C::pi / 180.0, "rad");
    casa::Quantum<casa::Double> refLatLon(45.0 * casa::C::pi / 180.0, "rad");
    casa::DirectionCoordinate dir(casa::MDirection::J2000, casa::Projection(casa::Projection::SIN),
                            refLatLon, refLatLon, incLon, incLat,
                            xform, data.shape()(0) / 2, data.shape()(1) / 2);
    store(dir, data, name);
  }

  template <class T>
  void store(const casa::DirectionCoordinate &dir, const casa::Cube<T> &data,
             const string &name)
  {
//     AlwaysAssert(data.shape()(2) == 4, SynthesisError);
    //cout<<"Saving... "<<name<<endl;
    casa::Vector<casa::Int> stokes(4);
    stokes(0) = casa::Stokes::XX;
    stokes(1) = casa::Stokes::XY;
    stokes(2) = casa::Stokes::YX;
    stokes(3) = casa::Stokes::YY;
    casa::CoordinateSystem csys;
    csys.addCoordinate(dir);
    csys.addCoordinate(casa::StokesCoordinate(stokes));
    csys.addCoordinate(casa::SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));
    casa::PagedImage<T>
      im(casa::TiledShape(casa::IPosition(4, data.shape()(0), data.shape()(1), 4, 1)),
         csys, name);
    im.putSlice(data, casa::IPosition(4, 0, 0, 0, 0));
  }
      /* template <class T> */
      /*   void store(const Cube<T> &data, const string &name) */
      /*   { */

      /*     CoordinateSystem csys; */
      /*     casa::Matrix<casa::Double> xform(2, 2); */
      /*     xform = 0.0; */
      /*     xform.diagonal() = 1.0; */
      /*     Quantum<casa::Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad"); */
      /*     Quantum<casa::Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad"); */
      /*     Quantum<casa::Double> refLatLon(45.0 * C::pi / 180.0, "rad"); */
      /*     csys.addCoordinate(DirectionCoordinate(MDirection::J2000, Projection(Projection::SIN), */
      /*                        refLatLon, refLatLon, incLon, incLat, */
      /*                        xform, data.shape()(0) / 2, data.shape()(1) / 2)); */

      /*     casa::Vector<casa::Int> stokes(4); */
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
