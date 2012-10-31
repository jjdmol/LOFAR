//# casaimwrap.cc:
//# Copyright (C) 2007
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

#include <casa/Containers/ValueHolder.h>
#include <casa/Containers/Record.h>
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
#include <tables/Tables/TableIter.h>

#include <casa/Utilities/CountedPtr.h>
#include <casa/OS/HostInfo.h>

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

//    CountedPtr<PagedImage<Float> >  image;
    TempImage<Float>            image;

    Memento()
        :   obsKnown(false),
            it(0),
            eq(0),
            ft(0)
    {
    }

    ~Memento()
    {
        delete ft;
        delete eq;
        delete it;
    }
};

MDirection readDelayReference(const Memento &memento, unsigned int idField)
{
    ROMSFieldColumns field(memento.ms.field());
    AlwaysAssert(field.nrow() > idField, AipsError);
    AlwaysAssert(!field.flagRow()(idField), AipsError);

//    MDirection refDirJ2000(MDirection::Convert(field.delayDirMeas(idField),
//        MDirection::J2000)());
//    Quantum<Vector<Double> > angles = refDirJ2000.getAngle();
//    return angles.getBaseValue();
    return field.delayDirMeas(idField);
}

MDirection readPhaseReference(const Memento &memento, unsigned int idField)
{
    ROMSFieldColumns field(memento.ms.field());
    AlwaysAssert(field.nrow() > idField, AipsError);
    AlwaysAssert(!field.flagRow()(idField), AipsError);

//    MDirection refDirJ2000(MDirection::Convert(field.phaseDirMeas(idField),
//        MDirection::J2000)());
//    Quantum<Vector<Double> > angles = refDirJ2000.getAngle();
//    return angles.getBaseValue();
    return field.phaseDirMeas(idField);
}

