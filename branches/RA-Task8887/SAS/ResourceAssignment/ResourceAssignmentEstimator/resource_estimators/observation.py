""" O
"""

import logging
from math import ceil, floor
from base_resource_estimator import BaseResourceEstimator
from .parameterset import ParameterSet

logger = logging.getLogger(__name__)


class ObservationResourceEstimator(BaseResourceEstimator):
    """ ObservationResourceEstimator
    """
    def __init__(self, kwargs):
        BaseResourceEstimator.__init__(self, name='observation')
        logger.debug("init ObservationResourceEstimator")
        self.parset = ParameterSet(kwargs).make_subset('observation')
        self.used_keys = ('sample_clock', 'duration', 'channels_per_subband', 'intergration_time', 'antenna_mode',
                          'stations', 'flys_eye', 'output.coherent_stokes.enabled', 'output.coherent_stokes.type',
                          'output.coherent_stokes.integration_factor', 'output.incoherent_stokes.enabled',
                          'output.incoherent_stokes.type')

        if self.check_parset():
            self.estimate()

    def estimate(self):
        """ estimate
        """
        logger.debug("start estimate '{}'".format(self.name))
        self.correlated()
        self.coherentstokes()
        self.incoherentstokes()

    def correlated(self):
        """ Estimate number of files and size of file"""
        logger.debug("calculate correlated datasize")
        size_of_header = self.parset.get('size_of_header', 512)
        size_of_overhead = self.parset.get('size_of_overhead', 600000)
        size_of_short = self.parset.get('size_of_short', 2)
        size_of_complex = self.parset.get('size_of_complex', 8)
        duration = int(self.parset.get('duration', 0))
        nr_polarizations = int(self.parset.get('nr_polarizations', 2))
        channels_per_subband = int(self.parset.get('channels_per_subband', 64))
        intergration_time = int(self.parset.get('intergration_time', 1))
        no_virtual_stations = self._virtual_stations()

        integrated_seconds = floor(duration / intergration_time)
        nr_baselines = no_virtual_stations * (no_virtual_stations + 1.0) / 2.0
        data_size = ceil((nr_baselines * channels_per_subband * nr_polarizations**2 * size_of_complex) / 512.0) * 512.0
        n_sample_size = ceil((nr_baselines * channels_per_subband * size_of_short) / 512.0) * 512.0

        # sum of all subbands in all digital beams
        nr_subbands = 0
        beam_nr = 0
        while 'beam[{}]'.format(beam_nr) in self.parset:
            nr_subbands += int(self.parset['beam[{}]'.format(beam_nr)]['nr_subbands'])
            beam_nr += 1

        file_size = (data_size + n_sample_size + size_of_header) * integrated_seconds + size_of_overhead
        self.output_files['dp_correlated_uv'] = {'nr_files': nr_subbands, 'file_size': int(file_size)}
        logger.debug("dp_correlated_uv: {} files {} bytes each".format(nr_subbands, int(file_size)))

        self.total_data_size += ceil(file_size * nr_subbands)  # bytes
        self.total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second
        return

    def coherentstokes(self):
        """ calculate coherent stokes
        """
        if self.parset['output']['coherent_stokes']['enabled'] == 'false':
            return
        logger.debug("calculate coherentstokes datasize")
        coherent_type = self.parset['output']['coherent_stokes']['type']
        subbands_per_file = 512.
        samples_per_second = self._samples_per_second()
        duration = int(self.parset.get('duration', 0))
        integration_factor = self.parset['output']['coherent_stokes']['integration_factor']

        total_files_summed = 0
        total_files = 0

        nr_coherent = 4 if coherent_type in ('DATA_TYPE_XXYY', 'DATA_TYPE_STOKES_IQUV') else 1

        if coherent_type in ('DATA_TYPE_XXYY',):
            size_per_subband = samples_per_second * 4.0 * duration
        else:
            size_per_subband = (samples_per_second * 4.0 * duration) / integration_factor

        max_nr_subbands = 0

        beam_nr = 0
        while 'beam[{}]'.format(beam_nr) in self.parset:
            beam = self.parset['beam[{}]'.format(beam_nr)]
            logger.debug("add beam {}".format(beam_nr))
            max_nr_subbands = max(max_nr_subbands, int(beam['nr_subbands']))

            tied_array_beam_nr = 0
            while 'tied_array_beam[{}]'.format(tied_array_beam_nr) in beam:
                tied_array_beam = beam['tied_array_beam[{}]'.format(tied_array_beam_nr)]
                logger.debug("add tied_array_beam {}".format(tied_array_beam_nr))
                if tied_array_beam['coherent'] == 'true':
                    total_files_min = nr_coherent
                    total_files_summed += total_files_min
                    total_files += total_files_min * ceil(int(beam['nr_subbands']) / subbands_per_file)
                tied_array_beam_nr += 1

            nr_tab_rings = int(beam['nr_tab_rings'])
            if nr_tab_rings > 0:
                logger.debug("add size for {} tab_rings".format(nr_tab_rings))
                total_files_min = (3 * nr_tab_rings * (nr_tab_rings + 1) + 1) * nr_coherent
                total_files_summed += total_files_min
                total_files += total_files_min * ceil(int(beam['nr_subbands']) / subbands_per_file)

            if self.parset['flys_eye']['enabled'] == 'true':
                logger.debug("calculate flys eye data size")
                total_files_min = self._virtual_stations() * nr_coherent
                total_files_summed += total_files_min
                total_files += total_files_min * ceil(int(beam['nr_subbands']) / subbands_per_file)
            beam_nr += 1

        nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
        size_per_file = nr_subbands_per_file * size_per_subband

        self.output_files['dp_coherent_stokes'] = {'nr_files': int(total_files), 'file_size': int(size_per_file)}
        logger.debug("dp_coherent_stokes: {} files {} bytes each".format(int(total_files), int(size_per_file)))

        self.total_data_size += ceil(total_files_summed * max_nr_subbands * size_per_subband)
        self.total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second

    def incoherentstokes(self):
        """ Estimate incoherentstokes()
        calculates: datasize (number of files, file size), bandwidth

        :return:
        """
        if self.parset['output']['incoherent_stokes']['enabled'] == 'false':
            return
        logger.debug("calculate incoherentstokes data size")
        incoherent_type = self.parset['output']['incoherent_stokes']['type']
        subbands_per_file = 512.0
        samples_per_second = self._samples_per_second()
        duration = int(self.parset.get('duration', 0))
        time_integration_factor = self.parset.get('incoherent_time_integration_factor', 1)
        channels_per_subband = int(self.parset.get('channels_per_subband', 64))
        incoherent_channels_per_subband = self.parset.get('incoherent_channels_per_subband', 0)

        total_files_summed = 0
        total_files = 0

        nr_incoherent = 4 if incoherent_type in ('DATA_TYPE_STOKES_IQUV',) else 1

        max_nr_subbands = 0
        beam_nr = 0
        while 'beam[{}]'.format(beam_nr) in self.parset:
            beam = self.parset['beam[{}]'.format(beam_nr)]
            logger.debug("add beam {}".format(beam_nr))
            max_nr_subbands = max(max_nr_subbands, int(beam['nr_subbands']))
            tied_array_beam_nr = 0
            while 'tied_array_beam[{}]'.format(tied_array_beam_nr) in beam:
                tied_array_beam = beam['tied_array_beam[{}]'.format(tied_array_beam_nr)]
                logger.debug("add tied_array_beam {}".format(tied_array_beam_nr))
                if tied_array_beam['coherent'] == 'false':
                    total_files_summed += nr_incoherent
                    total_files += nr_incoherent * ceil(int(beam['nr_subbands']) / subbands_per_file)
                tied_array_beam_nr += 1
            beam_nr += 1

        if incoherent_channels_per_subband > 0:
            channel_integration_factor = channels_per_subband / incoherent_channels_per_subband
        else:
            channel_integration_factor = 1

        if total_files > 0:
            nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
            size_per_subband = (samples_per_second * 4) / time_integration_factor / channel_integration_factor * duration
            size_per_file = nr_subbands_per_file * size_per_subband

            self.output_files['dp_incoherent_stokes'] = {'nr_files': int(total_files), 'file_size': int(size_per_file)}
            logger.debug("dp_incoherent_stokes: {} files {} bytes each".format(int(total_files), int(size_per_file)))

            self.total_data_size += ceil(total_files_summed * max_nr_subbands * size_per_subband)  # bytes
            self.total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/sec
        return

    def _samples_per_second(self):
        """ set samples per second
        """
        samples_160mhz = 155648
        samples_200mhz = 196608
        samples = samples_160mhz if '160' in self.parset['sample_clock'] else samples_200mhz
        logger.debug("samples per second for {} MHz clock = {}".format(self.parset['sample_clock'], samples))
        return samples

    def _virtual_stations(self):
        """ calculate virtualnumber of stations
        """
        nr_virtual_stations = 0
        if self.parset['antenna_mode'] in ('HBA_DUAL', 'HBA_DUAL_INNER'):
            for station in self.parset['stations']:
                if 'CS' in station:
                    nr_virtual_stations += 2
                else:
                    nr_virtual_stations += 1
        else:
            nr_virtual_stations = len(self.parset['stations'])
        logger.debug("number of virtual stations = {}".format(nr_virtual_stations))
        return nr_virtual_stations

