#!/usr/bin/env python

import unittest
import logging
import LTAIngest.ingestpipeline as ingestpipeline
from LTAIngest.job_parser import JobScheduled
from LTAIngest.unspecifiedSIP import makeSIP

class TestIngestPipeline(unittest.TestCase):
  """Tests for various IngestPipeline methods"""

  def testSIPContents(self):
    """test CheckSIPContent method with valid and invalid SIP"""

    #logging
    logger = logging.getLogger('Slave')
    logger.addHandler(logging.StreamHandler())
    logger.setLevel(logging.DEBUG)

    #create mock job
    job = {}
    job['ExportID'] = '0000'
    job['JobId'] = 'X_0000_0000_1234567_12345678_L123456_SB000_uv.MS'
    job['Status'] = JobScheduled
    job['DataProduct'] = 'L123456_SB000_uv.MS'
    job['FileName'] = job['DataProduct'] + '.tar'
    job['MomId'] = 12345678
    job['ObservationId'] = 123456
    job["Type"] = 'MoM'
    job['Project'] = 'test'
    job['Location'] = 'locus099:/data/L123456/L123456_SB000_uv.MS'
    
    #create mock storage ticket
    ticket = 'ABCDEF123456789'
    
    #create mock momClient
    class MockMoMClient():
      pass    
    momClient = MockMoMClient()

    #create mock ltaClient
    class MockLTAClient():
      pass
    ltaClient = MockLTAClient()
    
    #create misc mock args
    ltacphost = 'mock-ltacphost'
    ltacpport = -1
    mailCommand = ''
    srmInit = ''

    #create pipeline instance with mock info
    pipeline =  ingestpipeline.IngestPipeline('/tmp', job, momClient, ltaClient, ltacphost, ltacpport, mailCommand, 1, 1, 1, srmInit)

    #end of setup
    #start testing

    #create proper SIP with valid ticket, momid and filename
    pipeline.SIP = makeSIP(job['Project'], job['ObservationId'], job['MomId'], ticket, job['FileName'], 0, 0, 0, job["Type"])
    pipeline.ticket = ticket

    #check SIP content
    self.assertTrue(pipeline.CheckSIPContent(), pipeline.SIP)

    #now invalidate the SIP in various ways

    #with wrong momid
    pipeline.SIP = makeSIP(job['Project'], job['ObservationId'], 'wrong_MomId', ticket, job['FileName'], 0, 0, 0, job["Type"])
    self.assertFalse(pipeline.CheckSIPContent(), pipeline.SIP)
    
    #without momid
    pipeline.SIP = pipeline.SIP.replace('<identifier>wrong_MomId</identifier>', '')
    self.assertFalse(pipeline.CheckSIPContent(), pipeline.SIP)

    #with wrong filename
    pipeline.SIP = makeSIP(job['Project'], job['ObservationId'], job['MomId'], ticket, 'wrong_filename', 0, 0, 0, job["Type"])
    self.assertFalse(pipeline.CheckSIPContent(), pipeline.SIP)

    #without filename
    pipeline.SIP = pipeline.SIP.replace('<name>wrong_filename</name>', '')
    self.assertFalse(pipeline.CheckSIPContent(), pipeline.SIP)

    #with wrong ticket
    pipeline.SIP = makeSIP(job['Project'], job['ObservationId'], job['MomId'], 'wrong_ticket', job['FileName'], 0, 0, 0, job["Type"])
    self.assertFalse(pipeline.CheckSIPContent(), pipeline.SIP)

    #without ticket
    pipeline.SIP = pipeline.SIP.replace('<storageTicket>wrong_ticket</storageTicket>', '')
    self.assertFalse(pipeline.CheckSIPContent(), pipeline.SIP)
    
#run tests if main
if __name__ == '__main__':
    unittest.main()
