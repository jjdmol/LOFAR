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

struct Memento
{
    MPosition                   obs;
    bool                        obsKnown;
    CoordinateSystem            csys;

    MeasurementSet              ms;
    ROVisibilityIterator        *it;
    MFCleanImageSkyModel        mdl;
    LofarCubeSkyEquation*       eq;
    LofarFTMachine*             ft;
    SimpleComponentFTMachine    cft;

    TempImage<Float>            image;

    TempImage<Complex>          gridImage;
    TempImage<Float>            gridWeightImage;
    Matrix<Float>               gridWeight;
    VisBufferStub*              gridBuffer;
    bool                        gridPSF;

    vector<Array<Complex> > itsGridsParallel;
    vector<Array<DComplex> > itsGridsParallel2;

    Memento()
        :   obsKnown(false),
            it(0),
            eq(0),
            ft(0),
            gridBuffer(0)
    {
    }

    ~Memento()
    {
        delete gridBuffer;
        delete ft;
        delete eq;
        delete it;
    }
};

MDirection readPhaseReference(const MeasurementSet &ms, unsigned int idField)
{
    ROMSFieldColumns field(ms.field());
    AlwaysAssert(field.nrow() > idField, AipsError);
    AlwaysAssert(!field.flagRow()(idField), AipsError);
    return field.phaseDirMeas(idField);
}

//Bool Imager::getRestFreq(Vector<Double>& restFreq, const Int& spw)
bool getRestFreq(const MeasurementSet &ms, Vector<Double> &restFreq, Int spw)
{
    // MS Doppler tracking utility
    MSDopplerUtil msdoppler(ms);
    restFreq.resize();

//  if(restFreq_p.getValue() > 0){// User defined restfrequency
//    restFreq.resize(1);
//    restFreq[0]=restFreq_p.getValue("Hz");
//  }
//  else{
    // Look up first rest frequency found (for now)

    Int fieldid = 0;

    try
    {
        msdoppler.dopplerInfo(restFreq, spw, fieldid);
    }
    catch(...)
    {
      restFreq.resize();
    }
//  }

    return (restFreq.nelements() > 0);
}

