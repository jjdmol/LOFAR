#!/usr/bin/env python

import sys
import argparse

import pylab
import numpy
import lofar.casaimwrap
import pyrap.images
import pyrap.tables

class BeamParameters:
    def __init__(self, majorAxis = 1.0, minorAxis = 1.0, positionAngle = 0.0):
        self._major = abs(majorAxis)
        self._minor = abs(minorAxis)
        self._angle = positionAngle

    def setMajorAxis(self, major):
        self._major = abs(major)

    def setMinorAxis(self, minor):
        self._minor = abs(minor)

    def setPositionAngle(self, angle):
        self._angle = angle

    def majorAxis(self):
        return self._major

    def minorAxis(self):
        return self._minor

    def positionAngle(self):
        return self._angle

class CleanOptions:
    def __init__(self):
        self._numberIterations = 100
        self._gain = 1.0
        self._tolerance = 0.000001
        self._threshold = 0.0
        self._free = False
        self._mode = ""
        self._cycleFactor = 1.5
        self._cycleSpeedup = -1.0
        self._cycleMaxPsfFraction = 0.8

    def setFree(self):
        self._free = True

    def setNotFree(self):
        self._free = False

    def free(self):
        return self._free

    def setNumberIterations(self, n):
        self._numberIterations = n

    def setGain(self, gain):
        self._gain = gain

    def setTolerance(self, tolerance):
        self._tolerance = tolerance

    def setThreshold(self, threshold):
        self._threshold = threshold

    def setMode(self, mode):
        self._mode = mode

    def setCycleFactor(self, factor):
        self._cycleFactor = factor

    def setCycleSpeedup(self, speedup):
        self._cycleSpeedup = speedup

    def setCycleMaxPsfFraction(self, fraction):
        self._cycleMaxPsfFraction = fraction

    def numberIterations(self):
        return self._numberIterations

    def gain(self):
        return self._gain

    def tolerance(self):
        return self._tolerance

    def threshold(self):
        return self._threshold

    def mode(self):
        return self._mode

    def cycleFactor(self):
        return self._cycleFactor

    def cycleSpeedup(self):
        return self._cycleSpeedup

    def cycleMaxPsfFraction(self):
        return self._cycleMaxPsfFraction

def show_image(title, data):
    fig, axes = pylab.subplots(nrows=2, ncols=2) #, sharex=True, sharey=True, squeeze=True)
    fig.suptitle(title, fontsize=14)

    vmin = numpy.min(data)
    vmax = numpy.max(data)
    for k, ax in zip(range(4), axes.flat):
        __im = ax.imshow(data[k,:,:], origin="lower", interpolation="nearest", cmap="bone", vmin=vmin, vmax=vmax) # aspect="auto")
        __im.axes.get_xaxis().set_visible(False)
        __im.axes.get_yaxis().set_visible(False)

    fig.subplots_adjust(right=0.70, wspace=0.0, hspace=0.0)
    cax = fig.add_axes([0.75, 0.1, 0.03, 0.8])
    cbr = fig.colorbar(__im, cax=cax)
    cbr.set_clim(vmin, vmax)

def validatePSF(csys, psf, beam):
#    if(isSolveable(model)) {

    nCh = psf.shape[0]

    # find channel with psf for polarization 0 > threshold
    threshold = 0.8 * numpy.max(psf)
    psfmax = 0.0
    i = 0
    while i < nCh:
        psfmax = numpy.max(psf[i, 0, :, :])
        if psfmax >= threshold:
            break
        i += 1

#    print "found channel:", i

    psfmin = numpy.min(psf[i, 0, :, :])

#    4 pixels:  pretty arbitrary, but only look for sidelobes
#    outside the inner (2n+1) * (2n+1) square
    ncent = 4

    # locate peak size
#   BUG:: Should this not be min(abs(...))??
    incr = numpy.abs(numpy.min(csys.get_increment()[2]))
    if incr > 0.0:
        ncent = max(ncent, max(beam.majorAxis() / incr, beam.minorAxis() / incr))

#    print "incr:", incr, "ncent:", ncent

    psfpatch = 3 * ncent + 1

    psfmaxouter = lofar.casaimwrap.maxOuter(psf[i, 0, :, :], ncent)

    print "max, min, maxOuter PSF:", psfmax, psfmin, psfmaxouter

    maxSidelobe = 0.0
    if abs(psfmin) > maxSidelobe:
        maxSidelobe = abs(psfmin)
    if psfmaxouter > maxSidelobe:
        maxSidelobe = psfmaxouter

#    print "maxSidelobe:", maxSidelobe
#    print "psfpatch:", psfpatch

    return (psfmin, psfmax, psfmaxouter, int(psfpatch), maxSidelobe)


