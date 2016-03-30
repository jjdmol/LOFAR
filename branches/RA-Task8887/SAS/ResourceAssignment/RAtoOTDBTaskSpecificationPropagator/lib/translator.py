#!/usr/bin/env python

# Copyright (C) 2015
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id: assignment.py 1580 2015-09-30 14:18:57Z loose $

"""
RAtoOTDBTaskSpecificationPropagator gets a task to be scheduled in OTDB,
reads the info from the RA DB and sends it to OTDB in the correct format.
"""

import logging
from lofar.common.util import to_csv_string
from math import ceil, floor

#from lofar.parameterset import parameterset

logger = logging.getLogger(__name__)

""" Prefix that is common to all parset keys, depending on the exact source. """
PREFIX="LOFAR.ObsSW.Observation."
##TODO use this.

class RAtoOTDBTranslator():
    def __init__(self):
        """
        RAtoOTDBTranslator translates values from the RADB into parset keys to be stored in an OTDB Tree
        """

    def CreateCorrelated(self, otdb_id, storage_properties):
        sb_nr = 0
        locations = []
        filenames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            logging.debug('processing sap: %s' % sap)
            if "nr_of_uv_files" in sap['properties']:
                for _ in xrange(sap['properties']['nr_of_uv_files']):
                    locations.append("CEP4:/data/projects/test/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_uv.MS" % (otdb_id, sap['sap_nr'], sb_nr))
                    sb_nr += 1
        result[PREFIX + 'DataProducts.Output_Correlated.locations'] = '[' + to_csv_string(locations) + ']'
        result[PREFIX + 'DataProducts.Output_Correlated.filenames'] = '[' + to_csv_string(filenames) + ']'
        return result

    def CreateCoherentStokes(self, otdb_id, storage_properties):
        SB_nr = 0
        locations = []
        filenames = []
        result = {}
        nr_stokes = storage_properties['nr_of_cs_stokes']
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_cs_files" in sap['properties']:
                nr_files = sap['properties']['nr_of_cs_files']
                nr_tabs  = sap['properties']['nr_of_tabs']
                nr_parts = int(ceil(nr_files/float(nr_tabs * nr_stokes)))
                for tab in xrange(nr_tabs):
                    for stokes in xrange(nr_stokes):
                        for part in xrange(nr_parts):
                            locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                            filenames.append("L%d_SAP%03d_B%03d_S%d_P%03d_bf.h5" % (otdb_id, sap['sap_nr'], tab, stokes, part))
        result[PREFIX + 'DataProducts.Output_CoherentStokes.locations'] = '[' + to_csv_string(locations) + ']'
        result[PREFIX + 'DataProducts.Output_CoherentStokes.filenames'] = '[' + to_csv_string(filenames) + ']'
        return result

    def CreateIncoherentStokes(self, otdb_id, storage_properties):
        SB_nr = 0
        locations = []
        filenames = []
        result = {}
        nr_stokes = storage_properties['nr_of_is_stokes']
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_is_files" in sap['properties']:
                nr_files = sap['properties']['nr_of_is_files']
                nr_tabs  = sap['properties']['nr_of_tabs']
                nr_parts = int(ceil(nr_files/float(nr_tabs * nr_stokes)))
                for tab in xrange(nr_tabs):
                    for stokes in xrange(nr_stokes):
                        for part in xrange(nr_parts):
                            locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                            filenames.append("L%d_SAP%03d_B%03d_S%d_P%03d_bf.h5" % (otdb_id, sap['sap_nr'], tab, stokes, part))
        result[PREFIX + 'DataProducts.Output_IncoherentStokes.locations'] = '[' + to_csv_string(locations) + ']'
        result[PREFIX + 'DataProducts.Output_IncoherentStokes.filenames'] = '[' + to_csv_string(filenames) + ']'
        return result

    def CreateCreateInstrumentModel(self, otdb_id, storage_properties):
        SB_nr = 0
        locations = []
        filenames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_im_files" in sap['properties']:
                for _ in range(sap['properties']['nr_of_im_files']):
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_inst.INST" % (otdb_id, sap['sap_nr'], sb_nr))
        result[PREFIX + 'DataProducts.Output_InstrumentModel.locations'] = '[' + to_csv_string(locations) + ']'
        result[PREFIX + 'DataProducts.Output_InstrumentModel.filenames'] = '[' + to_csv_string(filenames) + ']'
        return result

    def CreateSkyImage(self, otdb_id, storage_properties):
        SB_nr = 0
        locations = []
        filenames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_img_files" in sap['properties']:
                for _ in range(sap['properties']['nr_of_img_files']):
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_sky.IM" % (otdb_id, sap['sap_nr'], sb_nr))
        result[PREFIX + 'DataProducts.Output_SkyImage.locations'] = '[' + to_csv_string(locations) + ']'
        result[PREFIX + 'DataProducts.Output_SkyImage.filenames'] = '[' + to_csv_string(filenames) + ']'
        return result

    def CreatePulsarPipeline(self, otdb_id, storage_properties):
        SB_nr = 0
        locations = []
        filenames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_uv_files" in sap['properties']:
                for _ in range(sap['properties']['nr_of_pulp_files']):
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_bf.h5" % (otdb_id, sap['sap_nr'], sb_nr))
        result[PREFIX + 'DataProducts.Output_Pulsar.locations'] = '[' + to_csv_string(locations) + ']'
        result[PREFIX + 'DataProducts.Output_Pulsar.filenames'] = '[' + to_csv_string(filenames) + ']'
        return result


    def CreateStorageKeys(self, otdb_id, storage_properties):
        logging.debug(otdb_id, storage_properties)
        result = {}
        if 'nr_of_uv_files' in storage_properties:
            result.update(self.CreateCorrelated(otdb_id, storage_properties))
        if 'nr_of_cs_files' in storage_properties:
            result.update(self.CreateCoherentStokes(otdb_id, storage_properties))
        if 'nr_of_is_files' in storage_properties:
            result.update(self.CreateIncoherentStokes(otdb_id, storage_properties))
        if 'nr_of_im_files' in storage_properties:
            result.update(self.CreateInstrumentModel(otdb_id, storage_properties))
        if 'nr_of_img_files' in storage_properties:
            result.update(self.CreateSkyImage(otdb_id, storage_properties))
        if 'nr_of_pulp_files' in storage_properties:
            result.update(self.CreatePulsarPipeline(otdb_id, storage_properties))
        return result

    def parseStorageProperties(self, storage_claim):
        result = {}
        result['saps'] = []
        for s in storage_claim['saps']:
            properties = {}
            for p in s['properties']:
                properties[p['type_name']] = p['value']
            result['saps'].append({'sap_nr' : s['sap_nr'], 'properties': properties})
        for p in storage_claim['properties']:
            result[p['type_name']] = p['value']
        return result

    def CreateParset(self, otdb_id, ra_info):
        logger.info('CreateParset: start=%s, end=%s' % (ra_info['starttime'], ra_info['endtime']))

        parset = {}
        #parset[PREFIX+'momID'] = str(mom_id)
        parset[PREFIX+'startTime'] = ra_info['starttime'].strftime('%Y-%m-%d %H:%M:%S')
        parset[PREFIX+'stopTime'] = ra_info['endtime'].strftime('%Y-%m-%d %H:%M:%S')

        if 'storage' in ra_info:
            logging.debug(ra_info['storage'])
            parset.update(self.CreateStorageKeys(otdb_id, self.parseStorageProperties(ra_info['storage'])))
        if 'stations' in ra_info:
            parset[PREFIX+'VirtualInstrument.stationList'] = ra_info["stations"]
        return parset






