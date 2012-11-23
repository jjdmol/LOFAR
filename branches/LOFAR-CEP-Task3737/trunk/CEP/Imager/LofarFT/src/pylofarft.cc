#include <lofar_config.h>

#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <ms/MeasurementSets/MSIter.h>

#include <LofarFT/LofarVBStore.h>
#include <LofarFT/LofarCFStore.h>
#include <LofarFT/LofarConvolutionFunction.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/ROVisibilityIteratorProxy.h>
#include <LofarFT/VisBufferProxy.h>
#include <LofarFT/VisibilityResamplerProxy.h>
#include <LofarFT/PythonFTMachine.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycValueHolder.h>
#include <pyrap/Converters/PycRecord.h>
#include <pyrap/Converters/PycArray.h>

#include <boost/python.hpp>
#include <boost/python/args.hpp>

#include <iostream>

using namespace std;
using namespace casa;
using namespace boost::python;

void changeCStokesRep(ImageProxy& image, SkyModel::PolRep polRep) {
  ImageInterface<Complex> *imageinterface = dynamic_cast<ImageInterface<Complex>*>(image.getLattice());
  StokesImageUtil::changeCStokesRep( *imageinterface, polRep);
}

class LofarVBStoreProxy {
//   LofarVBStoreProxy():dopsf_p(False) {};
//   ~LofarVBStoreProxy() {};
public:
  
  LofarVBStoreProxy() : itsLofarVBStore(LofarVBStore()) {}
  
  LofarVBStoreProxy(LofarVBStore vbs) : itsLofarVBStore(vbs) {}

  Int get_nRow() {
    return itsLofarVBStore.nRow_p;
  }
  void set_nRow(Int v) {
    itsLofarVBStore.nRow_p = v;
  }
  
  Int get_beginRow() {
    return itsLofarVBStore.beginRow_p;
  }
  void set_beginRow(Int v) {
    itsLofarVBStore.beginRow_p = v;
  }
  
  Int get_endRow() {
    return itsLofarVBStore.endRow_p;
  }
  void set_endRow(Int v) {
    itsLofarVBStore.endRow_p = v;
  }
  
  ValueHolder get_uvw() const {
    return ValueHolder(itsLofarVBStore.uvw_p);
  }
  void set_uvw(ValueHolder v) {
    itsLofarVBStore.uvw_p = v.asArrayDouble();
  }
  
  ValueHolder get_selection() const {
    return ValueHolder(itsLofarVBStore.selection_p);
  }
  void set_selection(ValueHolder v) {
    itsLofarVBStore.selection_p = v.asArrayuInt();
  }
  
  ValueHolder get_rowFlag() const {
    return ValueHolder(itsLofarVBStore.rowFlag_p);
  }
  void set_rowFlag(const ValueHolder v) {
    itsLofarVBStore.rowFlag_p = v.asArrayBool();
  }
  
  ValueHolder get_flagCube() const {
    return ValueHolder(itsLofarVBStore.flagCube_p);
  }
  void set_flagCube(const ValueHolder v) {
    itsLofarVBStore.flagCube_p = v.asArrayBool();
  }
  
  ValueHolder get_imagingWeight() const {
    return ValueHolder(itsLofarVBStore.imagingWeight_p);
  }
  void set_imagingWeight(ValueHolder v) {
    itsLofarVBStore.imagingWeight_p = v.asArrayFloat();
  }

  ValueHolder get_visCube() const {
    return ValueHolder(itsLofarVBStore.visCube_p);
  }
  void set_visCube(ValueHolder v) {
    itsLofarVBStore.visCube_p = v.asArrayComplex();
  }

  ValueHolder get_modelCube() const {
    return ValueHolder(itsLofarVBStore.modelCube_p);
  }
  void set_modelCube(ValueHolder v) {
    itsLofarVBStore.modelCube_p = v.asArrayComplex();
  }