def maxField(ggS, residual):
    # compute (signed) max of ggS
    max_ggS = numpy.max(ggS)

    print "shapes:", residual.shape, ggS.shape

    absmax = 0.0
    imin = 1e20
    imax = -1e20
    for ch in range(residual.shape[0]):
        # re-weight residual
        residual[ch, :, :, :] *= numpy.sqrt(ggS[ch, :, :, :] / max_ggS)

        fmax = numpy.max(residual[ch, :, :, :])
        fmin = numpy.min(residual[ch, :, :, :])

        if fmax < 0.99 * -1e20:
            fmax = 0.0

        if fmin > 0.99 * 1e20:
            fmin = 0.0

        absmax = max(absmax, max(abs(fmax), abs(fmin)))
        imin = min(imin, fmin)
        imax = max(imax, fmax)

    return (absmax, imin, imax)


def restore(csys, image, residual, beam):
    if beam.majorAxis() > 0.0 and beam.minorAxis() > 0.0:
        restored = lofar.casaimwrap.convolveWithBeam(csys, image, beam.majorAxis() * 180.0 * 3600.0 / numpy.pi, beam.minorAxis() * 180.0 * 3600.0 / numpy.pi, beam.positionAngle() * 180.0 / numpy.pi)
    else:
        restored = numpy.copy(image)

    restored += residual
    return restored

#        if(freqFrameValid_p)
#        {
#            CoordinateSystem cs=residIm.coordinates();
#            String errorMsg;
#            if (CoordinateUtil::setSpectralConversion (errorMsg, cs,MFrequency::showType(freqFrame_p)))
#            {
#                residIm.setCoordinateInfo(cs);
#                if(dorestore)
#                    restored.setCoordinateInfo(cs);
#            }
#        }

#        //
#        // Using minPB_p^2 below to make it consistent with the normalization in SkyEquation.
#        //
#        Float cutoffval=minPB_p;
#        if(ft_p->name()=="MosaicFT")
#        {
#            cutoffval=minPB_p*minPB_p;
#        }

#        if (sm_p->doFluxScale(thismodel))
#        {
#            TempImage<Float> cover(modelIm.shape(),modelIm.coordinates());
#            if(ft_p->name()=="MosaicFT")
#            {
#                se_p->getCoverageImage(thismodel, cover);
#            }
#            else
#            {
#                cover.copyData(sm_p->fluxScale(thismodel));
#            }

#            if(scaleType_p=="NONE")
#            {
#                if(dorestore)
#                {
#                    LatticeExpr<Float> le(iif(cover < minPB_p,
#                        0.0,(restored/(sm_p->fluxScale(thismodel)))));
#                        restored.copyData(le);
#                }

#                LatticeExpr<Float> le1(iif(cover < minPB_p,
#                    0,(residIm/(sm_p->fluxScale(thismodel)))));
#                    residIm.copyData(le1);
#            }

#            //Setting the bit-mask for mosaic image
#            LatticeExpr<Bool> lemask(iif((cover < cutoffval), False, True));
#            if(dorestore)
#            {
#                ImageRegion outreg=restored.makeMask("mask0", False, True);
#                LCRegion& outmask=outreg.asMask();
#                outmask.copyData(lemask);
#                restored.defineRegion("mask0", outreg, RegionHandler::Masks, True);
#                restored.setDefaultMask("mask0");
#            }
#        }

#void makeEmpty (Imager& imager, const String& imgName, Int fieldid)
#{
#  CoordinateSystem coords;
#  ASSERT (imager.imagecoordinates(coords));
#  String name(imgName);
#  imager.makeEmptyImage(coords, name, fieldid);
#  imager.unlock();
#}

#void applyFactors (PagedImage<Float>& image, const Array<Float>& factors)
#{
#  Array<Float> data;
#  image.get (data);
#  ///  cout << "apply factor to " << data.data()[0] << ' ' << factors.data()[0]<<endl;
#  // Loop over channels
#  for (ArrayIterator<Float> iter1(data, 3); !iter1.pastEnd(); iter1.next()) {
#    // Loop over Stokes.
#    ArrayIterator<Float> iter2(iter1.array(), 2);
#    while (! iter2.pastEnd()) {
#      iter2.array() *= factors;
#      iter2.next();
#    }
#  }
#  image.put (data);
#  ///  cout << "applied factor to " << data.data()[0] << ' ' << factors.data()[0]<<endl;
#}

#void correctImages (const String& restoName, const String& modelName,
#                    const String& residName, const String& imgName,
#                    LOFAR::LofarImager& imager, Bool CorrectElement)
#{
#  (void)imager;

#  // Copy the images to .corr ones.
#  {
#    Directory restoredIn(restoName);
#    restoredIn.copy (restoName+".corr");
#    Directory modelIn(modelName);
#    modelIn.copy (modelName+".corr");
#    Directory residualIn(residName);
#    residualIn.copy (residName+".corr");
#  }
#  // Open the images.
#  PagedImage<Float> restoredImage(restoName+".corr");
#  PagedImage<Float> modelImage(modelName+".corr");
#  PagedImage<Float> residualImage(residName+".corr");
#  AlwaysAssert (residualImage.shape() == modelImage.shape()  &&
#                restoredImage.shape() == modelImage.shape(), SynthesisError);

