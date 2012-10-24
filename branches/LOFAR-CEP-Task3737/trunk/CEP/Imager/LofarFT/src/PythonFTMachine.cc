#include <lofar_config.h>
#include <LofarFT/PythonFTMachine.h>
#include <LofarFT/VisBufferProxy.h>
#include <LofarFT/VisibilityResamplerProxy.h>
#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycValueHolder.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycArray.h>
#include <images/Images/ImageProxy.h>


using namespace casa;
using namespace boost::python;

namespace LOFAR {
  
  PythonFTMachine::PythonFTMachine(String pythonmodule, String pythonclassname, 
                 Long cachesize, Int tilesize,  CountedPtr<VisibilityResamplerBase>& visResampler, String convType, const MeasurementSet& ms,
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
                ) : LofarFTMachine(cachesize, tilesize,  visResampler, convType, ms, nwPlanes, mLocation, padding, usezero, useDoublePrec, wmax,
                                   verbose, maxsupport, oversample, imageName, gridMuellerMask, degridMuellerMask, RefFreq, Use_Linear_Interp_Gridder,
                                   Use_EJones, StepApplyElement, PBCut, PredictFT, PsfOnDisk, UseMasksDegrid, ReallyDoPSF, parameters)
 {
  
    Py_Initialize();
    casa::pyrap::register_convert_excp();
    casa::pyrap::register_convert_basicdata();
    casa::pyrap::register_convert_casa_valueholder();
    casa::pyrap::register_convert_casa_record();
    try {
      object main_module = import("__main__");
      lofar_imager_module = import("lofar.imager");
      object embedded_module = import(pythonmodule.c_str());
      object pyFTMachine = embedded_module.attr(pythonclassname.c_str());
      pyftmachine = pyFTMachine(ptr(this));
      object pyrap_images_module = import("pyrap.images");
      pyrap_images_image = pyrap_images_module.attr("image");
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }
  
  // Copy constructor
  PythonFTMachine::PythonFTMachine(const PythonFTMachine &other) : LofarFTMachine() {
    operator=(other);
  }
  
  // Assignment operator
  PythonFTMachine& PythonFTMachine::operator=(const PythonFTMachine &other)
  {
    if(this!=&other) 
    {
      PythonFTMachine::operator=(other);
      pyftmachine = other.pyftmachine;
    }
    return *this;
  }

  // Clone
  PythonFTMachine* PythonFTMachine::clone() const {
    cout << "Cloning" << endl;
  }
  
  PythonFTMachine::~PythonFTMachine() {
  }
  
  
  void PythonFTMachine::initializeToVis(ImageInterface<Complex>& image, const VisBuffer& vb) {
    try {
      pyftmachine.attr("initializeToVis")(image, VisBufferProxy(vb));
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }
  // Initialize transform to Visibilities
  // Forward calls from python to base class
  void PythonFTMachine::_initializeToVis(ImageProxy& image_in, const VisBufferProxy& vb_in) {
    ImageInterface<Complex>* image_out = dynamic_cast<ImageInterface<Complex>*>(image_in.getLattice());
    const VisBuffer *vb_out = vb_in.getConstVisBuffer();
    LofarFTMachine::initializeToVis(*image_out, *vb_out);
  }

  // Finalize transform to Visibility plane: flushes the image
  // cache and shows statistics if it is being used.
  void PythonFTMachine::finalizeToVis() {
    try {
      pyftmachine.attr("finalizeToVis")();
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }

  // Initialize transform to Sky plane: initializes the image
  void PythonFTMachine::initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight, const VisBuffer& vb) {
    try {
      ImageProxy imageproxy(CountedPtr<LatticeBase>(&image, False));
      object imageobject = pyrap_images_image(object(imageproxy));
      VisBufferProxy vbp(vb);
//       cout << "vbp.isConstant = " << vbp.isConstant << endl
      pyftmachine.attr("initializeToSky")(imageobject, ValueHolder(weight), vbp);
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }

  // Initialize transform to Sky plane: initializes the image
  // Forward calls from python to base class
  void PythonFTMachine::_initializeToSky(ImageProxy& image_in, const ValueHolder& weight_in, const VisBufferProxy& vb_in) {
    ImageInterface<Complex>* image_out = dynamic_cast<ImageInterface<Complex>*>(image_in.getLattice());
    Matrix<Float> weight_out = weight_in.asArrayFloat();
    const VisBuffer *vb_out = vb_in.getConstVisBuffer();
    LofarFTMachine::initializeToSky(*image_out,  weight_out,  *vb_out);
  }

  // Finalize transform to Sky plane: flushes the image
  // cache and shows statistics if it is being used. DOES NOT
  // DO THE FINAL TRANSFORM!
  void PythonFTMachine::finalizeToSky() {
    try {
      pyftmachine.attr("finalizeToSky")();
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }

  void PythonFTMachine::_finalizeToSky() {
    LofarFTMachine::finalizeToSky();
  }
  
  // Get actual coherence from grid by degridding
  void PythonFTMachine::get(VisBuffer& vb, Int row) {
    try {
      pyftmachine.attr("get")(VisBufferProxy(vb), row);
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }
  
  void PythonFTMachine::_get(VisBufferProxy& vb_in, Int row) {
    VisBuffer *vb_out = vb_in.getVisBuffer();
    LofarFTMachine::get(*vb_out, row);
  }

  void PythonFTMachine::put(const VisBuffer& vb, Int row, Bool dopsf, FTMachine::Type type) {
    try {
      pyftmachine.attr("put")(VisBufferProxy(vb), row, dopsf, type);
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }
  
  void PythonFTMachine::_put(const VisBufferProxy& vb_in, Int row, Bool dopsf, FTMachine::Type type) {
    const VisBuffer *vb_out = vb_in.getConstVisBuffer();
    LofarFTMachine::put(*vb_out, row, dopsf, type);
  }
  
  // Make the entire image
  void PythonFTMachine::makeImage(FTMachine::Type type,
                 VisSet& vs,
                 ImageInterface<Complex>& image,
                 Matrix<Float>& weight) {
    ImageProxy imageproxy(CountedPtr<LatticeBase>(&image, False));
    object imageobject = pyrap_images_image(object(imageproxy));
    try {
      pyftmachine.attr("makeImage")(type, vs, imageobject, ValueHolder(weight));
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
  }

  void PythonFTMachine::_makeImage(FTMachine::Type type,
                 VisSet& vs,
                 ImageProxy& image_in,
                 const ValueHolder& weight_in) {
    ImageInterface<Complex>* image_out = dynamic_cast<ImageInterface<Complex>*>(image_in.getLattice());
    Matrix<Float> weight_out = weight_in.asArrayFloat();
    LofarFTMachine::makeImage(type, vs, *image_out, weight_out);
  }
  
  void PythonFTMachine::makeImage(FTMachine::Type type,
                 ROVisibilityIterator& vi,
                 ImageInterface<Complex>& image,
                 Matrix<Float>& weight) {
    
    ImageProxy imageproxy(CountedPtr<LatticeBase>(&image, False));
    object imageobject = pyrap_images_image(object(imageproxy));
    object ROVisibilityIterator_class = lofar_imager_module.attr("ROVisibilityIterator");
    object weight_object = object(ValueHolder(weight));
    {
      tuple shape = extract<tuple>(weight_object.attr("shape"));
      cout << "weight.shape = " << extract<int>(shape[0]) << ", " << extract<int>(shape[1]) << endl;
    }
    try {
      pyftmachine.attr("makeImage")(type, ROVisibilityIterator_class(ROVisibilityIteratorProxy(vi)), imageobject, weight_object);
    }
    catch(error_already_set const &)
    {
      // handle the exception in some way
      PyErr_Print();
    }
    {
      tuple shape = extract<tuple>(weight_object.attr("shape"));
      cout  << "weight.shape = " << extract<int>(shape[0]) << ", " << extract<int>(shape[1]) << endl;
    }
  }

  ValueHolder PythonFTMachine::_makeImage( FTMachine::Type type,
                 ROVisibilityIteratorProxy& vi_in,
                 ImageProxy& image_in,
                 const ValueHolder& weight_in) {
    ROVisibilityIterator* vi_out = vi_in.getROVisibilityIterator();
    ImageInterface<Complex>* image_out = dynamic_cast<ImageInterface<Complex>*>(image_in.getLattice());
    Matrix<Float> weight_out = weight_in.asArrayFloat();
    FTMachine::makeImage(type, *vi_out, *image_out, weight_out);
    return ValueHolder(weight_out);
  }
  
  ImageInterface<Complex>& PythonFTMachine::getImage(Matrix<Float>& weights, Bool normalize) {
    cout << weights.shape() << endl;
    object weight_object = object(ValueHolder(weights));
    pyftmachine.attr("getImage")(weight_object, normalize);
    ValueHolder v = extract<ValueHolder>(weight_object);
    Matrix<Float> a(v.asArrayFloat());
    cout << "getimage: " << a.shape() << endl;
    weights = a;
    cout << "getimage: " <<  weights.shape() << endl;
  }

  object PythonFTMachine::_getImage(ValueHolder weights, Bool normalize) {
    Matrix<Float> weights_array = weights.asArrayFloat();
    ImageInterface<Complex> *image = &LofarFTMachine::getImage(weights_array, normalize);
    ImageProxy imageproxy(CountedPtr<LatticeBase>(image, False));
    weights = ValueHolder(weights_array);
    return make_tuple(pyrap_images_image(object(imageproxy)), weights);
  }
  
//   // Get the average primary beam.
//   Matrix<Float>& PythonFTMachine::getAveragePB(){
//   }
// 
//   void PythonFTMachine::normalizeAvgPB() {
//   }
//   
//   void PythonFTMachine::normalizeAvgPB(ImageInterface<Complex>& inImage,
//                       ImageInterface<Float>& outImage) {
//   }
//   
//   void PythonFTMachine::makeSensitivityImage(const VisBuffer& vb, const ImageInterface<Complex>& imageTemplate,
//                                       ImageInterface<Float>& sensitivityImage) {
//   }
// 
//   void PythonFTMachine::normalizeImage(Lattice<Complex>& skyImage,
//                       const Matrix<Double>& sumOfWts,
//                       Lattice<Float>& sensitivityImage,
//                       Bool fftNorm) {
//   }
//   
//   void PythonFTMachine::normalizeImage(Lattice<Complex>& skyImage,
//                       const Matrix<Double>& sumOfWts,
//                       Lattice<Float>& sensitivityImage,
//                       Lattice<Complex>& sensitivitySqImage,
//                       Bool fftNorm) {
//   }
// 
//   // Get the final weights image
//   void PythonFTMachine::getWeightImage(ImageInterface<Float>&, Matrix<Float>&) {
//   }
// 
//   // Save and restore the LofarFTMachine to and from a record
//   Bool PythonFTMachine::toRecord(String& error, RecordInterface& outRec, Bool withImage) {
//   }
//   
//   Bool PythonFTMachine::fromRecord(String& error, const RecordInterface& inRec) {
//   }
// 
//   String PythonFTMachine::name() {
//   }
//   
//   void PythonFTMachine::ComputeResiduals(VisBuffer&vb, Bool useCorrected) {
//   }
// 
//   void PythonFTMachine::makeConjPolMap(const VisBuffer& vb, const Vector<Int> cfPolMap, Vector<Int>& conjPolMap) {
//   }
// 
//   void PythonFTMachine::makeCFPolMap(const VisBuffer& vb, const Vector<Int>& cfstokes, Vector<Int>& polM) {
//   }
//   
  
}
