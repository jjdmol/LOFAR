from abc import ABCMeta, abstractmethod

class ImageWeight:
    NATURAL, UNIFORM, ROBUST, RADIAL = range(4)

class Normalization:
    NONE, FLAT_NOISE, FLAT_GAIN = range(3)

class DataProcessorBase:

    __metaclass__ = ABCMeta

    @abstractmethod
    def __init__(self, measurement, options):
        """
        """

    @abstractmethod
    def capabilities(self):
        """
        """

    @abstractmethod
    def phase_reference(self):
        """
        """

    @abstractmethod
    def channel_frequency(self):
        """
        """

    @abstractmethod
    def channel_width(self):
        """
        """

    @abstractmethod
    def maximum_baseline_length(self):
        """
        """

    @abstractmethod
    def point_spread_function(self, coordinates, shape):
        """
        """

    @abstractmethod
    def response(self, coordinates, shape):
        """
        """

    @abstractmethod
    def grid(self, coordinates, shape, normalization):
        """
        """

    @abstractmethod
    def grid_chunk(self, coordinates, shape, normalization, chunksize):
        """
        """

    @abstractmethod
    def degrid(self, coordinates, model, normalization):
        """
        """

    @abstractmethod
    def degrid_chunk(self, coordinates, model, normalization, chunksize):
        """
        """

    @abstractmethod
    def residual(self, coordinates, model, normalization_model,
        normalization_residual):
        """
        """