#  // Get average primary beam and spheroidal.
#  Matrix<Float> avgPB = LOFAR::LofarConvolutionFunction::getAveragePB(imgName+"0");
#  Matrix<Float> spheroidCut = LOFAR::LofarConvolutionFunction::getSpheroidCut(imgName+"0");
#  String nameii("Spheroid_cut_im_element.img");
#  ostringstream nameiii(nameii);
#  PagedImage<Float> tmpi(nameiii.str().c_str());
#  Slicer slicei(IPosition(4,0,0,0,0), tmpi.shape(), IPosition(4,1,1,1,1));
#  Array<Float> spheroidCutElement;
#  tmpi.doGetSlice(spheroidCutElement, slicei);
#  // Use the inner part of the beam and spheroidal.
#  Int nximg = restoredImage.shape()[0];
#  Int nxpb  = avgPB.shape()[0];
#  Int nxsph = spheroidCut.shape()[0];
#  AlwaysAssert (restoredImage.shape()[1] == nximg  &&
#                avgPB.shape()[1] == nxpb  &&
#                spheroidCut.shape()[1] == nxsph  &&
#                nxsph >= nximg  &&  nxpb >= nximg, SynthesisError);
#  // Get inner parts of beam and spheroid.
#  Int offpb  = (nxpb  - nximg) / 2;
#  Int offsph = (nxsph - nximg) / 2;
#  Array<Float> pbinner  = avgPB(Slicer(IPosition(2, offpb, offpb),
#                                       IPosition(2, nximg, nximg)));
#  Array<Float> sphinner = spheroidCut(Slicer(IPosition(2, offsph, offsph),
#                                             IPosition(2, nximg, nximg)));
#  Array<Float> factors;
#  if(CorrectElement){
#    Array<Float> sphinner_el = (spheroidCutElement(Slicer(IPosition(4, offsph, offsph,0,0),
#							  IPosition(4, nximg, nximg,1,1)))).nonDegenerate();
#    factors = sphinner_el *sphinner / sqrt(pbinner);//sphinner_el * sphinner / sqrt(pbinner);
#  } else{
#
#    factors = sphinner / sqrt(pbinner);//sphinner_el * sphinner / sqrt(pbinner);
#  }
#  applyFactors (restoredImage, factors);
#  applyFactors (modelImage, factors);
#  applyFactors (residualImage, factors);
#}

#void Imager::savePSF(const Vector<String>& psf){

#  if( (psf.nelements() > 0) && (Int(psf.nelements()) <= sm_p->numberOfModels())){

#    for (Int thismodel=0;thismodel<Int(psf.nelements());++thismodel) {
#      if(removeTable(psf(thismodel))) {
#	Int whichmodel=thismodel;
#	if(facets_p >1 && thismodel > 0)
#	  whichmodel=facets_p*facets_p-1+thismodel;
#	IPosition shape=images_p[thismodel]->shape();
#	PagedImage<Float> psfimage(shape,
#				   images_p[thismodel]->coordinates(),
#				   psf(thismodel));
#	if(freqFrameValid_p){
#	  CoordinateSystem cs=psfimage.coordinates();
#	  String errorMsg;
#	  if (CoordinateUtil::setSpectralConversion (errorMsg, cs,MFrequency::showType(freqFrame_p))) {
#	    psfimage.setCoordinateInfo(cs);
#	  }
#        }
#	psfimage.set(0.0);
#	if((shape[0]*shape[1]) > ((sm_p->PSF(whichmodel)).shape()[0]*(sm_p->PSF(whichmodel)).shape()[1])){
#	  IPosition blc(4, 0, 0, 0, 0);
#	  IPosition trc=shape-1;
#	  blc[0]=(shape[0]-(sm_p->PSF(whichmodel)).shape()[0])/2;
#	  trc[0]=(sm_p->PSF(whichmodel)).shape()[0]+blc[0]-1;
#	  blc[1]=(shape[1]-(sm_p->PSF(whichmodel)).shape()[1])/2;
#	  trc[1]=(sm_p->PSF(whichmodel)).shape()[1]+blc[1]-1;
#	  Slicer sl(blc, trc, Slicer::endIsLast);
#	  SubImage<Float> sub(psfimage, sl, True);
#	  sub.copyData(sm_p->PSF(whichmodel));
#	}
#	else{
#	  psfimage.copyData(sm_p->PSF(whichmodel));
#	}
#      }
#    }



#  }

#}

