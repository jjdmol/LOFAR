# $Id$

from data_processor_parallel import DataProcessorParallel
from data_processor_parallel_low_level import DataProcessorParallelLowLevel
import os

def create_data_processor(measurement, options):
    """Factory function that can be used to switch to a specialized class
    depending on the options and the measurement(s) to be processed.
    """
    
    return DataProcessorParallel(measurement, options)

    
def create_data_processor_low_level(measurement, options):
    """Factory function that can be used to switch to a specialized class
    depending on the options and the measurement(s) to be processed.
    """
    
    return DataProcessorParallelLowLevel(measurement, options)

    
    