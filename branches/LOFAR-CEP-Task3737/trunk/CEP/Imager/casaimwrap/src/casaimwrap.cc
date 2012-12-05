//# casaimwrap.cc: Thin wrapper around some CASA functions that are required
//# by the Python imager.
//#
//# Copyright (C) 2012
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
//# $Id: pystationresponse.cc 20560 2012-03-26 12:06:49Z zwieten $

#include <lofar_config.h>

#include <casaimwrap/VisBufferStub.h>

#include <casa/Containers/ValueHolder.h>
#include <casa/Containers/Record.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/OS/HostInfo.h>

#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/Projection.h>

#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <measures/Measures/MeasTable.h>

#include <synthesis/MeasurementEquations/StokesImageUtil.h>
#include <synthesis/MeasurementComponents/SimpleComponentFTMachine.h>
#include <synthesis/MeasurementComponents/MFCleanImageSkyModel.h>
#include <synthesis/MeasurementEquations/ConvolutionEquation.h>
#include <synthesis/MeasurementEquations/ClarkCleanModel.h>

#include <LofarFT/LofarCubeSkyEquation.h>
#include <LofarFT/LofarFTMachine.h>
#include <LofarFT/LofarVisResampler.h>
#include <LofarFT/LofarImager.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSAntenna.h>
#include <ms/MeasurementSets/MSAntennaParse.h>
#include <ms/MeasurementSets/MSAntennaColumns.h>
#include <ms/MeasurementSets/MSDataDescription.h>
#include <ms/MeasurementSets/MSDataDescColumns.h>
#include <ms/MeasurementSets/MSField.h>
#include <ms/MeasurementSets/MSFieldColumns.h>
#include <ms/MeasurementSets/MSObservation.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSPolarization.h>
#include <ms/MeasurementSets/MSPolColumns.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <ms/MeasurementSets/MSSelection.h>
#include <ms/MeasurementSets/MSDopplerUtil.h>
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <tables/Tables/TableIter.h>

#include <pyrap/Converters/PycExcp.h>
#include <pyrap/Converters/PycBasicData.h>
#include <pyrap/Converters/PycValueHolder.h>
#include <pyrap/Converters/PycRecord.h>

#include <boost/python.hpp>
#include <boost/python/args.hpp>

using namespace casa;
using namespace casa::pyrap;
using namespace boost::python;

namespace LOFAR
{
namespace casaimwrap
{

struct CASAContext
{
    MeasurementSet              ms;
    CountedPtr<LofarFTMachine>  ft;

    IPosition                   shape;
    CoordinateSystem            coordinates;
    TempImage<Complex>          image;
    Matrix<Float>               weight;
    CountedPtr<VisBufferStub>   buffer;
    bool                        psf;

