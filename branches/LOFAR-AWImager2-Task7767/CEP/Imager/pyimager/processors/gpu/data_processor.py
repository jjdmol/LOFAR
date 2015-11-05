import numpy
from ...processors import Normalization

class DataProcessor:
    def __init__(self, measurement, options):

        #self._create_processor(measurement, options)

        # Since the coordinatesystem and shape are arguments to the grid(), etc.
        # functions, the density and average response have to be recomputed
        # internally when the supplied coordinatesystem or shape do not match
        # those of the previous call.
        self._coordinates = None
        self._shape = None
        self._cached_density = None
        self._cached_response = None
        self._response_available = False
        #self._weighting_needs_density = (options["weighttype"] != "natural")

    def capabilities(self):
        raise RuntimeError("GPU dataprocessor capabilities not implemented")

    def phase_reference(self):
        raise RuntimeError("GPU dataprocessor phase_reference not implemented")

    def channel_frequency(self):
        raise RuntimeError("GPU dataprocessor channel_frequency not implemented")

    def channel_width(self):
        raise RuntimeError("GPU dataprocessor channel_width not implemented")

    def maximum_baseline_length(self):
        raise RuntimeError("GPU dataprocessor maximum_baseline_length not implemented")

    def point_spread_function(self, coordinates, shape):
        raise RuntimeError("GPU dataprocessor point_spread_function not implemented")

    def response(self, coordinates, shape):
        raise RuntimeError("GPU dataprocessor response not implemented")

    def grid(self, coordinates, shape, normalization):
        raise RuntimeError("GPU dataprocessor grid not implemented")

    def degrid(self, coordinates, model, normalization):
        raise RuntimeError("GPU dataprocessor degrid not implemented")

    def residual(self, coordinates, model, normalization_model, \
        normalization_residual):
        raise RuntimeError("GPU dataprocessor residual not implemented")

    def normalize(self, coordinates, image, normalization_in, \
        normalization_out):

        # Identity.
        if normalization_in == normalization_out:
            return numpy.copy(image)

        self._update_image_configuration(coordinates, image.shape)

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

    def _update_image_configuration(self, coordinates, shape):
        if self._coordinates != coordinates or self._shape != shape:
            self._coordinates = coordinates
            self._shape = shape
            self._cached_density = None
            self._cached_response = None
            self._response_available = False
