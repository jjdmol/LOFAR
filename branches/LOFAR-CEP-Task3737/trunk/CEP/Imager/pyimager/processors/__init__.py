# $Id$

class ImageWeight:
    NATURAL, UNIFORM, ROBUST, RADIAL = range(4)

class Normalization:
    NONE, FLAT_NOISE, FLAT_GAIN = range(3)

def create_data_processor(name, measurement, options):
    """Factory function that attempts to create a DataProcessor instance by
    name."""
    try:
        # The call to __import__ fails when not passing the global variables.
        module = __import__(name, globals=globals())
    except ImportError:
        raise
        raise RuntimeError("Unable to import data processor implementation: \
            %s" % name)

    try:
        processor = module.create_data_processor(measurement, options)
    except AttributeError:
        raise RuntimeError("Non-conforming data processor implementation: %s" \
            % name)

    return processor
