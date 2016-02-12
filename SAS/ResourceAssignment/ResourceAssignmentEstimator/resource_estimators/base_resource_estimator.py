""" Base class for Resource Estimators
"""
import logging

logger = logging.getLogger(__name__)


class BaseResourceEstimator(object):
    """ Base class for all other resource estmiater classes
    """
    def __init__(self, name):
        self.name = name
        self.error = ""
        self.parset = {}
        self.required_keys = ()
        self.input_files = {}
        self.output_files = {}
        self.duration = 0  # in seconds
        self.total_data_size = 0  # size in bytes
        self.total_bandwidth = 0  # in bytes/second

    def checkParsetForRequiredKeys(self):
        """ check if all required keys needed are available"""
        logger.debug("required keys: %s" % ', '.join(self.required_keys))
        logger.debug("parset   keys: %s" % ', '.join(self.parset.keys()))

        missing_keys = set(self.required_keys) - set(self.parset.keys())

        if missing_keys:
            logger.error("missing keys: %s" % ', '.join(missing_keys))
            return False

        return True

    def estimate(self):
        raise NotImplementedError('estimate() in base class is called. Please implement estimate() in your subclass')

    def result_as_dict(self):
        """ return estimated values as dict """
        result = {}
        result[self.name] = {}
        result[self.name]['total_data_size'] = int(self.total_data_size)
        result[self.name]['total_bandwidth'] = int(self.total_bandwidth)
        result[self.name]['output_files'] = self.output_files
        return result
