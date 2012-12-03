import numpy
import data_processor_low_level

class DataProcessor:
    def __init__(self, measurement, options):
        self._processor = \
            data_processor_low_level.DataProcessorLowLevel(measurement, options)

        # Since the coordinatesystem and shape are arguments to the grid(), etc.
        # functions, the density and average response have to be recomputed
        # internally when the supplied coordinatesystem or shape do not match
        # those of the previous call.
        self._coordinates = None
        self._shape = None
        self._cached_density = None
        self._cached_response = None

    def capabilities(self):
        return self._processor.capabilities()

    def phase_reference(self):
        return self._processor.phase_reference()

    def channel_frequency(self):
        return self._processor.channel_frequency()

    def channel_width(self):
        return self._processor.channel_width()

    def maximum_baseline_length(self):
        return self._processor.maximum_baseline_length()

    def point_spread_function(self, coordinates, shape):
        self._update_image_configuration(coordinates, shape)

        psf, weight = self._processor.point_spread_function(self._coordinates,
            self._shape, self._density(), False)

        # TODO: This is a sanity check that can be removed when we refactor
        # LofarFTMachine.
        planar_weight = numpy.max(numpy.max(weight, axis=3), axis=2)
        assert(numpy.all(weight == planar_weight[:, :, numpy.newaxis, \
            numpy.newaxis]))

        return numpy.where(weight > 0.0, psf / weight, 0.0)

    def response(self, coordinates, shape):
        self._update_image_configuration(coordinates, shape)
        return self._response()

    def grid(self, coordinates, shape, normalization):
        self._update_image_configuration(coordinates, shape)

        image, weight = self._processor.grid(self._coordinates, self._shape, \
            self._density(), False)

        # TODO: Normalization to "normalization".
        return (numpy.where(weight > 0.0, image / weight, 0.0), weight)

    def degrid(self, coordinates, model, normalization):
        self._update_image_configuration(coordinates, model.shape)

        # TODO: Normalization to FLAT_GAIN from "normalization".
        self._processor.degrid(self._coordinates, model, False)

    def residual(self, coordinates, model, normalization_model, \
        normalization_residual):

        self._update_image_configuration(coordinates, model.shape)

        # TODO: Normalization to FLAT_GAIN from "normalization_model".

        residual, weight = self._processor.residual(self._coordinates, model, \
            self._density(), False)

        # TODO: Normalization to "normalization_residual".
        return (numpy.where(weight > 0.0, residual / weight, 0.0), weight)

    def _density(self):
        if self._cached_density is None:
            self._cached_density = self._processor.density(self._coordinates, \
                self._shape)
        return self._cached_density

    def _response(self):
        if self._cached_response is None:
            self._cached_response = \
                self._processor.response(self._coordinates, self._shape, \
                self._density())
        return self._cached_response

    def _update_image_configuration(self, coordinates, shape):
        if self._coordinates != coordinates or self._shape != shape:
            self._coordinates = coordinates
            self._shape = shape
            self._cached_density = None
            self._cached_response = None
