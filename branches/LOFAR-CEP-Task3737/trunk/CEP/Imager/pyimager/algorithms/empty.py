import pylab
import numpy
import pyrap.images

import lofar.casaimwrap
import lofar.pyimager.processors as processors
import util

def empty(args):
    options = {}

    max_baseline = args.B if args.B > 0.0 else 10000.0
    options["wmax"] = max_baseline
    options["padding"] = 1.0
    options["image"] = args.image

    proc = processors.create_data_processor(args.processor, args.ms, options)

    channel_freq = proc.channel_frequency()
    channel_width = proc.channel_width()

    max_freq = numpy.max(channel_freq)
    image_size = 2.0 * util.full_width_half_max(70.0, max_freq)

    # TODO: Cyril mentioned above image size estimation is too conservative.
    # Need to check this and find a better estimate if necessary. For now, will
    # just multiply estimated FOV by 2.0.
    image_size *= 2.0

    (n_px, delta_px) = util.image_configuration(image_size, max_freq, \
        max_baseline)

    print "image configuration:"
    print "    size: %d x %d pixel" % (n_px, n_px)
    print "    angular size: %.2f deg" \
        % (image_size * 180.0 / numpy.pi)
    print "    angular resolution @ 3 pixel/beam: %.2f arcsec/pixel" \
        % (3600.0 * delta_px * 180.0 / numpy.pi)

    n_ch = len(channel_freq)
    n_cr = 4

    image_shape = (n_ch, n_cr, n_px, n_px)
    image_coordinates = pyrap.images.coordinates.coordinatesystem( \
        lofar.casaimwrap.makeCoordinateSystem(image_shape[2:], [delta_px, \
        delta_px], proc.phase_reference(), channel_freq, channel_width))

    pyrap.images.image(args.image, shape=image_shape, \
        coordsys=image_coordinates)
