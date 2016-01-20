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
        self.used_keys = ()
        self.input_files = {}
        self.output_files = {}
        self.duration = 0  # in seconds
        self.total_data_size = 0  # size in bytes
        self.total_bandwidth = 0  # in bytes/second

    def check_parset(self):
        """ check if all keys needed are available
        """
        missing_keys = False
        for key in self.used_keys:
            key_set = key.split('.')
            parset = self.parset
            for k in key_set:
                if k in parset:
                    parset = parset[k]
                else:
                    logger.error("missing key [{}]".format(key))
                    missing_keys = True

        if missing_keys:
            self.error = 'missing key(s)'
            return False
        return True

    def estimate(self):
        """ empty estimate function
        """
        self.error = "estimate function not defined"
        logger.info("estimate in base class is called")
        return

    def result_as_dict(self):
        """ return estimated values as dict """
        result = {}
        result[self.name] = {}
        result[self.name]['total_data_size'] = int(self.total_data_size)
        result[self.name]['total_bandwidth'] = int(self.total_bandwidth)
        result[self.name]['output_files'] = self.output_files
        return result