//Bool Imager::getRestFreq(Vector<Double>& restFreq, const Int& spw)
bool getRestFreq(const Memento &memento, Vector<Double> &restFreq, Int spw)
{
    // MS Doppler tracking utility
    MSDopplerUtil msdoppler(memento.ms);
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

CoordinateSystem makeCoordinateSystem(const Memento &memento, unsigned int size,
    double delta)
{
    Matrix<Double> xform(2,2);
    xform = 0.0; xform.diagonal() = 1.0;
//    Vector<Double> refDelay = readDelayReference(memento, 0);
//    Vector<Double> refPhase = readPhaseReference(memento, 0);

    MDirection mRefPhase = readPhaseReference(memento, 0);
    MVDirection mvRefPhase(mRefPhase.getAngle());

    MVAngle ra = mvRefPhase.get()(0);
    MVAngle dec = mvRefPhase.get()(1);
    // Normalize correctly (??) (see imagecoordinates2(), in Imager2.cc).
    ra(0.0);

    Vector<Double> refPhase(2);
    refPhase(0) = ra.get().getValue();
    refPhase(1) = dec;

//    DirectionCoordinate directionCoordinate(MDirection::J2000,
    DirectionCoordinate directionCoordinate(MDirection::Types(mRefPhase.getRefPtr()->getType()),
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
    MFrequency::Types obsFreqRef = MFrequency::DEFAULT;
    ROScalarColumn<Int> measFreqRef(memento.ms.spectralWindow(),
        MSSpectralWindow::columnName(MSSpectralWindow::MEAS_FREQ_REF));

    //using the first frame of reference; TO DO should do the right thing
    //for different frames selected.
    //Int eh = spectralwindowids_p(0);
    if(measFreqRef(0) >= 0)
    {
        obsFreqRef = (MFrequency::Types) measFreqRef(0);
    }

    cout << "obsFreqRef: " << (Int) obsFreqRef << " MFrequency::REST: "
        << (Int) MFrequency::REST << " MFrequency::DEFAULT: "
        << (Int) MFrequency::DEFAULT <<" MFrequency::LSRK: "
        << (Int) MFrequency::LSRK << endl;

    ROMSColumns msc(memento.ms);
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

    cout << "fmin: " << fmin << " fmax: " << fmax << " fmean: " << fmean
        << endl;

    Double restFreq = fmean;
    Vector<Double> restFreqArray;
    if(getRestFreq(memento, restFreqArray, 0))
    {
        restFreq = restFreqArray[0];
    }
    cout << "restFreq: " << restFreq << endl;

    Double finc = (fmax - fmin);

    // LSRK seems to be the default (also used in awimager implicitly, via
    // the default parameter value of Imager::defineImage().
    MFrequency::Types freqFrame_p(MFrequency::LSRK);

    SpectralCoordinate spectralCoordinate(freqFrame_p, fmean, finc, refChan,
        restFreq);
    cout << "Center frequency = "
       << MFrequency(Quantity(fmean, "Hz")).get("MHz").getValue()
       << " GHz, synthesized continuum bandwidth = "
       << MFrequency(Quantity(finc, "Hz")).get("MHz").getValue()
       << " GHz" << LogIO::POST;

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

bool init(Memento &memento, const string &msName, const Record &parms)
{
    // -------------------------------------------------------------------------
    // This method is a stripped combination of Imager::clean(),
    // Imager::makeVisSet(), Imager::createSkyEquation(),
    // LofarImager::createFTMachine(), and LofarImager::setSkyEquation().
    // -------------------------------------------------------------------------

    MeasurementSet ms(msName);
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

    // << open model images >>
//    memento.image = new PagedImage<Float>(parms.asString("imagename"));
//    memento.image = new PagedImage<Float>("casa_image_template");
//    memento.image->set(0.0);

    // << create temporary model image >>
    memento.csys = makeCoordinateSystem(memento, parms.asInt("n_px"),
        parms.asDouble("delta"));

    IPosition shape(4, parms.asInt("n_px"), parms.asInt("n_px"),
        parms.asInt("n_cr"), parms.asInt("n_ch"));
//    memento.image = new PagedImage<Float>(shape, csys, parms.asString("imagename"));
    memento.image = TempImage<Float>(shape, memento.csys);
    memento.image.set(0.0);

    // << determine number of XFRS >>
    // (not sure what this is for, but as long as we image only one field,
    // and do not use beam squint related stuff, this is always set to #models
    // + 1)
    Int numOfXFR = nmodels_p + 1;

    if(memento.mdl.add(memento.image, numOfXFR) != 0)
    {
        cout << "error adding model!" << endl;
        return false;
    }

    // << add masks to sky equation >>

    // -------------------------------------------------------------------------
    // Stripped version of Imager::imagecoordinates2() (see Imager2.cc)
    // -------------------------------------------------------------------------

    ROMSColumns msc(memento.ms);

    //defining observatory...needed for position on earth
    String telescop = msc.observation().telescopeName()(0);
    cout << "telescope: " << telescop << endl;

    //Now finding the position of the telescope on Earth...needed for proper
    //frequency conversions
    MPosition obsPosition;
    if(!(MeasTable::Observatory(obsPosition, telescop)))
    {
        cout << "Did not get the position of " << telescop
           << " from data repository" << endl;
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
        parms.asDouble("PBCut"),
        parms.asBool("PredictFT"),
        parms.asString("PsfImage"),
        parms.asBool("UseMasksDegrid"),
        parms.asBool("doPSF"),
        parms);

    Int rowBlock = parms.asInt("RowBlock");
//    Int rowBlock = 23780;
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

    cout << "casaimwrap::init(): rowBlock: " << rowBlock << endl;
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

    return true;
}

bool clean(Memento &memento)
{
    bool converged = memento.eq->solveSkyModel();
    return converged;
}

void casa_clean(const Record &parms)
{
    String msName    = parms.asString("ms");
    String operation = "mfclark";

    double timewindow = parms.asDouble("timewindow");
    double wmax       = parms.asDouble("wmax");
    Matrix<Bool> muelgrid   = parms.asArrayBool("mueller.grid");
    Matrix<Bool> mueldegrid = parms.asArrayBool("mueller.degrid");
    Int verbose      = parms.asInt("verbose");
    Int maxsupport   = parms.asInt("maxsupport");
    Int oversample   = parms.asInt("oversample");
    String imgName = parms.asString("imagename");
    Bool UseLIG         = parms.asBool("UseLIG");
    Bool UseEJones      = parms.asBool("UseEJones");
    Double PBCut   = parms.asDouble("PBCut");
    Int StepApplyElement   = parms.asInt("StepApplyElement");
    if ((StepApplyElement%2 == 0)&&((StepApplyElement%2 != 0))) {
      StepApplyElement++;
    }
    String PsfImage    = parms.asString("PsfImage");
    Bool Use_masks    = parms.asBool("UseMasksDegrid");
    Int RowBlock   = parms.asInt("RowBlock");
    Bool applyIonosphere = parms.asBool("applyIonosphere");
    Bool applyBeam = parms.asBool("applyBeam");
    Bool splitbeam = parms.asBool("splitbeam");

    String stokes = "IQUV";

    Int npix         = parms.asInt("n_px");
    Quantity qcellsize(parms.asDouble("delta"), "rad");

    Vector<Int> nchan(1, 1);
    Vector<Int> chanstart(1, 0);
    Vector<Int> chanstep(1, 1);
    String chanmode = "channel";
    Int fieldid = 0;
    String uvdist;

    String mode      = "mfs";
    Int img_nchan    = 1;
    Int img_start    = 0;
    Int img_step     = 1;
    Int nfacet       = 1;

    String weight = "default";
    Int wplanes      = parms.asInt("wplanes");
    Double padding   = parms.asDouble("padding");
    Long cachesize   = 512;

    Int niter        = parms.asInt("niter");
    Double gain      = parms.asDouble("gain");
    Quantity threshold(parms.asDouble("threshold"), "Jy");
    Double cyclefactor   = parms.asDouble("cyclefactor");
    Double cyclespeedup  = parms.asDouble("cyclespeedup");

    String maskName;
    Bool fixed          = false;
    Bool displayProgress= false;

//    String imageType = "DATA";
//    imageType.downcase();
//    if (imageType == "data") {
//      imageType = "observed";
//    } else if (imageType == "corrected_data") {
//      imageType = "corrected";
//    } else if (imageType == "model_data") {
//      imageType = "model";
//    } else if (imageType == "residual_data") {
//      imageType = "residual";
//    }

    String select;
    if (select.empty()) {
      select = "ANTENNA1 != ANTENNA2";
    } else {
      select = '(' + select + ") && ANTENNA1 != ANTENNA2";
    }

//    if (imgName.empty()) {
//      imgName = msName;
//      imgName.gsub (Regex("\\..*"), "");
//      imgName.gsub (Regex(".*/"), "");
//      imgName += '-' + stokes + '-' + mode + String::toString(img_nchan)
//	+ ".img";
//    }

    String fitsname;
//    if (fitsName == "no") {
//      fitsName = String();
//    } else if (fitsName.empty()) {
//      fitsName = imgName + ".fits";
//    }
    String hdf5Name;
//    if (hdf5Name == "no") {
//      hdf5Name = String();
//    }
    String priorName, modelName, restoName, residName, psfName;
    if (priorName.empty()) {
      priorName = imgName + ".prior";
    }
    if (modelName.empty()) {
      modelName = imgName + ".model";
    }
    if (restoName.empty()) {
      restoName = imgName + ".restored";
    }
    if (residName.empty()) {
      residName = imgName + ".residual";
    }
    if (psfName.empty()) {
      psfName = imgName + ".psf";
    }

//    if (weight == "robust") {
//      weight = "briggs";
//    } else if (weight == "robustabs") {
//      weight = "briggsabs";
//    }
//    string rmode = "norm";
//    if (weight == "briggsabs") {
//      weight = "briggs";
//      rmode  = "abs";
//    } else if (weight == "uniform") {
//      rmode = "none";
//    }

    bool doShift = False;
    MDirection phaseCenter;
//    if (! phasectr.empty()) {
//      doShift = True;
//      phaseCenter = readDirection (phasectr);
//    }

    operation.downcase();
    AlwaysAssertExit (operation=="empty" || operation=="image" ||
                      operation=="csclean" || operation=="msmfs" || operation=="mfclark" ||
                      operation=="predict" || operation=="psf");

    ///AlwaysAssertExit (operation=="empty" || operation=="image" || operation=="hogbom" || operation=="clark" || operation=="csclean" || operation=="multiscale" || operation =="entropy");
//    IPosition maskBlc, maskTrc;
//    Quantity threshold;
//    Quantity sigma;
//    Quantity targetFlux;
//    Bool doClean = (operation != "empty"  &&  operation != "image"&&  operation != "psf");
//    if (doClean) {
//      maskBlc = readIPosition (mstrBlc);
//      maskTrc = readIPosition (mstrTrc);
//      threshold = readQuantity (threshStr);
//      sigma = readQuantity (sigmaStr);
//      targetFlux = readQuantity (targetStr);
//    }

    Bool doPSF =(operation=="psf");
    if(doPSF==true){
      operation="csclean";
      niter=0;
    }
//    // Get axis specification from filter.
//    Quantity bmajor, bminor, bpa;
//    readFilter (filter, bmajor, bminor, bpa);

    // Set the various imager variables.
    // The non-parameterized values used are the defaults in imager.g.
    MeasurementSet ms(msName, Table::Update);

    Record params;
    params.define ("timewindow", timewindow);
    params.define ("wmax", wmax);
    params.define ("mueller.grid", muelgrid);
    params.define ("mueller.degrid", mueldegrid);
    params.define ("verbose", verbose);
    params.define ("maxsupport", maxsupport);
    params.define ("oversample", oversample);
    params.define ("imagename", imgName);
    params.define ("UseLIG", UseLIG);
    params.define ("UseEJones", UseEJones);
    //params.define ("ApplyElement", ApplyElement);
    params.define ("PBCut", PBCut);
    params.define ("StepApplyElement", StepApplyElement);
    Bool PredictFT(false);
    if(operation=="predict"){PredictFT=true;}
    params.define ("PredictFT", PredictFT);
    params.define ("PsfImage", PsfImage);
    params.define ("UseMasksDegrid", Use_masks);
    params.define ("RowBlock", RowBlock);
    params.define ("doPSF", doPSF);
    params.define ("applyIonosphere", applyIonosphere);
    params.define ("applyBeam", applyBeam);
    params.define ("splitbeam", splitbeam);
    //params.define ("FillFactor", FillFactor);

    LOFAR::LofarImager imager(ms, params);

    MSSpWindowColumns window(ms.spectralWindow());

    Vector<Int> wind(window.nrow());
    for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};

    ROArrayColumn<Double> chfreq(window.chanFreq());
    cout<<"Number of channels: "<<chfreq(0).shape()[0]<<endl;

    Vector<Int> chansel(1);
    chansel(0)=chfreq(0).shape()[0];

    imager.setdata (chanmode,                       // mode
		    chansel,//nchan,
		    chanstart,
            chanstep,
		    MRadialVelocity(),              // mStart
		    MRadialVelocity(),              // mStep
		    wind,//spwid,
		    Vector<Int>(1,fieldid),
		    select,                         // msSelect
            String(),                       // timerng
            String(),                       // fieldnames
            Vector<Int>(),                  // antIndex
            String(),                       // antnames
            String(),                       // spwstring
            uvdist,                       // uvdist
            String(),                       // scan
            False);                         // useModelCol


    imager.setmfcontrol(cyclefactor,          //Float cyclefactor,
  			cyclespeedup,         //Float cyclespeedup,
  			0.8,                        //Float cyclemaxpsffraction,
  			2,                          //Int stoplargenegatives,
  			-1,                         //Int stoppointmode,
  			"",                         //String& scaleType,
  			0.1,                        //Float minPB,
  			0.4,                        //loat constPB,
  			Vector<String>(1, ""),      //Vector<String>& fluxscale,
  			true);                      //Bool flatnoise);

    imager.defineImage (npix,                       // nx
                        npix,                       // ny
                        qcellsize,                  // cellx
                        qcellsize,                  // celly
                        stokes,                     // stokes
                        phaseCenter,                // phaseCenter
                        doShift  ?  -1 : fieldid,   // fieldid
                        mode,                       // mode
                        img_nchan,                  // nchan
                        img_start,                  // start
                        img_step,                   // step
                        MFrequency(),               // mFreqstart
                        MRadialVelocity(),          // mStart
                        Quantity(1,"km/s"),         // qstep, Def=1 km/s
			            wind,//spwid,                      // spectralwindowids
                        nfacet);                    // facets

//    if (operation=="predict"){
//      String ftmachine("ft");
//      if (wplanes > 0) {
//        ftmachine = "wproject";
//      }
//      imager.setoptions(ftmachine,                    // ftmachine
//                        cachesize*1024*(1024/8),      // cache
//                        16,                           // tile
//                        "SF",                         // gridfunction
//                        MPosition(),                  // mLocation
//                        padding,                      // padding
//                        wplanes);                     // wprojplanes
//      imager.ft(Vector<String>(1, modelName), "",
//		False);

//      // Define weighting.
//      if (weight != "default") {
//        imager.weight (weight,                      // type
//                       rmode,                       // rmode
//                       Quantity(noise, "Jy"),       // briggsabs noise
//                       robust,                      // robust
//                       Quantity(0, "rad"),          // fieldofview
//                       0);                          // npixels
//      }

//      // If multiscale, set its parameters.
//      if (operation == "multiscale") {
//        String scaleMethod;
//        Vector<Float> userVector(userScaleSizes.shape());
//        convertArray (userVector, userScaleSizes);
//        if (userScaleSizes.size() > 1) {
//          scaleMethod = "uservector";
//        } else {
//          scaleMethod = "nscales";
//        }
//        imager.setscales(scaleMethod, nscales, userVector);
//      }
//      if (! filter.empty()) {
//        imager.filter ("gaussian", bmajor, bminor, bpa);
//      }
      String ftmachine("ft");
      if (wplanes > 0) {
        ftmachine = "wproject";
      }

      imager.setoptions(ftmachine,                    // ftmachine
                        cachesize*1024*(1024/8),      // cache
                        16,                           // tile
                        "SF",                         // gridfunction
                        MPosition(),                  // mLocation
                        padding,                      // padding
                        wplanes);                     // wprojplanes


	  Vector<String> modelNames(2);
	  modelNames[0]="model.main";
	  modelNames[1]="model.outlier";
	  File Dir_masks_file("JAWS_masks_degrid");
	  Directory Dir_masks("JAWS_masks_degrid");
	  if(Dir_masks_file.exists()){
	    Dir_masks.removeRecursive();
	  }
	  if(Use_masks){
	    Dir_masks.create();
	  }

      imager.clean(operation,                     // algorithm,
                   niter,                         // niter
                   gain,                          // gain
                   threshold,                     // threshold
                   displayProgress,               // displayProgress
                   Vector<String>(1, modelName),  // model
                   Vector<Bool>(1, fixed),        // fixed
                   "",                            // complist
                   Vector<String>(1, maskName),   // mask
                   Vector<String>(1, restoName),  // restored
                   Vector<String>(1, residName),  // residual
                   Vector<String>(1, psfName));   // psf

        // Do the final correction for primary beam and spheroidal.
//	ApplyElement=false;
//	if(StepApplyElement>0){ApplyElement=true;}
//        correctImages (restoName, modelName, residName, imgName, imager, ApplyElement);
//        precTimer.stop();
//        timer.show ("clean");
//	///        imager.showTimings (cout, precTimer.getReal());
//        // Convert result to fits if needed.
//        if (! fitsName.empty()) {
//          String error;
//          PagedImage<float> img(restoName);
//          if (! ImageFITSConverter::ImageToFITS (error,
//                                                 img,
//                                                 fitsName,
//                                                 64,         // memoryInMB
//                                                 preferVelocity)) {
//            THROW(AWImagerException, error);
//          }
//        }
//      }
//    }
//    } }catch (Exception& ex) {
//    cerr << ex << endl;
//    return 1;
//  }
//  cout << "awimager normally ended" << endl;
//  return 0;
}

//void ImageSkyModel::makeApproxPSFs(SkyEquation& se)
Record makeApproxPSF(Memento &memento)
{
    // Create temporary images to hold PSFs.
    PtrBlock<TempImage<Float>*> psf_p(memento.mdl.numberOfModels());
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        psf_p[i] = new TempImage<Float>(TiledShape(memento.mdl.image(i).shape(),
			           memento.mdl.image(i).niceCursorShape()),
		               memento.mdl.image(i).coordinates(),
                       0);
//		        workDirOnNFS_p ? memoryMB : 0);

        AlwaysAssert(psf_p[i], AipsError);
        psf_p[i]->set(0.0);
        psf_p[i]->setMaximumCacheSize(psf_p[i]->shape().product());
        psf_p[i]->clearCache();
    }

    memento.eq->makeApproxPSF(psf_p);

//    // TODO: It does not seem to be required to write back the PSFs.
//    // Nasty hack to put PSF back into MFCleanImageSkyModel.
//    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
//    {
//        String error;
//        Record tmp;
//        bool ok = static_cast<ImageInterface<Float>*>(psf_p[i])->toRecord(error, tmp);
//        cout << "toRecord: " << ok << " " << error << endl;
//        memento.mdl.PSF(i).fromRecord(error, tmp);
//        cout << "fromRecord: " << ok << " " << error << endl;
//    }

    Record record;
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        ostringstream oss;
        oss << "psf-" << i;
        record.define(oss.str(), psf_p[i]->get());
    }

    // Clean up temporary images.
    for(Int i = 0; i < memento.mdl.numberOfModels(); i++)
    {
        delete psf_p[i];
    }

    return record;
}

