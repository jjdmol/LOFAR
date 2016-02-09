""" P
"""

import logging
from math import ceil
from base_resource_estimator import BaseResourceEstimator
from .parameterset import ParameterSet

logger = logging.getLogger(__name__)

class PulsarPipelineResourceEstimator(BaseResourceEstimator):
    """ PulsarPipelineResourceEstimator

    """
    def __init__(self, kwargs, input_files):
        BaseResourceEstimator.__init__(self, name='pulsar_pipeline')
        self.parset = ParameterSet(kwargs).make_subset('dp.output')
        self.duration = int(kwargs.get('observation.duration', 1))
        self.coherent_stokes_type = kwargs.get('observation.coherent_stokes.type')
        self.input_files = input_files
        self.used_keys = ('pulsar.enabled',)
        if self.check_parset():
            self.estimate()
        return

    def estimate(self):
        """ Estimate for pulsar pipeline
        calculates: datasize (number of files, file size), bandwidth
        """
        logger.debug("start estimate '{}'".format(self.name))
        if self.parset['pulsar']['enabled'] == 'true':
            logger.debug("calculate pulsar data size")
            nr_output_files = 0
            if 'dp_coherent_stokes' in self.input_files:
                nr_input_files = int(self.input_files['dp_coherent_stokes']['nr_files'])
                if self.coherent_stokes_type == 'DATA_TYPE_XXYY':
                    nr_output_files += nr_input_files / 4
                else:
                    nr_output_files += nr_input_files

            if 'dp_incoherent_stokes' in self.input_files:
                nr_input_files = int(self.input_files['dp_incoherent_stokes']['nr_files'])
                nr_output_files += nr_input_files

            self.output_files['dp_pulsar'] = {'nr_files': nr_output_files, 'file_size': 1000}
            logger.debug("dp_pulsar: {} files {} bytes each".format(nr_output_files, 1000))

            # count total data size
            for values in self.output_files.itervalues():
                self.total_data_size += values['nr_files'] * values['file_size']
            self.total_bandwidth = ceil((self.total_data_size * 8) / self.duration)  # bits/second
        return
