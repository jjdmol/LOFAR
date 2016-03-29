#!/usr/bin/env python

import unittest
import sys
import datetime
import logging
import inspect
from lofar.messaging.RPC import RPC

logger = logging.getLogger(__name__)
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

try:
    from mock import MagicMock
    from mock import patch
except ImportError:
    print 'Cannot run test without python MagicMock'
    print 'Please install MagicMock: pip install mock'
    exit(3)


print 'TODO: fix test'
exit(3)

# the system under test is the ResourceAssigner, not the RARPC
# so, patch (mock) the RARPC class during these tests.
# when the ResourceAssigner instantiates an RARPC it will get the mocked class.
with patch('lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.rpc.RARPC', autospec=True) as MockRARPC, \
    patch.object(RPC, 'execute') as mockRPC_execute, \
    patch.object(RPC, 'open'), \
    patch.object(RPC, 'close'):
    mockRARPC = MockRARPC.return_value

    # modify the return values of the various RARPC methods with pre-cooked answers
    mockRARPC.getTask.return_value = {u'status': u'active', u'status_id': 600, u'type_id': 0, u'specification_id': 8, u'starttime': datetime.datetime(2016, 2, 14, 20, 0), u'mom_id': 634163, u'endtime': datetime.datetime(2016, 2, 14, 21, 30), u'type': u'Observation', u'id': 9355, u'otdb_id': 431140}

    #mock the RPC execute method
    def mockRPCExecute(*arg, **kwarg):
        #trick to get the servicename via the callstack from within this mock method
        servicename = inspect.stack()[3][0].f_locals['self'].ServiceName

        #give pre-cooked answer depending on called service
        if servicename == 'ResourceEstimator':
            return {'Observation':{'total_data_size':1, 'total_bandwidth':1, 'output_files':1}}, "OK"
        elif servicename == 'SSDBService.GetActiveGroupNames':
            return {0:'storagenodes', 1:'computenodes', 2:'archivenodes', 3:'locusnodes', 4:'cep4'}, "OK"
        elif servicename == 'SSDBService.GetHostForGID':
            return {u'groupname': u'cep4', u'nodes': [{u'claimedspace': 0, u'totalspace': 702716, u'statename': u'Active', u'usedspace': 23084, u'id': 1, u'groupname': u'cep4', u'path': u'/lustre', u'hostname': u'lustre001'}]}, "OK"

        return None, None

    mockRPC_execute.side_effect = mockRPCExecute

    ## import ResourceAssigner now, so it will use the mocked classes and methods
    #from lofar.sas.resourceassignment.ratootdbtaskspecificationpropagator.? import ?

    ##define the test class
    #class RAtoOTDBTaskSpecificationPropagatorTest(unittest.TestCase):
        #'''Test the logic in the RAtoOTDBTaskSpecificationPropagator'''

        #def testRAtoOTDBTaskSpecificationPropagator(self):
            #with RAtoOTDBTaskSpecificationPropagator() as rotsp:
                ##define inputs
                #sasId='431140'
                #parsets={u'431140': {u'Observation.DataProducts.Output_InstrumentModel.enabled': False, u'Observation.stopTime': u'2016-02-14 21:30:00', u'Observation.VirtualInstrument.stationList': [u'CS005', u'CS001', u'CS011', u'CS401', u'CS002', u'CS007', u'CS201', u'CS032', u'CS003', u'CS101', u'CS028', u'CS017', u'CS024', u'CS103', u'CS026', u'CS501', u'CS031', u'CS301', u'CS030', u'CS302', u'CS004', u'CS006', u'CS021'], u'Observation.DataProducts.Input_CoherentStokes.enabled': False, u'Observation.DataProducts.Output_CoherentStokes.enabled': True, u'Task.type': u'Observation', u'Observation.Beam[0].subbandList': [51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450], u'Observation.DataProducts.Input_Correlated.skip': [], u'Observation.antennaSet': u'LBA_OUTER', u'Observation.nrBitsPerSample': u'8', u'Observation.Beam[0].nrTabRings': u'0', u'Version.number': u'33385', u'Observation.DataProducts.Output_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_IncoherentStokes.enabled': False, u'Observation.DataProducts.Input_Correlated.enabled': False, u'Observation.Beam[0].TiedArrayBeam[0].coherent': True, u'Observation.DataProducts.Output_Pulsar.enabled': False, u'Observation.DataProducts.Input_CoherentStokes.skip': [], u'Observation.DataProducts.Output_SkyImage.enabled': False, u'Task.subtype': u'BFMeasurement', u'Observation.momID': u'634163', u'Observation.startTime': u'2016-02-14 20:00:00', u'Observation.nrBeams': u'1', u'Observation.DataProducts.Input_IncoherentStokes.skip': [], u'Observation.DataProducts.Output_Correlated.enabled': False, u'Observation.sampleClock': u'200'}}

                ##test the main assignment method
                #rotsp.do?(sasId, parsets)

                ##TODO: added test asserts etc

    #unittest.main(verbosity=2)
