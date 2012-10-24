#ifndef LOFARFT_PYTHONFTMACHINE_H
#define LOFARFT_PYTHONFTMACHINE_H

#include <synthesis/MeasurementComponents/FTMachine.h>
#include <casa/OS/File.h>
#include <casa/OS/PrecTimer.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarFTMachine.h>
#include <LofarFT/VisBufferProxy.h>
#include <LofarFT/ROVisibilityIteratorProxy.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/FFTServer.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <images/Images/ImageProxy.h>


#include <boost/python.hpp>

#include <Common/OpenMP.h>

namespace LOFAR {

class PythonFTMachine : public LofarFTMachine {
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
//        String convType="SF", Float padding=1.0, Bool usezero=True, Bool useDoublePrec=False);
  PythonFTMachine(casa::String pythonmodule, casa::String pythonclass, 
                 casa::Long cachesize, casa::Int tilesize,  casa::CountedPtr<casa::VisibilityResamplerBase>& visResampler,
                 casa::String convType, const casa::MeasurementSet& ms,
                 casa::Int nwPlanes,
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
                );

  // Copy constructor
  PythonFTMachine(const PythonFTMachine &other);

  // Assignment operator
  PythonFTMachine &operator=(const PythonFTMachine &other);

  // Clone
  PythonFTMachine* clone() const;


  ~PythonFTMachine();

  // Initialize transform to Visibility plane using the image
  // as a template. The image is loaded and Fourier transformed.
  void initializeToVis(ImageInterface<Complex>& image,
                       const VisBuffer& vb);
  void _initializeToVis(ImageProxy& image_in,  const VisBufferProxy& vb_in);

  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void finalizeToVis();
  void _finalizeToVis();

  // Initialize transform to Sky plane: initializes the image
  void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
                       const VisBuffer& vb);
  
  void _initializeToSky(ImageProxy& image_in,  const ValueHolder& weight_in, const VisBufferProxy& vb_in);

  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. DOES NOT
  // DO THE FINAL TRANSFORM!
  void finalizeToSky();
  void _finalizeToSky();


  // Get actual coherence from grid by degridding
  void get(VisBuffer& vb, Int row=-1);
  void _get(VisBufferProxy& vb_in, Int row);


  // Put coherence to grid by gridding.
  void put(const VisBuffer& vb, Int row=-1, Bool dopsf=False, FTMachine::Type type=FTMachine::OBSERVED);
  void _put(const VisBufferProxy& vb, Int row=-1, Bool dopsf=False, FTMachine::Type type=FTMachine::OBSERVED);

  // Make the entire image
  void makeImage(FTMachine::Type type,
                 VisSet& vs,
                 ImageInterface<Complex>& image,
                 Matrix<Float>& weight);
  void _makeImage(FTMachine::Type type,
                 VisSet& vs,
                 ImageProxy& image,
                 const ValueHolder& weight);
  
  // Make the entire image
  void makeImage(FTMachine::Type type,
                 ROVisibilityIterator& vi,
                 ImageInterface<Complex>& image,
                 Matrix<Float>& weight);
  ValueHolder _makeImage(FTMachine::Type type,
                 ROVisibilityIteratorProxy& vi,
                 ImageProxy& image,
                 const ValueHolder& weight);
  
 

  // Get the final image: do the Fourier transform and
  // grid-correct, then optionally normalize by the summed weights
  ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);

  boost::python::object _getImage(ValueHolder, Bool normalize=True);


  
  
  //   // Get the average primary beam.