//Bool ImageSkyModel::makeNewtonRaphsonStep(SkyEquation& se, Bool incremental,
//					  Bool modelToMS)
Record makeNewtonRaphsonStep(Memento &memento, bool modelToMS = false)
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
    // Now make a convolution equation for this
    // residual image and psf and then clean it

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

//Float MFCleanImageSkyModel::maxOuter(Lattice<Float> & lat, const uInt nCenter )
float maxOuter(const ValueHolder &in, unsigned int nCenter)
{
    Float myMax=0.0;
    Float myMin=0.0;

    //  Array<Float> arr = lat.get();
    Array<Float> arr = in.asArrayFloat();
    IPosition pos( arr.shape() );
    //  uInt nx = lat.shape()(0);
    //  uInt ny = lat.shape()(1);
    uInt nx = arr.shape()(0);
    uInt ny = arr.shape()(1);
    uInt nxc = 0;
    uInt nyc = 0;
    Float amax = 0.0;
    for (uInt ix = 0; ix < nx; ix++)
    {
        for (uInt iy = 0; iy < ny; iy++)
        {
            if (arr(IPosition(4, ix, iy, 0, 0)) > amax)
            {
                nxc = ix;
                nyc = iy;
                amax = arr(IPosition(4, ix, iy, 0, 0));
            }
        }
    }

    uInt nxL = nxc - nCenter;
    uInt nxH = nxc + nCenter;
    uInt nyL = nyc - nCenter;
    uInt nyH = nyc + nCenter;

    for (uInt ix = 0; ix < nx; ix++)
    {
        for (uInt iy = 0; iy < ny; iy++)
        {
            if ( !(ix >= nxL && ix <= nxH &&  iy >= nyL && iy <= nyH) )
            {
                if (arr(IPosition(4, ix, iy, 0, 0)) > myMax)
                {
                    myMax = arr(IPosition(4, ix, iy, 0, 0));
                }

                if (arr(IPosition(4, ix, iy, 0, 0)) < myMin)
                {
                    myMin = arr(IPosition(4, ix, iy, 0, 0));
                }
            }
        }
    }

    Float absMax = max( abs(myMin), myMax );
    return absMax;
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

Record makeCoordinateSystemPy(const Memento &memento, unsigned int size,
    double delta)
{
    CoordinateSystem csys = makeCoordinateSystem(memento, size, delta);

    Record record;
    csys.save(record, "coordinate_system");
    return record.subRecord(0);
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
        boost::python::arg("msName"), boost::python::arg("parms")));
