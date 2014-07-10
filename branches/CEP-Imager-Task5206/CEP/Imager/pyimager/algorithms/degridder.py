import numpy
import pyrap.images

import lofar.casaimwrap
import lofar.pyimager.processors as processors
import util

def degridder(options):
    max_baseline = options.max_baseline if options.max_baseline > 0.0 else \
        10000.0

    processor_options = {}
    processor_options["threads"] = options.threads
    processor_options["processor"] = options.processor
    processor_options["w_max"] = max_baseline
    processor_options["padding"] = 1.0
    processor_options["image"] = options.image
    processor_options["weighttype"] = options.weighttype
    processor_options["rmode"] = options.rmode
    processor_options["noise"] = options.noise
    processor_options["robustness"] = options.robustness
    processor_options["profile"] = options.profile
    
    processor_options["gridding.ATerm.name"] = "ATermPython"
    processor_options["ATermPython.module"] = "lofar.imager.myaterm"
    processor_options["ATermPython.class"] = "MyATerm"
    
    processor = processors.create_data_processor(options.ms, processor_options)

    '''channel_freq = processor.channel_frequency()
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
    util.notice("    angular size: %.2f deg" % (image_size * 180.0 / numpy.pi))
    util.notice("    angular resolution @ 3 pixel/beam: %.2f arcsec/pixel"
        % (3600.0 * delta_px * 180.0 / numpy.pi))

    image_shape = (1, 4, n_px, n_px)
    image_coordinates = pyrap.images.coordinates.coordinatesystem(
        lofar.casaimwrap.make_coordinate_system(image_shape[2:], [delta_px,
        delta_px], processor.phase_reference(), channel_freq, channel_width))'''


    #Read input model image
    modelim = pyrap.images.image(processor_options['image'])

    util.notice("Model name: %s"%processor_options['image'])
    print type(modelim)
    util.notice('model shape: %s'%modelim.shape());

    #Get image coordinates
    model_coordinates = modelim.coordinates()
    model = modelim.getdata()

    print type(model_coordinates)
    print type(model)
    print 'model shape after getdata: ',model.shape;

    #degrid
    util.notice("Predicting visibilities...")
    processor.degrid(model_coordinates, model, processors.Normalization.FLAT_GAIN)
