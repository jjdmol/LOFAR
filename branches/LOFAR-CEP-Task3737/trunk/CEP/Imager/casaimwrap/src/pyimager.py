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
    print "title:", title, "max:", numpy.max(data), "min:", numpy.min(data), "mean:", numpy.mean(data)
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

# Return maximum abs outer sidelobe, more than nCenter pixels from the center
# Re-implementation of MFCleanImageSkyModel::maxOuter().
def max_outer(image, distance):
    assert(len(image.shape) == 2)
    assert(numpy.product(image.shape) > 0)

    peak = numpy.unravel_index(numpy.argmax(image), image.shape)
#    print "peak location:", peak

    top = peak[0] - distance
    bottom = peak[0] + distance + 1
    left = peak[1] - distance
    right = peak[1] + distance + 1

    sidelobe = 0.0

    # Top.
    if top > 0:
        sidelobe = max(sidelobe, numpy.max(numpy.abs(image[:top, :])))

    # Bottom.
    if bottom < image.shape[-2]:
        sidelobe = max(sidelobe, numpy.max(numpy.abs(image[bottom:, :])))

    # Left.
    if left > 0:
        sidelobe = max(sidelobe, numpy.max(numpy.abs(image[:, :left])))

    # Right.
    if right < image.shape[-1]:
        sidelobe = max(sidelobe, numpy.max(numpy.abs(image[:, right:])))

    return sidelobe

# Re-implementation of a part of MFCleanImageSkyModel::solve().
def validate_psf(csys, psf, beam):
    assert(len(psf) == len(beam))

    psf_patch_size = 51
    max_sidelobe = 0.0

    min_psf = [0.0 for i in range(len(psf))]
    max_psf = [0.0 for i in range(len(psf))]
    max_psf_outer = [0.0 for i in range(len(psf))]

    for model in range(len(psf)):
        threshold = 0.8 * numpy.max(psf[model])

        # Find channel for which the (signed) maximum of the first polarization
        # is larger than threshold.
        ch = 0
        n_ch = psf[model].shape[0]
        while ch < n_ch:
            max_psf[model] = numpy.max(psf[model][ch, 0, :, :])
            if max_psf[model] >= threshold:
                break
            ch += 1

        # Compute the minimum for the same channel.
        min_psf[model] = numpy.min(psf[model][ch, 0, :, :])

        # Comment from CASA source code:
        #    4 pixels:  pretty arbitrary, but only look for sidelobes
        #    outside the inner (2n+1) * (2n+1) square
        distance = 4

        # Comment from CASA source code:
        #    locate peak size
        # TODO: Should this not be min(abs(...))??
        # TODO: Should each model image have its own coordinate system?
        increment = numpy.abs(numpy.min(csys.get_increment()[2]))
        assert(csys.get_unit()[2] == ["rad", "rad"])
        if increment > 0.0:
            distance = max(distance, int(numpy.ceil(max(beam[model].majorAxis() / increment, beam[model].minorAxis() / increment))))

        # TODO: psf_patch_size will always be set based on the distance computed for
        # the last (solvable) model. This seems rather arbitrary?
        psf_patch_size = 3 * distance + 1

        max_psf_outer[model] = max_outer(psf[model][ch, 0, :, :], distance)
#        print "model:", model, "PSF max, min, max_outer:", max_psf[model], min_psf[model], max_psf_outer[model]

        # TODO: What does this do exactly?
        max_sidelobe = max(max_sidelobe, abs(min_psf[model]))
        max_sidelobe = max(max_sidelobe, max_psf_outer[model])

    return (min_psf, max_psf, max_psf_outer, psf_patch_size, max_sidelobe)

# Return the maximum absolute value per field.
# Re-implementation of MFCleanImageSkyModel::maxField().
def max_field(ggS, residual):
    assert(len(ggS) == len(residual))
    n_model = len(ggS)

    # Compute the (signed) maximum of ggS over all models.
    max_ggS = 0.0
    for model in range(n_model):
        max_ggS = max(max_ggS, numpy.max(ggS[model]))

    absmax = 0.0
    min_residual = [1e20 for i in range(n_model)]
    max_residual = [-1e20 for i in range(n_model)]
    for model in range(n_model):
        assert(ggS[model].shape == residual[model].shape)

        # TODO: Why is this done per channel?
        n_ch = residual[model].shape[0]
        for ch in range(n_ch):
            # Re-weight residual
            residual[model][ch, :, :, :] *= numpy.sqrt(ggS[model][ch, :, :, :] / max_ggS)

            fmax = numpy.max(residual[model][ch, :, :, :])
            fmin = numpy.min(residual[model][ch, :, :, :])

            # TODO: What is the logic behind this?
            if fmax < 0.99 * -1e20:
                fmax = 0.0
            if fmin > 0.99 * 1e20:
                fmin = 0.0

            absmax = max(absmax, max(abs(fmax), abs(fmin)))
            min_residual[model] = min(min_residual[model], fmin)
            max_residual[model] = max(max_residual[model], fmax)

    return (absmax, min_residual, max_residual)

