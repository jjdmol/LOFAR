# $Id$

import os
import json
from data_processor_base import DataProcessorBase, ImageWeight, Normalization
from data_processor_low_level_base import DataProcessorLowLevelBase

def read_data_descriptor(descriptor):
    with open(descriptor) as fin:
        return json.load(fin)

def create_data_processor(measurement, options):
    """Factory function that attempts to create a DataProcessor instance by
    name."""

    if isinstance(measurement, basestring) and os.path.isfile(measurement):
        try:
            measurement = read_data_descriptor(measurement)
        except EnvironmentError:
            raise RuntimeError("Unable to load JSON data descriptor from: %s"
                % measurement)

    if isinstance(measurement, dict) :
        import parallel
        return parallel.create_data_processor(measurement, options)

    if isinstance(measurement, list) :
        import serial
        return serial.create_data_processor(measurement, options)

    name = options["processor"]

    try:
        # The call to __import__ fails when not passing the global variables.
        module = __import__(name, globals=globals())
    except ImportError:
        raise RuntimeError("Unable to import data processor implementation: %s"
            % name)

    try:
        processor = module.create_data_processor(measurement, options)
    except AttributeError:
        try:
            processor = module.DataProcessor(measurement, options)
        except AttributeError:
            raise RuntimeError("Non-conforming data processor implementation:"
                " %s" % name)

    if not isinstance(processor, DataProcessorBase) :
        raise RuntimeError("Non-conforming data processor implementation: %s"
            % name)

    return processor

def create_data_processor_low_level(measurement, options):
    """Factory function that attempts to create a DataProcessor instance by
    name."""

    if isinstance(measurement, dict) :
        import parallel
        return parallel.create_data_processor_low_level(measurement, options)

    if isinstance(measurement, list) :
        import serial
        return serial.create_data_processor_low_level(measurement, options)

    name = options["processor"]

    try:
        # The call to __import__ fails when not passing the global variables.
        module = __import__(name, globals = globals())
    except ImportError:
        raise RuntimeError("Unable to import data processor implementation: %s"
            % name)

    try:
        processor = module.create_data_processor_low_level(measurement, options)
    except AttributeError:
        # Apparently there is no factory function, try to create instance
        # directly
        try:
            processor = module.DataProcessorLowLevel(measurement, options)
        except AttributeError:
            raise RuntimeError("No DataProcessorLowLevel found in module: %s"
                % name)

    if not isinstance(processor, DataProcessorLowLevelBase):
        raise RuntimeError("Non-conforming DataProcessorLowLevel"
            " implementation: %s" % name)

    return processor