//    def("init2", LOFAR::casaimwrap::init2, (boost::python::arg("memento"),
//        boost::python::arg("msName")));
//   def("makeCoordinateSystem", LOFAR::casaimwrap::makeCoordinateSystem,
//        (boost::python::arg("memento"), boost::python::arg("ra"),
//        boost::python::arg("dec"), boost::python::arg("size"),
//        boost::python::arg("delta")));
   def("makeCoordinateSystem", LOFAR::casaimwrap::makeCoordinateSystemPy,
        (boost::python::arg("memento"), boost::python::arg("size"),
        boost::python::arg("delta")));
//    def("readDelayReference", LOFAR::casaimwrap::readDelayReferencePy,
//        (boost::python::arg("memento")));
    def("clean", LOFAR::casaimwrap::clean, (boost::python::arg("memento")));
    def("makeApproxPSF", LOFAR::casaimwrap::makeApproxPSF,
        (boost::python::arg("memento")));
    def("maxOuter", LOFAR::casaimwrap::maxOuter, (boost::python::arg("arr"), boost::python::arg("nCenter")));
    def("makeNewtonRaphsonStep", LOFAR::casaimwrap::makeNewtonRaphsonStep, (boost::python::arg("memento"), boost::python::arg("modelToMS")));
    def("clarkClean", LOFAR::casaimwrap::clarkClean, (boost::python::arg("psf"), boost::python::arg("residual"), boost::python::arg("wmask"), boost::python::arg("iterations"), boost::python::arg("options")));

//    def("setResidual", LOFAR::casaimwrap::setResidual, (boost::python::arg("memento"), boost::python::arg("id"), boost::python::arg("residual")));
    def("setImage", LOFAR::casaimwrap::setImage, (boost::python::arg("memento"), boost::python::arg("id"), boost::python::arg("image")));
    def("convolveWithBeam", LOFAR::casaimwrap::convolveWithBeam, (boost::python::arg("csys"), boost::python::arg("image"), boost::python::arg("major"), boost::python::arg("minor"), boost::python::arg("pa")));
    def("fitGaussianPSF", LOFAR::casaimwrap::fitGaussianPSF, (boost::python::arg("csys"), boost::python::arg("psf")));
    def("casa_clean", LOFAR::casaimwrap::casa_clean, (boost::python::arg("parms")));
}
