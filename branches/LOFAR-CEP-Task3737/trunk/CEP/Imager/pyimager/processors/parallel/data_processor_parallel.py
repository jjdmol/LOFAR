from ..data_processor_default import DataProcessorDefault
from data_processor_parallel_low_level import DataProcessorParallelLowLevel

class DataProcessorParallel(DataProcessorDefault):

    def _create_processor(self, measurement, options):
        self._processor = DataProcessorParallelLowLevel(measurement, options)