  ValueHolder get_correctedCube() const {
    return ValueHolder(itsLofarVBStore.correctedCube_p);
  }
  void set_correctedCube(ValueHolder v) {
    itsLofarVBStore.correctedCube_p = v.asArrayComplex();
  }
 
  ValueHolder get_freq() const {
    return ValueHolder(itsLofarVBStore.freq_p);
  }
  void set_freq(ValueHolder v) {
    itsLofarVBStore.freq_p = v.asArrayDouble();
  }

  Bool get_dopsf() const {
    return itsLofarVBStore.dopsf_p;
  }
  void set_dopsf(Bool v) {
    itsLofarVBStore.dopsf_p = v;
  }

  Bool get_useCorrected() const {
    return itsLofarVBStore.useCorrected_p;
  }
  void set_useCorrected(Bool v) {
    itsLofarVBStore.useCorrected_p = v;
  }

private:
  LofarVBStore itsLofarVBStore;
};


BOOST_PYTHON_MODULE(pylofarft)
{
  casa::pyrap::register_convert_excp();
  casa::pyrap::register_convert_basicdata();
  casa::pyrap::register_convert_casa_valueholder();
  casa::pyrap::register_convert_casa_record();

  class_<LofarVBStoreProxy> ("LofarVBStore", "Hi there")
    .add_property("nRow", &LofarVBStoreProxy::get_nRow, &LofarVBStoreProxy::set_nRow)
    .add_property("beginRow", &LofarVBStoreProxy::get_beginRow, &LofarVBStoreProxy::set_beginRow)
    .add_property("endRow", &LofarVBStoreProxy::set_endRow, &LofarVBStoreProxy::set_endRow)
    .add_property("uvw", &LofarVBStoreProxy::get_uvw, &LofarVBStoreProxy::set_uvw)
    .add_property("selection", &LofarVBStoreProxy::get_selection, &LofarVBStoreProxy::set_selection)
    .add_property("rowFlag", &LofarVBStoreProxy::get_rowFlag, &LofarVBStoreProxy::set_rowFlag)
    .add_property("flagCube", &LofarVBStoreProxy::get_flagCube, &LofarVBStoreProxy::set_flagCube)
    .add_property("imagingWeight", &LofarVBStoreProxy::get_imagingWeight, &LofarVBStoreProxy::set_imagingWeight)
    .add_property("visCube", &LofarVBStoreProxy::get_visCube, &LofarVBStoreProxy::set_visCube)
    .add_property("modelCube", &LofarVBStoreProxy::get_modelCube, &LofarVBStoreProxy::set_modelCube)
    .add_property("correctedCube", &LofarVBStoreProxy::get_correctedCube, &LofarVBStoreProxy::set_correctedCube)
    .add_property("freq", &LofarVBStoreProxy::get_freq, &LofarVBStoreProxy::set_freq)
    .add_property("dopsf", &LofarVBStoreProxy::get_dopsf, &LofarVBStoreProxy::set_dopsf)
    .add_property("useCorrected", &LofarVBStoreProxy::get_useCorrected, &LofarVBStoreProxy::set_useCorrected)
  ;
  
  class_<LOFAR::LofarCFStore> ("LofarCFStore", "Nothing to see here, move along!")
  ;

  class_<LOFAR::VisBufferProxy> ("VisBuffer", init<LOFAR::ROVisibilityIteratorProxy&>())
    .def("__len__", &LOFAR::VisBufferProxy::get_nRow)
    .add_property("nRow", &LOFAR::VisBufferProxy::get_nRow)
    .add_property("nChannel", &LOFAR::VisBufferProxy::get_nChannel)
    .add_property("newMS", &LOFAR::VisBufferProxy::get_newMS)
    .add_property("spectralWindow", &LOFAR::VisBufferProxy::get_spectralWindow)
    .add_property("imagingWeight", &LOFAR::VisBufferProxy::get_imagingWeight)
    .add_property("uvw", &LOFAR::VisBufferProxy::get_uvw)
    .add_property("antenna1", &LOFAR::VisBufferProxy::get_antenna1)
    .add_property("antenna2", &LOFAR::VisBufferProxy::get_antenna2)
    .add_property("flagRow", &LOFAR::VisBufferProxy::get_flagRow)
    .add_property("timeCentroid", &LOFAR::VisBufferProxy::get_timeCentroid)
    .add_property("visCube", &LOFAR::VisBufferProxy::get_visCube)
    .add_property("polFrame", &LOFAR::VisBufferProxy::get_polFrame)
  ;
  
  class_<casa::VisSet> ("VisSet", no_init)
    .add_property("msname", &casa::VisSet::msName)
  ;
  
  class_<LOFAR::ROVisibilityIteratorProxy> ("_ROVisibilityIterator", init<LOFAR::ROVisibilityIteratorProxy&>())
    .def("ms", &LOFAR::ROVisibilityIteratorProxy::ms)
    .def("__iter__", &LOFAR::ROVisibilityIteratorProxy::iter, return_value_policy<reference_existing_object>())
    .def("next", &LOFAR::ROVisibilityIteratorProxy::next)
  ;
  
  class_<LOFAR::LofarConvolutionFunction> ("LofarConvolutionFunction", 
    init<IPosition&, DirectionCoordinate&, MeasurementSet&, uInt, double, uInt, Int, Int, String&, Bool, Bool, int, 
      Record&, vector< vector< vector < Matrix<Complex> > > > &>())
    .def("makeConvolutionFunction", &LOFAR::LofarConvolutionFunction::makeConvolutionFunction)
  ;
  
  class_<LOFAR::VisibilityResamplerProxy> ("VisibilityResampler")
  ;

  class_<LOFAR::PythonFTMachine> ("_LofarFTMachine", no_init)
    .def("initializeToSky", &LOFAR::PythonFTMachine::_initializeToSky)
    .def("initializeToVis", &LOFAR::PythonFTMachine::_initializeToVis)
    .def("put", &LOFAR::PythonFTMachine::_put)
    .def("get", &LOFAR::PythonFTMachine::_get)
    .def("finalizeToSky", &LOFAR::PythonFTMachine::_finalizeToSky)
    .def("makeImage_vs", (void (LOFAR::PythonFTMachine::*)(FTMachine::Type, VisSet&, ImageProxy&, const ValueHolder&)) &LOFAR::PythonFTMachine::_makeImage)
    .def("makeImage_vi", (ValueHolder (LOFAR::PythonFTMachine::*)(FTMachine::Type, LOFAR::ROVisibilityIteratorProxy&, ImageProxy&, const ValueHolder&)) &LOFAR::PythonFTMachine::_makeImage)
    .def("getImage", &LOFAR::PythonFTMachine::_getImage)

  ;

  enum_<FTMachine::Type>("FTMachineType")
    .value("OBSERVED", FTMachine::OBSERVED)
    .value("MODEL", FTMachine::MODEL)
    .value("CORRECTED", FTMachine::CORRECTED)
    .value("RESIDUAL", FTMachine::RESIDUAL)
    .value("PSF", FTMachine::PSF)
    .value("COVERAGE", FTMachine::COVERAGE)
    .value("N_types", FTMachine::N_types)
    .value("DEFAULT", FTMachine::DEFAULT)      
  ;

  enum_<SkyModel::PolRep>("PolRep")
    .value("CIRCULAR", SkyModel::CIRCULAR)
    .value("LINEAR", SkyModel::LINEAR)
  ;
  
  enum_<MSIter::PolFrame> ("PolFrame")
    .value("Circular", MSIter::Circular)
    .value("Linear", MSIter::Linear)
  ;
  
  def("changeCStokesRep", changeCStokesRep); 
  
}

  

