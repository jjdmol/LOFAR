//# FTMachine.h: Definition for FTMachine
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

#ifndef LOFAR_LOFARFT_FTMACHINE_H
#define LOFAR_LOFARFT_FTMACHINE_H

#include <LofarFT/DynamicObjectFactory.h>
#include <LofarFT/VisResampler.h>
#include <LofarFT/ConvolutionFunction.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <synthesis/MSVis/VisBuffer.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Containers/SimOrdMap.h>
#include <casa/OS/Mutex.h>
#include <casa/OS/PrecTimer.h>
#include <casa/Arrays/Matrix.h>
#include <images/Images/ImageInterface.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <scimath/Mathematics/FFTServer.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>

namespace LOFAR {
namespace LofarFT {
  
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

class FTMachine : public casa::FTMachine {
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
//  LofarFTMachineOld(Long cachesize, Int tilesize, CountedPtr<VisibilityResamplerBase>& visResampler,
//	  String convType="SF", Float padding=1.0, Bool usezero=True, Bool useDoublePrec=False);
  
  FTMachine(
    const casa::MeasurementSet& ms, 
    const casa::Record& parameters);

  // Copy constructor
  FTMachine(const FTMachine &other);

  // Assignment operator
  FTMachine &operator=(const FTMachine &other);

  // Clone
  virtual FTMachine* clone() const = 0;
  
  // Clone
  // casa::FTMachine declares the virtual clone method as cloneFTM
  virtual casa::FTMachine* cloneFTM() {return clone();}


  ~FTMachine();
  
  // Show the relative timings of the various steps.
  void showTimings (std::ostream&, double duration) const;

  // Initialize transform to Visibility plane using the image
  // as a template. The image is loaded and Fourier transformed.
  using casa::FTMachine::initializeToVis;
  void initializeToVis(
    casa::ImageInterface<casa::Complex>& image,
    const casa::VisBuffer& vb);

  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void finalizeToVis();

  // Initialize transform to Sky plane: initializes the image
  using casa::FTMachine::initializeToSky;
  void initializeToSky(
    casa::ImageInterface<casa::Complex>& image,  
    casa::Matrix<casa::Float>& weight,
    const casa::VisBuffer& vb);

  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. DOES NOT
  // DO THE FINAL TRANSFORM!
  using casa::FTMachine::finalizeToSky;
  void finalizeToSky();

  // Make the entire image
  using casa::FTMachine::makeImage;
  void makeImage(
    casa::FTMachine::Type type,
    casa::ROVisibilityIterator& vi,
    casa::ImageInterface<casa::Complex>& image,
    casa::Matrix<casa::Float>& weight);

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  casa::ImageInterface<casa::Complex>& getImage(
    casa::Matrix<casa::Float>&, 
    casa::Bool normalize = casa::True);

  // Get the average primary beam.
  const casa::Matrix<casa::Float>& getAveragePB() const;

  // Get the spheroidal cut.
  const casa::Matrix<casa::Float>& getSpheroidCut() const
    { return itsConvFunc->getSpheroidCut(); }


  ///  virtual void normalizeImage(Lattice<Complex>& skyImage,
  ///			      const Matrix<Double>& sumOfWts,
  ///			      Lattice<Float>& sensitivityImage,
  ///			      Bool fftNorm)
  ///    {throw(AipsError("LofarFTMachineOld::normalizeImage() called"));}

  void normalizeAvgPB();
  void normalizeAvgPB(casa::ImageInterface<casa::Complex>& inImage,
                      casa::ImageInterface<casa::Float>& outImage);
    //
    // Make a sensitivity image (sensitivityImage), given the gridded
    // weights (wtImage).  These are related to each other by a
    // Fourier transform and normalization by the sum-of-weights
    // (sumWt) and normalization by the product of the 2D FFT size
    // along each axis.  If doFFTNorm=False, normalization by the FFT
    // size is not done.  If sumWt is not provided, normalization by
    // the sum of weights is also not done.
    //
  


    virtual void makeSensitivityImage(
      casa::Lattice<casa::Complex>&,
      casa::ImageInterface<casa::Float>&,
      const casa::Matrix<casa::Float>& = casa::Matrix<casa::Float>(),
      const casa::Bool& = casa::True) {}
				      
    virtual void makeSensitivityImage(
      const casa::VisBuffer& vb, 
      const casa::ImageInterface<casa::Complex>& imageTemplate,
      casa::ImageInterface<casa::Float>& sensitivityImage);

    inline virtual casa::Float pbFunc(
      const casa::Float& a, 
      const casa::Float& limit)
    {
      if (abs(a) >= limit) 
      {
        return (a);
      }
      else
      {
        return 1.0;
      };
    }
    
    inline virtual casa::Complex pbFunc(
      const casa::Complex& a, 
      const casa::Float& limit)
    {
      if (abs(a)>=limit)
      {
        return (a);
      }
      else
      {
        return casa::Complex(1.0,0.0);
      };
    }
    
    //
    // Given the sky image (Fourier transform of the visibilities),
    // sum of weights and the sensitivity image, this method replaces
    // the skyImage with the normalized image of the sky.
    //
    using casa::FTMachine::normalizeImage;
    virtual void normalizeImage(
      casa::Lattice<casa::Complex>& skyImage,
      const casa::Matrix<casa::Double>& sumOfWts,
      casa::Lattice<casa::Float>& sensitivityImage,
      casa::Bool fftNorm = casa::True);
    
    virtual void normalizeImage(
      casa::Lattice<casa::Complex>& skyImage,
      const casa::Matrix<casa::Double>& sumOfWts,
      casa::Lattice<casa::Float>& sensitivityImage,
      casa::Lattice<casa::Complex>& sensitivitySqImage,
      casa::Bool fftNorm = casa::True);

