import numpy
import pyrap.images

import lofar.casaimwrap
import lofar.pyimager.processors as processors
import util

def dirty(options):
    # Create the data processor. The data processor is an abstration over
    # different gridding / degridding algorithms. The idea is that the data
    # processor transforms from image to visibilities and vice versa. The rest
    # of the code only works on images and does not (need to) accesss visibility
    # data.
    #
    # Several implementation of the data processor interface (see
    # processors/data_processor_base.py) are available. The idea is to have
    # optimized implementations for specific cases, as well as (possibly slower)
    # generic implementations.
    #
    # TODO: Need to create a smaller set of options that are required when the
    # data processor is instantiated. For example, to create an empty image,
    # details about the weighting scheme are not important.
    #
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

    # Estimate the size of the image in radians, based on an esitmate of the
    # FWHM of the station beam, assuming a station diameter of 70 meters.
    max_freq = numpy.max(channel_freq)
    image_size = 2.0 * util.full_width_half_max(70.0, max_freq)

    # TODO: Cyril mentioned above image size estimation is too conservative.
    # Need to check this and find a better estimate if necessary. For now, will
    # just multiply estimated FOV by 2.0.
    image_size *= 2.0

    # Estimate the number of pixels and the pixels size in radians such that
    # the image is sampled at approximately 3 pixels per beam.
    (n_px, delta_px) = util.image_configuration(image_size, max_freq,
        max_baseline)

    util.notice("image configuration:")
    util.notice("    size: %d x %d pixel" % (n_px, n_px))
    util.notice("    angular size: %.2f deg" % (image_size * 180.0 / numpy.pi))
    util.notice("    angular resolution @ 3 pixel/beam: %.2f arcsec/pixel"
        % (3600.0 * delta_px * 180.0 / numpy.pi))

    # Create an empty image. For the moment, the implementation is limited to
    # single channel images.
    image_shape = (1, 4, n_px, n_px)
    image_coordinates = pyrap.images.coordinates.coordinatesystem(
        lofar.casaimwrap.make_coordinate_system(image_shape[2:], [delta_px,
        delta_px], processor.phase_reference(), channel_freq, channel_width))

    # Call the data processor to grid the visibility data (i.e. compute the
    # dirty image).
    util.notice("creating dirty image...")
    dirty_image, _ = processor.grid(image_coordinates, image_shape,
        processors.Normalization.FLAT_NOISE)

    # Store output images. Store both a flat noise and a flat gain image.
    util.notice("storing dirty images...")
    util.store_image(options.image + ".dirty.flat_noise",
        image_coordinates, dirty_image)
    util.store_image(options.image + ".dirty", image_coordinates,
        processor.normalize(image_coordinates, dirty_image,
        processors.Normalization.FLAT_NOISE,
        processors.Normalization.FLAT_GAIN))
