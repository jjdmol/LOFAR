from data_processor import DataProcessor
from data_processor_parallel_low_level import DataProcessorParallelLowLevel

class DataProcessorParallel(DataProcessor):

    def __init__(self, measurement, options):
        DataProcessor.__init__(self, measurement, options)
        self._cached_channels = None

    def _create_processor(self, measurement, options):
        self._processor = DataProcessorParallelLowLevel(measurement, options)

    def channels(self):
        if self._cached_channels is None:
            self._cached_channels = self._processor.channels()
        return self._cached_channels

    def channel_frequency(self):
        return [channel[0] for channel in self.channels()]

    def channel_width(self):
        return [channel[1] for channel in self.channels()]