CoordinateSystem makeCoordinateSystem(const MeasurementSet &ms,
    unsigned int size, double delta)
{
    Matrix<Double> xform(2,2);
    xform = 0.0; xform.diagonal() = 1.0;

    MDirection mRefPhase = readPhaseReference(ms, 0);
    MVDirection mvRefPhase(mRefPhase.getAngle());

    MVAngle ra = mvRefPhase.get()(0);
    MVAngle dec = mvRefPhase.get()(1);
    // Normalize correctly (??) (see imagecoordinates2(), in Imager2.cc).
    ra(0.0);

    Vector<Double> refPhase(2);
    refPhase(0) = ra.get().getValue();
    refPhase(1) = dec;

    AlwaysAssert(MDirection::Types(mRefPhase.getRefPtr()->getType())
        == MDirection::J2000, AipsError);
    DirectionCoordinate directionCoordinate(MDirection::J2000,
        Projection(Projection::SIN), refPhase(0), refPhase(1), -delta, delta,
        xform, size / 2, size / 2);

    Vector<Int> stokes(4);
    stokes(0) = Stokes::I;
    stokes(1) = Stokes::Q;
    stokes(2) = Stokes::U;
    stokes(3) = Stokes::V;

    // -------------------------------------------------------------------------
    // Stripped part of Imager2::imagecoordinates2().
    // -------------------------------------------------------------------------

//    Not used anywhere for MFS imaging?
//    MFrequency::Types obsFreqRef = MFrequency::DEFAULT;
//    ROScalarColumn<Int> measFreqRef(ms.spectralWindow(),
//        MSSpectralWindow::columnName(MSSpectralWindow::MEAS_FREQ_REF));

//    //using the first frame of reference; TO DO should do the right thing
//    //for different frames selected.
//    //Int eh = spectralwindowids_p(0);
//    if(measFreqRef(0) >= 0)
//    {
//        obsFreqRef = (MFrequency::Types) measFreqRef(0);
//    }

//    cout << "obsFreqRef: " << (Int) obsFreqRef << " MFrequency::REST: "
//        << (Int) MFrequency::REST << " MFrequency::DEFAULT: "
//        << (Int) MFrequency::DEFAULT <<" MFrequency::LSRK: "
//        << (Int) MFrequency::LSRK << endl;

    ROMSColumns msc(ms);
    Double refChan = 0.0;

    // Spectral synthesis
    // For mfs band we set the window to include all spectral windows
//    Int nspw = 1;

    Double fmin = C::dbl_max;
    Double fmax = -(C::dbl_max);
    Double fmean = 0.0;

    Int spw = 0;
    Vector<Double> chanFreq = msc.spectralWindow().chanFreq()(spw);
    Vector<Double> freqResolution = msc.spectralWindow().chanWidth()(spw);

    // Assumed datamode == "none" (should be same as "channel" with start 0,
    // step 1).

    fmin = min(chanFreq - abs(0.5 * freqResolution));
    fmax = max(chanFreq + abs(0.5 * freqResolution));

    fmean = (fmax + fmin) / 2.0;

//    cout << "fmin: " << fmin << " fmax: " << fmax << " fmean: " << fmean
//        << endl;

    Double restFreq = fmean;
    Vector<Double> restFreqArray;
    if(getRestFreq(ms, restFreqArray, 0))
    {
        restFreq = restFreqArray[0];
    }
//    cout << "restFreq: " << restFreq << endl;

    Double finc = (fmax - fmin);

    // LSRK seems to be the default (also used in awimager implicitly, via
    // the default parameter value of Imager::defineImage().
    MFrequency::Types freqFrame_p(MFrequency::LSRK);

    SpectralCoordinate spectralCoordinate(freqFrame_p, fmean, finc, refChan,
        restFreq);

    cout << "center frequency = "
       << MFrequency(Quantity(fmean, "Hz")).get("MHz").getValue()
       << " MHz, synthesized continuum bandwidth = "
       << MFrequency(Quantity(finc, "Hz")).get("MHz").getValue()
       << " MHz" << endl;

//    bool valid = memento.obsKnown && (obsFreqRef != MFrequency::REST);
//    if(valid)
//        spectralCoordinate.setReferenceConversion(MFrequency::LSRK, obsEpoch,
//            obsPosition, phaseCenter_p);
//    }

    CoordinateSystem csys;
    csys.addCoordinate(directionCoordinate);
    csys.addCoordinate(StokesCoordinate(stokes));
    csys.addCoordinate(spectralCoordinate);
    return csys;
}

// Stripped Imager::makeVisSet() (Imager2.cc)
void makeVisIterator(Memento &memento)
{
    Block<Int> sort(4);
    sort[0] = MS::ARRAY_ID;
    sort[1] = MS::FIELD_ID;
    sort[2] = MS::DATA_DESC_ID;
    sort[3] = MS::TIME;

    Double timeInterval = 0;
    memento.it = new ROVisibilityIterator(memento.ms, sort, timeInterval);

    VisImagingWeight weight("natural");
    memento.it->useImagingWeight(weight);
}