#Bool Imager::writeFluxScales(const Vector<String>& fluxScaleNames)
#{
#  LogIO os(LogOrigin("imager", "writeFluxScales()", WHERE));
#  Bool answer = False;
#  ImageInterface<Float> *cover;
#  if(fluxScaleNames.nelements()>0) {
#    for (Int thismodel=0;thismodel<Int(fluxScaleNames.nelements());++thismodel) {
#      if(fluxScaleNames(thismodel)!="") {
#        PagedImage<Float> fluxScale(images_p[thismodel]->shape(),
#                                    images_p[thismodel]->coordinates(),
#                                    fluxScaleNames(thismodel));
#	PagedImage<Float> coverimage(images_p[thismodel]->shape(),
#				     images_p[thismodel]->coordinates(),
#				     fluxScaleNames(thismodel)+".pbcoverage");
#        coverimage.table().markForDelete();
#	if(freqFrameValid_p){
#	  CoordinateSystem cs=fluxScale.coordinates();
#	  String errorMsg;
#	  if (CoordinateUtil::setSpectralConversion (errorMsg, cs,MFrequency::showType(freqFrame_p))) {
#	    fluxScale.setCoordinateInfo(cs);
#            coverimage.setCoordinateInfo(cs);
#	  }
#        }
#        if (sm_p->doFluxScale(thismodel)) {
#	  cover=&(sm_p->fluxScale(thismodel));
#	  answer = True;
#          fluxScale.copyData(sm_p->fluxScale(thismodel));
#	  Float cutoffval=minPB_p;
#	  if(ft_p->name()=="MosaicFT"){
#	    cutoffval=minPB_p*minPB_p;
#	    se_p->getCoverageImage(thismodel, coverimage);
#            cover=&(coverimage);
#	    //Do the sqrt
#	    coverimage.copyData(( LatticeExpr<Float> )(iif(coverimage > 0.0, sqrt(coverimage), 0.0)));
#            coverimage.table().unmarkForDelete();
#	    LatticeExpr<Bool> lemask(iif((*cover) < sqrt(cutoffval),
#				       False, True));
#	    ImageRegion outreg=coverimage.makeMask("mask0", False, True);
#	    LCRegion& outmask=outreg.asMask();
#	    outmask.copyData(lemask);
#	    coverimage.defineRegion("mask0", outreg,RegionHandler::Masks, True);
#	    coverimage.setDefaultMask("mask0");
#	  }
#	  LatticeExpr<Bool> lemask(iif((*cover) < minPB_p,
#				       False, True));
#	  ImageRegion outreg=fluxScale.makeMask("mask0", False, True);
#	  LCRegion& outmask=outreg.asMask();
#	  outmask.copyData(lemask);
#	  fluxScale.defineRegion("mask0", outreg,RegionHandler::Masks, True);
#	  fluxScale.setDefaultMask("mask0");


#        } else {
#	  answer = False;
#          os << LogIO::NORMAL // Loglevel INFO
#             << "No flux scale available (or required) for model " << thismodel
#             << LogIO::POST;
#          os << LogIO::NORMAL // Loglevel INFO
#             << "(This is only pertinent to mosaiced images)" << LogIO::POST;
#          os << LogIO::NORMAL // Loglevel INFO
#             << "Writing out image of constant 1.0" << LogIO::POST;
#          fluxScale.set(1.0);
#        }
#      }
#    }
#  }
#  return answer;
#}

################################################################################

def empty(args):
    print "empty:"
    print args

def casamfclean(args):
#    print "mfclean:"
#    print args

    ms = pyrap.tables.table(args.ms)
    uvw = ms.getcol("UVW")
    del ms

    B_max = numpy.max(numpy.sqrt(numpy.sum(numpy.square(uvw), 1)))
    B_max = min(float(args.B), B_max)
    print
    print "maximum baseline length:", B_max, "m"

    nu = 60e6
    n_ch = 1
    print "\n=> TODO: read reference frequency and channel count from the MS\n"

    n_cr = 4
    print "\n=> TODO: read polarization count from the MS\n"

    D = 70.0
    C = 299792458.0
    wl = C / nu
    fwhm = wl / D
    print "FWHM:", fwhm * 180.0 / numpy.pi, "deg"

    delta_lm = wl / (3.0 * B_max)
    n_px = 2 * int(fwhm / delta_lm)

    print "angular scale @ 3 pixel/beam: %.2f arcsec/pixel" % (3600.0 * delta_lm * 180.0 / numpy.pi)
    print "image size:", n_px, "x", n_px, "pixel"

    if n_px > 200:
        print "image too large!"
        return

    # Defaults from awimager.
    parms = {}
    parms["timewindow"] = 300
    parms["wmax"] = B_max
    parms["mueller.grid"] = numpy.ones((4, 4), dtype=bool)
    parms["mueller.degrid"] = numpy.ones((4, 4), dtype=bool)
    parms["verbose"] = 0                # 1, 2 for more output
    parms["maxsupport"] = 1024
    parms["oversample"] = 8
    parms["imagename"] = args.image
    parms["UseLIG"] = False             # linear interpolation
    parms["UseEJones"] = True
    #parms["ApplyElement"] = True
    parms["PBCut"] = 1e-2
    parms["StepApplyElement"] = 0       # if 0 don't apply element beam
    parms["PredictFT"] = False
    parms["PsfImage"] = ""
    parms["UseMasksDegrid"] = True
    parms["RowBlock"] = 0
    parms["doPSF"] = False
    parms["applyIonosphere"] = False
    parms["applyBeam"] = True
    parms["splitbeam"] = True
    parms["padding"] = args.padding
    parms["wplanes"] = args.w_planes

    parms["gain"] = 0.1
    parms["cyclefactor"] = 1.5
    parms["cyclespeedup"] = -1.0
    parms["threshold"] = 0.0
    parms["niter"] = 1000

    parms["delta"] = delta_lm
    parms["n_px"] = n_px
    parms["n_ch"] = n_ch
    parms["n_cr"] = n_cr