def restore_image(csys, image, residual, beam):
    if beam.majorAxis() > 0.0 and beam.minorAxis() > 0.0:
        major = beam.majorAxis() * 180.0 * 3600.0 / numpy.pi
        minor = beam.minorAxis() * 180.0 * 3600.0 / numpy.pi
        pa = beam.positionAngle() * 180.0 / numpy.pi
        restored = lofar.casaimwrap.convolveWithBeam(csys, image, major, minor, pa)
    else:
        restored = numpy.copy(image)

    restored += residual
    return restored

def empty(args):
    ms = pyrap.tables.table(args.ms)
    uvw = ms.getcol("UVW")
    del ms

    B_max = numpy.max(numpy.sqrt(numpy.sum(numpy.square(uvw), 1)))
    B_max = min(float(args.B), B_max)
    print "maximum baseline length: %.2f m" % B_max

    nu = 60e6
    n_ch = 1
    print "==> TODO: read reference frequency and channel count from the MS"

    n_cr = 4
    print "==> TODO: read polarization count from the MS"

    D = 70.0
    print "==> TODO: read maximal (effective) station diameter from the MS"

    C = 299792458.0
    wl = C / nu
    fwhm = wl / D
    print "full width half maximum: %.2f deg" % (fwhm * 180.0 / numpy.pi)

    delta_lm = wl / (3.0 * B_max)
    n_px = 2 * int(fwhm / delta_lm)

    print "angular scale @ 3 pixel/beam: %.2f arcsec/pixel" % (3600.0 * delta_lm * 180.0 / numpy.pi)
    print "image size:", n_px, "x", n_px, "pixel"

    image_shape = (n_ch, n_cr, n_px, n_px)
    csys = pyrap.images.coordinates.coordinatesystem(lofar.casaimwrap.makeCoordinateSystem(args.ms, n_px, delta_lm))
    im_empty = pyrap.images.image(args.image, shape=image_shape, coordsys=csys)
    return True

def mfclean(args):
    ms = pyrap.tables.table(args.ms)
    uvw = ms.getcol("UVW")
    del ms

    B_max = numpy.max(numpy.sqrt(numpy.sum(numpy.square(uvw), 1)))
    B_max = min(float(args.B), B_max)
    print "maximum baseline length: %.2f m" % B_max

    nu = 60e6
    n_ch = 1
    print "==> TODO: read reference frequency and channel count from the MS"

    n_cr = 4
    print "==> TODO: read polarization count from the MS"

    D = 70.0
    print "==> TODO: read maximal (effective) station diameter from the MS"

    C = 299792458.0
    wl = C / nu
    fwhm = wl / D
    print "full width half maximum: %.2f deg" % (fwhm * 180.0 / numpy.pi)

    # TODO: Cyril mentioned above FOV estimation is too conservative. Need to
    # check this and find a better estimate if necessary. For now, will just
    # multiply estimated FOV by 2.0.
#    fwhm *= 2.0

#    fov = 2.0 * fwhm

#    lm_max = numpy.sin(fov / 2.0)
#    r_max = numpy.sqrt(2) * lm_max
#    assert(r_max < 1.0)

#    delta_lm = wl / (3.0 * B_max)
#    n_px = 2 * int(lm_max / delta_lm)

    delta_lm = wl / (3.0 * B_max)
    n_px = 2 * int(fwhm / delta_lm)