void init(Memento &memento, const string &name, const Record &csys,
    const ValueHolder &shape, const Record &parms)
{
    // -------------------------------------------------------------------------
    // This method is a stripped combination of Imager::clean(),
    // Imager::makeVisSet(), Imager::createSkyEquation(),
    // LofarImager::createFTMachine(), and LofarImager::setSkyEquation().
    // -------------------------------------------------------------------------
    MeasurementSet ms(name);
    memento.ms = ms(ms.col("ANTENNA1") != ms.col("ANTENNA2")
        && ms.col("OBSERVATION_ID") == 0
        && ms.col("FIELD_ID") == 0
        && ms.col("DATA_DESC_ID") == 0);
    makeVisIterator(memento);

    memento.mdl.setSubAlgorithm("clark");
    memento.mdl.setAlgorithm("clean");

    // -------------------------------------------------------------------------
    // Stripped version of Imager::createSkyEquation() (see Imager2.cc)
    // -------------------------------------------------------------------------

    // << read polarization information from the MS >>

    // Send the data correlation type to the SkyModel.
    // This needs to be done before the first call to "ImageSkyModel::cImage()".
    memento.mdl.setDataPolFrame(SkyModel::LINEAR);

    // << add component list >>

    Int nmodels_p = 1;

    // << create temporary model image >>
    CoordinateSystem *tmp_csys = CoordinateSystem::restore(csys, "");
    AlwaysAssert(tmp_csys != 0, AipsError);
    memento.csys = *tmp_csys;
    delete tmp_csys;

    // Translate image shape from Python (note reversal)!
    Vector<Int> tmp_shape = shape.asArrayInt();
    AlwaysAssert(tmp_shape.size() == 4, AipsError);
    IPosition ipos_shape(4, tmp_shape[3], tmp_shape[2], tmp_shape[1],
        tmp_shape[0]);

    memento.image = TempImage<Float>(ipos_shape, memento.csys);
    memento.image.set(0.0);

    // << determine number of XFRS >>
    // (not sure what this is for, but as long as we image only one field,
    // and do not use beam squint related stuff, this is always set to #models
    // + 1)
    Int numOfXFR = nmodels_p + 1;

    Int status = memento.mdl.add(memento.image, numOfXFR);
    AlwaysAssert(status == 0, AipsError);

    // << add masks to sky equation >>

    // -------------------------------------------------------------------------
    // Stripped version of Imager::imagecoordinates2() (see Imager2.cc)
    // -------------------------------------------------------------------------

    ROMSColumns msc(memento.ms);

    //defining observatory...needed for position on earth
    String telescop = msc.observation().telescopeName()(0);
//    cout << "telescope: " << telescop << endl;

    //Now finding the position of the telescope on Earth...needed for proper
    //frequency conversions
    MPosition obsPosition;
    if(!(MeasTable::Observatory(obsPosition, telescop)))
    {
        cout << "Did not get the position of " << telescop
           << " from data repository." << endl;
        cout << "Please contact CASA to add it to the repository."
           << endl;
        cout << "Frequency conversion will not work " << endl;
    }
    else{
        memento.obs = obsPosition;
        memento.obsKnown = true;
    }

    // -------------------------------------------------------------------------
    // Stripped version of LofarImager::createFTMachine()
    // -------------------------------------------------------------------------

    // Defaults from awimager (see Imager::setoptions() call).
    Long cache_p = 512*1024*(1024/8); // 512 MB
    Int tile_p = 16;
    String gridfunction_p = "sf";
    MPosition mLocation_p;
    Float padding_p = parms.asFloat("padding");
    Int wprojPlanes_p = parms.asInt("wplanes");

    // From LofarImager::createFTMachine().
    CountedPtr<VisibilityResamplerBase> visResampler;
    Bool useDoublePrecGrid = False;

    // RefFreq doesn't look to be used by LofarFTMachine (!)
    Double RefFreq = memento.mdl.getReferenceFrequency();

//    cout << "casaimwrap::init(): mLocation_p: " << mLocation_p << " RefFreq: " << RefFreq << endl;
//    cout << "casaimwrap::init(): Parms (C++): " << endl << parms << endl;

    memento.ft = new LofarFTMachine(cache_p/2, tile_p,
        visResampler, gridfunction_p,
        memento.ms, wprojPlanes_p, mLocation_p,
        padding_p, false, useDoublePrecGrid,
        parms.asDouble("wmax"),
        parms.asInt("verbose"),
        parms.asInt("maxsupport"),
        parms.asInt("oversample"),
        parms.asString("imagename"),
        parms.asArrayBool("mueller.grid"),
        parms.asArrayBool("mueller.degrid"),
        RefFreq,
        parms.asBool("UseLIG"),
        parms.asBool("UseEJones"),
        parms.asInt("StepApplyElement"),
        parms.asInt("ApplyBeamCode"),
        parms.asDouble("PBCut"),
        parms.asBool("PredictFT"),
        parms.asString("PsfImage"),
        parms.asBool("UseMasksDegrid"),
        parms.asBool("doPSF"),
        parms.asDouble("UVmin"),
        parms.asDouble("UVmax"),
        parms.asBool("MakeDirtyCorr"),
        parms);

    memento.ft->initGridThreads(memento.itsGridsParallel,
        memento.itsGridsParallel2);

    Int rowBlock = parms.asInt("RowBlock");
    if(rowBlock <= 0)
    {
        // Try to automatically determenine row blocking parameter.
        TableIterator iter(memento.ms, "TIME", TableIterator::Ascending,
            TableIterator::NoSort);
        uInt nrowPerTime = iter.table().nrow();
        double interval = ROScalarColumn<double>(iter.table(), "INTERVAL")(0);
        Int ntime = parms.asDouble("timewindow") / interval;
        rowBlock = nrowPerTime * max(1, ntime);
    }

//    cout << "casaimwrap::init(): rowBlock: " << rowBlock << endl;
    memento.it->setRowBlocking(rowBlock);

    // -------------------------------------------------------------------------
    // Stripped version of LofarImager::setSkyEquation()
    // -------------------------------------------------------------------------

    // TODO: Have to use noModelCol == false here because of strange effects
    // caused by ft_-setNoPadding() in LofarCubeSkyEquation:: makeSimplePSF().
    memento.eq = new LofarCubeSkyEquation(memento.mdl, *memento.it, *memento.ft,
        memento.cft, false);

    // -------------------------------------------------------------------------
    // Continuation of the stripped version of Imager::createSkyEquation()
    // -------------------------------------------------------------------------

    // Defaults from awimager.cc (see call to Imager::setmfcontrol()).
    memento.eq->setImagePlaneWeighting("", 0.1, 0.4);
    memento.eq->doFlatNoise(true);

//    // -------------------------------------------------------------------------
//    // Stripped version of Imager::addResiduals()
//    // -------------------------------------------------------------------------
//    memento.residual = new PagedImage<Float>(TiledShape(memento.image->shape(),
//        memento.image->niceCursorShape()),
//		memento.image->coordinates(),
//        parms.asString("imagename") + ".residual");
//	memento.residual->setUnits(Unit("Jy/beam"));

    // -------------------------------------------------------------------------
    // Continuation of the stripped version of Imager::clean()
    // -------------------------------------------------------------------------

    // Defaults from awimager.cc.
    memento.mdl.setGain(parms.asDouble("gain"));
    memento.mdl.setNumberIterations(parms.asInt("niter"));
    memento.mdl.setThreshold(parms.asDouble("threshold"));
    memento.mdl.setCycleFactor(parms.asDouble("cyclefactor"));
    memento.mdl.setCycleSpeedup(parms.asDouble("cyclespeedup"));
    memento.mdl.setCycleMaxPsfFraction(0.8);

    memento.gridBuffer = new VisBufferStub(memento.ms);
}

