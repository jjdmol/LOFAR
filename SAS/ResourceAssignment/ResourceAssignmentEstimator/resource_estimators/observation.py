# observation.py
#
# Copyright (C) 2016
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: observation.py 33534 2016-02-08 14:28:26Z schaap $

import logging
from math import ceil, floor
from base_resource_estimator import BaseResourceEstimator
from lofar.parameterset import parameterset

logger = logging.getLogger(__name__)

COBALT = "ObservationControl.OnlineControl.Cobalt."

class ObservationResourceEstimator(BaseResourceEstimator):
    """ ResourceEstimator for LOFAR Observations
    """
    def __init__(self):
        logger.info("init ObservationResourceEstimator")
        super(ObservationResourceEstimator, self).__init__(name='Observation')
        self.required_keys = ('sampleClock',
                              'startTime',
                              'stopTime',
                              'antennaSet',
                              COBALT + 'Correlator.nrChannelsPerSubband',
                              COBALT + 'Correlator.integrationTime',
                              COBALT + 'BeamFormer.flysEye',
                              COBALT + 'BeamFormer.CoherentStokes.timeIntegrationFactor',
                              COBALT + 'BeamFormer.IncoherentStokes.timeIntegrationFactor',
                              'VirtualInstrument.stationList',
                              'DataProducts.Output_CoherentStokes.enabled',
                              'DataProducts.Output_CoherentStokes.type',
                              'DataProducts.Output_IncoherentStokes.enabled',
                              'DataProducts.Output_IncoherentStokes.type'
                              )

    def _calculate(self, parset, input_files={}):
        """ Calculate the combined resources needed by the different observation types that
        can be in a single observation.
        """
        logger.info("start estimate '{}'".format(self.name))
        #logger.info('parsetDict: %s ' % parsetDict)
        parset = parameterset(parset).makeSubset('Observation.')
        logger.info('parset: %s ' % parset)
        duration = _getDuration(parset.getString('startTime'), parset.getString('stopTime'))
        
        correlated_size, correlated_bandwidth, correlated_files = self.correlated(parset, duration)
        coherentstokes_size, coherentstokes_bandwidth, coherentstokes_files = self.coherentstokes(parset, duration)
        incoherentstokes_size, incoherentstokes_bandwith, incoherentstokes_files = self.incoherentstokes(parset, duration)
        
        result = {}
        result['total_data_size'] = correlated_size + coherentstokes_size + incoherentstokes_size
        result['total_bandwidth'] = correlated_bandwidth + coherentstokes_bandwidth + incoherentstokes_bandwith
        result['output_files'] = {}
        result['output_files'].append(correlated_files)
        result['output_files'].append(coherentstokes_files)
        result['output_files'].append(incoherentstokes_files)
        return result

    def correlated(self, parset, duration):
        """ Estimate number of files, file size and bandwidth needed for correlated data"""
        logger.info("calculating correlated datasize")
        output_files = {}
        size_of_header   = 512 #TODO More magic numbers (probably from Alwin). ScS needs to check these. They look ok though.
        size_of_overhead = 600000
        size_of_short    = 2
        size_of_complex  = 8
        nr_polarizations = 2
        channels_per_subband = parset.getInt(COBALT + 'BeamFormer.CoherentStokes.nrChannelsPerSubband', 64)
        intergration_time = parset.getFloat(COBALT + 'Correlator.integrationTime', 1)
        nr_of_virtual_stations = self._virtual_stations()

        integrated_seconds = floor(duration / intergration_time)
        nr_baselines = no_virtual_stations * (nr_of_virtual_stations + 1.0) / 2.0 #Why is this done in float?
        data_size = ceil((nr_baselines * channels_per_subband * nr_polarizations**2 * size_of_complex) / 512.0) * 512.0
        n_sample_size = ceil((nr_baselines * channels_per_subband * size_of_short) / 512.0) * 512.0

        # sum of all subbands in all digital beams
        nr_subbands = 0
        for beam_nr in xrange(parset.getInt(Observation.nrBeams)):
            subbandList = parset.getStringVector('Beam[%d].subbandList' % beam_nr)
            nr_subbands += len(subbandList)

        file_size = (data_size + n_sample_size + size_of_header) * integrated_seconds + size_of_overhead
        output_files['correlated_uv'] = {'nr_files': nr_subbands, 'file_size': int(file_size)}
        logger.info("correlated_uv: {} files {} bytes each".format(nr_subbands, int(file_size)))

        total_data_size += ceil(file_size * nr_subbands)  # bytes
        total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second
        return (total_data_size, total_bandwidth, output_files)

    def coherentstokes(self, parset, duration):
        """  Estimate number of files, file size and bandwidth needed for coherent stokes
        """
        if not parset.getBool([DataProducts.Output_CoherentStokes.enabled) == 'false':
            return (0,0, {"bf_coherentstokes": {'nr_files': 0, 'file_size': 0}})
            
        logger.info("calculate coherentstokes datasize")
        coherent_type = parset.getString('DataProducts.Output_CoherentStokes.type')
        subbands_per_file = parset.getInt(COBALT + 'BeamFormer.CoherentStokes.subbandsPerFile') ##TODO 512.0
        samples_per_second = self._samples_per_second()
        integration_factor = parset.getInt(COBALT + 'BeamFormer.CoherentStokes.timeIntegrationFactor')

        nr_coherent = 4 if coherent_type in ('DATA_TYPE_XXYY', 'DATA_TYPE_STOKES_IQUV') else 1
        if coherent_type in ('DATA_TYPE_XXYY',):
            size_per_subband = samples_per_second * 4.0 * duration
        else:
            size_per_subband = (samples_per_second * 4.0 * duration) / integration_factor
        total_files_summed = 0
        total_files = 0

        max_nr_subbands = 0
        for sap_nr in xrange(parset.getInt('Observation.nrBeams')):
            logger.info("checking SAP {}".format(sap_nr))
            subbandList = parset.getStringVector('Beam[%d].subbandList' % sap_nr)
            nr_subbands = len(subbandList)
            max_nr_subbands = max(nr_subbands, max_nr_subbands)
            for tab_nr in xrange(parset.getInt('Observation.Beam[%d].nrTiedArrayBeams' % sap_nr)):
                logger.info("checking TAB {}".format(tab_nr))
                if parset.getBool("Observation.Beam[%d].TiedArrayBeam[%d].coherent" % (sap_nr, tab_nr)):
                    logger.info("adding coherentstokes size")
                    total_files_min = nr_coherent #TODO what does min mean here?
                    total_files_summed += total_files_min
                    total_files += total_files_min * ceil(nr_subbands / subbands_per_file)

            nr_tab_rings = parset.getInt('Observation.Beam[%d].nrTabRings' % sap_nr)
            if nr_tab_rings > 0:
                logger.info("adding size for {} tab_rings".format(nr_tab_rings))
                total_files_min = (3 * nr_tab_rings * (nr_tab_rings + 1) + 1) * nr_coherent
                total_files_summed += total_files_min
                total_files += total_files_min * ceil(nr_subbands / subbands_per_file)

            if parset.getBool(COBALT + 'BeamFormer.flysEye'):
                logger.info("adding flys eye data size")
                total_files_min = self._virtual_stations() * nr_coherent
                total_files_summed += total_files_min
                total_files += total_files_min * ceil(nr_subbands / subbands_per_file)

        nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
        size_per_file = nr_subbands_per_file * size_per_subband

        output_files['bf_coherentstokes'] = {'nr_files': int(total_files), 'file_size': int(size_per_file)}
        logger.info("coherentstokes: {} files {} bytes each".format(int(total_files), int(size_per_file)))

        total_data_size = ceil(total_files_summed * max_nr_subbands * size_per_subband)
        total_bandwidth = ceil((self.total_data_size * 8) / duration)  # bits/second
        return (total_data_size, total_bandwidth, output_files)

    def incoherentstokes(self, parset, duration):
        """  Estimate number of files, file size and bandwidth needed for incoherentstokes
        """
        if not parset.getBool('DataProducts.Output_IncoherentStokes.enabled'):
            return (0,0, {"bf_coherentstokes": {'nr_files': 0, 'file_size': 0}})
            
        logger.info("calculate incoherentstokes data size")
        incoherent_type = self.parset['output']['inCoherentStokes']['type']
        subbands_per_file = 512.0
        samples_per_second = self._samples_per_second(parset)
        time_integration_factor = self.parset.get('incoherent_time_integration_factor', 1)
        channels_per_subband = int(self.parset.get('channels_per_subband', 64))
        incoherent_channels_per_subband = self.parset.get('incoherent_channels_per_subband', 0)

        total_files_summed = 0
        total_files = 0

        nr_incoherent = 4 if incoherent_type in ('DATA_TYPE_STOKES_IQUV',) else 1

        max_nr_subbands = 0
TODO        for sap_nr in xrange(parset.getInt('Observation.nrBeams')):
            logger.info("checking SAP {}".format(sap_nr))
            subbandList = parset.getStringVector('Beam[%d].subbandList' % sap_nr)
            nr_subbands = len(subbandList)
            max_nr_subbands = max(nr_subbands, max_nr_subbands)
            for tab_nr in xrange(parset.getInt('Observation.Beam[%d].nrTiedArrayBeams' % sap_nr)):
                logger.info("checking TAB {}".format(tab_nr))
                if parset.getBool("Observation.Beam[%d].TiedArrayBeam[%d].coherent" % (sap_nr, tab_nr)):
                    logger.info("adding coherentstokes size")
                    total_files_min = nr_coherent #TODO what does min mean here?
                    total_files_summed += total_files_min
                    total_files += total_files_min * ceil(nr_subbands / subbands_per_file)
#         while 'beam[{}]'.format(beam_nr) in self.parset:
#             beam = self.parset['beam[{}]'.format(beam_nr)]
#             logger.info("add beam {}".format(beam_nr))
#             max_nr_subbands = max(max_nr_subbands, int(beam['nr_subbands']))
#             tied_array_beam_nr = 0
#             while 'tied_array_beam[{}]'.format(tied_array_beam_nr) in beam:
#                 tied_array_beam = beam['tied_array_beam[{}]'.format(tied_array_beam_nr)]
#                 logger.info("add tied_array_beam {}".format(tied_array_beam_nr))
#                 if tied_array_beam['coherent'] == 'false':
#                     total_files_summed += nr_incoherent
#                     total_files += nr_incoherent * ceil(int(beam['nr_subbands']) / subbands_per_file)
#                 tied_array_beam_nr += 1
#             beam_nr += 1

        if incoherent_channels_per_subband > 0:
            channel_integration_factor = channels_per_subband / incoherent_channels_per_subband
        else:
            channel_integration_factor = 1

        if total_files > 0:
            nr_subbands_per_file = min(subbands_per_file, max_nr_subbands)
            size_per_subband = (samples_per_second * 4) / time_integration_factor / channel_integration_factor * duration
            size_per_file = nr_subbands_per_file * size_per_subband

            output_files['dp_inCoherentStokes'] = {'nr_files': int(total_files), 'file_size': int(size_per_file)}
            logger.info("dp_inCoherentStokes: {} files {} bytes each".format(int(total_files), int(size_per_file)))

            total_data_size = ceil(total_files_summed * max_nr_subbands * size_per_subband)  # bytes
            total_bandwidth = ceil((total_data_size * 8) / duration)  # bits/sec
        return (total_data_size, total_bandwidth, output_files)

    def _samples_per_second(self, parset):
        """ set samples per second
        """
        samples_160mhz = 155648
        samples_200mhz = 196608
        samples = samples_160mhz if '160' in parset['sample_clock'] else samples_200mhz
        logger.info("samples per second for {} MHz clock = {}".format(self.parset['sample_clock'], samples))
        return samples

    def _virtual_stations(self, parset):
        """ calculate virtualnumber of stations
        """
        stationList = parset.getStringVector('VirtualInstrument.stationList')
        nr_virtual_stations = 0
        if parset['antennaSet'] in ('HBA_DUAL', 'HBA_DUAL_INNER'):
            for station in stationList:
                if 'CS' in station:
                    nr_virtual_stations += 2
                else:
                    nr_virtual_stations += 1
        else:
            nr_virtual_stations = len(stationList)
        logger.info("number of virtual stations = {}".format(nr_virtual_stations))
        return nr_virtual_stations