#    n_px = int(fwhm / delta_lm) / 2

    print "angular scale @ 3 pixel/beam: %.2f arcsec/pixel" % (3600.0 * delta_lm * 180.0 / numpy.pi)
    print "image size:", n_px, "x", n_px, "pixel"

    if n_px > 500:
        print "image too large!"
        return

    image_shape = (n_ch, n_cr, n_px, n_px)

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

    csys = pyrap.images.coordinates.coordinatesystem(lofar.casaimwrap.makeCoordinateSystem(args.ms, n_px, delta_lm))

    memento = lofar.casaimwrap.Memento()
    lofar.casaimwrap.init(memento, args.ms, csys.dict(), image_shape, parms)

    ms = pyrap.tables.table(args.ms)
    ms = ms.query("ANTENNA1 != ANTENNA2")
    first = True
    for chunk in ms.iter(["ARRAY_ID", "FIELD_ID", "DATA_DESC_ID", "TIME"]):
        x = {}
        x["antenna1"] = chunk.getcol("ANTENNA1")
        x["antenna2"] = chunk.getcol("ANTENNA2")
        x["uvw"] = chunk.getcol("UVW")
        x["time"] = chunk.getcol("TIME")
        x["centroid"] = chunk.getcol("TIME_CENTROID")
        x["flag_row"] = chunk.getcol("FLAG_ROW")
        x["weight"] = chunk.getcol("WEIGHT")
        x["flag"] = chunk.getcol("FLAG")
        x["data"] = chunk.getcol("DATA")
#        x["data"] = chunk.getcol("CORRECTED_DATA")

        print "data:", numpy.min(x["data"]), numpy.max(x["data"])
        if first:
            lofar.casaimwrap.begin_grid(memento, False, x)
            first = False
        else:
            lofar.casaimwrap.grid(memento, x)

#    x = {}
#    x["antenna1"] = ms.getcol("ANTENNA1")
#    x["antenna2"] = ms.getcol("ANTENNA2")
#    x["uvw"] = ms.getcol("UVW")
#    x["time"] = ms.getcol("TIME")
#    x["centroid"] = ms.getcol("TIME_CENTROID")
#    x["flag_row"] = ms.getcol("FLAG_ROW")
#    x["weight"] = ms.getcol("WEIGHT")
#    x["flag"] = ms.getcol("FLAG")
#    x["data"] = ms.getcol("CORRECTED_DATA")

#    lofar.casaimwrap.begin_grid(memento, False, x)
    result = lofar.casaimwrap.end_grid(memento, True)
    gridImage = result["image"]
    gridWeight = result["weight"]

    print gridImage.shape
    print gridWeight.shape
    show_image("image", gridImage[0,:,:,:]);
    show_image("weight", gridWeight[0,:,:,:]);
    show_image("image normalized", gridImage[0,:,:,:] / gridWeight[0,:,:,:]);

#    y = {}
#    y["antenna1"] = ms.getcol("ANTENNA1")
#    y["antenna2"] = ms.getcol("ANTENNA2")
#    y["uvw"] = ms.getcol("UVW")
#    y["time"] = ms.getcol("TIME")
#    y["centroid"] = ms.getcol("TIME_CENTROID")
#    y["flag_row"] = ms.getcol("FLAG_ROW")
#    y["weight"] = ms.getcol("WEIGHT")
#    y["flag"] = ms.getcol("FLAG")

#    result = lofar.casaimwrap.begin_degrid(memento, gridImage[0,:,:,:] / gridWeight[0,:,:,:], y)
#    degrid_data = result["data"]
#    print degrid_data.shape

#    tmp_data = ms.getcol("CORRECTED_DATA")
#    delta = numpy.abs(degrid_data[:1000, 0, 0] / tmp_data[:1000, 0, 0])
#    print "delta: max:", numpy.max(delta), "min:", numpy.min(delta), "mean:", numpy.mean(delta)
    pylab.show()
    return True

    n_model = 1
    # TODO: Double check code for n_model > 1!
    assert(n_model == 1)

    doPolJoint = True
    # TODO: Double check code for doPolJoint = False!
    assert(doPolJoint)

    psf = [None for i in range(n_model)]
    gS = [None for i in range(n_model)]
    ggS = [None for i in range(n_model)]
    beam = [None for i in range(n_model)]
    image = [numpy.zeros(image_shape) for i in range(n_model)]
    delta = [numpy.zeros(image_shape) for i in range(n_model)]
    residual = [numpy.zeros(image_shape) for i in range(n_model)]

    # Make approximate PSF images for all models.
    print "making approximate PSF images for all fields..."
    result = lofar.casaimwrap.makeApproxPSF(memento)
    for model in range(n_model):
        psf[model] = result["psf-%d" % model]
        fit = lofar.casaimwrap.fitGaussianPSF(csys.dict(), psf[model])
        assert(fit["ok"])
        beam[model] = BeamParameters((fit["major"] * numpy.pi) / (3600.0 * 180.0), (fit["minor"] * numpy.pi) / (3600.0 * 180.0), (fit["angle"] * numpy.pi) / 180.0)
        print "PSF: model:", i, "major axis:", abs(fit["major"]), "arcsec", "minor axis:", abs(fit["minor"]), "arcsec", "position angle:", fit["angle"], "deg"

    show_image("approximate PSF", psf[0][0,:,:,:])

    (min_psf, max_psf, max_psf_outer, psf_patch_size, max_sidelobe) = validate_psf(csys, psf, beam)

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

    iterations = [[] for i in range(n_model)]
    maxIterations = 0
    oldMaxIterations = 0