//void ImageSkyModel::makeApproxPSFs(SkyEquation& se)
Record makeApproxPSF(Memento &memento)
{
    // Create temporary images to hold the PSFs.
    PtrBlock<TempImage<Float>*> psf(memento.mdl.numberOfModels());
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        psf[i] = new TempImage<Float>(TiledShape(memento.mdl.image(i).shape(),
            memento.mdl.image(i).niceCursorShape()),
            memento.mdl.image(i).coordinates(), 0);
        AlwaysAssert(psf[i], AipsError);

        psf[i]->set(0.0);
        psf[i]->setMaximumCacheSize(psf[i]->shape().product());
        psf[i]->clearCache();
    }

    // Compute PSFs.
    memento.eq->makeApproxPSF(psf);

    // Build result Record.
    Record record;
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        ostringstream oss;
        oss << "psf-" << i;
        record.define(oss.str(), psf[i]->get());
    }

    // Clean up temporary images.
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        delete psf[i];
    }

    return record;
}

//Bool ImageSkyModel::makeNewtonRaphsonStep(SkyEquation& se, Bool incremental,
//    Bool modelToMS)
Record makeNewtonRaphsonStep(Memento &memento, bool modelToMS)
{
    memento.eq->gradientsChiSquared(false, modelToMS);

    Record record;
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        ostringstream oss;
        oss << "gS-" << i;
        record.define(oss.str(), memento.mdl.gS(i).get());

        oss.str("");
        oss << "ggS-" << i;
        record.define(oss.str(), memento.mdl.ggS(i).get());
    }
    return record;
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
    cleaner.setNumberIterations(options.asInt("numberIterations"));
    cleaner.setInitialNumberIterations(iterations);
    cleaner.setThreshold(options.asFloat("cycleThreshold"));
    cleaner.setPsfPatchSize(IPosition(2, options.asInt("psfpatch")));
    cleaner.setMaxNumberMajorCycles(10);
    cleaner.setHistLength(1024);
    cleaner.setMaxNumPix(32 * 1024);
    cleaner.setChoose(false);
    cleaner.setCycleSpeedup(options.asFloat("cycleSpeedup"));
    cleaner.setSpeedup(0.0);
    cleaner.singleSolve(eqn, residualArray);

    Record result;
    result.define("numberIterations", cleaner.numberIterations());
    result.define("delta", delta);

    // TODO: It seems that delta already has the clean components, so this
    // step should not be necessary (unless delta does not have the updates of
    // the last iteration?). For now, always verify that both images are equal.
    Array<Float> tmp;
    cleaner.getModel(tmp);
    AlwaysAssert(allTrue(delta == tmp), AipsError);

    return result;
}

