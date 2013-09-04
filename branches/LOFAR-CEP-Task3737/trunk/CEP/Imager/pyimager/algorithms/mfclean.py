"""This module is a straight-forward Python port of the multi field Clark clean
algorithm as implemented in CASA (see MFCleanImageSkyModel.cc). In constrast to
the CASA implementation, masks are not supported, nor is the option to use
Hogbohm instead of Clark clean in the minor cycle.
"""

import numpy
import pyrap.images

import lofar.casaimwrap
import lofar.pyimager.processors as processors
import util

class BeamParameters:
    def __init__(self, major_axis = 1.0, minor_axis = 1.0,
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

    major_axis = property(_major_axis, _set_major_axis,
        doc="Major axis size (rad).")

    minor_axis = property(_minor_axis, _set_minor_axis,
        doc="Minor axis size (rad).")

    position_angle = property(_position_angle, _set_position_angle,
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

    for i in range(len(psf)):
        threshold = 0.8 * numpy.max(psf[i])

        # Find channel for which the (signed) maximum of the first correlation
        # is larger than threshold.
        ch = 0
        while ch < len(psf[i]):
            max_psf[i] = numpy.max(psf[i][ch, 0, :, :])
            if max_psf[i] >= threshold:
                break
            ch += 1

        # Compute the minimum for the same channel.
        min_psf[i] = numpy.min(psf[i][ch, 0, :, :])

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
            distance = max(distance,
                int(numpy.ceil(max(beam[i].major_axis / increment,
                beam[i].minor_axis / increment))))

        # TODO: psf_patch_size will always be set based on the distance computed
        # for the last (solvable) model. This seems rather arbitrary?
        psf_patch_size = 3 * distance + 1

        max_psf_outer[i] = max_outer(psf[i][ch, 0, :, :], distance)

        # TODO: What does this do exactly?
        max_sidelobe = max(max_sidelobe, abs(min_psf[i]))
        max_sidelobe = max(max_sidelobe, max_psf_outer[i])

    return (min_psf, max_psf, max_psf_outer, psf_patch_size, max_sidelobe)

def max_field(residual, weight):
    """Re-normalize the residual and return the (signed) minimum and maximum
    residual, as well as the (absolute) maximum residual.

    Re-implementation of MFCleanImageSkyModel::maxField().
    """
    assert(len(weight) == len(residual))
    n_model = len(weight)

    # Compute the (signed) maximum of weight over all models.
    max_weight = 0.0
    for i in range(n_model):
        max_weight = max(max_weight, numpy.max(weight[i]))

    absmax = 0.0
    min_residual = [1e20 for i in range(n_model)]
    max_residual = [-1e20 for i in range(n_model)]
    for i in range(n_model):
        for ch in range(len(residual[i])):
            # TODO: Why is the residual re-weighted here? In practice for LOFAR
            # all weights seem to be equal, which causes the residual to be
            # re-weighted by a factor 1.0. Ensure that this is the case, such
            # that it does not go unnoticed when the re-weighting would have had
            # an effect.
            #
#            residual[i][ch, :, :, :] *= \
#                numpy.sqrt(weight[i][ch, :, :, :] / max_weight)
            assert(numpy.all(weight[i][ch, :] == max_weight))

            fmax = numpy.max(residual[i][ch, :, :, :])
            fmin = numpy.min(residual[i][ch, :, :, :])

            # TODO: What is the logic behind this?
            if fmax < 0.99 * -1e20:
                fmax = 0.0
            if fmin > 0.99 * 1e20:
                fmin = 0.0

            absmax = max(absmax, max(abs(fmax), abs(fmin)))
            min_residual[i] = min(min_residual[i], fmin)
            max_residual[i] = max(max_residual[i], fmax)

    return (absmax, min_residual, max_residual)

def restore_image(csys, image, residual, beam):
    """Convolve the input clean component image with a Gaussian restoring beam
    and add the residual."""
    if beam.major_axis > 0.0 and beam.minor_axis > 0.0:
        major = beam.major_axis * 180.0 * 3600.0 / numpy.pi
        minor = beam.minor_axis * 180.0 * 3600.0 / numpy.pi
        pa = beam.position_angle * 180.0 / numpy.pi
        restored = lofar.casaimwrap.convolve_with_beam(csys, image, major,
            minor, pa)
    else:
        restored = numpy.copy(image)

    restored += residual
    return restored

def mfclean(options):
    clark_options = {}
    clark_options["gain"] = options.gain
    clark_options["iterations"] = options.iterations
    clark_options["cycle_speedup"] = options.cycle_speedup

    max_baseline = options.max_baseline if options.max_baseline > 0.0 else \
        10000.0

    processor_options = {}
    processor_options["processor"] = options.processor
    processor_options["w_max"] = max_baseline
    processor_options["padding"] = 1.0
    processor_options["image"] = options.image
    processor_options["threads"] = options.threads
    processor_options["weighttype"] = options.weighttype
    processor_options["rmode"] = options.rmode
    processor_options["noise"] = options.noise
    processor_options["robustness"] = options.robustness
    processor_options["profile"] = options.profile
    processor = processors.create_data_processor(options.ms, processor_options)

    channel_freq = processor.channel_frequency()
    channel_width = processor.channel_width()

    max_freq = numpy.max(channel_freq)
    image_size = 2.0 * util.full_width_half_max(70.0, max_freq)

    # TODO: Cyril mentioned above image size estimation is too conservative.
    # Need to check this and find a better estimate if necessary. For now, will
    # just multiply estimated FOV by 2.0.
    image_size *= 2.0

    (n_px, delta_px) = util.image_configuration(image_size, max_freq,
        max_baseline)

    util.notice("image configuration:")
    util.notice("    size: %d x %d pixel" % (n_px, n_px))
    util.notice("    angular size: %.2f deg"
        % (image_size * 180.0 / numpy.pi))
    util.notice("    angular resolution @ 3 pixel/beam: %.2f arcsec/pixel"
        % (3600.0 * delta_px * 180.0 / numpy.pi))

    # TODO: Need to implement support for multiple channel images. Currently,
    # all data channels are combined into a single MFS image per correlation.
    image_shape = (1, 4, n_px, n_px)
    image_coordinates = pyrap.images.coordinates.coordinatesystem(
        lofar.casaimwrap.make_coordinate_system(image_shape[2:], [delta_px,
        delta_px], processor.phase_reference(), channel_freq, channel_width))

    n_model = 1
    # TODO: Check code for n_model > 1!
    assert(n_model == 1)

    # Comment from CASA source code:
    #
    # Set to search for peak in I^2+Q^2+U^2+V^2 domain or each stokes plane
    # seperately. Ignored for hogbom and msclean for now.
#    join_stokes = False
    join_stokes = True

    # Compute approximate PSFs.
    util.notice("computing approximate point spread functions...")
    psf = [None for i in range(n_model)]
    beam = [None for i in range(n_model)]
    for i in range(n_model):
        psf[i] = processor.point_spread_function(image_coordinates, image_shape)
        fit = lofar.casaimwrap.fit_gaussian_psf(image_coordinates.dict(),
            psf[i])
        assert(fit["ok"])

        beam[i] = BeamParameters((fit["major"] * numpy.pi) / (3600.0 * 180.0),
            (fit["minor"] * numpy.pi) / (3600.0 * 180.0), (fit["angle"]
            * numpy.pi) / 180.0)

        util.notice("model %d/%d: major axis: %f arcsec, minor axis: %f arcsec,"
            " position angle: %f deg" % (i, n_model - 1, abs(fit["major"]),
            abs(fit["minor"]), fit["angle"]))

    # Validate PSFs.
    (min_psf, max_psf, max_psf_outer, psf_patch_size, max_sidelobe) = \
        validate_psf(image_coordinates, psf, beam)
    clark_options["psf_patch_size"] = psf_patch_size

    updated = [False for i in range(n_model)]
    weight = [None for i in range(n_model)]
    model = [numpy.zeros(image_shape) for i in range(n_model)]
    delta = [numpy.zeros(image_shape) for i in range(n_model)]
    residual = [numpy.zeros(image_shape) for i in range(n_model)]

    if join_stokes:
        iterations = numpy.zeros((n_model, 1, image_shape[0]))
        stokes = ["JOINT"]
        cr_slices = [slice(None)]
    else:
        iterations = numpy.zeros((n_model, image_shape[1], image_shape[0]))
        stokes = image_coordinates.get_coordinate("stokes").get_stokes()
        cr_slices = [slice(i, i + 1) for i in range(4)]

    cycle = 0
    diverged = False
    absmax = options.threshold
    previous_absmax = 1e30

    while absmax >= options.threshold and numpy.max(iterations) \
        < options.iterations and (cycle == 0 or any(updated)):

        util.notice(">> starting major cycle: %d <<" % cycle)

        # Comment from CASA source code:
        #
        # Make the residual images. We do an incremental update for cycles after
        # the first one. If we have only one model then we use convolutions to
        # speed the processing
        util.notice("computing residuals...")

        # TODO: If n_models > 1, need to compute residuals from the sum of
        # the degridded visibilities (see LofarCubeSkyEquation.cc).
        assert(n_model == 1)
        if cycle == 0:
            # Assuming the initial models are zero, the residual visibilities
            # equal the observed visibilities and therefore we only need to
            # grid them.
            for i in range(n_model):
                residual[i], weight[i] = processor.grid(image_coordinates,
                    image_shape, processors.Normalization.FLAT_NOISE)
        else:
            for i in range(n_model):
                if updated[i]:
                    residual[i], weight[i] = \
                        processor.residual(image_coordinates, model[i],
                            processors.Normalization.FLAT_NOISE,
                            processors.Normalization.FLAT_NOISE)
                updated[i] = False

        # Compute residual statistics.
        (absmax, resmin, resmax) = max_field(residual, weight)

        # Print some statistics.
        for i in range(n_model):
            util.notice("model %d/%d: min residual: %f, max residual: %f"
                % (i, n_model - 1, resmin[i], resmax[i]))
        util.notice("peak residual: %f" % absmax)

        # Comment from CASA source code:
        #
        # Check if absmax is 5% above its previous value.
        #
        # TODO: Value used does not look like 5%?
        if absmax >= 1.000005 * previous_absmax:
            diverged = True
            break

        # Store absmax of this major cycle for later reference.
        previous_absmax = absmax

        # Check stop criterium.
        if absmax < options.threshold:
            break

        # TODO: What is this really used for? And does the max weight indeed
        # correspond to sensitivity in Jy/beam?
        if cycle == 0:
            max_weight = 0.0
            for i in range(n_model):
                max_weight = max(max_weight, numpy.max(weight[i]))
            util.notice("maximum sensitivity: %f Jy/beam" % (1.0
                / numpy.sqrt(max_weight)))

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
        fraction_of_psf = min(options.cycle_max_psf_fraction,
            options.cycle_factor * max_sidelobe)

        if fraction_of_psf > 0.8:
            util.warning("PSF fraction for threshold computation is too"
                " high: %f. Forcing to 0.8 to ensure that the threshold is"
                " smaller than the peak residual!" % fraction_of_psf)
            fraction_of_psf = 0.8   # painfully slow!

        # Update cycle threshold.
        cycle_threshold = max(0.95 * options.threshold, fraction_of_psf
            * absmax)
        clark_options["cycle_threshold"] = cycle_threshold

        util.notice("minor cycle threshold max(0.95 * %f, peak residual * %f):"
            " %f" % (options.threshold, fraction_of_psf, cycle_threshold))

        # Execute the minor cycle (Clark clean) for each channel of each model.
        util.notice("starting minor cycle...")
        for i in range(n_model):
            if max(abs(resmin[i]), abs(resmax[i])) < cycle_threshold:
                util.notice("model %d/%d: peak residual below threshold"
                    % (i, n_model - 1))
                continue

            if max_psf[i] <= 0.0:
                util.warning("model %d/%d: point spread function negative or"
                    " zero" % (i, n_model - 1))
                continue

            # Zero the delta image for this model.
            delta[i].fill(0.0)

            for (cr, cr_slice) in enumerate(cr_slices):
                for ch in range(len(residual[i])):
                    # TODO: The value of max_weight is only updated during
                    # cycle 0. Is this correct?
                    #
                    assert(len(weight[i].shape) == 2
                        and weight[i].shape[:2] == residual[i].shape[:2])

                    plane_weight = numpy.sqrt(weight[i][ch, cr_slice]
                        / max_weight)
                    if numpy.any(plane_weight > 0.01):
                        weight_mask = numpy.ones((residual[i].shape[2:]))
                    else:
                        weight_mask = numpy.zeros((residual[i].shape[2:]))

                    # Call CASA Clark clean implementation (minor cycle).
                    # TODO: When cleaning each Stokes parameter separately,
                    # the PSF of Stokes I is used for all others as well?
                    #
                    # Comment from CASA source code:
                    #
                    # We only want the PSF for the first polarization so we
                    # iterate over polarization LAST.
                    #
                    result = lofar.casaimwrap.clark_clean(psf[i][ch,0,:,:],
                        residual[i][ch,cr_slice,:,:], weight_mask,
                        iterations[i,cr,ch], clark_options)

                    if result["iterations"] > iterations[i,cr,ch]:
                        updated[i] = True
                        delta[i][ch,cr_slice,:,:] = result["delta"]
                        iterations[i,cr,ch] = result["iterations"]
                    else:
                        assert(numpy.all(result["delta"] == 0.0))

                util.notice("model %d/%d: stokes: %s, cleaned: %f Jy, "
                    "iterations per channel: %s" % (i, n_model - 1,
                    stokes[cr], numpy.sum(delta[i][ch,cr_slice,:,:]),
                    str(iterations[i,cr,:])))

        # Update model images if required.
        for i in range(n_model):
            if updated[i]:
                model[i] += delta[i]

        # Update major cycle counter.
        cycle += 1

    if any(updated):
        util.notice("finalizing residual images for all fields...")
        for i in range(n_model):
            if updated[i]:
                residual[i], weight[i] = processor.residual(image_coordinates,
                    model[i], processors.Normalization.FLAT_NOISE,
                    processors.Normalization.FLAT_NOISE)
        (absmax, resmin, resmax) = max_field(residual, weight)

        # Print some statistics.
        for i in range(n_model):
            util.notice("model %d/%d: min residual: %f, max residual: %f"
                % (i, n_model - 1, resmin[i], resmax[i]))
        util.notice("peak residual: %f" % absmax)
    else:
        util.notice("residual images for all fields are up-to-date...")

    # Store output images.
    util.notice("storing average response...")
    util.store_image(options.image + ".response", image_coordinates,
        processor.response(image_coordinates, image_shape))

    util.notice("storing model images...")
    for i in range(n_model):
        util.store_image(options.image + ".model.flat_noise",
            image_coordinates, model[i])
        util.store_image(options.image + ".model", image_coordinates,
            processor.normalize(image_coordinates, model[i],
            processors.Normalization.FLAT_NOISE,
            processors.Normalization.FLAT_GAIN))

    util.notice("storing residual images...")
    for i in range(n_model):
        util.store_image(options.image + ".residual.flat_noise",
            image_coordinates, residual[i])
        util.store_image(options.image + ".residual", image_coordinates,
            processor.normalize(image_coordinates, residual[i],
            processors.Normalization.FLAT_NOISE,
            processors.Normalization.FLAT_GAIN))

    util.notice("storing restored images...")
    for i in range(n_model):
        restored = restore_image(image_coordinates.dict(), model[i],
            residual[i], beam[i])

        util.store_image(options.image + ".restored.flat_noise",
            image_coordinates, restored)
        util.store_image(options.image + ".restored", image_coordinates,
            processor.normalize(image_coordinates, restored,
            processors.Normalization.FLAT_NOISE,
            processors.Normalization.FLAT_GAIN))

    # Print some statistics.
    for i in range(n_model):
        util.notice("model %d/%d: clean flux: %f, residual rms: %f" % (i,
            n_model - 1, numpy.sum(model[i]), numpy.std(residual[i])))

    if diverged:
        util.error("clean diverged.")
    elif absmax < options.threshold:
        util.notice("clean converged.")
    else:
        util.warning("clean did not reach threshold: %f Jy."
            % options.threshold)
