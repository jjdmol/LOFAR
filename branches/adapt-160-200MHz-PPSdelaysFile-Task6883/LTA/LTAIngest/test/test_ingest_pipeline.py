#!/usr/bin/env python

import unittest
import logging
import xmlrpclib
import LTAIngest.ingestpipeline as ingestpipeline
from LTAIngest.job_parser import JobScheduled
from LTAIngest.unspecifiedSIP import makeSIP

class TestIngestPipeline(unittest.TestCase):
  """Tests for various IngestPipeline methods"""

  def setUp(self):
    #logging
    self.logger = logging.getLogger('Slave')
    self.logger.addHandler(logging.StreamHandler())
    self.logger.setLevel(logging.DEBUG)

    #create mock self.job
    self.job = {}
    self.job['ExportID'] = '0000'
    self.job['JobId'] = 'X_0000_0000_1234567_12345678_L123456_SB000_uv.MS'
    self.job['Status'] = JobScheduled
    self.job['DataProduct'] = 'L123456_SB000_uv.MS'
    self.job['FileName'] = self.job['DataProduct'] + '.tar'
    self.job['ArchiveId'] = 12345678
    self.job['ObservationId'] = 123456
    self.job["Type"] = 'MoM'
    self.job['Project'] = 'test'
    self.job['Location'] = 'locus099:/data/L123456/L123456_SB000_uv.MS'
    
    #create mock storage self.ticket
    self.ticket = 'ABCDEF123456789'
    
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

    #create self.pipeline instance with mock info
    self.pipeline =  ingestpipeline.IngestPipeline('/tmp', self.job, momClient, ltaClient, ltacphost, ltacpport, mailCommand, 1, 1, 1, srmInit)

    #end of setup

  def testSIPContents(self):
    """test CheckSIPContent method with valid and invalid SIP"""

    #create proper SIP with valid self.ticket, ArchiveId and filename
    self.pipeline.SIP = makeSIP(self.job['Project'], self.job['ObservationId'], self.job['ArchiveId'], self.ticket, self.job['FileName'], 0, 0, 0, self.job["Type"])
    self.pipeline.ticket = self.ticket

    #check SIP content
    self.assertTrue(self.pipeline.CheckSIPContent(), self.pipeline.SIP)

    #now invalidate the SIP in various ways

    #with wrong ArchiveId
    self.pipeline.SIP = makeSIP(self.job['Project'], self.job['ObservationId'], 'wrong_ArchiveId', self.ticket, self.job['FileName'], 0, 0, 0, self.job["Type"])
    self.assertFalse(self.pipeline.CheckSIPContent(), self.pipeline.SIP)
    
    #without ArchiveId
    self.pipeline.SIP = self.pipeline.SIP.replace('<identifier>wrong_ArchiveId</identifier>', '')
    self.assertFalse(self.pipeline.CheckSIPContent(), self.pipeline.SIP)

    #with wrong filename
    self.pipeline.SIP = makeSIP(self.job['Project'], self.job['ObservationId'], self.job['ArchiveId'], self.ticket, 'wrong_filename', 0, 0, 0, self.job["Type"])
    self.assertFalse(self.pipeline.CheckSIPContent(), self.pipeline.SIP)

    #without filename
    self.pipeline.SIP = self.pipeline.SIP.replace('<name>wrong_filename</name>', '')
    self.assertFalse(self.pipeline.CheckSIPContent(), self.pipeline.SIP)

    #with wrong self.ticket
    self.pipeline.SIP = makeSIP(self.job['Project'], self.job['ObservationId'], self.job['ArchiveId'], 'wrong_self.ticket', self.job['FileName'], 0, 0, 0, self.job["Type"])
    self.assertFalse(self.pipeline.CheckSIPContent(), self.pipeline.SIP)

    #without self.ticket
    self.pipeline.SIP = self.pipeline.SIP.replace('<storageTicket>wrong_self.ticket</storageTicket>', '')
    self.assertFalse(self.pipeline.CheckSIPContent(), self.pipeline.SIP)
    
  def testHidePassword(self):
    """test _hidePassword method"""

    password = 'secret'
    url = 'https://someuser:%s@somesite.domain:1234' % password
    self.pipeline.ltaClient = xmlrpclib.ServerProxy(url)

    messageWithPassword = 'some message from %s where something happened' % url
    self.assertTrue(password in messageWithPassword)
    
    messageWithHiddenPassword = self.pipeline._hidePassword(messageWithPassword)
    self.assertFalse(password in messageWithHiddenPassword)
    
    
#run tests if main
if __name__ == '__main__':
    unittest.main()
