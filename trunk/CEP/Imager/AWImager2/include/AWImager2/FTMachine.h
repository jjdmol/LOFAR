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

#include <AWImager2/DynamicObjectFactory.h>
#include <AWImager2/VisResampler.h>
#include <AWImager2/ConvolutionFunction.h>
#include <AWImager2/VisBuffer.h>
#include <Common/ParameterSet.h>
#include <synthesis/TransformMachines/FTMachine.h>
#include <msvis/MSVis/VisBuffer.h>
#include <casacore/casa/Arrays/Array.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/casa/Containers/SimOrdMap.h>
#include <casacore/casa/Containers/Block.h>
#include <casacore/casa/OS/Mutex.h>
#include <casacore/casa/OS/PrecTimer.h>
#include <casacore/casa/Arrays/Matrix.h>
#include <casacore/images/Images/ImageInterface.h>
#include <casacore/scimath/Mathematics/ConvolveGridder.h>
#include <casacore/scimath/Mathematics/FFTServer.h>
#include <casacore/lattices/Lattices/LatticeCache.h>
#include <casacore/lattices/Lattices/ArrayLattice.h>

namespace LOFAR {
namespace LofarFT {
  
// <summary>  An FTMachine for Gridded Fourier transforms </summary>



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
  
  enum domain 
  {
    IMAGE=0,
    UV
  };
  
  FTMachine(
    const casa::MeasurementSet& ms, 
    const LOFAR::ParameterSet& parset);

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
  virtual void initializeToVis(
    casa::PtrBlock<casa::ImageInterface<casa::Float>* > &model_images, 
    casa::Bool normalize);
  
  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void finalizeToVis();

  // Initialize transform to Sky plane: initializes the image
  
  virtual void initializeToSky(
    casa::PtrBlock<casa::ImageInterface<casa::Float>* > &images,
    casa::Bool doPSF);

  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. 
  // DOES *NOT* DO THE FINAL TRANSFORM!
  virtual void finalizeToSky();
  
  virtual void initializeResidual(
    casa::PtrBlock<casa::ImageInterface<casa::Float>* > model_images,
    casa::PtrBlock<casa::ImageInterface<casa::Float>* > images,
    casa::Bool normalize);

  virtual void finalizeResidual();
  
  virtual void get(casa::VisBuffer& vb, casa::Int row=-1);
  virtual void get(VisBuffer& vb, casa::Int row=-1)=0;

  virtual void put(
    const casa::VisBuffer& vb, 
    casa::Int row = -1, 
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
  
  virtual void put(
    const VisBuffer& vb, 
    casa::Int row = -1, 
    casa::Bool dopsf = casa::False,
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED)=0;

  virtual void residual(
    VisBuffer& vb, 
    casa::Int row = -1, 
    casa::FTMachine::Type type = casa::FTMachine::OBSERVED);
  
  // Make the entire image
  using casa::FTMachine::makeImage;
  void makeImage(
    casa::FTMachine::Type type,
    casa::ROVisibilityIterator& vi,
    casa::ImageInterface<casa::Float>& image,
    casa::Matrix<casa::Float>& weight);

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  virtual void getImages(
    casa::Matrix<casa::Float>& weights, 
    casa::Bool normalize);
  
  // Get the average primary beam.
  virtual casa::Matrix<casa::Float> getAveragePB();

  // Get the spheroidal cut.
  virtual casa::Matrix<casa::Float> getSpheroidal();



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
    
  // Can this FTMachine be represented by Fourier convolutions?
  virtual casa::Bool isFourier() 
  {
    return casa::True;
  }

  virtual void setMiscInfo(const casa::Int qualifier){(void)qualifier;};
  
  void getWeightImage(casa::ImageInterface<casa::Float>& weightImage, casa::Matrix<casa::Float>& weights);

  
  // pure virtual functions that we do not use, 
  // implementation only throws a not implemented exception
  
  virtual void initializeToVis(casa::ImageInterface<casa::Complex>& image, const casa::VisBuffer& vb);
  virtual void initializeToSky(casa::ImageInterface<casa::Complex>& image, casa::Matrix<casa::Float>& weight, const casa::VisBuffer& vb);
  
  virtual casa::ImageInterface<casa::Complex>& getImage(
    casa::Matrix<casa::Float>&, 
    casa::Bool normalize = casa::True);

  virtual void ComputeResiduals(
    casa::VisBuffer&vb, 
    casa::Bool useCorrected);

   
  
protected:
  
  virtual void initialize_model_grids(casa::Bool normalize);
  
  void finalize_model_grids();

  void initialize_grids();

  void normalize(casa::ImageInterface<casa::Complex> &image, casa::Bool normalize, casa::Bool spheroidal);
  
  casa::StokesCoordinate get_stokes_coordinates();
  
  // the images and model images are owned by SkyModel
  // can use a raw pointer here
  casa::PtrBlock<casa::ImageInterface<casa::Float> *> itsModelImages; 
  casa::PtrBlock<casa::ImageInterface<casa::Float>*> itsImages;

  // the complex images and complex model images are created locally
  // use a counted pointer to ensure proper desctruction  
  casa::Block<casa::CountedPtr<casa::ImageInterface<casa::Complex> > > itsComplexModelImages;
  casa::Block<casa::CountedPtr<casa::ImageInterface<casa::Complex> > > itsComplexImages;

  casa::Block<casa::Array<casa::Complex> >  itsModelGrids;

  // Arrays for non-tiled gridding (one per thread).
  vector< casa::Array<casa::Complex> >  itsGriddedData;
  vector< casa::Array<casa::DComplex> > itsGriddedData2;
  domain itsGriddedDataDomain;

  casa::Bool itsNormalizeModel;
  casa::Int itsNX; 
  casa::Int itsNY; 
  casa::Int itsPaddedNX; 
  casa::Int itsPaddedNY; 
  casa::Int itsNPol; 
  casa::Int itsNChan; 
  
  casa::Bool itsUseDoubleGrid; 
  casa::Vector<casa::Int> itsChanMap;
  casa::Vector<casa::Int> itsPolMap;

  // Padding in FFT
  casa::Float itsPadding;

  void ok();

  void init(const casa::ImageInterface<casa::Float> &image);

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

  vector< casa::Matrix<casa::Complex> > itsSumPB;
  vector< casa::Matrix<casa::Double> >  itsSumWeight;
  vector< double > itsSumCFWeight;
  mutable casa::Matrix<casa::Float> itsAveragePB;
  mutable casa::Matrix<casa::Float> itsSpheroidal;

  casa::Int itsPriorCacheSize;

  //Check if using put that avoids non-necessary reads
  casa::Bool itsUsePut2;

  LOFAR::ParameterSet itsParset;
  
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
  virtual VisResampler* visresampler() {return &*itsVisResampler;}
  

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
//  casa::Matrix<casa::Bool> itsGridMuellerMask;
//  casa::Matrix<casa::Bool> itsDegridMuellerMask;
  double itsGriddingTime;
  double itsDegriddingTime;
  double itsCFTime;
  casa::PrecTimer itsTotalTimer;
  
};

// Factory that can be used to generate new FTMachine objects.
// The factory is defined as a singleton.
typedef Singleton<DynamicObjectFactory<FTMachine*(const casa::MeasurementSet& ms, const ParameterSet& parset)> > FTMachineFactory;


} //# end namespace LofarFT
} //# end namespace LOFAR

#endif