#    memento = lofar.casaimwrap.Memento()
#    status = lofar.casaimwrap.init(memento, args.ms, parms)
#    assert(status)

#    status = lofar.casaimwrap.init2(memento, args.ms)
#    assert(status)

#    refDelay = lofar.casaimwrap.readDelayReference(memento, 0)
#    csys = pyrap.images.coordinates.coordinatesystem(lofar.casaimwrap.makeCoordinateSystem(memento, refDelay[0], refDelay[1], n_px,
#        delta_lm))

#    csys = pyrap.images.coordinates.coordinatesystem(lofar.casaimwrap.makeCoordinateSystem(memento, n_px, delta_lm))

#    # Shape is reversed by pyrap as well?
#    im = pyrap.images.image(args.image, shape=(n_ch, n_cr, n_px, n_px),
#        coordsys=csys)


    # this needs to be worked out further.
#    converged = lofar.casaimwrap.clean(memento)
#    print "mfclean: converged:", converged
#    return converged

    parms["ms"] = args.ms
    lofar.casaimwrap.casa_clean(parms)


def mfclean(args):
#    print "mfclean:"
#    print args

    ms = pyrap.tables.table(args.ms)
    uvw = ms.getcol("UVW")
    del ms

    B_max = numpy.max(numpy.sqrt(numpy.sum(numpy.square(uvw), 1)))
    B_max = min(float(args.B), B_max)
    print
    print "maximum baseline length:", B_max, "m"

    nu = 60e6
    n_ch = 1
    print "\n=> TODO: read reference frequency and channel count from the MS\n"

    n_cr = 4
    print "\n=> TODO: read polarization count from the MS\n"

    D = 70.0
    C = 299792458.0
    wl = C / nu
    fwhm = wl / D
    print "FWHM:", fwhm * 180.0 / numpy.pi, "deg"

#    fov = 2.0 * fwhm

#    lm_max = numpy.sin(fov / 2.0)
#    r_max = numpy.sqrt(2) * lm_max
#    assert(r_max < 1.0)

#    delta_lm = wl / (3.0 * B_max)
#    n_px = 2 * int(lm_max / delta_lm)

    delta_lm = wl / (3.0 * B_max)
    n_px = 2 * int(fwhm / delta_lm)

    print "angular scale @ 3 pixel/beam: %.2f arcsec/pixel" % (3600.0 * delta_lm * 180.0 / numpy.pi)
    print "image size:", n_px, "x", n_px, "pixel"

    if n_px > 500:
        print "image too large!"
        return

    # Defaults from awimager.
    parms = {}
    parms["timewindow"] = 300
    parms["wmax"] = B_max
    parms["mueller.grid"] = numpy.ones((4, 4), dtype=bool)
    parms["mueller.degrid"] = numpy.ones((4, 4), dtype=bool)
    parms["verbose"] = 0                # 1, 2 for more output
    parms["maxsupport"] = 1024
    parms["oversample"] = 8
    parms["imagename"] = args.image
    parms["UseLIG"] = False             # linear interpolation
    parms["UseEJones"] = True
    #parms["ApplyElement"] = True
    parms["PBCut"] = 1e-2
    parms["StepApplyElement"] = 0       # if 0 don't apply element beam
    parms["PredictFT"] = False
    parms["PsfImage"] = ""
    parms["UseMasksDegrid"] = True
    parms["RowBlock"] = 0
    parms["doPSF"] = False
    parms["applyIonosphere"] = False
    parms["applyBeam"] = True
    parms["splitbeam"] = True
    parms["padding"] = args.padding
    parms["wplanes"] = args.w_planes

    parms["gain"] = 0.1
    parms["cyclefactor"] = 1.5
    parms["cyclespeedup"] = -1.0
    parms["threshold"] = 0.0
    parms["niter"] = 1000

    parms["delta"] = delta_lm
    parms["n_px"] = n_px
    parms["n_ch"] = n_ch
    parms["n_cr"] = n_cr

    memento = lofar.casaimwrap.Memento()
    status = lofar.casaimwrap.init(memento, args.ms, parms)
    assert(status)

#    # this needs to be worked out further.
#    converged = lofar.casaimwrap.clean(memento)
#    print "CASA mfclean: converged:", converged
#    return converged

    csys_rec = lofar.casaimwrap.makeCoordinateSystem(memento, n_px, delta_lm)
    csys = pyrap.images.coordinates.coordinatesystem(csys_rec)

