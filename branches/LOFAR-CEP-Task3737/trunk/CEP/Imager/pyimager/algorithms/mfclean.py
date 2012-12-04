"""This module is a straight-forward Python port of the multi field Clark clean
algorithm as implemented in CASA (see MFCleanImageSkyModel.cc). In constrast to
the CASA implementation, masks are not supported, nor is the option to use
Hogbohm instead of Clark clean in the minor cycle.
"""

import pylab
import numpy
import pyrap.images

import lofar.casaimwrap
import lofar.pyimager.processors as processors
import util

class BeamParameters:
    def __init__(self, major_axis = 1.0, minor_axis = 1.0, \
        position_angle = 0.0):

        self._major = abs(major_axis)
        self._minor = abs(minor_axis)
        self._angle = position_angle

    def _set_major_axis(self, major):
        self._major = abs(major)

    def _set_minor_axis(self, minor):
        self._minor = abs(minor)

    def _set_position_angle(self, angle):
        self._angle = angle

    def _major_axis(self):
        return self._major

    def _minor_axis(self):
        return self._minor

    def _position_angle(self):
        return self._angle

    major_axis = property(_major_axis, _set_major_axis, \
        doc="Major axis size (rad).")

    minor_axis = property(_minor_axis, _set_minor_axis, \
        doc="Minor axis size (rad).")

    position_angle = property(_position_angle, _set_position_angle, \
        doc="Position angle (rad).")

def max_outer(image, distance):
    """Return maximum absolute outer sidelobe, more than distance pixels from
    the center.

    Re-implementation of MFCleanImageSkyModel::maxOuter().
    """
    assert(len(image.shape) == 2)
    assert(numpy.product(image.shape) > 0)

    peak = numpy.unravel_index(numpy.argmax(image), image.shape)

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

def validate_psf(csys, psf, beam):
    """Re-implementation of a section of MFCleanImageSkyModel::solve()."""
    assert(len(psf) == len(beam))

    psf_patch_size = 51
    max_sidelobe = 0.0

    min_psf = [0.0 for i in range(len(psf))]
    max_psf = [0.0 for i in range(len(psf))]
    max_psf_outer = [0.0 for i in range(len(psf))]

    for model in range(len(psf)):
        threshold = 0.8 * numpy.max(psf[model])

        # Find channel for which the (signed) maximum of the first correlation
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
        #
        distance = 4

        # Comment from CASA source code:
        #    locate peak size
        #
        # TODO: Should this not be min(abs(...))??
        # TODO: Should each model image have its own coordinate system?
        increment = numpy.abs(numpy.min(csys.get_increment()[2]))
        assert(csys.get_unit()[2] == ["rad", "rad"])
        if increment > 0.0:
            distance = max(distance, \
                int(numpy.ceil(max(beam[model].major_axis / increment, \
                beam[model].minor_axis / increment))))

        # TODO: psf_patch_size will always be set based on the distance computed
        # for the last (solvable) model. This seems rather arbitrary?
        psf_patch_size = 3 * distance + 1

        max_psf_outer[model] = max_outer(psf[model][ch, 0, :, :], distance)

        # TODO: What does this do exactly?
        max_sidelobe = max(max_sidelobe, abs(min_psf[model]))
        max_sidelobe = max(max_sidelobe, max_psf_outer[model])

    return (min_psf, max_psf, max_psf_outer, psf_patch_size, max_sidelobe)

