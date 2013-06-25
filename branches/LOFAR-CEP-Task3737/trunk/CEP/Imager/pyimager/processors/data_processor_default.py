from abc import ABCMeta, abstractmethod
from data_processor_base import *

import numpy

class DataProcessorDefault(DataProcessorBase):

    def __init__(self, measurement, options):
        """
        """
        # Since the coordinatesystem and shape are arguments to the grid(), etc.
        # functions, the density and average response have to be recomputed
        # internally when the supplied coordinatesystem or shape do not match
        # those of the previous call.
        self._coordinates = None
        self._shape = None
        self._cached_density = None
        self._cached_response = None
        self._weighting_needs_density = (options["weighttype"] != "natural")
        self._create_processor(measurement, options)

    @abstractmethod
    def _create_processor(self, measurement, options) :
        """
        Overload this method with a method that sets _processor to an instance of a class derived from
        DataProcessorLowLevelBase
        """
        self._processor = None
    
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
            self._shape, False)

        exp_weight = weight[:, :, numpy.newaxis, numpy.newaxis]
        return numpy.where(exp_weight > 0.0, psf / exp_weight, 0.0)

    def density(self, coordinates, shape):
        self._update_image_configuration(coordinates, shape)
        return self._density()
        
    def response(self, coordinates, shape):
        self._update_image_configuration(coordinates, shape)
        return self._response()

    def grid(self, coordinates, shape, normalization = Normalization.FLAT_NOISE):

        self._update_image_configuration(coordinates, shape)
        # Grid.
        image, weight = self._processor.grid(self._coordinates, self._shape, \
            False)

        # Divide out the summed weight.
        exp_weight = weight[:, :, numpy.newaxis, numpy.newaxis]
        image = numpy.where(exp_weight > 0.0, image / exp_weight, 0.0)

        # Normalize residual image to the requested normalization. Note that the
        # image produced by gridding is flat noise by default.
        return (self.normalize(coordinates, image, Normalization.FLAT_NOISE, \
            normalization), weight)

    def degrid(self, coordinates, model, normalization = Normalization.FLAT_GAIN):
        self._update_image_configuration(coordinates, model.shape)

        model = self.normalize(coordinates, model, normalization, \
            Normalization.FLAT_GAIN)

        # Degrid
        self._processor.degrid(self._coordinates, model, False)

    def residual(self, coordinates, model, normalization_model = Normalization.FLAT_GAIN, \
        normalization_residual = Normalization.FLAT_NOISE):

        self._update_image_configuration(coordinates, model.shape)

        model = self.normalize(coordinates, model, normalization_model, \
            Normalization.FLAT_NOISE)

        # Compute the residual image.
        residual, weight = self._processor.residual(self._coordinates, model, \
            False)

        # Divide out the summed weight.
        exp_weight = weight[:, :, numpy.newaxis, numpy.newaxis]
        residual = numpy.where(exp_weight > 0.0, residual / exp_weight, 0.0)

        # Normalize residual image to the requested normalization. Note that the
        # residual image is flat noise by default.
        return (self.normalize(coordinates, residual, \
            Normalization.FLAT_NOISE, normalization_residual), weight)

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
            
            if self._weighting_needs_density:
                self._processor.set_density(self._density(), self._coordinates)

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

    def _density(self):
        if self._cached_density is None:
            self._cached_density = self._processor.density(self._coordinates, \
                self._shape)
        return self._cached_density

    def _response(self):
        if self._cached_response is None:
            self._cached_response = self._processor.response(self._coordinates, \
                self._shape)
        return self._cached_response