################################################################################

    nmodels = 1

    # TODO: Double check code for nmodels > 1!!
    assert(nmodels == 1)

    doPolJoint = True

    psf = [None for i in range(nmodels)]
    gS = [None for i in range(nmodels)]
    ggS = [None for i in range(nmodels)]
    beam = [None for i in range(nmodels)]
    image = [numpy.zeros((n_ch, n_cr, n_px, n_px)) for i in range(nmodels)]
    delta = [numpy.zeros((n_ch, n_cr, n_px, n_px)) for i in range(nmodels)]
    residual = [numpy.zeros((n_ch, n_cr, n_px, n_px)) for i in range(nmodels)]

    resmax = [1e20 for i in range(nmodels)]
    resmin = [-1e20 for i in range(nmodels)]

    psfmin = None
    psfmax = None
    psfmaxouter = None
    psfpatch = None
    maxSidelobe = None

    # Make approximate PSF images for all models.
    result = lofar.casaimwrap.makeApproxPSF(memento)
    for i in range(nmodels):
        psf[i] = result["psf-%d" % i]
        fit = lofar.casaimwrap.fitGaussianPSF(csys_rec, psf[i])
        assert(fit["ok"])
        beam[i] = BeamParameters((fit["major"] * numpy.pi) / (3600.0 * 180.0), (fit["minor"] * numpy.pi) / (3600.0 * 180.0), (fit["angle"] * numpy.pi) / 180.0)
        print "PSF model:", i, "major axis:", abs(fit["major"]), "arcsec", "minor axis:", abs(fit["minor"]), "arcsec", "position angle:", fit["angle"], "deg"

    # TODO: do this for all psfs??
    (psfmin, psfmax, psfmaxouter, psfpatch, maxSidelobe) = validatePSF(csys, psf[0], beam[0])

    options = CleanOptions()
    options.setGain(parms["gain"])
    options.setNumberIterations(parms["niter"])
    options.setThreshold(parms["threshold"])
    options.setCycleFactor(parms["cyclefactor"])
    options.setCycleSpeedup(parms["cyclespeedup"])
    options.setCycleMaxPsfFraction(0.8)

    modified = True

    absmax = options.threshold()
    oldabsmax = 1e30
    cycleThreshold = 0.0

    iterations = [[] for i in range(nmodels)]
    maxIterations = 0
    oldMaxIterations = 0

#    Loop over major cycles
    cycle = 0
    stop = False
    diverging = False

    maxggS = 0.0
    lastCycleWriteModel = False

    while absmax >= options.threshold() and maxIterations < options.numberIterations() and not stop and not diverging and cycle < 2:
        print "*** Starting major cycle", cycle
        cycle += 1

#        Make the residual images. We do an incremental update
#        for cycles after the first one. If we have only one
#        model then we use convolutions to speed the processing

        print "Making residual images for all fields"

        if modified:
            result = lofar.casaimwrap.makeNewtonRaphsonStep(memento, False)
            print result.keys()
            for i in range(nmodels):
                gS[i] = result["gS-%d" % i]
                ggS[i] = result["ggS-%d" % i]
                residual[i] = numpy.where(ggS[i] > 0.0, -gS[i] / ggS[i], 0.0)
                print "model:", i, "gS:", numpy.max(gS[i]), numpy.min(gS[i]), "ggS:", numpy.max(ggS[i]), numpy.min(ggS[i])

#            pylab.figure()
#            pylab.title("gS, cycle %d" % cycle)
#            pylab.imshow(numpy.abs(gS[0][0, 0, :, :]))
#            pylab.colorbar()

#            pylab.figure()
#            pylab.title("ggS, cycle %d" % cycle)
#            pylab.imshow(numpy.abs(ggS[0][0, 0, :, :]))
#            pylab.colorbar()

            modified = False

        if cycle == 1:
            maxggS = 0.0
            for i in range(nmodels):
                maxggS = max(maxggS, numpy.max(ggS[i]))
            print "Maximum sensitivity:", 1.0 / numpy.sqrt(maxggS), "Jy/beam"

        (absmax, _min, _max) = maxField(ggS[0], residual[0])