def max_field(weight, residual):
    """Re-normalize the residual and return the (signed) minimum and maximum
    residual, as well as the (absolute) maximum residual.

    Re-implementation of MFCleanImageSkyModel::maxField().
    """
    assert(len(weight) == len(residual))
    n_model = len(weight)

    # Compute the (signed) maximum of weight over all models.
    max_weight = 0.0
    for model in range(n_model):
        max_weight = max(max_weight, numpy.max(weight[model]))

    absmax = 0.0
    min_residual = [1e20 for i in range(n_model)]
    max_residual = [-1e20 for i in range(n_model)]
    for model in range(n_model):
        n_ch = residual[model].shape[0]
        for ch in range(n_ch):
            # TODO: Why is the residual re-weighted here? In practice for LOFAR
            # all weights seem to be equal, which causes the residual to be
            # re-weighted by a factor 1.0. Ensure that this is the case, such
            # that it does not go unnoticed when the re-weighting would have had
            # an effect.
            #
#            residual[model][ch, :, :, :] *= \
#                numpy.sqrt(weight[model][ch, :, :, :] / max_weight)
            assert(numpy.all(weight[model][ch, :] == max_weight))

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
    """Convolve the input clean component image with a Gaussian restoring beam
    and add the residual."""
    if beam.major_axis > 0.0 and beam.minor_axis > 0.0:
        major = beam.major_axis * 180.0 * 3600.0 / numpy.pi
        minor = beam.minor_axis * 180.0 * 3600.0 / numpy.pi
        pa = beam.position_angle * 180.0 / numpy.pi
        restored = lofar.casaimwrap.convolveWithBeam(csys, image, major, \
            minor, pa)
    else:
        restored = numpy.copy(image)

    restored += residual
    return restored

def show_image(title, data):
    print "%s:" % title, "min:", numpy.min(data), "max:", numpy.max(data), "median:", numpy.median(data)
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

def mfclean(options):
    clark_options = {}
    clark_options["gain"] = options.gain
    clark_options["iterations"] = options.iterations
    clark_options["cycle_speedup"] = options.cycle_speedup

    max_baseline = options.max_baseline if options.max_baseline > 0.0 else \
        10000.0
    processor_options = {}
    processor_options["wmax"] = max_baseline
    processor_options["padding"] = 1.0
    processor_options["image"] = options.image

    processor = processors.create_data_processor(options.processor, \
        options.ms, processor_options)

    channel_freq = processor.channel_frequency()
    channel_width = processor.channel_width()

    max_freq = numpy.max(channel_freq)
    image_size = 2.0 * util.full_width_half_max(70.0, max_freq)

    # TODO: Cyril mentioned above image size estimation is too conservative.
    # Need to check this and find a better estimate if necessary. For now, will
    # just multiply estimated FOV by 2.0.
    image_size *= 2.0

    (n_px, delta_px) = util.image_configuration(image_size, max_freq, \
        max_baseline)

    util.notice("image configuration:")
    util.notice("    size: %d x %d pixel" % (n_px, n_px))
    util.notice("    angular size: %.2f deg" \
        % (image_size * 180.0 / numpy.pi))
    util.notice("    angular resolution @ 3 pixel/beam: %.2f arcsec/pixel" \
        % (3600.0 * delta_px * 180.0 / numpy.pi))

    if n_px > 750:
        util.error("image too large!")
        return

    # TODO: Need to implement support for multiple channel images. Currently,
    # all data channels are combined into a single MFS image per correlation.
    image_shape = (1, 4, n_px, n_px)
    image_coordinates = pyrap.images.coordinates.coordinatesystem( \
        lofar.casaimwrap.makeCoordinateSystem(image_shape[2:], [delta_px, \
        delta_px], processor.phase_reference(), channel_freq, channel_width))

    n_model = 1
    # TODO: Check code for n_model > 1!
    assert(n_model == 1)

    do_cr_joint = True
    # TODO: Check code for do_cr_joint = False!
    assert(do_cr_joint)

    # Make approximate PSF images for all models.
    util.notice("making approximate PSF images for all fields...")
    psf = [None for i in range(n_model)]
    beam = [None for i in range(n_model)]
    for model in range(n_model):
        psf[model] = processor.point_spread_function(image_coordinates, \
            image_shape)
        fit = lofar.casaimwrap.fitGaussianPSF(image_coordinates.dict(), \
            psf[model])
        assert(fit["ok"])

        beam[model] = BeamParameters((fit["major"] * numpy.pi) \
            / (3600.0 * 180.0), (fit["minor"] * numpy.pi) / (3600.0 * 180.0), \
            (fit["angle"] * numpy.pi) / 180.0)
        util.notice("PSF: model: %d major axis: %f arcsec minor axis: %f arcsec"
            " position angle: %f deg" % (model, abs(fit["major"]), \
            abs(fit["minor"]), fit["angle"]))
