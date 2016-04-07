#!/usr/bin/env python

import unittest
import sys
import datetime
import logging
import inspect
from lofar.messaging.RPC import RPC

logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger = logging.getLogger(__name__)

try:
    from mock import MagicMock
    from mock import patch
except ImportError:
    print 'Cannot run test without python MagicMock'
    print 'Please install MagicMock: pip install mock'
    exit(3)

# the system under test is the ResourceAssigner, not the RARPC
# so, patch (mock) the RARPC class during these tests.
# when the ResourceAssigner instantiates an RARPC it will get the mocked class.
with patch('lofar.sas.resourceassignment.resourceassignmentservice.rpc.RARPC', autospec=True) as MockRARPC, \
    patch.object(RPC, 'execute') as mockRPC_execute, \
    patch.object(RPC, 'open'), \
    patch.object(RPC, 'close'):
    mockRARPC = MockRARPC.return_value

    # modify the return values of the various RARPC methods with pre-cooked answers
    mockRARPC.getTask.return_value = {"endtime": datetime.datetime(2016, 3, 25, 22, 47, 31), "id": 2299, "mom_id": 351543, "name": "IS HBA_DUAL", "otdb_id": 1290472, "predecessor_ids": [], "project_mom_id": 2, "project_name": "test-lofar", "specification_id": 2323, "starttime": datetime.datetime(2016, 3, 25, 21, 47, 31), "status": "prescheduled", "status_id": 350, "successor_ids": [], "type": "observation", "type_id": 0}
    mockRARPC.getResourceTypes.return_value = [{"id": 0, "name": "rsp", "unit": "rsp_channel_bit", "unit_id": 0}, {"id": 1, "name": "tbb", "unit": "bytes", "unit_id": 1}, {"id": 2, "name": "rcu", "unit": "rcu_board", "unit_id": 2}, {"id": 3, "name": "bandwidth", "unit": "bytes/second", "unit_id": 3}, {"id": 4, "name": "processor", "unit": "cores", "unit_id": 4}, {"id": 5, "name": "storage", "unit": "bytes", "unit_id": 1}]
    mockRARPC.getResources.return_value = [{"id": 116, "name": "cep4bandwidth", "type": "bandwidth", "type_id": 3, "unit": "bytes/second"},{"id": 117, "name": "cep4storage", "type": "storage", "type_id": 5, "unit": "bytes"}]
    mockRARPC.getResourceClaimPropertyTypes.return_value = [{"id": 0, "name": "nr_of_is_files"}, {"id": 1, "name": "nr_of_cs_files"}, {"id": 2, "name": "nr_of_uv_files"}, {"id": 3, "name": "nr_of_im_files"}, {"id": 4, "name": "nr_of_cores"}, {"id": 5, "name": "nr_of_beamlets"}, {"id": 6, "name": "nr_of_bits"}, {"id": 7, "name": "is_file_size"}]

    def mockRARPC_insertResourceClaims(*arg, **kwarg):
        logger.info("insertResourceClaims: %s" % ', '.join(str(x) for x in arg))
        return {'ids':range(len(arg[1]))}
    mockRARPC.insertResourceClaims.side_effect = mockRARPC_insertResourceClaims

    #mock the RPC execute method
    def mockRPCExecute(*arg, **kwarg):
        #trick to get the servicename via the callstack from within this mock method
        servicename = inspect.stack()[3][0].f_locals['self'].ServiceName
        logger.info("mockRPCExecute servicename=%s" % servicename)

        #give pre-cooked answer depending on called service
        if servicename == 'ResourceEstimation':
            return {'1290472': {'observation': {'bandwidth': {'total_size': 9372800}, 'storage': {'total_size': 140592000, 'output_files': {'is': {'is_nr_stokes': 1, 'is_file_size': 36864000, 'nr_of_is_files': 1}, 'uv': {'nr_of_uv_files': 50, 'uv_file_size': 2074560}, 'saps': [{'sap_nr': 0, 'properties': {'nr_of_uv_files': 50, 'nr_of_is_files': 1}}]}}}}}, "OK"
        elif servicename == 'SSDBService.GetActiveGroupNames':
            return {0:'storagenodes', 1:'computenodes', 2:'archivenodes', 3:'locusnodes', 4:'cep4'}, "OK"
        elif servicename == 'SSDBService.GetHostForGID':
            return {u'groupname': u'cep4', u'nodes': [{u'claimedspace': 0, u'totalspace': 702716, u'statename': u'Active', u'usedspace': 23084, u'id': 1, u'groupname': u'cep4', u'path': u'/lustre', u'hostname': u'lustre001'}]}, "OK"

        return None, None

    mockRPC_execute.side_effect = mockRPCExecute

    # import ResourceAssigner now, so it will use the mocked classes and methods
    from lofar.sas.resourceassignment.resourceassigner.assignment import ResourceAssigner

    #define the test class
    class ResourceAssignerTest(unittest.TestCase):
        '''Test the logic in the ResourceAssigner'''

        #def test_doAssignment(self):
            #with ResourceAssigner() as assigner:
                ##define inputs
                #specification_tree={u'task_type': u'pipeline', u'specification': {u'Observation.DataProducts.Output_InstrumentModel.enabled': False, u'Observation.stopTime': u'2016-03-25 14:15:37', u'Observation.VirtualInstrument.stationList': [], u'Observation.DataProducts.Input_CoherentStokes.enabled': False, u'Observation.DataProducts.Output_CoherentStokes.enabled': False, u'Observation.DataProducts.Input_Correlated.skip': [0, 0, 0, 0], u'Observation.antennaSet': u'LBA_INNER', u'Observation.nrBitsPerSample': u'16', u'Observation.ObservationControl.PythonControl.LongBaseline.subbandgroups_per_ms': u'2', u'Observation.DataProducts.Output_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_Correlated.enabled': True, u'Observation.DataProducts.Output_Pulsar.enabled': False, u'Observation.DataProducts.Input_CoherentStokes.skip': [], u'Observation.DataProducts.Output_SkyImage.enabled': False, u'Version.number': u'33774', u'Observation.momID': u'351557', u'Observation.startTime': u'2016-03-25 14:14:57', u'Observation.ObservationControl.PythonControl.LongBaseline.subbands_per_subbandgroup': u'2', u'Observation.nrBeams': u'0', u'Observation.DataProducts.Input_IncoherentStokes.skip': [], u'Observation.DataProducts.Output_Correlated.enabled': True, u'Observation.sampleClock': u'200'}, u'task_subtype': u'long baseline pipeline', u'state': u'prescheduled', u'otdb_id': 1290494, u'predecessors': [{u'task_subtype': u'averaging pipeline', u'specification': {u'Observation.DataProducts.Output_InstrumentModel.enabled': False, u'Observation.stopTime': u'2016-03-25 13:51:05', u'Observation.VirtualInstrument.stationList': [], u'Observation.DataProducts.Input_CoherentStokes.enabled': False, u'Observation.DataProducts.Output_CoherentStokes.enabled': False, u'Observation.DataProducts.Output_SkyImage.enabled': False, u'Observation.DataProducts.Input_Correlated.skip': [0, 0, 0, 0], u'Observation.antennaSet': u'LBA_INNER', u'Observation.nrBitsPerSample': u'16', u'Observation.ObservationControl.PythonControl.LongBaseline.subbandgroups_per_ms': u'1', u'Observation.DataProducts.Output_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_Correlated.enabled': True, u'Observation.DataProducts.Output_Pulsar.enabled': False, u'Observation.DataProducts.Input_CoherentStokes.skip': [], u'Observation.ObservationControl.PythonControl.DPPP.demixer.demixtimestep': u'10', u'Version.number': u'33774', u'Observation.momID': u'351556', u'Observation.startTime': u'2016-03-25 13:49:55', u'Observation.ObservationControl.PythonControl.LongBaseline.subbands_per_subbandgroup': u'1', u'Observation.nrBeams': u'0', u'Observation.DataProducts.Input_IncoherentStokes.skip': [], u'Observation.ObservationControl.PythonControl.DPPP.demixer.demixfreqstep': u'64', u'Observation.DataProducts.Output_Correlated.enabled': True, u'Observation.sampleClock': u'200'}, u'task_type': u'pipeline', u'otdb_id': 1290496, u'predecessors': [{u'task_subtype': u'bfmeasurement', u'specification': {u'Observation.DataProducts.Output_InstrumentModel.enabled': False, u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.timeIntegrationFactor': u'1', u'Observation.stopTime': u'2016-03-26 00:33:31', u'Observation.VirtualInstrument.stationList': [u'RS205', u'RS503', u'CS013', u'RS508', u'RS106'], u'Observation.DataProducts.Input_CoherentStokes.enabled': False, u'Observation.DataProducts.Output_CoherentStokes.enabled': False, u'Observation.ObservationControl.OnlineControl.Cobalt.Correlator.nrChannelsPerSubband': u'64', u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.which': u'I', u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.which': u'I', u'Observation.Beam[0].subbandList': [100, 101, 102, 103], u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.IncoherentStokes.subbandsPerFile': u'512', u'Observation.DataProducts.Input_Correlated.skip': [], u'Observation.antennaSet': u'HBA_DUAL', u'Observation.nrBitsPerSample': u'8', u'Observation.Beam[0].nrTabRings': u'0', u'Observation.Beam[0].nrTiedArrayBeams': u'0', u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.flysEye': False, u'Observation.nrBeams': u'1', u'Observation.ObservationControl.OnlineControl.Cobalt.Correlator.integrationTime': u'1.0', u'Observation.DataProducts.Output_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_Correlated.enabled': False, u'Observation.DataProducts.Output_Pulsar.enabled': False, u'Observation.DataProducts.Input_CoherentStokes.skip': [], u'Observation.DataProducts.Output_SkyImage.enabled': False, u'Version.number': u'33774', u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.timeIntegrationFactor': u'1', u'Observation.momID': u'351539', u'Observation.startTime': u'2016-03-26 00:31:31', u'Observation.ObservationControl.OnlineControl.Cobalt.BeamFormer.CoherentStokes.subbandsPerFile': u'512', u'Observation.DataProducts.Input_IncoherentStokes.skip': [], u'Observation.DataProducts.Output_Correlated.enabled': True, u'Observation.sampleClock': u'200'}, u'task_type': u'observation', u'otdb_id': 1290476, u'predecessors': []}]}]}

                ##test the main assignment method
                #assigner.doAssignment(specification_tree)

                ##TODO: added test asserts etc

        def test_claimResources(self):
            with ResourceAssigner() as assigner:
                #define inputs
                estimator_output,_=assigner.rerpc()
                needed_resources=estimator_output['1290472']
                task = assigner.radbrpc.getTask(1290472)

                #test claimResources method
                assigner.claimResources(needed_resources, task)

                #TODO: added test asserts etc

    unittest.main(verbosity=2)