void setImage(Memento &memento, unsigned int id, const ValueHolder &image)
{
    memento.mdl.image(id).put(image.asArrayFloat());
}

Record fitGaussianPSF(const Record &csys, const ValueHolder &psf)
{
    CoordinateSystem *tmp_csys = CoordinateSystem::restore(csys, "");
    AlwaysAssert(tmp_csys != 0, AipsError);

    Array<Float> array_psf = psf.asArrayFloat();
    TempImage<Float> tmp_image(array_psf.shape(), *tmp_csys);
    tmp_image.put(array_psf);

    Vector<Float> beam(3, 0.0f);
    Bool ok = StokesImageUtil::FitGaussianPSF(tmp_image, beam);

    // Clean up CoordinateSystem instace.
    delete tmp_csys;

    Record result;
    result.define("ok", ok);
    result.define("major", beam[0]);
    result.define("minor", beam[1]);
    result.define("angle", beam[2]);
    return result;
}

ValueHolder convolveWithBeam(const Record &csys, const ValueHolder &image,
    float major, float minor, float pa)
{
    CoordinateSystem *tmp_csys = CoordinateSystem::restore(csys, "");
    AlwaysAssert(tmp_csys != 0, AipsError);

    Array<Float> array_image = image.asArrayFloat();
    TempImage<Float> tmp_image(array_image.shape(), *tmp_csys);
    tmp_image.put(array_image);

    StokesImageUtil::Convolve(tmp_image, major, minor, pa);
    delete tmp_csys;

    return ValueHolder(tmp_image.get());
}