    vector<Array<Complex> >     grids;
    vector<Array<DComplex> >    gridsComplex;
};

CoordinateSystem makeCoordinateSystem(const Vector<Int> &shape,
    const Vector<Double> &delta, const Vector<Double> &reference,
    const Vector<Double> &frequency, const Vector<Double> &width)
{
    AlwaysAssert(shape.size() == 2, AipsError);
    AlwaysAssert(delta.size() == 2, AipsError);
    AlwaysAssert(reference.size() == 2, AipsError);
    AlwaysAssert(frequency.size() == width.size(), AipsError);
    AlwaysAssert(width.size() > 0, AipsError);

    MVAngle ra(reference(0));
    MVAngle dec(reference(1));
    // Normalize correctly (see imagecoordinates2(), in Imager2.cc).
    // This normalizes RA between 0 and 2pi.
    ra(0.0);

    Vector<Double> refPhase(2);
    refPhase(0) = ra.get().getValue();
    refPhase(1) = dec;

    Matrix<Double> xform(2,2);
    xform = 0.0; xform.diagonal() = 1.0;

    DirectionCoordinate directionCoordinate(MDirection::J2000,
        Projection(Projection::SIN), refPhase(0), refPhase(1), -delta(1),
        delta(0), xform, shape(1) / 2, shape(0) / 2);

    Vector<Int> stokes(4);
    stokes(0) = Stokes::I;
    stokes(1) = Stokes::Q;
    stokes(2) = Stokes::U;
    stokes(3) = Stokes::V;

    Double fmin = min(frequency - abs(0.5 * width));
    Double fmax = max(frequency + abs(0.5 * width));
    Double fmean = (fmax + fmin) / 2.0;
    Double finc = fmax - fmin;

    SpectralCoordinate spectralCoordinate(MFrequency::TOPO, fmean, finc, 0,
        0.0);

    CoordinateSystem coordinates;
    coordinates.addCoordinate(directionCoordinate);
    coordinates.addCoordinate(StokesCoordinate(stokes));
    coordinates.addCoordinate(spectralCoordinate);
    return coordinates;
}

void init(CASAContext &context, const String &name, const Record &options)
{
    MeasurementSet ms(name);
    context.ms = ms(ms.col("ANTENNA1") != ms.col("ANTENNA2")
        && ms.col("OBSERVATION_ID") == 0
        && ms.col("FIELD_ID") == 0
        && ms.col("DATA_DESC_ID") == 0);

    // -------------------------------------------------------------------------
    // Stripped version of LofarImager::createFTMachine()
    // -------------------------------------------------------------------------

    // Defaults from awimager (see Imager::setoptions() call).
    Long cache_p = 512*1024*(1024/8); // 512 MB
    Int tile_p = 16;
    String gridfunction_p = "sf";
    MPosition mLocation_p;
    Float padding_p = options.asFloat("padding");
    Int wprojPlanes_p = options.asInt("wplanes");

    // From LofarImager::createFTMachine().
    CountedPtr<VisibilityResamplerBase> visResampler;
    Bool useDoublePrecGrid = False;

    // RefFreq doesn't look to be used by LofarFTMachine (!)
    Double RefFreq = 0.0;

    context.ft = CountedPtr<LofarFTMachine>(new LofarFTMachine(cache_p / 2,
        tile_p, visResampler, gridfunction_p,
        context.ms, wprojPlanes_p, mLocation_p,
        padding_p, false, useDoublePrecGrid,
        options.asDouble("wmax"),
        options.asInt("verbose"),
        options.asInt("maxsupport"),
        options.asInt("oversample"),
        options.asString("imagename"),
        options.asArrayBool("mueller.grid"),
        options.asArrayBool("mueller.degrid"),
        RefFreq,
        options.asBool("UseLIG"),
        options.asBool("UseEJones"),
        options.asInt("StepApplyElement"),
        options.asInt("ApplyBeamCode"),
        options.asDouble("PBCut"),
        options.asBool("PredictFT"),
        options.asString("PsfImage"),
        options.asBool("UseMasksDegrid"),
        options.asBool("doPSF"),
        options.asDouble("UVmin"),
        options.asDouble("UVmax"),
        options.asBool("MakeDirtyCorr"),
        options));

    context.ft->initGridThreads(context.grids, context.gridsComplex);
    context.buffer = CountedPtr<VisBufferStub>(new VisBufferStub(context.ms));
}

Record clarkClean(const ValueHolder &psf, const ValueHolder &residual,
    const ValueHolder &mask, unsigned int iterations, const Record &options)
{
    // Now make a convolution equation for this residual image and psf and then
    // clean it.

    Array<Float> residualArray = residual.asArrayFloat();
    ConvolutionEquation eqn(psf.asArrayFloat(), residualArray);

    Array<Float> delta(residualArray.shape(), 0.0);

    ClarkCleanModel cleaner(delta);
    cleaner.setMask(mask.asArrayFloat());
    cleaner.setGain(options.asFloat("gain"));
    cleaner.setNumberIterations(options.asInt("iterations"));
    cleaner.setInitialNumberIterations(iterations);
    cleaner.setThreshold(options.asFloat("cycle_threshold"));
    cleaner.setPsfPatchSize(IPosition(2, options.asInt("psf_patch_size")));
    cleaner.setMaxNumberMajorCycles(10);
    cleaner.setHistLength(1024);
    cleaner.setMaxNumPix(32 * 1024);
    cleaner.setChoose(false);
    cleaner.setCycleSpeedup(options.asFloat("cycle_speedup"));
    cleaner.setSpeedup(0.0);
    cleaner.singleSolve(eqn, residualArray);

    Record result;
    result.define("iterations", cleaner.numberIterations());
    result.define("delta", delta);

    // TODO: It seems that delta already has the clean components, so this
    // step should not be necessary (unless delta does not have the updates of
    // the last iteration?). For now, always verify that both images are equal.
    Array<Float> tmp;
    cleaner.getModel(tmp);
    AlwaysAssert(allTrue(delta == tmp), AipsError);

    return result;
}

Record fitGaussianPSF(const Record &coordinates, const ValueHolder &psf)
{
    CountedPtr<CoordinateSystem> tmp_coordinates =
        CountedPtr<CoordinateSystem>(CoordinateSystem::restore(coordinates,
        ""));
    AlwaysAssert(!tmp_coordinates.null(), AipsError);

    Array<Float> array_psf = psf.asArrayFloat();
    TempImage<Float> tmp_image(array_psf.shape(), *tmp_coordinates);
    tmp_image.put(array_psf);

    Vector<Float> beam(3, 0.0f);
    Bool ok = StokesImageUtil::FitGaussianPSF(tmp_image, beam);

    Record result;
    result.define("ok", ok);
    result.define("major", beam[0]);
    result.define("minor", beam[1]);
    result.define("angle", beam[2]);
    return result;
}

ValueHolder convolveWithBeam(const Record &coordinates, const ValueHolder &image,
    float major, float minor, float pa)
{
    CountedPtr<CoordinateSystem> tmp_coordinates =
        CountedPtr<CoordinateSystem>(CoordinateSystem::restore(coordinates,
        ""));
    AlwaysAssert(!tmp_coordinates.null(), AipsError);

    Array<Float> array_image = image.asArrayFloat();
    TempImage<Float> tmp_image(array_image.shape(), *tmp_coordinates);
    tmp_image.put(array_image);

    StokesImageUtil::Convolve(tmp_image, major, minor, pa);
    return ValueHolder(tmp_image.get());
}

Record makeCoordinateSystemPy(const ValueHolder &size, const ValueHolder &delta,
    const ValueHolder &reference, const ValueHolder &frequency,
    const ValueHolder &width)
{
    CoordinateSystem coordinates = makeCoordinateSystem(size.asArrayInt(),
        delta.asArrayDouble(), reference.asArrayDouble(),
        frequency.asArrayDouble(), width.asArrayDouble());

    Record record;
    coordinates.save(record, "coordinate_system");
    return record.subRecord(0);
}

ValueHolder average_response(const CASAContext &context)
{
    return ValueHolder(context.ft->getAverageResponse());
}

Record begin_degrid(CASAContext &context, const Record &coordinates,
    const ValueHolder &image, const Record &chunk)
{
    CountedPtr<CoordinateSystem> tmp_coordinates =
        CountedPtr<CoordinateSystem>(CoordinateSystem::restore(coordinates,
        ""));
    AlwaysAssert(!tmp_coordinates.null(), AipsError);
    context.coordinates = *tmp_coordinates;

    Array<Float> pixels = image.asArrayFloat();
    context.shape = pixels.shape();

    // Create temporary image.
    TempImage<Float> tmp_image(context.shape, context.coordinates);
    tmp_image.put(pixels);

    // Complex image to degrid.
    context.image = TempImage<Complex>(context.shape, context.coordinates);
    StokesImageUtil::changeCStokesRep(context.image, SkyModel::LINEAR);

    // Convert from Stokes to complex.
    StokesImageUtil::From(context.image, tmp_image);

    // Update temporary VisBuffer.
    context.buffer->setChunk(chunk.asArrayInt("ANTENNA1"),
        chunk.asArrayInt("ANTENNA2"),
        chunk.asArrayDouble("UVW"),
        chunk.asArrayDouble("TIME"),
        chunk.asArrayDouble("TIME_CENTROID"),
        chunk.asArrayBool("FLAG_ROW"),
        chunk.asArrayFloat("WEIGHT"),
        chunk.asArrayBool("FLAG"),
        true);

    context.ft->initializeToVis(context.image, *context.buffer);
    context.ft->get(*context.buffer);

    Record result;
    result.define("data", context.buffer->modelVisCube());
    return result;
}

Record degrid(CASAContext &context, const Record &chunk)
{
    // Update temporary VisBuffer.
    context.buffer->setChunk(chunk.asArrayInt("ANTENNA1"),
        chunk.asArrayInt("ANTENNA2"),
        chunk.asArrayDouble("UVW"),
        chunk.asArrayDouble("TIME"),
        chunk.asArrayDouble("TIME_CENTROID"),
        chunk.asArrayBool("FLAG_ROW"),
        chunk.asArrayFloat("WEIGHT"),
        chunk.asArrayBool("FLAG"),
        false);

    // Degrid into buffer.
    context.ft->get(*context.buffer);

    Record result;
    result.define("data", context.buffer->modelVisCube());
    return result;
}

void end_degrid(CASAContext &context)
{
    context.ft->finalizeToVis();
}

void begin_grid(CASAContext &context, const ValueHolder &shape,
    const Record &coordinates, bool psf, const Record &chunk)
{
    context.psf = psf;

    CountedPtr<CoordinateSystem> tmp_coordinates =
        CountedPtr<CoordinateSystem>(CoordinateSystem::restore(coordinates,
        ""));
    AlwaysAssert(!tmp_coordinates.null(), AipsError);
    context.coordinates = *tmp_coordinates;

    Vector<Int> tmp_shape = shape.asArrayInt();
    AlwaysAssert(tmp_shape.size() == 4, AipsError);
    context.shape = IPosition(4, tmp_shape[3], tmp_shape[2], tmp_shape[1],
        tmp_shape[0]);

    // Create temporary image.
    context.image = TempImage<Complex>(context.shape, context.coordinates);
    StokesImageUtil::changeCStokesRep(context.image, SkyModel::LINEAR);
    context.image.set(0.0);

    // Create weight matrix.
    // ==> No need, will be resized by LofarFTMachine.

    // Update temporary VisBuffer.
    context.buffer->setChunk(chunk.asArrayInt("ANTENNA1"),
        chunk.asArrayInt("ANTENNA2"),
        chunk.asArrayDouble("UVW"),
        chunk.asArrayDouble("TIME"),
        chunk.asArrayDouble("TIME_CENTROID"),
        chunk.asArrayBool("FLAG_ROW"),
        chunk.asArrayFloat("WEIGHT"),
        chunk.asArrayBool("FLAG"),
        chunk.asArrayComplex("DATA"),
        true);

    // Initialize static information in temporary VisBuffer.
    context.ft->initializeToSky(context.image, context.weight, *context.buffer);

    // Grid data.
    context.ft->put(*context.buffer, -1, context.psf, FTMachine::OBSERVED);
}

void grid(CASAContext &context, const Record &chunk)
{
    // Update temporary VisBuffer.
    context.buffer->setChunk(chunk.asArrayInt("ANTENNA1"),
        chunk.asArrayInt("ANTENNA2"),
        chunk.asArrayDouble("UVW"),
        chunk.asArrayDouble("TIME"),
        chunk.asArrayDouble("TIME_CENTROID"),
        chunk.asArrayBool("FLAG_ROW"),
        chunk.asArrayFloat("WEIGHT"),
        chunk.asArrayBool("FLAG"),
        chunk.asArrayComplex("DATA"),
        false);

    // Grid data.
    context.ft->put(*context.buffer, -1, context.psf, FTMachine::OBSERVED);
}

Record end_grid(CASAContext &context, bool normalize)
{
    context.ft->finalizeToSky();

    ImageInterface<Complex> &image = context.ft->getImage(context.weight,
        normalize);

    TempImage<Float> tmp_image(context.shape, context.coordinates);
    tmp_image.set(0.0);

    if(context.psf)
    {
        StokesImageUtil::ToStokesPSF(tmp_image, image);
    }
    else
    {
        StokesImageUtil::To(tmp_image, image);
    }

    Record result;
    result.define("weight", context.weight);
    result.define("image", tmp_image.get());
    return result;
}

} // namespace casaimwrap
} // namespace LOFAR

