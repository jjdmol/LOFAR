# $Id$

from data_processor import DataProcessor

def create_data_processor(measurement, options):
    """Factory function that can be used to switch to a specialized class
    depending on the options and the measurement(s) to be processed.
    """
    
    return DataProcessor(measurement, options)