#    show_image("approximate PSF", psf[0][0,:,:,:])

    # Analyse PSF images.
    (min_psf, max_psf, max_psf_outer, psf_patch_size, max_sidelobe) = \
        validate_psf(image_coordinates, psf, beam)
    clark_options["psf_patch_size"] = psf_patch_size

    weight = [None for i in range(n_model)]
    image = [numpy.zeros(image_shape) for i in range(n_model)]
    delta = [numpy.zeros(image_shape) for i in range(n_model)]
    residual = [numpy.zeros(image_shape) for i in range(n_model)]
    iterations = [[] for i in range(n_model)]

    converged = False
    modified = True
    absmax = options.threshold
    old_absmax = 1e30
    cycle_threshold = 0.0
    max_iterations = 0
    old_max_iterations = 0
    cycle = 0

    while absmax >= options.threshold and max_iterations < options.iterations:
        util.notice("starting major cycle: %d" % cycle)

        # Comment from CASA source code:
        #
        # Make the residual images. We do an incremental update for cycles after
        # the first one. If we have only one model then we use convolutions to
        # speed the processing
        util.notice("making residual images for all fields...")
        if modified:
            # TODO: If n_models > 1, need to compute residuals from the sum of
            # the degridded visibilities (see LofarCubeSkyEquation.cc).
            assert(n_model == 1)
            for i in range(n_model):
                residual[i], weight[i] = processor.residual(image_coordinates, \
                    image[i], processors.Normalization.FLAT_NOISE, \
                    processors.Normalization.FLAT_NOISE)
            modified = False

        if cycle == 0:
            max_weight = 0.0
            for i in range(n_model):
                max_weight = max(max_weight, numpy.max(weight[i]))
            util.notice("maximum sensitivity: %f Jy/beam" % (1.0 \
                / numpy.sqrt(max_weight)))

        (absmax, resmin, resmax) = max_field(weight, residual)

        # Check for divergence.
        if cycle > 0:
            # Comment from CASA source code:
            #
            # Check if absmax is 5% above its previous value.
            if absmax < 1.000005 * old_absmax:
                old_absmax = absmax
            else:
                util.error("clean diverging!")
                break

        for i in range(n_model):
            util.notice("model: %d min residual: %f max residual: %f" % (i, \
                resmin[i], resmax[i]))

        # Check stop criterium.
        if absmax < options.threshold:
            util.notice("reached peak residual treshold: %f" % absmax)
            converged = True
            break

        # Comment from CASA source code:
        #
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
        fraction_of_psf = min(options.cycle_max_psf_fraction, \
            options.cycle_factor * max_sidelobe)

        if fraction_of_psf > 0.8:
            util.warning("PSF fraction for threshold computation is too"
                " high: %f. Forcing to 0.8 to ensure that the threshold is"
                " smaller than the peak residual!" % fraction_of_psf)
            fraction_of_psf = 0.8   # painfully slow!

        # Update cycle threshold.
        cycle_threshold = max(0.95 * options.threshold, fraction_of_psf \
            * absmax)
        clark_options["cycle_threshold"] = cycle_threshold

        util.notice("minor cycle threshold max(0.95 * %f, peak residual * %f):"
            " %f" % (options.threshold, fraction_of_psf, cycle_threshold))
        util.notice("peak residual: %f, cleaning down to: %f" % (absmax, \
            cycle_threshold))

        # Execute the minor cycle (Clark clean) for each channel of each model.
        for model in range(n_model):
            util.notice("processing model %d/%d" % (model, n_model))

            n_x = image[model].shape[-1]
            n_y = image[model].shape[-2]
            n_cr = image[model].shape[-3]
            n_ch = image[model].shape[-4]
            n_cr_cube = n_cr if do_cr_joint else 1

            if cycle == 0:
                # TODO: Useless code in MFCleanImageSkyModel.cc at this
                # location?
                iterations[model] = [0 for i in range(n_ch * n_cr)]

            # Zero the delta image for this model.
            delta[model].fill(0.0)

            if max(abs(resmin[model]), abs(resmax[model])) <= cycle_threshold:
                util.notice("    peak residual below threshold")
            elif max_psf[model] > 0.0:
                modified = True

                for ch in range(n_ch):
                    # TODO: The value of max_weight is only updated during
                    # cycle 0. Is this correct?
                    #
                    assert(len(weight[model].shape) == 2 \
                        and weight[model].shape[0] == n_ch \
                        and weight[model].shape[1] == n_cr)

                    plane_weight = numpy.sqrt(weight[model][ch, :] / max_weight)
                    if numpy.any(plane_weight > 0.01):
                        weight_mask = numpy.ones((n_y, n_x))
                    else:
                        weight_mask = numpy.zeros((n_y, n_x))

                    # Call CASA Clark clean implementation (minor cycle).
                    result = lofar.casaimwrap.clarkClean(psf[model][ch,0,:,:], \
                        residual[model][ch,:,:,:], weight_mask, \
                        iterations[model][ch * n_cr_cube], clark_options)
                    iterations[model][ch * n_cr_cube] = result["iterations"]
                    delta[model][ch,:,:,:] = result["delta"]

                    max_iterations = max(max_iterations, \
                        iterations[model][ch * n_cr_cube])

                    util.notice("    cleaned channel: %d/%d iterations: %d" \
                        % (ch, n_ch, iterations[model][ch * n_cr_cube]))
            else:
                    util.warning("    psf negative or zero")

        if max_iterations != old_max_iterations:
            old_max_iterations = max_iterations

            # Update model images.
            for model in range(n_model):
                image[model] += delta[model]
                util.notice("%f Jy <- cleaned in this cycle for model %d" \
                    % (numpy.sum(delta[model]), model))
        else:
            util.notice("no more iterations left in this major cycle," \
                " stopping now")
            converged = True
            break

        # Update major cycle counter.
        cycle += 1

    if modified:
        util.notice("finalizing residual images for all fields...")
        for i in range(n_model):
            residual[i], weight[i] = processor.residual(image_coordinates, \
                image[i], processors.Normalization.FLAT_NOISE, \
                processors.Normalization.FLAT_NOISE)
        modified = False

        (final_absmax, resmin, resmax) = max_field(weight, residual)
        show_image("final residual", residual[0][0,:,:,:])

        util.notice("final peak residual: %f" % final_absmax)
        converged = (final_absmax < 1.05 * options.threshold)

        for model in range(n_model):
            util.notice("model: %d min residual: %f max residual: %f clean" \
                " flux: %f residual rms: %f" % (model, resmin[model], \
                resmax[model], numpy.sum(image[model]), \
                numpy.std(residual[model])))
    else:
        util.notice("residual images for all fields are up-to-date...")

    util.notice("saving restored image...")
    restored = restore_image(image_coordinates.dict(), image[0], residual[0], \
        beam[0])
    show_image("restored image", restored[0,:,:,:])

    im_restored = pyrap.images.image(options.image + ".restored", \
        shape=image_shape, coordsys=image_coordinates)
    im_restored.putdata(restored)

    if converged:
        util.notice("clean converged.")
    else:
        util.error("clean did NOT converge.")

    pylab.show()