#    Loop over major cycles
    cycle = 0
    stop = False
    diverging = False

    maxggS = 0.0
    lastCycleWriteModel = False

    while absmax >= options.threshold() and maxIterations < options.numberIterations() and not stop and not diverging:
        print "==> starting major cycle:", cycle
        cycle += 1

#        Make the residual images. We do an incremental update
#        for cycles after the first one. If we have only one
#        model then we use convolutions to speed the processing

        print "making residual images for all fields..."

        if modified:
            result = lofar.casaimwrap.makeNewtonRaphsonStep(memento, False)
            for i in range(n_model):
                gS[i] = result["gS-%d" % i]
                ggS[i] = result["ggS-%d" % i]
                residual[i] = numpy.where(ggS[i] > 0.0, -gS[i] / ggS[i], 0.0)
#                print "model:", i, "gS:", numpy.max(gS[i]), numpy.min(gS[i]), "ggS:", numpy.max(ggS[i]), numpy.min(ggS[i])

            modified = False

        if cycle == 1:
            maxggS = 0.0
            for i in range(n_model):
                maxggS = max(maxggS, numpy.max(ggS[i]))
            print "maximum sensitivity:", 1.0 / numpy.sqrt(maxggS), "Jy/beam"

        (absmax, resmin, resmax) = max_field(ggS, residual)
#        print "absmax:", absmax, "resmin:", resmin, "resmax:", resmax

        if cycle > 1:
            # check if its 5% above previous value
            if absmax < 1.000005 * oldabsmax:
                oldabsmax = absmax
            else:
                diverging = True
                print "clean not converging!"

        for i in range(n_model):
            print "model:", i, "max, min (weighted) residuals =", resmax[i], ",", resmin[i]

        # Can we stop?
        if absmax < options.threshold():
            print "reached stopping peak residual:", absmax
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
            fractionOfPsf = min(options.cycleMaxPsfFraction(), options.cycleFactor() * max_sidelobe)
            if fractionOfPsf > 0.8:
                print "PSF fraction for threshold computation is too high:", fractionOfPsf, ".  Forcing to 0.8 to ensure that the threshold is smaller than the peak residual !"
                print "Current values of max-PSF-fraction, cycle-factor and max PSF sidelobe level result in a stopping threshold more than 80% of the peak residual. Forcing the maximum threshold to 80% of the peak."
                fractionOfPsf = 0.8   # painfully slow!

            cycleThreshold = max(0.95 * options.threshold(), fractionOfPsf * absmax)
            print "minor-cycle threshold MAX[ 0.95 x", options.threshold(), ", peak residual x", fractionOfPsf, "]:", cycleThreshold
            print "maximum residual:", absmax, ", cleaning down to:", cycleThreshold

            for model in range(n_model):
                nx = image[model].shape[-1]
                ny = image[model].shape[-2]
                npol = image[model].shape[-3]
                nchan = image[model].shape[-4]

                npolcube = npol if doPolJoint else 1
                if cycle == 1:
                    # TODO: some useless code in MFCleanImageSkyModel at this point??
                    iterations[model] = [0 for i in range(nchan * npol)]

                # NB: zero delta image!!
                delta[model].fill(0.0)

                if max(abs(resmin[model]), abs(resmax[model])) > cycleThreshold:
                    for ch in range(nchan):
                        if max_psf[model] <= 0.0:
                            continue

                        if nchan > 1:
                            print "processing channel %d/%d" % (ch, nchan)

                        # TODO: maxggS is only updated during cycle 0. Is this
                        # correct?
                        wmask = numpy.zeros((ny, nx))
                        for pol in range(npol):
                            wmask[numpy.sqrt(ggS[model][ch, pol, :, :] / maxggS) > 0.01] = 1.0

                        ccoptions = {}
                        ccoptions["gain"] = options.gain()
                        ccoptions["numberIterations"] = options.numberIterations()
                        ccoptions["cycleThreshold"] = cycleThreshold
                        ccoptions["psfpatch"] = psf_patch_size
                        ccoptions["cycleSpeedup"] = options.cycleSpeedup()

                        ccresult = lofar.casaimwrap.clarkClean(psf[model][ch,0,:,:], residual[model][ch,:,:,:], wmask, iterations[model][ch * npolcube], ccoptions)
                        iterations[model][ch * npolcube] = ccresult["numberIterations"]
                        delta[model][ch,:,:,:] = ccresult["delta"]

                        maxIterations = max(maxIterations, iterations[model][ch * npolcube])
                        modified = True

                        print "finished Clark clean inner cycle"

                        # TODO: This is rather dubious because it only depends on the value of maxIterations of the last channel???
                        stop = (maxIterations == 0)

                        print "clean used %d iterations to approach a threshold of %f" % (iterations[model][ch * npolcube], cycleThreshold)

                else:
                        print "no need to clean model %d: peak residual below threshold" % (model)

            if maxIterations != oldMaxIterations:
                oldMaxIterations = maxIterations

                for model in range(n_model):
                    image[model] += delta[model]

                    # Update image in the MFCleanImageSkyModel class, such that
                    # it gets picked up by LofarCubeSkyEquation.
                    lofar.casaimwrap.setImage(memento, model, image[model])
                    print "%f Jy <- cleaned in this cycle for model %d" % (numpy.sum(delta[model]), model)

            else:
                print "no more iterations left in this major cycle - stopping now"
                stop = True
                converged = True


    if modified or lastCycleWriteModel:
        print "finalizing residual images for all fields..."

        result = lofar.casaimwrap.makeNewtonRaphsonStep(memento, False) #, True) //committing model to MS
        for i in range(n_model):
            gS[i] = result["gS-%d" % i]
            ggS[i] = result["ggS-%d" % i]
            residual[i] = numpy.where(ggS[i] > 0.0, -gS[i] / ggS[i], 0.0)
        modified = False