Record makeCoordinateSystemPy(const string &name, unsigned int size,
    double delta)
{
    MeasurementSet ms(name);
    ms = ms(ms.col("ANTENNA1") != ms.col("ANTENNA2")
        && ms.col("OBSERVATION_ID") == 0
        && ms.col("FIELD_ID") == 0
        && ms.col("DATA_DESC_ID") == 0);

    CoordinateSystem csys = makeCoordinateSystem(ms, size, delta);

    Record record;
    csys.save(record, "coordinate_system");
    return record.subRecord(0);
}

Record begin_degrid(Memento &memento, const ValueHolder &image,
    const Record &chunk)
{
    // Create temporary image.
    TempImage<Float> tmp_image(memento.image.shape(),
        memento.image.coordinates());
    tmp_image.put(image.asArrayFloat());

    // Complex image to degrid.
    memento.gridImage = TempImage<Complex>(memento.image.shape(),
        memento.image.coordinates());
    StokesImageUtil::changeCStokesRep(memento.gridImage, SkyModel::LINEAR);

    // Convert from Stokes to Complex
    StokesImageUtil::From(memento.gridImage, tmp_image);

    // Update temporary VisBuffer.
    memento.gridBuffer->setChunk(chunk.asArrayInt("antenna1"),
        chunk.asArrayInt("antenna2"),
        chunk.asArrayDouble("uvw"),
        chunk.asArrayDouble("time"),
        chunk.asArrayDouble("centroid"),
        chunk.asArrayBool("flag_row"),
        chunk.asArrayFloat("weight"),
        chunk.asArrayBool("flag"),
        true);

    memento.ft->initializeToVis(memento.gridImage, *memento.gridBuffer);
    memento.ft->get(*memento.gridBuffer);

    Record result;
    result.define("data", memento.gridBuffer->modelVisCube());
    return result;
}

Record degrid(Memento &memento, const Record &chunk)
{
    // Update temporary VisBuffer.
    memento.gridBuffer->setChunk(chunk.asArrayInt("antenna1"),
        chunk.asArrayInt("antenna2"),
        chunk.asArrayDouble("uvw"),
        chunk.asArrayDouble("time"),
        chunk.asArrayDouble("centroid"),
        chunk.asArrayBool("flag_row"),
        chunk.asArrayFloat("weight"),
        chunk.asArrayBool("flag"),
        false);

    // Grid data.
    memento.ft->get(*memento.gridBuffer);

    Record result;
    result.define("data", memento.gridBuffer->modelVisCube());
    return result;
}

void end_degrid(Memento &memento)
{
    memento.ft->finalizeToVis();
}

void begin_grid(Memento &memento, bool gridPSF, const Record &chunk)
{
    memento.gridPSF = gridPSF;

    // Create temporary image.
    memento.gridImage = TempImage<Complex>(memento.image.shape(),
        memento.image.coordinates());
    StokesImageUtil::changeCStokesRep(memento.gridImage, SkyModel::LINEAR);
    memento.gridImage.set(0.0);

    // Create weight matrix.
    // ==> No need, will be resized by LofarFTMachine.

    // Update temporary VisBuffer.
    memento.gridBuffer->setChunk(chunk.asArrayInt("antenna1"),
        chunk.asArrayInt("antenna2"),
        chunk.asArrayDouble("uvw"),
        chunk.asArrayDouble("time"),
        chunk.asArrayDouble("centroid"),
        chunk.asArrayBool("flag_row"),
        chunk.asArrayFloat("weight"),
        chunk.asArrayBool("flag"),
        chunk.asArrayComplex("data"),
        true);

    // Initialize static information in temporary VisBuffer.
    memento.ft->initializeToSky(memento.gridImage, memento.gridWeight,
        *memento.gridBuffer);

    // Grid data.
    memento.ft->put(*memento.gridBuffer, -1, memento.gridPSF,
        FTMachine::OBSERVED);
}