BOOST_PYTHON_MODULE(_casaimwrap)
{
    casa::pyrap::register_convert_excp();
    casa::pyrap::register_convert_basicdata();
    casa::pyrap::register_convert_casa_valueholder();
    casa::pyrap::register_convert_casa_record();

    class_<LOFAR::casaimwrap::CASAContext>("CASAContext");

    def("init", LOFAR::casaimwrap::init,
        (boost::python::arg("context"),
        boost::python::arg("name"),
        boost::python::arg("options")));

    def("makeCoordinateSystem", LOFAR::casaimwrap::makeCoordinateSystemPy,
        (boost::python::arg("size"),
        boost::python::arg("delta"),
        boost::python::arg("reference"),
        boost::python::arg("frequency"),
        boost::python::arg("width")));

    def("clarkClean", LOFAR::casaimwrap::clarkClean,
        (boost::python::arg("psf"),
        boost::python::arg("residual"),
        boost::python::arg("wmask"),
        boost::python::arg("iterations"),
        boost::python::arg("options")));

   def("convolveWithBeam", LOFAR::casaimwrap::convolveWithBeam,
        (boost::python::arg("coordinates"),
        boost::python::arg("image"),
        boost::python::arg("major"),
        boost::python::arg("minor"),
        boost::python::arg("pa")));

    def("fitGaussianPSF", LOFAR::casaimwrap::fitGaussianPSF,
        (boost::python::arg("coordinates"),
        boost::python::arg("psf")));

    def("average_response", LOFAR::casaimwrap::average_response,
        (boost::python::arg("context")));

    def("begin_degrid", LOFAR::casaimwrap::begin_degrid,
        (boost::python::arg("context"),
        boost::python::arg("coordinates"),
        boost::python::arg("image"),
        boost::python::arg("chunk")));

    def("degrid", LOFAR::casaimwrap::degrid,
        (boost::python::arg("context"),
        boost::python::arg("chunk")));

    def("end_degrid", LOFAR::casaimwrap::end_degrid,
        (boost::python::arg("context")));

    def("begin_grid", LOFAR::casaimwrap::begin_grid,
        (boost::python::arg("context"), boost::python::arg("shape"),
        boost::python::arg("coordinates"), boost::python::arg("psf"),
        boost::python::arg("chunk")));

    def("grid", LOFAR::casaimwrap::grid,
        (boost::python::arg("context"),
        boost::python::arg("chunk")));

    def("end_grid", LOFAR::casaimwrap::end_grid,
        (boost::python::arg("context"),
        boost::python::arg("normalize")));
}