#        print "Final max ggS:", numpy.max(ggS[0])
        (finalabsmax, resmin, resmax) = max_field(ggS, residual)

        show_image("final residual", residual[0][0,:,:,:])

        print "final maximum residual:", finalabsmax
        converged = (finalabsmax < 1.05 * options.threshold())

        for model in range(n_model):
            print "model", model, ": max, min residuals:", resmax[model], ",", resmin[model], "clean flux", numpy.sum(image[model])
    else:
        print "residual images for all fields are up-to-date..."

    if stop:
        converged = True

    restored = restore_image(csys.dict(), image[0], residual[0], beam[0])

    show_image("final restored image", restored[0,:,:,:])
    pylab.show()

    im_restored = pyrap.images.image(args.image + ".restored", shape=image_shape, coordsys=csys)
    im_restored.putdata(restored)

    return converged

def main():
    parser = argparse.ArgumentParser(description="Python imager")
    subparsers = parser.add_subparsers(help="operation to perform")

    subparser = subparsers.add_parser("empty", help="create an empty image")
    subparser.add_argument("-B", type=float, help="maximum projected baseline length (m)")

    subparser.add_argument("ms", help="input measurement set")
    subparser.add_argument("image", help="output image")
    subparser.set_defaults(func=empty)

    subparser = subparsers.add_parser("mfclean", help="multi-field Clark clean")
    subparser.add_argument("-B", type=float, help="maximum projected baseline length (m)")
    subparser.add_argument("-w", "--w-planes", dest="w_planes", type=int, default=128, help="number of W-planes")
    subparser.add_argument("-p", "--padding", type=float, default=1.0, help="image plane padding factor (>= 1.0)")
#    subparser.add_argument("-g", choices=["awz", "aw", "w"], help="gridder to use")
#    subparser.add_argument("-G", dest="gridder_options", action="append", metavar="OPTION", help="gridder specific option")

    subparser.add_argument("ms", help="input measurement set")
    subparser.add_argument("image", help="output image")
    subparser.set_defaults(func=mfclean)

#    subparser = subparsers.add_parser("casamfclean", help="casamfclean help")
#    subparser.add_argument("-B", type=float, help="maximum projected baseline length to use (m)")
#    subparser.add_argument("-w", "--w-planes", dest="w_planes", type=int, default=128, help="number of W-planes to use")
#    subparser.add_argument("-p", "--padding", type=float, default=1.0, help="maximum projected baseline length to use (m)")

#    subparser.add_argument("ms", help="input measurement set")
#    subparser.add_argument("image", help="output image")
#    subparser.set_defaults(func=casamfclean)

    args = parser.parse_args()
    args.func(args)

if __name__ == "__main__":
    main()