void grid(Memento &memento, const Record &chunk)
{
    // Update temporary VisBuffer.
    memento.gridBuffer->setChunk(chunk.asArrayInt("antenna1"),
        chunk.asArrayInt("antenna2"),
        chunk.asArrayDouble("uvw"),
        chunk.asArrayDouble("time"),
        chunk.asArrayDouble("centroid"),
        chunk.asArrayBool("flag_row"),
        chunk.asArrayFloat("weight"),
        chunk.asArrayBool("flag"),
        chunk.asArrayComplex("data"),
        false);

    // Grid data.
    memento.ft->put(*memento.gridBuffer, -1, memento.gridPSF,
        FTMachine::OBSERVED);
}

Record end_grid(Memento &memento, bool normalize)
{
    // For each model...
    memento.ft->finalizeToSky();

    ImageInterface<Complex> &image =
        memento.ft->getImage(memento.gridWeight, normalize);

    TempImage<Float> tmp_image(memento.image.shape(),
        memento.image.coordinates());
    tmp_image.set(0.0);

    if(memento.gridPSF)
    {
        StokesImageUtil::ToStokesPSF(tmp_image, image);
    }
    else
    {
        StokesImageUtil::To(tmp_image, image);
    }

//    {
//        Array<Complex> a0 = image.get();
//        Array<Complex> a1 = memento.gridImage.get();

//        AlwaysAssert(allTrue(a0 == a1), AipsError);
//    }

    // Create temporary weight image.
    memento.gridWeightImage = TempImage<Float>(memento.image.shape(),
        memento.image.coordinates());
    memento.gridWeightImage.set(0.0);
    memento.ft->getWeightImage(memento.gridWeightImage, memento.gridWeight);

    Record result;
    result.define("weight", memento.gridWeightImage.get());
//    result.define("image", memento.gridImage.get());
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

    class_<LOFAR::casaimwrap::Memento>("Memento");

    def("init", LOFAR::casaimwrap::init, (boost::python::arg("memento"),
        boost::python::arg("name"), boost::python::arg("parms")));
    def("makeCoordinateSystem", LOFAR::casaimwrap::makeCoordinateSystemPy,
        (boost::python::arg("name"), boost::python::arg("size"),
        boost::python::arg("delta")));
    def("makeApproxPSF", LOFAR::casaimwrap::makeApproxPSF,
        (boost::python::arg("memento")));
    def("makeNewtonRaphsonStep", LOFAR::casaimwrap::makeNewtonRaphsonStep, (boost::python::arg("memento"), boost::python::arg("modelToMS")));
    def("clarkClean", LOFAR::casaimwrap::clarkClean, (boost::python::arg("psf"), boost::python::arg("residual"), boost::python::arg("wmask"), boost::python::arg("iterations"), boost::python::arg("options")));
    def("setImage", LOFAR::casaimwrap::setImage, (boost::python::arg("memento"), boost::python::arg("id"), boost::python::arg("image")));
    def("convolveWithBeam", LOFAR::casaimwrap::convolveWithBeam, (boost::python::arg("csys"), boost::python::arg("image"), boost::python::arg("major"), boost::python::arg("minor"), boost::python::arg("pa")));
    def("fitGaussianPSF", LOFAR::casaimwrap::fitGaussianPSF, (boost::python::arg("csys"), boost::python::arg("psf")));

    def("begin_degrid", LOFAR::casaimwrap::begin_degrid, (boost::python::arg("memento"), boost::python::arg("image"), boost::python::arg("chunk")));
    def("degrid", LOFAR::casaimwrap::degrid, (boost::python::arg("memento"), boost::python::arg("chunk")));
    def("end_degrid", LOFAR::casaimwrap::end_degrid, (boost::python::arg("memento")));
    def("begin_grid", LOFAR::casaimwrap::begin_grid, (boost::python::arg("memento"), boost::python::arg("gridPSF"), boost::python::arg("chunk")));
    def("grid", LOFAR::casaimwrap::grid, (boost::python::arg("memento"), boost::python::arg("chunk")));
    def("end_grid", LOFAR::casaimwrap::end_grid, (boost::python::arg("memento"), boost::python::arg("normalize")));
}
