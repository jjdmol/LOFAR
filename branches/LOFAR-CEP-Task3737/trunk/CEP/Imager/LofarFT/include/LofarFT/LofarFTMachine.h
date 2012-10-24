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
		 Double PBCut,
		 Bool PredictFT,
		 String PsfOnDisk,
		 Bool UseMasksDegrid,
		 Bool ReallyDoPSF,
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
  void setPsfOnDisk(String NamePsf){itsNamePsfOnDisk=NamePsf;}
  virtual String GiveNamePsfOnDisk(){return itsNamePsfOnDisk;}
  

protected:
  // Padding in FFT
  Float padding_p;
  Int thisterm_p;
  Double itsRefFreq;
  Bool itsPredictFT;
  Int itsTotalStepsGrid;
  Int itsTotalStepsDeGrid;
  Bool itsMasksAllDone;
  Bool its_UseMasksDegrid;
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
  void SumGridsOMP(Array<Complex>& grid, const Array<Complex>& GridToAdd){
    int y,ch,pol;
    int GridSize(grid.shape()[0]);
    int NPol(grid.shape()[2]);
    int NChan(grid.shape()[3]);
    Complex* gridPtr;
    const Complex* GridToAddPtr;
    
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

  void SumGridsOMP(Array<Complex>& grid, const vector< Array<Complex> >& GridToAdd0 ){

    for(uInt vv=0; vv<GridToAdd0.size();vv++){
      Array<Complex> GridToAdd(GridToAdd0[vv]);
      int y,ch,pol;
      int GridSize(grid.shape()[0]);
      int NPol(grid.shape()[2]);
      int NChan(grid.shape()[3]);
      Complex* gridPtr;
      const Complex* GridToAddPtr;
      
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

  // Arrays for non-tiled gridding (one per thread).
  vector< Array<Complex> >  itsGriddedData;
  Array<Complex> its_stacked_GriddedData;

  vector< Array<DComplex> > itsGriddedData2;
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
  Bool its_Apply_Element;
  Bool its_Already_Initialized;
  Bool                its_reallyDoPSF;
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



      template <class T>
        void store(const Cube<T> &data, const string &name)
        {

          CoordinateSystem csys;
          Matrix<Double> xform(2, 2);
          xform = 0.0;
          xform.diagonal() = 1.0;
          Quantum<Double> incLon((8.0 / data.shape()(0)) * C::pi / 180.0, "rad");
          Quantum<Double> incLat((8.0 / data.shape()(1)) * C::pi / 180.0, "rad");
          Quantum<Double> refLatLon(45.0 * C::pi / 180.0, "rad");
          csys.addCoordinate(DirectionCoordinate(MDirection::J2000, Projection(Projection::SIN),
                             refLatLon, refLatLon, incLon, incLat,
                             xform, data.shape()(0) / 2, data.shape()(1) / 2));

          Vector<Int> stokes(4);
          stokes(0) = Stokes::XX;
          stokes(1) = Stokes::XY;
          stokes(2) = Stokes::YX;
          stokes(3) = Stokes::YY;
          csys.addCoordinate(StokesCoordinate(stokes));
          csys.addCoordinate(SpectralCoordinate(casa::MFrequency::TOPO, 60e6, 0.0, 0.0, 60e6));

          PagedImage<T> im(TiledShape(IPosition(4, data.shape()(0), data.shape()(1), 4, 1)), csys, name);
          im.putSlice(data, IPosition(4, 0, 0, 0, 0));
        }


};

} //# end namespace

#endif
