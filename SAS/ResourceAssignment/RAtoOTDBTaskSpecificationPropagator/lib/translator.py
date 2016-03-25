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
        locations  = []
        filesnames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_uv_files" in sap:
                for sap['nr_of_uv_files']:
                    locations.append("CEP4:/data/projects/test/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_uv.MS" % (otdb_id, sb_nr, sap['sap_nr']))
        result[PREFIX + 'DataProducts.Output_Correlated.locations'] = to_csv_string(locations)
        result[PREFIX + 'DataProducts.Output_Correlated.filenames'] = to_csv_string(filenames)

    def CreateCoherentStokes(self, otdb_id, storage_properties):
        SB_nr = 0
        locations  = []
        filesnames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_cs_files" in sap:
                for sap['nr_of_cs_files']:
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_bf.h5" % (otdb_id, sb_nr, sap['sap_nr']))
        result[PREFIX + 'DataProducts.Output_CoherentStokes.locations'] = to_csv_string(locations)
        result[PREFIX + 'DataProducts.Output_CoherentStokes.filenames'] = to_csv_string(filenames)

    def CreateIncoherentStokes(self, otdb_id, storage_properties):
        SB_nr = 0
        locations  = []
        filesnames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_is_files" in sap:
                for sap['nr_of_is_files']:
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_bf.h5" % (otdb_id, sb_nr, sap['sap_nr']))
        result[PREFIX + 'DataProducts.Output_IncoherentStokes.locations'] = to_csv_string(locations)
        result[PREFIX + 'DataProducts.Output_IncoherentStokes.filenames'] = to_csv_string(filenames)

    def CreateCreateInstrumentModel(self, otdb_id, storage_properties):
        SB_nr = 0
        locations  = []
        filesnames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_im_files" in sap:
                for sap['nr_of_im_files']:
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_inst.INST" % (otdb_id, sb_nr, sap['sap_nr']))
        result[PREFIX + 'DataProducts.Output_InstrumentModel.locations'] = to_csv_string(locations)
        result[PREFIX + 'DataProducts.Output_InstrumentModel.filenames'] = to_csv_string(filenames)

    def CreateSkyImage(self, otdb_id, storage_properties):
        SB_nr = 0
        locations  = []
        filesnames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_img_files" in sap:
                for sap['nr_of_img_files']:
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_sky.IM" % (otdb_id, sb_nr, sap['sap_nr']))
        result[PREFIX + 'DataProducts.Output_SkyImage.locations'] = to_csv_string(locations)
        result[PREFIX + 'DataProducts.Output_SkyImage.filenames'] = to_csv_string(filenames)

    def CreatePulsarPipeline(self, otdb_id, storage_properties):
        SB_nr = 0
        locations  = []
        filesnames = []
        result = {}
        for sap in storage_properties["saps"]: ##We might need to sort saps?
            if "nr_of_uv_files" in sap:
                for sap['nr_of_uv_files']:
                    locations.append("CEP4:/data/projects/project/L%d" % otdb_id)
                    filenames.append("L%d_SAP%03d_SB%03d_bf.h5" % (otdb_id, sb_nr, sap['sap_nr']))
        result[PREFIX + 'DataProducts.Output_Pulsar.locations'] = to_csv_string(locations)
        result[PREFIX + 'DataProducts.Output_Pulsar.filenames'] = to_csv_string(filenames)


    def CreateStorageKeys(self, otdb_id, storage_properties):
        result = {}
        for property in storage_properties:
            if property = 'uv':
                result.update(CreateCorrelated(otdb_id, storage_properties))
            if property = 'cs':
                result.update(CreateCoherentStokes(otdb_id, storage_properties))
            if property = 'is':
                result.update(CreateIncoherentStokes(otdb_id, storage_properties))
            if property = 'im':
                result.update(CreateInstrumentModel(otdb_id, storage_properties))
            if property = 'img':
                result.update(CreateSkyImage(otdb_id, storage_properties))
            if property = 'pulp':
                result.update(CreatePulsarPipeline(otdb_id, storage_properties))

    def CreateParset(self, otdb_id, ra_info):
        logger.info('CreateParset: start=%s, end=%s' % (ra_info['starttime'], ra_info['endtime']))

        parset = {}
        #parset[PREFIX+'momID'] = str(mom_id)
        parset[PREFIX+'startTime'] = ra_info['starttime'].strftime('%Y-%m-%d %H:%M:%S')
        parset[PREFIX+'stopTime'] = ra_info['endtime'].strftime('%Y-%m-%d %H:%M:%S')

        if 'storage' in ra_info:
            parset.update(CreateStorageKeys(ra_info['storage']['properties']))
        if 'stations' in ra_info:
            parset[PREFIX+'VirtualInstrument.stationList'] = ra_info["stations"]
        return parset






