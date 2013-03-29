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

class ScopedGILRelease
{
public:
    inline ScopedGILRelease()
    {
        m_thread_state = PyEval_SaveThread();
    }

    inline ~ScopedGILRelease()
    {
        PyEval_RestoreThread(m_thread_state);
        m_thread_state = 0;
    }

private:
    PyThreadState *m_thread_state;
};

struct CASAContext
{
    MeasurementSet                          ms;
    CountedPtr<LofarFTMachine>              ft;

    CountedPtr<LofarConvolutionFunction>    cf;
    Record                                  options;

    IPosition                               shape;
    CoordinateSystem                        coordinates;
    TempImage<Complex>                      image;
    Matrix<Float>                           weight;
    CountedPtr<VisBufferStub>               buffer;
    bool                                    psf;

    vector<Array<Complex> >                 grids;
    vector<Array<DComplex> >                gridsComplex;
};

CoordinateSystem make_coordinate_system(const Vector<Int> &shape,
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
    context.options = options;

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

ValueHolder stokes_to_linear(CASAContext &context, const Record &coordinates,
    const ValueHolder &image)
{
    CountedPtr<CoordinateSystem> tmp_coordinates =
        CountedPtr<CoordinateSystem>(CoordinateSystem::restore(coordinates,
        ""));
    AlwaysAssert(!tmp_coordinates.null(), AipsError);

    Array<Float> imageArray = image.asArrayFloat();

    // Create temporary image.
    TempImage<Float> tmp_image(imageArray.shape(), *tmp_coordinates);
    tmp_image.put(imageArray);

    // Complex image to degrid.
    TempImage<Complex> linearImage(imageArray.shape(), *tmp_coordinates);
    StokesImageUtil::changeCStokesRep(linearImage, SkyModel::LINEAR);

    // Convert from Stokes to complex.
    StokesImageUtil::From(linearImage, tmp_image);
    return ValueHolder(linearImage.get());
}

Record clark_clean(const ValueHolder &psf, const ValueHolder &residual,
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

Record fit_gaussian_psf(const Record &coordinates, const ValueHolder &psf)
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

ValueHolder convolve_with_beam(const Record &coordinates,
    const ValueHolder &image, float major, float minor, float pa)
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

Record make_coordinate_system_py(const ValueHolder &size,
    const ValueHolder &delta, const ValueHolder &reference,
    const ValueHolder &frequency, const ValueHolder &width)
{
    CoordinateSystem coordinates = make_coordinate_system(size.asArrayInt(),
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

void init_cf(CASAContext &context, const ValueHolder &shape,
    const Record &coordinates)
{
    CountedPtr<CoordinateSystem> tmp_coordinates =
        CountedPtr<CoordinateSystem>(CoordinateSystem::restore(coordinates,
        ""));
    AlwaysAssert(!tmp_coordinates.null(), AipsError);

    Vector<Int> tmp_shape = shape.asArrayInt();
    AlwaysAssert(tmp_shape.size() == 4, AipsError);
    IPosition im_shape(4, tmp_shape[3], tmp_shape[2], tmp_shape[1],
        tmp_shape[0]);

    // StackMuellerNew is used to stack the CF for the computation of the
    // average (squared) response. This is not used while degridding, so for now
    // we just ignore it.
    vector<vector<vector< Matrix<Complex> > > > itsStackMuellerNew(2);

    Int idx = tmp_coordinates->findCoordinate(Coordinate::DIRECTION);

    typedef CountedPtr<LofarConvolutionFunction> PtrType;
    context.cf = PtrType(new LofarConvolutionFunction(im_shape,
        tmp_coordinates->directionCoordinate(idx),
        context.ms,
        context.options.asInt("wplanes"),
        context.options.asDouble("wmax"),
        context.options.asInt("oversample"),
        context.options.asInt("verbose"),
        context.options.asInt("maxsupport"),
        context.options.asString("imagename") + ".cf",
        context.options.asBool("UseEJones"),
        context.options.asInt("StepApplyElement") > 0,
        context.options.asInt("ApplyBeamCode"),
        context.options,
        itsStackMuellerNew));
}

void init_aterm(CASAContext &context, const Vector<Double> &time)
{
    Double timeChunk = 0.5 * (time(time.size() - 1) + time(0));

    context.cf->computeAterm(timeChunk);
    context.cf->computeVecAterm(time(0), time(time.size() - 1),
        context.options.asDouble("timewindow"));
}

ValueHolder spheroid(CASAContext &context)
{
    return ValueHolder(context.cf->getSpheroidCut());
}

Record w_index(CASAContext &context, const Vector<Float> &w, uInt spw)
{
    AlwaysAssert(!context.cf.null(), AipsError);

    Vector<Int> wIndex(w.size());
    for(uInt i = 0; i < w.size(); ++i)
    {
        wIndex(i) = context.cf->GiveWindexIncludeNegative(w(i), 0);
    }

    Record result;
    result.define("n_planes", context.cf->m_nWPlanes);
    result.define("w_index", wIndex);
    return result;
}

ValueHolder apply_w_term_image(CASAContext &context, const ValueHolder &image,
    int w_index)
{
    ScopedGILRelease __release;

    Array<Complex> imageArray = image.asArrayComplex();
    Array<Complex> resultArray(imageArray.shape(), 0.0);
    context.cf->ApplyWterm_Image(imageArray, resultArray, 0, true, w_index);
    return ValueHolder(resultArray);
}

ValueHolder make_convolution_function(CASAContext &context, uInt threadId,
    uInt stationA, uInt stationB, Double time, Double w)
{
    ScopedGILRelease __release;

    // Bogus arguments that need to be provided but either are not used within
    // makeConvolutionFunction() or are provided with the same value they would
    // have in the C++ implementation.
    Matrix<Complex> pb;
    double weight;
    Vector<uInt> chan_block(1, 0);
    vector<vector<Matrix<Complex> > > stack;

    LofarCFStore store = context.cf->makeConvolutionFunction(threadId, stationA,
        stationB, time, w, Matrix<bool>(), true, 0.0, pb, weight, chan_block, 0,
        0.0, stack, 0, false);

    // Assume only the array factor (scalar effect) is to be applied and that
    // the convolution function is the same for all channels. Hence, only return
    // the XXX'X' Mueller matrix element of channel 0 here.
    return ValueHolder((*store.vdata)[0][0][0]);
}

void ___degrid_reimplemented(Double incX, Double incY,
    const Array<Complex> &grid, const Matrix<Complex> &kernel,
    uInt oversample, const Matrix<Double> &uvw, const Vector<Double> &freq,
    const Cube<Bool> &flag, Cube<Complex> &vis, const Vector<uInt> &chMapGrid,
    const Vector<uInt> &crMapGrid, const Vector<uInt> &rows)
{
    // Array shapes used (Fortran order):
    // grid  : [nX, nY, nPol, nCh]
    // kernel: [nX, nY, nCh]        // <== for now only [nX, nY]
    // kernel: [nX, nY]
    // uvw   : [3, nRow]
    // freq  : [nCh]
    // flag  : [nCr, nCh, nRow]
    // vis   : [nCr, nCh, nRow]

    // Note that in general the kernel Array would be a list of a list of a list
    // of 2-D arrays, of dimensions [nCh][nMuellerRow][nMuellerCol][nY, nX],
    // where the dimensions of the 2-D arrays can be different for each channel,
    // Mueller row, and / or Mueller column.
    //
    // However, in the current LOFAR implementation it is implicitly assumed
    // that:
    //     - All convolution functions are of the same size.
    //     - Only a single element of the Mueller matrix is used, because the
    //       visibilities are only corrected for the array factor, which is a
    //       scalar effect.

    // Get size of convolution functions.
    Int nKernelX = kernel.shape()[0];
    Int nKernelY = kernel.shape()[1];

    // Get size of grid.
    Int nGridX    = grid.shape()[0];
    Int nGridY    = grid.shape()[1];
    Int nGridPol  = grid.shape()[2];
    Int nGridChan = grid.shape()[3];

    // Derived values.
    // TODO: indexing in coordinates.increment() assumes specific axis order!!
//    Double uScale = Double(nGridX) * coordinates.increment()(0);
//    Double vScale = Double(nGridY) * coordinates.increment()(1);
    Double uScale = Double(nGridX) * incX;
    Double vScale = Double(nGridY) * incY;
    Double uOffset = nGridX / 2;
    Double vOffset = nGridY / 2;

    // Get visibility data size.
    Int nVisPol   = flag.shape()[0];
    Int nVisChan  = flag.shape()[1];

    // Get oversampling and support size.
    Int sampx = oversample;
    Int sampy = oversample;
    Int supx = nKernelX / (2 * oversample);
    Int supy = nKernelY / (2 * oversample);

    // Loop over all visibility rows to process.
    for(uInt inx = 0; inx < rows.size(); ++inx)
    {
        uInt irow = rows[inx];

        const Double* __restrict__ uvwPtr = uvw.data() + 3 * irow;

        // Loop over all channels in the visibility data.
        // Map the visibility channel to the grid channel.
        // Skip channel if data are not needed.
        for(Int visChan = 0; visChan < nVisChan; ++visChan)
        {
            Int gridChan = chMapGrid[visChan];
//            Int kernelChan = chMapKernel[visChan];

            if(gridChan < 0 || gridChan >= nGridChan)
            {
                continue;
            }

            // Determine the grid position from the UV coordinates in
            // wavelengths.
            Double recipWvl = freq[visChan] / C::c;
            Double posx = uScale * uvwPtr[0] * recipWvl + uOffset;
            Double posy = vScale * uvwPtr[1] * recipWvl + vOffset;

            // Location in the grid.
            Int locx = SynthesisUtils::nint(posx);
            Int locy = SynthesisUtils::nint(posy);

            // Fractional difference.
            Double diffx = locx - posx;
            Double diffy = locy - posy;

            // Offset in the (oversampled) convolution kernel.
            Int offx = SynthesisUtils::nint(diffx * sampx);
            Int offy = SynthesisUtils::nint(diffy * sampy);
            offx += (nKernelX - 1) / 2;
            offy += (nKernelY - 1) / 2;

            // Only use visibility point if the full support is within the grid.
            if(locx - supx < 0 || locx + supx >= nGridX || locy - supy < 0
                || locy + supy >= nGridY)
            {
                continue;
            }

            // Get pointer to data and flags for this channel.
            Int voff = (irow * nVisChan + visChan) * nVisPol;
            Complex* __restrict__ visPtr = vis.data() + voff;
            const Bool* __restrict__ flagPtr = flag.data() + voff;

            for(Int ipol = 0; ipol < nVisPol; ++ipol)
            {
                if(flagPtr[ipol])
                {
                    continue;
                }

                // Initialize the visibility sample to zero.
                visPtr[ipol] = Complex(0,0);

                // Map to grid polarization. Only use pol if needed.
                Int gridPol = crMapGrid[ipol];
                if(gridPol < 0 || gridPol >= nGridPol)
                {
                    continue;
                }

                // Get the offset in the grid data array.
                Int goff = (gridChan * nGridPol + gridPol) * nGridY * nGridX;

                // Loop over the scaled support.
                for(Int sy = -supy; sy <= supy; ++sy)
                {
                    // Get the pointer in the grid for the first x in this y.
                    const Complex* __restrict__ gridPtr = grid.data() + goff +
                        (locy + sy) * nGridX + locx - supx;

                    // Get pointers to the first element to use in the 4
                    // convolution functions for this channel,pol.
                    const Complex* __restrict__ kernelPtr = kernel.data() +
                        (offy + sy * sampy) * nKernelX + offx - supx * sampx;

                    for(Int sx = -supx; sx <= supx; ++sx)
                    {
                        visPtr[ipol] += (*gridPtr) * (*kernelPtr);
                        kernelPtr += sampx;
                        gridPtr++;
                    }
                }
            } // end for ipol
        } // end for visChan
    } // end for inx
}

void degrid_reimplemented(Double incX, Double incY, uInt oversample,
    const ValueHolder &grid, const ValueHolder &kernel, const ValueHolder &uvw,
    const Vector<Double> &freq, const ValueHolder &flag, const ValueHolder &vis,
    const Vector<uInt> &rows)
{
    // vbs.flagCube_p
    // vbs.uvw_p
    // vbs.visCube_p
    // cfs.vdata (5-D: nX, nY, nMuellerCol, nMuellerRow, nFreq)
    // grid (4-D: nX, nY, nPol, nChan)
    //
    // cfs.sampling
    // cfs.x/ySupport
    //
    // chanMap_p
    // polMap_p
    // ChanCFMap
    // uvScale_p
    // offset_p
    //
    // !!!! goes wrong if gridded to the same location >1 times??? ==> NO
    //
    // * Frequency axis is not really handled properly, neither is Mueller col
    //   and row, because it is assumed that all kernels have the same support
    //   and sampling (only index 0 is being checked...).
    //
    // * cfMap_p, conjCFMap_p apparently not used?
    //
    // * Bug in LofarConvolutionFunction::makeConvolutionFunction(): Npix_out is
    //   assumed to be equal for all channel (blocks).
    //
    // * What does uvScale(2) (W-axis) mean?
    //
    // * uvOffset (floating point value) initialized using nx / 2, where nx is
    //  an integer (!).
    //
    // * About scaling with frequency: Should fsampx, fsampy not be used instead
    //   of sampx, sampy when determining the offset offx, offy?!?
    //
    ScopedGILRelease __release;

    vector<uInt> chMapGrid(1, 0);
    vector<uInt> crMapGrid(4);
    for(uInt i = 0; i < 4; ++i)
    {
        crMapGrid[i] = i;
    }

    Cube<Complex> __vis = vis.asArrayComplex();

    ___degrid_reimplemented(incX, incY, grid.asArrayComplex(),
        kernel.asArrayComplex(), oversample, uvw.asArrayDouble(), freq,
        flag.asArrayBool(), __vis, chMapGrid, crMapGrid, rows);
}

} // namespace casaimwrap
} // namespace LOFAR

BOOST_PYTHON_MODULE(_casaimwrap)
{
    casa::pyrap::register_convert_excp();
    casa::pyrap::register_convert_basicdata();
    casa::pyrap::register_convert_casa_valueholder();
    casa::pyrap::register_convert_casa_record();

    // For degrid_reimplemented().
    casa::pyrap::register_convert_casa_vector<casa::uInt>();

    class_<LOFAR::casaimwrap::CASAContext>("CASAContext");

    def("init", LOFAR::casaimwrap::init,
        (boost::python::arg("context"),
        boost::python::arg("name"),
        boost::python::arg("options")));

    def("average_response", LOFAR::casaimwrap::average_response,
        (boost::python::arg("context")));

    def("stokes_to_linear", LOFAR::casaimwrap::stokes_to_linear,
        (boost::python::arg("context"),
        boost::python::arg("coordinates"),
        boost::python::arg("image")));

    def("make_coordinate_system", LOFAR::casaimwrap::make_coordinate_system_py,
        (boost::python::arg("size"),
        boost::python::arg("delta"),
        boost::python::arg("reference"),
        boost::python::arg("frequency"),
        boost::python::arg("width")));

    def("clark_clean", LOFAR::casaimwrap::clark_clean,
        (boost::python::arg("psf"),
        boost::python::arg("residual"),
        boost::python::arg("wmask"),
        boost::python::arg("iterations"),
        boost::python::arg("options")));

    def("convolve_with_beam", LOFAR::casaimwrap::convolve_with_beam,
        (boost::python::arg("coordinates"),
        boost::python::arg("image"),
        boost::python::arg("major"),
        boost::python::arg("minor"),
        boost::python::arg("pa")));

    def("fit_gaussian_psf", LOFAR::casaimwrap::fit_gaussian_psf,
        (boost::python::arg("coordinates"),
        boost::python::arg("psf")));

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

    def("init_cf", LOFAR::casaimwrap::init_cf,
        (boost::python::arg("context"),
        boost::python::arg("shape"),
        boost::python::arg("coordinates")));

    def("init_aterm", LOFAR::casaimwrap::init_aterm,
        (boost::python::arg("context"),
        boost::python::arg("time")));

   def("spheroid", LOFAR::casaimwrap::spheroid,
        (boost::python::arg("context")));

    def("w_index", LOFAR::casaimwrap::w_index,
        (boost::python::arg("context"),
        boost::python::arg("w"),
        boost::python::arg("spw")));

    def("apply_w_term_image", LOFAR::casaimwrap::apply_w_term_image,
        (boost::python::arg("context"),
        boost::python::arg("image"),
        boost::python::arg("w_index")));

    def("make_convolution_function",
        LOFAR::casaimwrap::make_convolution_function,
        (boost::python::arg("context"),
        boost::python::arg("threadId"),
        boost::python::arg("stationA"),
        boost::python::arg("stationB"),
        boost::python::arg("time"),
        boost::python::arg("w")));

    def("degrid_reimplemented", LOFAR::casaimwrap::degrid_reimplemented,
        (boost::python::arg("incX"),
        boost::python::arg("incY"),
        boost::python::arg("oversample"),
        boost::python::arg("grid"),
        boost::python::arg("kernel"),
        boost::python::arg("uvw"),
        boost::python::arg("freq"),
        boost::python::arg("flag"),
        boost::python::arg("vis"),
        boost::python::arg("rows")));
}
