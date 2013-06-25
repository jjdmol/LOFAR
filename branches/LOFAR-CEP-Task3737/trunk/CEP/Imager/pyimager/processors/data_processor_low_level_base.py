from abc import ABCMeta, abstractmethod

class DataProcessorLowLevelBase:
    
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
    def point_spread_function(self, coordinates, shape, as_grid):
        """
        """

    @abstractmethod
    def response(self, coordinates, shape):
        """
        """

    @abstractmethod
    def grid(self, coordinates, shape, as_grid):
        """
        """

    @abstractmethod
    def degrid(self, coordinates, model, as_grid):
        """
        """

    @abstractmethod
    def residual(self, coordinates, model, as_grid):
        """
        """

    @abstractmethod
    def density(self, coordinates, shape):
        """
        """
        
    @abstractmethod    
    def set_density(self, density, coordinates) :
        """
        """
        
        
        






        
