import numpy
from ...processors import Normalization
from ..data_processor_default import DataProcessorDefault
import data_processor_low_level

class DataProcessor(DataProcessorDefault):

    def _create_processor(self, measurement, options) :
        self._processor = \
            data_processor_low_level.DataProcessorLowLevel(measurement, options)