    virtual casa::ImageInterface<casa::Float>& getSensitivityImage() {return *itsAvgPBImage;}
    
    virtual casa::Matrix<casa::Double>& getSumOfWeights() {return sumWeight;};
    
    virtual casa::Matrix<casa::Double>& getSumOfCFWeights() {return sumCFWeight;};

  // Get the final weights image
  void getWeightImage(casa::ImageInterface<casa::Float>&, casa::Matrix<casa::Float>&);

  // Can this FTMachine be represented by Fourier convolutions?
  virtual casa::Bool isFourier() 
  {
    return casa::True;
  }

  virtual void setNoPadding(casa::Bool nopad)
  {
    itsNoPadding = nopad;
  };

  virtual void setMiscInfo(const casa::Int qualifier){(void)qualifier;};
  
  virtual void ComputeResiduals(
    casa::VisBuffer&vb, 
    casa::Bool useCorrected);

  void makeConjPolMap(
    const casa::VisBuffer& vb, 
    const casa::Vector<casa::Int> cfPolMap, 
    casa::Vector<casa::Int>& conjPolMap);
  
  //    Vector<Int> makeConjPolMap(const VisBuffer& vb);
  
  void makeCFPolMap(
    const casa::VisBuffer& vb, 
    const casa::Vector<casa::Int>& cfstokes, 
    casa::Vector<casa::Int>& polM);

protected:
  
  casa::ImageInterface<casa::Complex>* &itsImage; // reference to casa::FTMachine::image
  casa::Int &itsNX; // reference to casa::FTMachine::nx
  casa::Int &itsNY; // reference to casa::FTMachine::ny
  casa::Int &itsNPol; // reference to casa::FTMachine::npol
  casa::Int &itsNChan; // reference to casa::FTMachine::nchan
  casa::Bool &itsUseDoubleGrid; // reference to casa::FTMachine::useDoubleGrid_p
  casa::Vector<casa::Int> &itsChanMap; // reference to casa::FTMachine::chanMap
  casa::Vector<casa::Int> &itsPolMap; // reference to casa::FTMachine::polMap

  // Padding in FFT
  casa::Float itsPadding;

  void ok();

  void init();

  // Below are references to data members of casa::FTMachine
  // They function as aliases for the casa::FTMachine data members
  // The names of the references follow the naming convention of the LOFAR C++ coding standard
  // The references are initialized in the constructor
  

  // Is this record on Grid? check both ends. This assumes that the
  // ends bracket the middle
  casa::Bool recordOnGrid(const casa::VisBuffer& vb, casa::Int rownr) const;

  // Image cache
  casa::LatticeCache<casa::Complex> * itsImageCache;

  casa::CountedPtr<casa::Lattice<casa::Complex> > itsLattice;

  casa::String itsConvType;

  casa::Float itsMaxAbsData;

  // Useful IPositions
  casa::IPosition itsCenterLoc;
  casa::IPosition itsOffsetLoc;

  // Image Scaling and offset
  casa::Vector<casa::Double> itsUVScale;
  casa::Vector<casa::Double> itsUVOffset;

  // Arrays for non-tiled gridding (one per thread).
  vector< casa::Array<casa::Complex> >  itsGriddedData;
  vector< casa::Array<casa::DComplex> > itsGriddedData2;
  vector< casa::Matrix<casa::Complex> > itsSumPB;
  vector< casa::Matrix<casa::Double> >  itsSumWeight;
  vector< double > itsSumCFWeight;
  mutable casa::Matrix<casa::Float> itsAvgPB;

  casa::Int itsPriorCacheSize;

  //force no padding
  casa::Bool itsNoPadding;

  //Check if using put that avoids non-necessary reads
  casa::Bool itsUsePut2;

  casa::Record itsParameters;
  
  //machine name
  casa::String itsMachineName;

  // Shape of the padded image
  casa::IPosition itsPaddedShape;

  casa::Int convSampling;
  casa::Float pbLimit_p;
  casa::Int sensitivityPatternQualifier_p;
  casa::String sensitivityPatternQualifierStr_p;
  casa::Vector<casa::Float> pbPeaks;
  casa::Bool pbNormalized_p;
  // The average PB for sky image normalization
  //
  casa::CountedPtr<casa::ImageInterface<casa::Float> > itsAvgPBImage;
  casa::CountedPtr<casa::ImageInterface<casa::Complex> > itsAvgPBSqImage;

  casa::CountedPtr<VisResampler> itsVisResampler;

  casa::MeasurementSet itsMS;
  casa::Int itsNWPlanes;
  double itsWMax;
  int itsNThread;
  int itsNGrid;

  casa::CountedPtr<ConvolutionFunction> itsConvFunc;
  casa::Vector<casa::Int> itsConjCFMap;
  casa::Vector<casa::Int> itsCFMap;
  casa::String itsBeamPath;
  int itsVerbose;
  int itsMaxSupport;
  casa::Int itsOversample;
  casa::String itsImageName;
  casa::Matrix<casa::Bool> itsGridMuellerMask;
  casa::Matrix<casa::Bool> itsDegridMuellerMask;
  double itsGriddingTime;
  double itsDegriddingTime;
  double itsCFTime;
  casa::PrecTimer itsTotalTimer;
  
};

// Factory that can be used to generate new Command objects.
// The factory is defined as a singleton.
typedef Singleton<DynamicObjectFactory<FTMachine*(const casa::MeasurementSet& ms, const casa::Record& parameters)> > FTMachineFactory;


} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