#        print "NEED TO PUT RESIDUAL BACK SOMEHOW!"
#        lofar.casaimwrap.setResidual(memento, 0, residual[0])
        print "absmax:", absmax, "imin:", _min, "imax:", _max

        resmin[0] = _min
        resmax[0] = _max

        if cycle > 1:
            # check if its 5% above previous value
            if absmax < 1.000005 * oldabsmax:
                oldabsmax = absmax
            else:
                diverging = True
                print "Clean not converging!"

        for i in range(nmodels):
            print "Model:", i, "max, min (weighted) residuals =", resmax[i], ",", resmin[i]

        # Can we stop?
        if absmax < options.threshold():
            print "Reached stopping peak residual:", absmax
            stop = True
            if cycle > 1:
                lastCycleWriteModel = True
        else:
            # Calculate the threshold for this cycle. Add a safety factor
            #
            # fractionOfPsf controls how deep the cleaning should go.
            # There are two user-controls.
            # cycleFactor_p : scale factor for the PSF sidelobe level.
            #                        1 : clean down to the psf sidelobe level
            #                        <1 : go deeper
            #                        >1 : shallower : stop sooner.
            #                        Default : 1.5
            # cycleMaxPsfFraction_p : scale factor as a fraction of the PSF peak
            #                                    must be 0.0 < xx < 1.0 (obviously)
            #                                    Default : 0.8
            fractionOfPsf = min(options.cycleMaxPsfFraction(), options.cycleFactor() * maxSidelobe)
            if fractionOfPsf > 0.8:
                print "PSF fraction for threshold computation is too high:", fractionOfPsf, ".  Forcing to 0.8 to ensure that the threshold is smaller than the peak residual !"
                print "Current values of max-PSF-fraction, cycle-factor and max PSF sidelobe level result in a stopping threshold more than 80% of the peak residual. Forcing the maximum threshold to 80% of the peak."
                fractionOfPsf = 0.8   # painfully slow!

            print "The minor-cycle threshold is MAX[ 0.95 x", options.threshold(), ", peak residual x", fractionOfPsf, "]"

            cycleThreshold = max(0.95 * options.threshold(), fractionOfPsf * absmax)
            print "Maximum residual:", absmax, ", cleaning down to", cycleThreshold

            for modelID in range(nmodels):
                nx = image[modelID].shape[-1]
                ny = image[modelID].shape[-2]
                npol = image[modelID].shape[-3]
                nchan = image[modelID].shape[-4]

                npolcube = npol if doPolJoint else 1
                if cycle == 1:
                    # TODO: some useless code in MFCleanImageSkyModel at this point??
                    iterations[modelID] = [0 for i in range(nchan * npol)]

                # NB: zero delta image!!
                delta[modelID].fill(0.0)

                if max(abs(resmin[modelID]), abs(resmax[modelID])) > cycleThreshold:

                    for ch in range(nchan):

#                        if psfmax[modelID] <= 0.0:
                        if psfmax <= 0.0:
                            continue

                        if nchan > 1:
                            print "Processing channel %d/%d" % (ch, nchan)

                        wmask = numpy.zeros((ny, nx))
                        for pol in range(npol):
                            wmask[numpy.sqrt(ggS[modelID][ch, pol, :, :] / maxggS) > 0.01] = 1.0
#                        wmask = numpy.ones((ny, nx))

                        ccoptions = {}
                        ccoptions["gain"] = options.gain()
                        ccoptions["numberIterations"] = options.numberIterations()
                        ccoptions["cycleThreshold"] = cycleThreshold
                        ccoptions["psfpatch"] = psfpatch
                        ccoptions["cycleSpeedup"] = options.cycleSpeedup()

                        print ccoptions

                        ccresult = lofar.casaimwrap.clarkClean(psf[modelID][ch,0,:,:], residual[modelID][ch,:,:,:], wmask, iterations[modelID][ch * npolcube], ccoptions)
                        iterations[modelID][ch * npolcube] = ccresult["numberIterations"]
                        delta[modelID][ch,:,:,:] = ccresult["delta"]

#                        pylab.figure()
#                        pylab.title("delta, cycle %d" % (cycle - 1))
#                        pylab.imshow(ccresult["delta"][0,:,:])
#                        pylab.colorbar()

                        maxIterations = max(maxIterations, iterations[modelID][ch * npolcube])
                        modified = True

                        print "Finished Clark clean inner cycle"

                        # TODO: This is rather dubious because it only depends on the value of maxIterations of the last channel???
                        stop = (maxIterations == 0)

                        print "Clean used %d iterations to approach a threshold of %f" % (iterations[modelID][ch * npolcube], cycleThreshold)

                else:
                        print "No need to clean model %d: peak residual below threshold" % (modelID)

            if maxIterations != oldMaxIterations:
                oldMaxIterations = maxIterations

                # NB. UPDATE OF MODEL IMAGE HERE!!!!!
                for modelID in range(nmodels):
#                    pylab.figure()
#                    pylab.title("delta, cycle %d" % (cycle - 1))
#                    pylab.imshow(delta[modelID][0,0,:,:])
#                    pylab.colorbar()

                    image[modelID] += delta[modelID]

#                    pylab.figure()
#                    pylab.title("image+delta, cycle %d" % (cycle - 1))
#                    pylab.imshow(numpy.abs(image[modelID][0,0,:,:]))
#                    pylab.colorbar()

                    lofar.casaimwrap.setImage(memento, modelID, image[modelID])
                    print "%f Jy <- cleaned in this cycle for model %d" % (numpy.sum(delta[modelID]), modelID)

            else:
                print "No more iterations left in this major cycle - stopping now"
                stop = True
                converged = True

