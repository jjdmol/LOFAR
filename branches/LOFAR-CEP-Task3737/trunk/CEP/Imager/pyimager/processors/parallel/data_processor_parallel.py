from ..data_processor_default import DataProcessorDefault
from data_processor_parallel_low_level import DataProcessorParallelLowLevel

class DataProcessorParallel(DataProcessorDefault):

    def __init__(self, name, measurement, options):
        self._create_processor(name, measurement, options)
        self._cached_channels = None

    def _create_processor(self, name, measurement, options):
        self._processor = DataProcessorParallelLowLevel(name, measurement, options)

    def channels(self):
        if self._cached_channels is None:
            self._cached_channels = self._processor.channels()
        return self._cached_channels

    def channel_frequency(self):
        return [channel[0] for channel in self.channels()]

    def channel_width(self):
        return [channel[1] for channel in self.channels()]
        
