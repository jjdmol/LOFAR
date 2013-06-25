# $Id$

from data_processor_base import DataProcessorBase, ImageWeight, Normalization
import os

def create_data_processor(name, measurement, options):
    """Factory function that attempts to create a DataProcessor instance by
    name."""

    if isinstance(measurement, basestring) and os.path.isfile(measurement):
        measurement = read_datadescription(measurement)
        
    if isinstance(measurement, dict) :
        import parallel
        return parallel.create_data_processor(measurement, options)

    if isinstance(measurement, list) :
        import serial
        return serial.create_data_processor(measurement, options)
    
    try:
        # The call to __import__ fails when not passing the global variables.
        module = __import__(name, globals=globals())
    except ImportError:
        raise RuntimeError("Unable to import data processor implementation:" \
            " %s" % name)

    try:
        processor = module.create_data_processor(measurement, options)
    except AttributeError:
        try:
            processor = module.DataProcessor(measurement, options)
        except AttributeError:
            raise RuntimeError("Non-conforming data processor implementation: %s" \
                % name)
    if not isinstance(processor, DataProcessorBase) :
        raise RuntimeError("Non-conforming data processor implementation: %s" \
            % name)
    
    return processor

def create_data_processor_low_level(name, measurement, options):
    """Factory function that attempts to create a DataProcessor instance by
    name."""
    
    if isinstance(measurement, dict) :
        import parallel
        return parallel.create_data_processor_low_level(name, measurement, options)

    if isinstance(measurement, list) :
        import serial
        return serial.create_data_processor_low_level(name, measurement, options)
    
    try:
        # The call to __import__ fails when not passing the global variables.
        module = __import__(name, globals=globals())
    except ImportError:
        raise RuntimeError("Unable to import data processor implementation:" \
            " %s" % name)

    try:
        processor = module.create_data_processor_low_level(measurement, options)
    except AttributeError:
        # Apparently there is no factory function, try to create instance directly
        try:
            processor = module.DataProcessorLowLevel(measurement, options)
        except AttributeError:
            raise RuntimeError("No DataProcessorLowLevel found in %s" \
                % name)
    if not isinstance(processor, DataProcessorLowLevelBase) :
        raise RuntimeError("Non-conforming DataProcessorLowLevel implementation: %s" \
            % name)
    
    return processor

    
    #if isinstance(measurement, dict) or 
        #return DataProcessorParallel(measurement, options)
    #else:  
        #return DataProcessor(measurement, options)
    