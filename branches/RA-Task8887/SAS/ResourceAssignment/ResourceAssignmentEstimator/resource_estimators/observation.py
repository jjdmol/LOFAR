""" O
"""

import logging
from math import ceil, floor
from base_resource_estimator import BaseResourceEstimator
from lofar.parameterset import parameterset
from lofar.common.datetimeutils import totalSeconds
from datetime import datetime, timedelta

logger = logging.getLogger(__name__)


class ObservationResourceEstimator(BaseResourceEstimator):
    """ ObservationResourceEstimator
    """
    def __init__(self, parsetDict):
        logger.info("init ObservationResourceEstimator")
        super(ObservationResourceEstimator, self).__init__(name='Observation')
        logger.info('parsetDict: %s ' % parsetDict)
        self.parset = parameterset(parsetDict).makeSubset('Observation.')
        logger.info('parset: %s ' % self.parset)
        self.required_keys = ('sampleClock',
                              'startTime',
                              'stopTime',
                              'antennaSet',
                              'ObservationControl.OnlineControl.Cobalt.Correlator.nrChannelsPerSubband',
                              'ObservationControl.OnlineControl.Cobalt.Correlator.integrationTime',
                              'ObservationControl.OnlineControl.Cobalt.BeamFormer.flysEye',
                              'ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor',
                              'ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor',
                              'VirtualInstrument.stationList',
                              'DataProducts.Output_CoherentStokes.enabled',
                              #'DataProducts.Output_CoherentStokes.type',
                              'DataProducts.Output_IncoherentStokes.enabled',
                              #'DataProducts.Output_IncoherentStokes.type'
                              )

        if self.checkParsetForRequiredKeys():
            self.estimate()

    def estimate(self):
        """ estimate
        """
        logger.info("start estimate '{}'".format(self.name))
        self.correlated()
        self.coherentstokes()
        self.incoherentstokes()

    def correlated(self):
        """ Estimate number of files and size of file"""
        logger.info("calculating correlated datasize")
        size_of_header = self.parset.getInt('size_of_header', 512)
        size_of_overhead = self.parset.getInt('size_of_overhead', 600000)
        size_of_short = self.parset.getInt('size_of_short', 2)
        size_of_complex = self.parset.getInt('size_of_complex', 8)
        startTime = datetime.strptime(self.parset.getString('startTime'), '%Y-%m-%d %H:%M:%S')
        endTime = datetime.strptime(self.parset.getString('stopTime'), '%Y-%m-%d %H:%M:%S')
        duration = totalSeconds(endTime - startTime)
        nr_polarizations = self.parset.getInt('nr_polarizations', 2)
        channels_per_subband = self.parset.getInt('ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.nrChannelsPerSubband', 64)
        intergration_time = self.parset.getFloat('ObservationControl.OnlineControl.Cobalt.Correlator.integrationTime', 1)
        no_virtual_stations = self._virtual_stations()

        integrated_seconds = floor(duration / intergration_time)
        nr_baselines = no_virtual_stations * (no_virtual_stations + 1.0) / 2.0
        data_size = ceil((nr_baselines * channels_per_subband * nr_polarizations**2 * size_of_complex) / 512.0) * 512.0
        n_sample_size = ceil((nr_baselines * channels_per_subband * size_of_short) / 512.0) * 512.0

        # sum of all subbands in all digital beams
        nr_subbands = 0
        for beam_nr in range(1024):
            try:
                subbandList = self.parset.getStringVector('Beam[%d].subbandList' % beam_nr)
                nr_subbands += len(subbandList)
            except RuntimeError: #why does parset not raise a KeyError???
                break

        file_size = (data_size + n_sample_size + size_of_header) * integrated_seconds + size_of_overhead
        self.output_files['dp_correlated_uv'] = {'nr_files': nr_subbands, 'file_size': int(file_size)}
        logger.info("dp_correlated_uv: {} files {} bytes each".format(nr_subbands, int(file_size)))

        self.total_data_size += ceil(file_size * nr_subbands)  # bytes
        self.total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second


    def coherentstokes(self):
        """ calculate coherent stokes
        """

        #TODO: implement properly
        return

        if self.parset['output']['CoherentStokes']['enabled'] == 'false':
            return
        logger.info("calculate coherentstokes datasize")
        coherent_type = self.parset['output']['CoherentStokes']['type']
        subbands_per_file = 512.
        samples_per_second = self._samples_per_second()
        duration = int(self.parset.get('duration', 0))
        integration_factor = self.parset['output']['CoherentStokes']['integration_factor']

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
            logger.info("add beam {}".format(beam_nr))
            max_nr_subbands = max(max_nr_subbands, int(beam['nr_subbands']))

            tied_array_beam_nr = 0
            while 'tied_array_beam[{}]'.format(tied_array_beam_nr) in beam:
                tied_array_beam = beam['tied_array_beam[{}]'.format(tied_array_beam_nr)]
                logger.info("add tied_array_beam {}".format(tied_array_beam_nr))
                if tied_array_beam['coherent'] == 'true':
                    total_files_min = nr_coherent
                    total_files_summed += total_files_min
                    total_files += total_files_min * ceil(int(beam['nr_subbands']) / subbands_per_file)
                tied_array_beam_nr += 1

            nr_tab_rings = int(beam['nr_tab_rings'])
            if nr_tab_rings > 0:
                logger.info("add size for {} tab_rings".format(nr_tab_rings))
                total_files_min = (3 * nr_tab_rings * (nr_tab_rings + 1) + 1) * nr_coherent
                total_files_summed += total_files_min
                total_files += total_files_min * ceil(int(beam['nr_subbands']) / subbands_per_file)

            if self.parset['flys_eye']['enabled'] == 'true':
                logger.info("calculate flys eye data size")
                total_files_min = self._virtual_stations() * nr_coherent
                total_files_summed += total_files_min
                total_files += total_files_min * ceil(int(beam['nr_subbands']) / subbands_per_file)
            beam_nr += 1

        nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
        size_per_file = nr_subbands_per_file * size_per_subband

        self.output_files['dp_CoherentStokes'] = {'nr_files': int(total_files), 'file_size': int(size_per_file)}
        logger.info("dp_CoherentStokes: {} files {} bytes each".format(int(total_files), int(size_per_file)))

        self.total_data_size += ceil(total_files_summed * max_nr_subbands * size_per_subband)
        self.total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second

    def incoherentstokes(self):
        """ Estimate incoherentstokes()
        calculates: datasize (number of files, file size), bandwidth

        :return:
        """

        #TODO: implement properly
        return

        if self.parset['output']['inCoherentStokes']['enabled'] == 'false':
            return
        logger.info("calculate incoherentstokes data size")
        incoherent_type = self.parset['output']['inCoherentStokes']['type']
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
            logger.info("add beam {}".format(beam_nr))
            max_nr_subbands = max(max_nr_subbands, int(beam['nr_subbands']))
            tied_array_beam_nr = 0
            while 'tied_array_beam[{}]'.format(tied_array_beam_nr) in beam:
                tied_array_beam = beam['tied_array_beam[{}]'.format(tied_array_beam_nr)]
                logger.info("add tied_array_beam {}".format(tied_array_beam_nr))
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

            self.output_files['dp_inCoherentStokes'] = {'nr_files': int(total_files), 'file_size': int(size_per_file)}
            logger.info("dp_inCoherentStokes: {} files {} bytes each".format(int(total_files), int(size_per_file)))

            self.total_data_size += ceil(total_files_summed * max_nr_subbands * size_per_subband)  # bytes
            self.total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/sec
        return

    def _samples_per_second(self):
        """ set samples per second
        """
        samples_160mhz = 155648
        samples_200mhz = 196608
        samples = samples_160mhz if '160' in self.parset['sample_clock'] else samples_200mhz
        logger.info("samples per second for {} MHz clock = {}".format(self.parset['sample_clock'], samples))
        return samples

    def _virtual_stations(self):
        """ calculate virtualnumber of stations
        """
        stationList = self.parset.getStringVector('VirtualInstrument.stationList')
        nr_virtual_stations = 0
        if self.parset['antennaSet'] in ('HBA_DUAL', 'HBA_DUAL_INNER'):
            for station in stationList:
                if 'CS' in station:
                    nr_virtual_stations += 2
                else:
                    nr_virtual_stations += 1
        else:
            nr_virtual_stations = len(stationList)
        logger.info("number of virtual stations = {}".format(nr_virtual_stations))
        return nr_virtual_stations

