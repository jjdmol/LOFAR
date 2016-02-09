""" C
"""

import logging
from math import ceil
from base_resource_estimator import BaseResourceEstimator
from .parameterset import ParameterSet

logger = logging.getLogger(__name__)


class CalibrationPipelineResourceEstimator(BaseResourceEstimator):
    """ CalibrationPipelineResourceEstimator

    """
    def __init__(self, kwargs, input_files):
        BaseResourceEstimator.__init__(self, name='calibration_pipeline')
        self.parset = ParameterSet(kwargs).make_subset('dp.output')
        self.duration = int(kwargs.get('observation.duration', 1))
        self.input_files = input_files
        self.used_keys = ('correlated.enabled', 'correlated.demixing_settings.freq_step',
                          'correlated.demixing_settings.time_step', 'instrument_model.enabled')
        if self.check_parset():
            self.estimate()
        return

    def estimate(self):
        """ Estimate for calibration pipeline
        calculates: datasize (number of files, file size), bandwidth
        """
        logger.debug("start estimate '{}'".format(self.name))
        if 'dp_correlated_uv' in self.input_files:
            if self.parset['correlated']['enabled'] == 'true':
                logger.debug("calculate correlated data size")
                freq_step = int(self.parset['correlated']['demixing_settings']['freq_step'])
                time_step = int(self.parset['correlated']['demixing_settings']['time_step'])
                reduction_factor = freq_step * time_step
                input_file_size = int(self.input_files['dp_correlated_uv']['file_size'])
                output_file_size = 0.0
                if reduction_factor > 0:
                    new_size = input_file_size / reduction_factor
                    output_file_size = new_size + new_size / 64.0 * (1.0 + reduction_factor) + new_size / 2.0
                self.output_files['dp_correlated_uv'] = {'nr_files': int(self.input_files['dp_correlated_uv']['nr_files']), 'file_size': int(output_file_size)}
                logger.debug("dp_correlated_uv: {} files {} bytes each".format(int(self.input_files['dp_correlated_uv']['nr_files']), int(output_file_size)))

            if self.parset['instrument_model']['enabled'] == 'true':
                logger.debug("calculate instrument-model data size")
                self.output_files['dp_instrument_model'] = {'nr_files': int(self.input_files['dp_correlated_uv']['nr_files']), 'file_size': 1000}
                logger.debug("dp_pulsar: {} files {} bytes each".format(int(self.input_files['dp_correlated_uv']['nr_files']), 1000))

            # count total data size
            for values in self.output_files.itervalues():
                self.total_data_size += values['nr_files'] * values['file_size']
            self.total_bandwidth = ceil((self.total_data_size * 8) / self.duration)  # bits/second
        return