//   Matrix<Float>& getAveragePB();
// 
//   // Get the spheroidal cut.
//   const Matrix<Float>& getSpheroidCut() const
//     { return itsConvFunc->getSpheroidCut(); }
// 
// 
//   void normalizeAvgPB();
//   void normalizeAvgPB(ImageInterface<Complex>& inImage,
//                       ImageInterface<Float>& outImage);
//     //
//     // Make a sensitivity image (sensitivityImage), given the gridded
//     // weights (wtImage).  These are related to each other by a
//     // Fourier transform and normalization by the sum-of-weights
//     // (sumWt) and normalization by the product of the 2D FFT size
//     // along each axis.  If doFFTNorm=False, normalization by the FFT
//     // size is not done.  If sumWt is not provided, normalization by
//     // the sum of weights is also not done.
//     //
// 
// 
// 
//     virtual void makeSensitivityImage(Lattice<Complex>&,
//                                       ImageInterface<Float>&,
//                                       const Matrix<Float>& =Matrix<Float>(),
//                                       const Bool& =True) {}
//     virtual void makeSensitivityImage(const VisBuffer& vb, const ImageInterface<Complex>& imageTemplate,
//                                       ImageInterface<Float>& sensitivityImage);
// 
//     inline virtual Float pbFunc(const Float& a, const Float& limit)
//     {if (abs(a) >= limit) return (a);else return 1.0;};
//     inline virtual Complex pbFunc(const Complex& a, const Float& limit)
//     {if (abs(a)>=limit) return (a); else return Complex(1.0,0.0);};
//     //
//     // Given the sky image (Fourier transform of the visibilities),
//     // sum of weights and the sensitivity image, this method replaces
//     // the skyImage with the normalized image of the sky.
//     //
//     virtual void normalizeImage(Lattice<Complex>& skyImage,
//                                 const Matrix<Double>& sumOfWts,
//                                 Lattice<Float>& sensitivityImage,
//                                 Bool fftNorm=True);
//     virtual void normalizeImage(Lattice<Complex>& skyImage,
//                                 const Matrix<Double>& sumOfWts,
//                                 Lattice<Float>& sensitivityImage,
//                                 Lattice<Complex>& sensitivitySqImage,
//                                 Bool fftNorm=True);
// 
//     virtual ImageInterface<Float>& getSensitivityImage() {return *avgPB_p;}
//     virtual Matrix<Double>& getSumOfWeights() {return sumWeight;};
//     virtual Matrix<Double>& getSumOfCFWeights() {return sumCFWeight;};
// 
//   // Get the final weights image
//   void getWeightImage(ImageInterface<Float>&, Matrix<Float>&);
// 
//   // Save and restore the LofarFTMachine to and from a record
//   virtual Bool toRecord(String& error, RecordInterface& outRec,
//                         Bool withImage=False);
//   virtual Bool fromRecord(String& error, const RecordInterface& inRec);
// 
//   // Can this FTMachine be represented by Fourier convolutions?
//   virtual Bool isFourier() {return True;}
// 
//   virtual void setNoPadding(Bool nopad){noPadding_p=nopad;};
// 
//   virtual String name();
//   //virtual void setMiscInfo(const Int qualifier){(void)qualifier;};
// 
//   //Cyr: The FTMachine has got to know the order of the Taylor term
//   virtual void setMiscInfo(const Int qualifier){thisterm_p=qualifier;};
//   virtual void ComputeResiduals(VisBuffer&vb, Bool useCorrected);
// 
// 
//   void makeConjPolMap(const VisBuffer& vb, const Vector<Int> cfPolMap, Vector<Int>& conjPolMap);
//   //    Vector<Int> makeConjPolMap(const VisBuffer& vb);
//   void makeCFPolMap(const VisBuffer& vb, const Vector<Int>& cfstokes, Vector<Int>& polM);
// 
//   String itsNamePsfOnDisk;
//   void setPsfOnDisk(String NamePsf){itsNamePsfOnDisk=NamePsf;}
//   virtual String GiveNamePsfOnDisk(){return itsNamePsfOnDisk;}
  

protected:
  boost::python::object pyftmachine;
  boost::python::object pyrap_images_image;
  boost::python::object lofar_imager_module;
};

} //# end namespace

#endif
