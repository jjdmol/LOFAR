import pylab
import numpy
import pyrap.images

import lofar.casaimwrap
import lofar.pyimager.processors as processors
import util

def empty(options):
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

    image_shape = (1, 4, n_px, n_px)
    image_coordinates = pyrap.images.coordinates.coordinatesystem( \
        lofar.casaimwrap.makeCoordinateSystem(image_shape[2:], [delta_px, \
        delta_px], processor.phase_reference(), channel_freq, channel_width))

    util.notice("creating empty image...")
    pyrap.images.image(options.image, shape=image_shape, \
        coordsys=image_coordinates)
