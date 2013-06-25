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
    def degrid(self, coordinates, model, normalization):
        """
        """

    @abstractmethod
    def residual(self, coordinates, model, normalization_model, \
        normalization_residual):
        """
        """

    def normalize(self, coordinates, image, normalization_in, \
        normalization_out):
        """
        """

        # Identity.
        if normalization_in == normalization_out:
            return numpy.copy(image)

        # NONE -> FLAT_NOISE or FLAT_NOISE -> FLAT_GAIN.
        if (normalization_in == Normalization.NONE \
            and normalization_out == Normalization.FLAT_NOISE) \
            or (normalization_in == Normalization.FLAT_NOISE \
            and normalization_out == Normalization.FLAT_GAIN):

            return image / numpy.sqrt(self._response())

        # NONE -> FLAT_GAIN.
        if (normalization_in == Normalization.NONE \
            and normalization_out == Normalization.FLAT_GAIN):

            return image / self._response()

        # FLAT_NOISE -> NONE or FLAT_GAIN -> FLAT_NOISE.
        if (normalization_in == Normalization.FLAT_NOISE \
            and normalization_out == Normalization.NONE) \
            or (normalization_in == Normalization.FLAT_GAIN \
            and normalization_out == Normalization.FLAT_NOISE):

            return image * numpy.sqrt(self._response())

        # FLAT_GAIN -> NONE.
        assert(normalization_in == Normalization.FLAT_GAIN \
            and normalization_out == Normalization.NONE)
        return image * self._response()