#            print "STOPPING FOR NOW (SHOULD BE REMOVED)!"
#            stop = True
    # WHILE LOOP ENDS HERE

    ################################################################################


    if modified or lastCycleWriteModel:
        print "Finalizing residual images for all fields"

        result = lofar.casaimwrap.makeNewtonRaphsonStep(memento, False) #, True) //committing model to MS
        for i in range(nmodels):
            gS[i] = result["gS-%d" % i]
            ggS[i] = result["ggS-%d" % i]
            residual[i] = numpy.where(ggS[i] > 0.0, -gS[i] / ggS[i], 0.0)
        modified = False

        print "Final max ggS:", numpy.max(ggS[0])
        (finalabsmax, _min, _max) = maxField(ggS[0], residual[0])
#        lofar.casaimwrap.setResidual(memento, 0, residual[0])
        resmin[0] = _min
        resmax[0] = _max

#        pylab.figure()
#        pylab.title("final gS")
#        pylab.imshow(gS[0][0,0,:,:])
#        pylab.colorbar()

#        pylab.figure()
#        pylab.title("final ggS")
#        pylab.imshow(ggS[0][0,0,:,:])
#        pylab.colorbar()

#        pylab.figure()
#        pylab.title("final image")
#        pylab.imshow(image[0][0,0,:,:])
#        pylab.colorbar()

#        pylab.figure()
#        pylab.title("final residual")

        show_image("final residual", residual[0][0,:,:,:])
#        fig, axes = pylab.subplots(nrows=2, ncols=2) #, sharex=True, sharey=True, squeeze=True)
#        vmin = numpy.min(residual[0])
#        vmax = numpy.max(residual[0])
#        for k, ax in zip(range(4), axes.flat):
#            __im = ax.imshow(residual[0][0,k,:,:], origin="lower", interpolation="nearest", cmap="bone", aspect="auto", vmin=vmin, vmax=vmax)
#            __im.axes.get_xaxis().set_visible(False)
#            __im.axes.get_yaxis().set_visible(False)

#        fig.subplots_adjust(right=0.70, wspace=0.0, hspace=0.0)
#        cax = fig.add_axes([0.75, 0.1, 0.03, 0.8])
#        cbr = fig.colorbar(__im, cax=cax)
#        cbr.set_clim(vmin, vmax)

        print "Final maximum residual:", finalabsmax
        converged = (finalabsmax < 1.05 * options.threshold())

        for modelID in range(nmodels):
            print "Model", modelID, ": max, min residuals:", resmax[modelID], ",", resmin[modelID], "clean flux", numpy.sum(image[modelID])
    else:
        print "Residual images for all fields are up-to-date"

    if stop:
        converged = True

    restored = restore(csys_rec, image[0], residual[0], beam[0])

    show_image("final restored image", restored[0,:,:,:])
#    pylab.figure()
#    pylab.title("final restored image")
#    pylab.imshow(restored[0,0,:,:], origin="lower", interpolation="nearest", cmap="bone")
#    pylab.colorbar()
    pylab.show()

    im_restored = pyrap.images.image(args.image + ".restored", shape=(n_ch, n_cr, n_px, n_px), coordsys=csys)
    im_restored.putdata(restored)

    return converged


def main():
    parser = argparse.ArgumentParser(description="Python imager")
    subparsers = parser.add_subparsers(help="operation to perform")

    subparser = subparsers.add_parser("empty", help="empty help")
    subparser.set_defaults(func=empty)

    subparser = subparsers.add_parser("mfclean", help="clean help")
    subparser.add_argument("-B", type=float, help="maximum projected baseline length to use (m)")
    subparser.add_argument("-w", "--w-planes", dest="w_planes", type=int, default=128, help="number of W-planes to use")
    subparser.add_argument("-p", "--padding", type=float, default=1.0, help="maximum projected baseline length to use (m)")
    subparser.add_argument("-g", choices=["awz", "aw", "w"], help="gridder to use")
    subparser.add_argument("-G", dest="gridder_options", action="append", metavar="OPTION", help="gridder specific option")

    subparser.add_argument("ms", help="input measurement set")
    subparser.add_argument("image", help="output image")
    subparser.set_defaults(func=mfclean)

    subparser = subparsers.add_parser("casamfclean", help="casamfclean help")
    subparser.add_argument("-B", type=float, help="maximum projected baseline length to use (m)")
    subparser.add_argument("-w", "--w-planes", dest="w_planes", type=int, default=128, help="number of W-planes to use")
    subparser.add_argument("-p", "--padding", type=float, default=1.0, help="maximum projected baseline length to use (m)")

    subparser.add_argument("ms", help="input measurement set")
    subparser.add_argument("image", help="output image")
    subparser.set_defaults(func=casamfclean)

#    subsubparsers = subparser.add_subparsers(help="clean algorithm to use")
#    subsubparser = subsubparsers.add_parser("multi-field", help="multi field")
#    subsubparser.add_argument("ms", help="input measurement set")
#    subsubparser.set_defaults(func=mfclean)

#    subsubparser = subsubparsers.add_parser("cotton-schwab", help="cotton-schwab")
#    subsubparser.add_argument("ms", help="input measurement set")
#    subsubparser.set_defaults(func=csclean)

    args = parser.parse_args()
#    print args
    args.func(args)

if __name__ == "__main__":
    main()
