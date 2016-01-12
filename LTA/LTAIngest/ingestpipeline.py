#!/usr/bin/env python
import logging, os, time, xmlrpclib, subprocess, random, unspecifiedSIP
import socket
from lxml import etree
from cStringIO import StringIO
from job_group import corr_type, bf_type, img_type, unspec_type, pulp_type
import ltacp
import getpass

def humanreadablesize(num, suffix='B'):
  """ converts the given size (number) to a human readable string in powers of 1024
  """
  try:
    for unit in ['','K','M','G','T','P','E','Z']:
      if abs(num) < 1024.0:
        return "%3.1f%s%s" % (num, unit, suffix)
      num /= 1024.0
    return "%.1f%s%s" % (num, 'Y', suffix)
  except TypeError:
    return str(num)
  
IngestStarted     = 10
## 20 not used
IngestSIPComplete = 30
IngestSuccessful  = 40
IngestFailed      = -10
Removed           = -20

PipelineJobFailedError      = 1
PipelineNoSourceError       = 2
PipelineAlreadyInLTAError   = 3
PipelineNoProjectInLTAError = 4
#---------------------- Custom Exception ----------------------------------------

class PipelineError(Exception):
  def __init__(self, message, source, type = PipelineJobFailedError):
    Exception.__init__(self, message)
    self.type     = type
    self.source   = source

#---------------------- IngestPipeline ------------------------------------------
class IngestPipeline():
  def __init__(self, logdir, job, momClient, ltaClient, ltacphost, ltacpport, mailCommand, momRetry, ltaRetry, srmRetry, srmInit):
    self.logdir          = logdir
    self.job             = job
    self.momClient       = momClient
    self.ltaClient       = ltaClient
    self.ltacphost       = ltacphost
    self.ltacpport       = ltacpport
    self.mailCommand     = mailCommand

    self.Project       = job['Project']
    self.DataProduct   = job['DataProduct']
    self.FileType      = unspec_type
    if 'sky' in self.DataProduct or 'FITS' in self.DataProduct: #Not for FITS and HDF5 Images
      self.FileName    = self.DataProduct
      self.FileType    = img_type
    elif '.tar' in self.DataProduct:
      self.FileName    = self.DataProduct
    else:
      self.FileName    = job['DataProduct'] + '.tar'
    if 'uv' in self.DataProduct: ## hacks needs a better solution
      self.FileType    = corr_type
    if 'bf' in self.DataProduct:
      if 'h5' in self.DataProduct:
        self.FileType    = bf_type
      else:
        self.FileType    = pulp_type
    if 'summary' in self.DataProduct:
      self.FileType    = pulp_type
    self.JobId         = job['JobId']
    self.ArchiveId         = int(job['ArchiveId'])
    self.ObsId         = int(job['ObservationId'])
    self.HostLocation  = job['Location'].split(':')[0]
    self.Location      = job['Location'].split(':')[1]
    pos = self.Location.find(self.DataProduct)
    if pos > 0: ## trick to support tar files with different names
      self.LocationDir = self.Location[:pos]
      if self.DataProduct[-3:] == '.h5' and 'bf' in self.DataProduct: #Temporary hack, should use h5_check.py
        self.Source = self.DataProduct + ' ' + self.DataProduct[:-3] + '.raw'
      else:
        self.Source       = self.DataProduct
    else:
      self.LocationDir = self.Location
      self.Source      = job['Source']
    self.ExportID      = job['ExportID']
    self.Type          = job["Type"]

    self.ticket          = ''
    self.FileSize        = '-1'
    self.MD5Checksum     = ''
    self.Adler32Checksum = ''
    self.ChecksumResult  = False
    self.SIP             = ''
    self.tempPrimary     = ''
    self.tempSecondary   = ''
    self.PrimaryUri      = ''
    self.SecondaryUri    = ''
    self.srmInit         = srmInit
    self.momRetry        = momRetry
    self.ltaRetry        = ltaRetry
    self.srmRetry        = srmRetry
    self.status          = IngestStarted

    ## Set logger
    self.logger =logging.getLogger('Slave')
    self.logger.info('--------- Job logger initialized ---------')

  def GetStorageTicket(self):
    try:
      start = time.time()
      result = self.ltaClient.GetStorageTicket(self.Project, self.FileName, self.FileSize, self.ArchiveId, self.JobId, self.ObsId, True, self.Type)
      self.logger.debug("GetStorageTicket for %s took %ds" % (self.JobId, time.time() - start))
    except xmlrpclib.Fault as err:
      self.logger.error('Received XML-RPC Fault: %s %s' % (err.faultCode, err.faultString))
      raise
    error = result['error']
    if error:
      self.logger.error(error) ## StorageTicket with mom ID "8948214" and ID source "MoM" already exists
      if 'StorageTicket with mom ID "%i"' % (self.ArchiveId) in error:
        if 'existing_ticket_id' in result and 'existing_ticket_state' in result:
          self.logger.warning("Got a Tier 1 GetStorageTicket error for an incomplete storage ticket %s with status %s" % (result['existing_ticket_id'],result['existing_ticket_state']))
          if result['existing_ticket_state'] < IngestSuccessful:
            try:
              self.ticket        = result['existing_ticket_id']
              self.logger.warning("trying to repair status of StorageTicket %s" % self.ticket)
              self.RetryRun(self.SendStatus, self.ltaRetry, 'Resetting LTA status', IngestFailed)
            except Exception as e:
              self.logger.exception('ResettingStatus IngestFailed failed for %s' % self.ticket)
            raise Exception ('Had to reset state for %s' % self.ticket)
          else:
            self.logger.warning("Tried to ingest a file that was already there %s" % self.JobId)
            raise PipelineError('Got Tier 1 GetStorageTicket error: Dataproduct already in LTA for %s' % (self.JobId), 'GetStorageTicket', PipelineAlreadyInLTAError)
        else:
          raise Exception('Got a Tier 1 GetStorageTicket error I can''t interpret: %s' % result)
      if 'no storage resources defined for project' in error or "project does not exists" in error:
        raise PipelineError('Got Tier 1 GetStorageTicket error for project not known in LTA: %s' % error, 'GetStorageTicket', PipelineNoProjectInLTAError)
      raise Exception('Got Tier 1 GetStorageTicket error: %s' % error)
    else:
      self.ticket        = result['ticket']
      self.tempPrimary   = result['primary_uri']
      self.tempSecondary = result['secondary_uri']
      self.PrimaryUri    = result['primary_uri_rnd']
      if 'secondary_uri_rnd' in result.keys():
        self.SecondaryUri  = result['secondary_uri_rnd']
    self.logger.debug('got tempURIs %s %s, random URIs %s %s and ticket %s' % (self.tempPrimary, self.tempSecondary, self.PrimaryUri, self.SecondaryUri, self.ticket))

  def ParseLTAcpLog(self, log):
    for l in log:
      if 'Checksums from server:' in l:
        if not '</checksums>' in l:
          self.logger.debug('checksums incomplete %s' % l)
          return False
        checksums = l.split()[8]
        pos = checksums.find('<value>')
        self.MD5Checksum     = checksums[pos+7:pos+39]
        self.Adler32Checksum = checksums[pos+105:pos+113]
        pos = checksums.find('<size>')
        try:
          self.FileSize = str(int(checksums[pos+6:checksums.find('</size>')])) #XML-RPC doesn't allow bigger than 32bit int
        except ValueError:
          self.logger.debug("No valid size found")
          return False
    return True

  def TransferFile(self):
    self.logger.debug('Starting file transfer')
    hostname = socket.getfqdn()
    javacmd = "java"
    if "lexar" in hostname:
      javacmd = "/data/java7/jdk1.7.0_55/bin/java"

    ltacppath = "/globalhome/%s/ltacp" % ("ingesttest" if self.ltacpport == 8801 else "ingest")

    if self.PrimaryUri:
      cmd = ["ssh",  "-T", "ingest@" +self.HostLocation, "cd %s;%s -Xmx256m -cp %s/qpid-properties/lexar001.offline.lofar:%s/ltacp.jar nl.astron.ltacp.client.LtaCp %s %s %s %s" % (self.LocationDir, javacmd, ltacppath, ltacppath, hostname, self.ltacpport, self.PrimaryUri, self.Source)]
    else:
      cmd = ["ssh",  "-T", "ingest@" + self.HostLocation, "cd %s;%s -Xmx256m -cp %s/qpid-properties/lexar001.offline.lofar:%s/ltacp.jar nl.astron.ltacp.client.LtaCp %s %s %s/%s %s" % (self.LocationDir, javacmd, ltacppath, ltacppath, hostname, self.ltacpport, self.tempPrimary, self.FileName, self.Source)]
    ## SecondaryUri handling not implemented
    self.logger.debug(cmd)
    start = time.time()
    p       = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    logs    = p.communicate()
    elapsed = time.time() - start
    self.logger.debug("File transfer for %s took %d sec" % (self.JobId, elapsed))
##    time.sleep(10)
##    logs = ("hoeba","bla")
    log     = logs[0].split('\n')
##    log = ["2012-04-10 14:18:51,161 INFO  client.LtaCp:272 - Checksums from server: <size>347074560</size><checksums><checksum><algorithm>MD5</algorithm><value>ae28093ed958e5aaf7f7cf5ff4188f37</value></checksum><checksum><algorithm>Adler32</algorithm><value>6367d2e1</value></checksum></checksums>",""]
    self.logger.debug('Shell command for %s exited with code %s' % (self.JobId, p.returncode))
    self.logger.debug('STD ERR of TransferFile command for %s:\n%s' % (self.JobId, logs[1]))
    self.logger.debug(log)
    if (not 'No such file or directory.' in logs[1]) and (not 'does not exist' in logs[0]):
        if not self.ParseLTAcpLog(log):
            self.logger.error("Parsing ltacp result failed for %s" % self.JobId)
            raise Exception('File transfer failed of %s' % self.JobId)
        else:
            try:
              if int(self.FileSize) > 0:
                avgSpeed = float(self.FileSize) / elapsed
                self.logger.debug("File transfer for %s  took %d sec with an average speed of %s for %s including ltacp overhead" % (self.JobId, elapsed, humanreadablesize(avgSpeed, 'Bps'), humanreadablesize(float(self.FileSize), 'B')))
            except Exception:
              pass
            self.CheckChecksums()
    else: # need to communicate that LTA transaction is to be rolled back but ingest not to be set to "hold"
        #os.system('echo "Dataproduct for %s not found on %s.\nConsidering dataproduct to be non existent"|mailx -s "Warning: Dataproduct not found on CEP host" ' % (self.JobId, self.HostLocation) + self.mailCommand)
        #self.logger.warn('Sent Warning: Dataproduct not found on CEP host to ' + self.mailCommand)
        raise PipelineError('Dataproduct for %s not found on %s'% (self.JobId, self.HostLocation), 'TransferFile', PipelineNoSourceError)
    self.logger.debug('Finished file transfer of %s' % self.JobId)


  def TransferFileNew(self):
    self.logger.debug('Starting new style file transfer for %s ' % self.JobId)

    try:
        start = time.time()

        host = self.HostLocation

        # eor dawn nodes are not known in dns
        # convert to ip address
        for i in range(1, 33):
            host = host.replace('node%d.intra.dawn.rug.nl' % (i+100,), '10.196.232.%d' % (i+10,))

        self.logger.info(os.path.join(self.LocationDir, self.Source))
        self.logger.info(self.PrimaryUri)

        cp = ltacp.LtaCp(host,
                         os.path.join(self.LocationDir, self.Source),
                         self.PrimaryUri)

        self.MD5Checksum, self.Adler32Checksum, self.FileSize = cp.transfer()

        elapsed = time.time() - start
        self.logger.debug("New style file transfer for %s took %d sec" % (self.JobId, elapsed))
        self.logger.debug('Finished new style file transfer of %s' % self.JobId)

        self.CheckChecksums()

        try:
            if int(self.FileSize) > 0:
                avgSpeed = float(self.FileSize) / elapsed
            self.logger.debug("New style file transfer for %s  took %d sec with an average speed of %s for %s including ltacp overhead" % (self.JobId, elapsed, humanreadablesize(avgSpeed, 'Bps'), humanreadablesize(float(self.FileSize), 'B')))
        except Exception:
            pass

    except ltacp.LtacpException as exp:
        if '550 File not found' in exp.value:
            self.logger.error('Destination directory does not exist. Creating %s in LTA for %s' % (self.PrimaryUri, self.JobId))

            if ltacp.create_missing_directories(self.PrimaryUri) == 0:
                self.logger.info('Created path %s in LTA for %s' % (self.PrimaryUri, self.JobId))

        raise Exception('New style file transfer failed of %s\n%s' % (self.JobId, str(exp)))


  def CheckChecksums(self):
    if self.MD5Checksum and self.Adler32Checksum and self.FileSize:
      try:
        self.logger.debug('Valid checksums found for %s with filesize %sB (%s)' % (self.JobId, self.FileSize, humanreadablesize(float(self.FileSize), 'B')))
      except:
        self.logger.debug('Valid checksums found for %s' % (self.JobId))
    else:
      self.logger.debug('Valid checksums not found for %s' % self.JobId)
      raise Exception('No valid checkums found for %s' % self.JobId)

  def SendChecksums(self):
    if self.PrimaryUri:
      uris = {'primary_uri':self.PrimaryUri, 'secondary_uri':self.SecondaryUri}
    else:
      uris = ''
    try:
      start = time.time()
      self.logger.debug("SendChecksums for %s: project=%s ticket=%s size=%s md5=%s a32=%s uris=%s" % (self.JobId, self.Project, self.ticket, self.FileSize,self.MD5Checksum,self.Adler32Checksum, uris))
      result = self.ltaClient.SendChecksums(self.Project, self.ticket, self.FileSize, {'MD5':self.MD5Checksum,'Adler32':self.Adler32Checksum}, uris)
      self.logger.debug("SendChecksums for %s took %ds" % (self.JobId, time.time() - start))
    except xmlrpclib.Fault as err:
      self.logger.error('Received XML-RPC Fault: %s %s' % (err.faultCode, err.faultString))
      raise
    error     = result['error']
    if not error:
      self.PrimaryUri   = result['primary_uri']
      self.SecondaryUri = result['secondary_uri']
    if error:
      self.logger.error('Got an error back in SendChecksums for %s: %s' % (self.JobId, error))
      raise Exception('Got Tier 1 SendChecksums error for %s: %s' % (self.JobId, error))
    self.logger.debug('got URIs %s %s' % (self.PrimaryUri, self.SecondaryUri))

  def SendStatus(self, state):
    try:
      start = time.time()
      result = self.ltaClient.UpdateUriState(self.Project, self.ticket, self.PrimaryUri, state)
      self.logger.debug("UpdateUriState for %s took %ds" % (self.JobId, time.time() - start))
    except xmlrpclib.Fault as err:
      self.logger.error('Received XML-RPC Fault: %s %s' % (err.faultCode, self._hidePassword(err.faultString)))
      raise
    except Exception as e:
      self.logger.error('Received unknown exception in SendStatus for %s: %s' % (self.JobId, self._hidePassword(str(e))))
      raise
    if result['result'] == 'ok':
      self.logger.debug('Status update for %s to %s was successful: %s' % (self.PrimaryUri, state, result))
    else:
      self.logger.error(result['error'])
      if "No DataProduct found for StorageTicket" in result['error']:
        self.logger.error('Database error, no dataproduct found for %s ' % self.JobId)
        raise PipelineError('Database error, no dataproduct found for %s ' % self.JobId, 'SetStatus', PipelineJobFailedError)
      else:
        self.logger.error('Got Tier 1 SendStatus error for %s: %s' % (self.JobId, result['error']))
        raise Exception('Got Tier 1 SendStatus error for %s: %s' % (self.JobId, result['error']))

## Not needed right now
##  def RenameFile(self):
##    self.logger.debug('Rename file')

  def CheckSIP(self):
      ##might do more than validate in the future
    try:
      start = time.time()
      f      = open('doc/LTA-SIP.xsd')
      xml    = etree.parse(f)
      schema = etree.XMLSchema(xml)
      sip    = StringIO(self.SIP)
      xml    = etree.parse(sip)
      result = schema.validate(xml)
      self.logger.debug("CheckSIP for %s took %ds" % (self.JobId, time.time() - start))
      return result
    except Exception as e:
      self.logger.error('CheckSIP failed: ' + str(e))
      return False

  def CheckSIPContent(self):
    try:
      start = time.time()
      sip   = StringIO(self.SIP)
      tree   = etree.parse(sip)
      root   = tree.getroot()
      dataProducts = root.xpath('dataProduct')
      if len(dataProducts) != 1:
        self.logger.error("CheckSIPContent for %s could not find single dataProduct in SIP" % (self.JobId))
        return False
      dataProductIdentifierIDs = dataProducts[0].xpath('dataProductIdentifier/identifier')
      if len(dataProductIdentifierIDs) != 1:
        self.logger.error("CheckSIPContent for %s could not find single dataProductIdentifier/identifier in SIP dataProduct" % (self.JobId))
        return False
      if dataProductIdentifierIDs[0].text != str(self.ArchiveId):
        self.logger.error("CheckSIPContent for %s dataProductIdentifier/identifier %s does not match expected %s" % (self.JobId, dataProductIdentifierIDs[0].text, self.ArchiveId))
        return False
      dataProductIdentifierNames = dataProducts[0].xpath('dataProductIdentifier/name')
      if len(dataProductIdentifierNames) != 1:
        self.logger.error("CheckSIPContent for %s could not find single dataProductIdentifier/name in SIP dataProduct" % (self.JobId))
        return False
      if not dataProductIdentifierNames[0].text in self.FileName:
        self.logger.error("CheckSIPContent for %s dataProductIdentifier/name %s does not match expected %s" % (self.JobId, dataProductIdentifierNames[0].text, self.FileName))
        return False
      storageTickets = dataProducts[0].xpath('storageTicket')
      if len(storageTickets) != 1:
        self.logger.error("CheckSIPContent for %s could not find single storageTickets in SIP dataProduct" % (self.JobId))
        return False
      if storageTickets[0].text != str(self.ticket):
        self.logger.error("CheckSIPContent for %s storageTicket %s does not match expected %s" % (self.JobId, storageTickets[0].text, self.ticket))
        return False
        
      return True
    except Exception as e:
      self.logger.error('CheckSIPContent failed: ' + str(e))
      return False

  def GetSIP(self):
    if self.Type == "MoM":
      try:
        start = time.time()
        self.logger.debug("GetSIP for %s with mom2DPId %s - StorageTicket %s - FileName %s - Uri %s" % (self.JobId, self.ArchiveId, self.ticket, self.FileName, self.PrimaryUri))
        sip = self.momClient.getSIP(self.ArchiveId, self.ticket, self.FileName, self.PrimaryUri, self.FileSize, self.MD5Checksum, self.Adler32Checksum)
        self.SIP = sip.replace('<stationType>Europe</stationType>','<stationType>International</stationType>')
        self.logger.debug("GetSIP for %s took %ds" % (self.JobId, time.time() - start))
      except:
        self.logger.exception('Getting SIP from MoM failed')
        raise
      self.logger.debug('SIP received for %s from MoM with size %d (%s): %s' % (self.JobId, len(self.SIP), humanreadablesize(len(self.SIP)), self.SIP[0:256]))
    elif self.Type.lower() == "eor":
      try:
        sip_host = job['SIPLocation'].split(':')[0]
        for i in range(1, 43):
            sip_host = sip_host.replace('node%d.intra.dawn.rug.nl' % (i+100,), '10.196.232.%d' % (i+10,))
        sip_path = job['SIPLocation'].split(':')[1]
        cmd = ['ssh', '-tt', '-n', '-x', '-q', '%s@%s' % (getpass.getuser(), sip_host), 'cat %s' % sip_path]
        self.logger.debug("GetSIP for %s with mom2DPId %s - StorageTicket %s - FileName %s - Uri %s - cmd %s" % (self.JobId, self.ArchiveId, self.ticket, self.FileName, self.PrimaryUri, ' ' .join(cmd)))
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = p.communicate()
        if p.returncode != 0:
            raise PipelineError('GetSIP error getting EoR SIP for %s: %s' % (self.JobId, err), 'GetSip')

        self.SIP = out

	with open('eor_sip1.xml', 'w') as f:
          f.write(self.SIP)

        # parse sip xml and add filesize, storageticket and checkums
        from xml.dom import minidom
        sip_dom = minidom.parseString(self.SIP)
        dp_node = sip_dom.getElementsByTagName('dataProduct')[0]

        for elem in dp_node.getElementsByTagName('checksum'):
            dp_node.removeChild(elem)

        for elem in dp_node.getElementsByTagName('size'):
            dp_node.removeChild(elem)

        for elem in dp_node.getElementsByTagName('storageTicket'):
            dp_node.removeChild(elem)

        sip_namespace = "http://www.astron.nl/SIP-Lofar"
        storageticket_node = sip_dom.createElementNS(sip_namespace, 'storageTicket')
        storageticket_node.appendChild(sip_dom.createTextNode(str(self.ticket)))

        size_node = sip_dom.createElementNS(sip_namespace, 'size')
        size_node.appendChild(sip_dom.createTextNode(str(self.FileSize)))

        checksum_md5_algo_node = sip_dom.createElementNS(sip_namespace, 'algorithm')
        checksum_md5_algo_node.appendChild(sip_dom.createTextNode('MD5'))
        checksum_md5_value_node = sip_dom.createElementNS(sip_namespace, 'value')
        checksum_md5_value_node.appendChild(sip_dom.createTextNode(str(self.MD5Checksum)))
        checksum_md5_node = sip_dom.createElementNS(sip_namespace, 'checksum')
        checksum_md5_node.appendChild(checksum_md5_algo_node)
        checksum_md5_node.appendChild(checksum_md5_value_node)

        checksum_a32_algo_node = sip_dom.createElementNS(sip_namespace, 'algorithm')
        checksum_a32_algo_node.appendChild(sip_dom.createTextNode('Adler32'))
        checksum_a32_value_node = sip_dom.createElementNS(sip_namespace, 'value')
        checksum_a32_value_node.appendChild(sip_dom.createTextNode(str(self.Adler32Checksum)))
        checksum_a32_node = sip_dom.createElementNS(sip_namespace, 'checksum')
        checksum_a32_node.appendChild(checksum_a32_algo_node)
        checksum_a32_node.appendChild(checksum_a32_value_node)

        filesize_node = sip_dom.createElementNS(sip_namespace, 'size')
        filesize_node.appendChild(sip_dom.createTextNode(str(self.FileSize)))

        dp_node.insertBefore(checksum_a32_node, dp_node.getElementsByTagName('fileName')[0])
        dp_node.insertBefore(checksum_md5_node, checksum_a32_node)
        dp_node.insertBefore(filesize_node, checksum_md5_node)
        dp_node.insertBefore(storageticket_node, filesize_node)

        self.SIP = sip_dom.toxml("utf-8")
        self.SIP = self.SIP.replace('<stationType>Europe</stationType>','<stationType>International</stationType>')

        with open('eor_sip2.xml', 'w') as f:
          f.write(self.SIP)

      except:
        self.logger.exception('Getting SIP from EoR failed')
        raise
      self.logger.debug('SIP received for %s from EoR with size %d (%s): \n%s' % (self.JobId, len(self.SIP), humanreadablesize(len(self.SIP)), self.SIP))
    else:
      self.SIP = unspecifiedSIP.makeSIP(self.Project, self.ObsId, self.ArchiveId, self.ticket, self.FileName, self.FileSize, self.MD5Checksum, self.Adler32Checksum, self.Type)
      self.FileType = unspec_type
    if not self.CheckSIP():
      self.logger.debug('Got a malformed SIP from MoM: %s' % self.SIP[0:50])
      try:
        self.SIP = unspecifiedSIP.makeSIP(self.Project, self.ObsId, self.ArchiveId, self.ticket, self.FileName, self.FileSize, self.MD5Checksum, self.Adler32Checksum, self.Type)
        self.FileType = unspec_type
      except Exception as e:
         self.logger.error('GetSIP failed: ' + str(e))
         raise
      self.logger.debug('Unspecified SIP created for %s: %s' % (self.JobId, self.SIP[0:400]))
      ###raise Exception('Got a malformed SIP from MoM: %s' % self.SIP[0:50])
    if not self.CheckSIPContent():
      self.logger.error('SIP has invalid content for %s\n%s' % (self.JobId, self.SIP))
      raise PipelineError('Got a SIP with wrong contents from MoM for %s : %s' % (self.JobId, self.SIP), func.__name__)

  def SendSIP(self):
    try:
      start = time.time()
      result = self.ltaClient.TransmitSIP(self.SIP, self.ticket)
      self.logger.debug("TransmitSIP for %s took %ds" % (self.JobId, time.time() - start))
    except xmlrpclib.Fault as err:
      self.logger.error('Received XML-RPC Fault: %s %s' % (err.faultCode, err.faultString))
      raise Exception('XML-RPC failed')
    if result['result'] == 'ok':
      self.logger.debug('Successfully sent SIP for %s' % self.JobId)
    else:
      self.logger.error(result['error'])
      if "Exception in TransmitSIP, could not use SIP" in result['error']:
        self.logger.error('Invalid SIP according to LTA catalog for %s' % self.JobId)
        raise PipelineError('Invalid SIP according to LTA catalog for %s' % self.JobId, 'SendSIP', PipelineJobFailedError)
      else:
        raise Exception('Got Tier 1 TransmitSIP error for %s: %s' % (self.JobId, result['error']))

  def RollBack(self):
    self.logger.debug('Rolling back file transfer for %s' % self.JobId)
    try:
      if self.PrimaryUri:
        cmd = ["bash", "-c", "source %s;srmrm %s" % (self.srmInit, self.PrimaryUri)]
      else:
        cmd = ["bash", "-c", "source %s;srmrm %s/%s" % (self.srmInit, self.tempPrimary, self.FileName)]
      ## SecondaryUri handling not implemented
      self.logger.debug(cmd)
      start   = time.time()
      p       = subprocess.Popen(cmd, stdout=subprocess.PIPE)
      log     = p.communicate()[0].split('\n')
      self.logger.debug("RollBack for %s took %ds" % (self.JobId, time.time() - start))
      self.logger.debug(log)
    except:
      self.logger.exception('Roll back failed for %s' % self.JobId)

  def RetryRun(self, func, times, errortext, *args):
    error = ''
    retry = 0
    while (retry < times):
      try:
        func(*args)
      except PipelineError as pe:
    ## function raised PipelineError itself. Assume retries not useful
        raise
      except Exception as e:
        error +=  '\n' + str(e)
      else:
        if retry:
          self.logger.debug(errortext + ' was tried %s times on %s before it succeeded. Got the following errors: %s' % (retry, self.JobId, error))
        else:
          self.logger.debug(errortext + ' ran without a problem on %s' % self.JobId)
        error = ''
        break  
      retry += 1
      if retry < times:
        time.sleep(random.randint(30, 60) * retry)
    if error:
      raise PipelineError(errortext + ' tried %s times but failed on %s. Got the following errors: %s' % (retry, self.JobId, error), func.__name__)

  def run(self):
    try:
      self.logger.debug("Ingest Pipeline started for %s" % self.JobId)
      start = time.time()
      self.RetryRun(self.GetStorageTicket, self.ltaRetry, 'Getting storage ticket')

      if self.Type.lower() == "eor":
        self.RetryRun(self.TransferFileNew, self.srmRetry , 'Transfering file')
      else:
        self.RetryRun(self.TransferFile, self.srmRetry , 'Transfering file')

      self.RetryRun(self.SendChecksums, self.ltaRetry, 'Sending Checksums')
#      self.RenameFile()
      self.RetryRun(self.GetSIP, self.momRetry, 'Get SIP from MoM')
      self.RetryRun(self.SendSIP, self.ltaRetry, 'Sending SIP')
      self.RetryRun(self.SendStatus, self.ltaRetry, 'Setting LTA status', IngestSuccessful)
      elapsed = time.time() - start
      try:
        if int(self.FileSize) > 0:
          avgSpeed = float(self.FileSize) / elapsed
          self.logger.debug("Ingest Pipeline finished for %s in %d sec with average speed of %s for %s including all overhead" % (self.JobId, elapsed, humanreadablesize(avgSpeed, 'Bps'), humanreadablesize(float(self.FileSize), 'B')))
      except Exception:
        self.logger.debug("Ingest Pipeline finished for %s in %d sec" % (self.JobId, elapsed))
      
    except PipelineError as pe:
      self.logger.debug('Encountered PipelineError for %s : %s' % (self.JobId, str(pe)))
      ## roll back transfer if necessary
      if self.PrimaryUri or self.tempPrimary:
        if not (pe.type == PipelineNoSourceError):
          self.RollBack()
      ## notify LTA the ingest has failed
      ## ...but catch exceptions as we do not want to raise a new type of error
      try:
        if self.ticket:
          self.RetryRun(self.SendStatus, self.ltaRetry, 'Setting LTA status', IngestFailed)
      except Exception as e:        
        os.system('echo "Received unknown exception in SendStatus for %s to %s while handling another error:\n%s\n\nCheck LTA catalog and SRM!\n%s"|mailx -s "Warning: LTA catalog status update failed" ' % (self.JobId, IngestFailed, self._hidePassword(str(e)), self.PrimaryUri) + self.mailCommand)
        self.logger.error('Sent Mail: LTA catalog status update failed to ' + self.mailCommand)
        self.logger.exception('SendStatus IngestFailed failed')
      if pe.type == PipelineJobFailedError:
        self.logger.debug('Encountered PipelineJobFailedError')
        raise
      elif pe.type == PipelineNoSourceError:
        self.logger.debug('Encountered PipelineNoSourceError')
        ## do not raise as it is not possible to continue trying to ingest the source file
      elif pe.type == PipelineAlreadyInLTAError:
        self.logger.debug('Encountered PipelineAlreadyInLTAError for %s' % (self.JobId))
        ## Do not raise as further attempts will generate the same result
      elif pe.type == PipelineNoProjectInLTAError:
        self.logger.debug('Encountered PipelineNoProjectInLTAError for %s' % (self.JobId))
        raise
      elif pe.source == "SendStatus":
        os.system('echo "Received unknown exception in SendStatus for %s to %s:\n%s\n\nCheck LTA catalog and SRM!\n%s"|mailx -s "Warning: LTA catalog status update failed" ' % (self.JobId, IngestFailed, self._hidePassword(str(e)), self.PrimaryUri) + self.mailCommand)
        self.logger.error('Sent Mail: LTA catalog status update failed to ' + self.mailCommand)
        self.logger.error('SendStatus IngestFailed failed')
      else:
        self.logger.warn('Encountered unexpected PipelineErrorType: %s' % pe.type)
        raise
    except:
      self.logger.debug('Encountered unexpected error for %s' % (self.JobId))
      if self.PrimaryUri or self.tempPrimary:
        self.RollBack()
      if self.ticket:
        self.RetryRun(self.SendStatus, self.ltaRetry, 'Setting LTA status', IngestFailed)
      raise
    
  def _hidePassword(self, message):
    ''' helper function which hides the password in the ltaClient url in the message
    '''
    try:
      url = self.ltaClient._ServerProxy__host
      password = url.split('@')[0].split(':')[-1] #assume url is http://user:pass@host:port
      return message.replace(':'+password, ':HIDDENPASSWORD')
    except Exception as e:
      return message
    

#----------------------------------------------------------------- selfstarter -
if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print 'usage: ingestpipeline.py <path_to_jobfile.xml>'

    logging.basicConfig(level=logging.DEBUG)
    logger = logging.getLogger('Slave')

    jobfile = sys.argv[1]
    import job_parser
    parser = job_parser.parser(logger)
    job = parser.parse(jobfile)
    job['filename'] = jobfile

    logger.info(str(job))

    if getpass.getuser() == 'ingest':
        import ingest_config as config
    else:
        import ingest_config_test as config

    #create misc mock args
    ltacphost = 'mock-ltacphost'
    ltacpport = -1
    mailCommand = ''
    srmInit = ''

    standalone = IngestPipeline(None, job, config.momClient, config.ltaClient, config.ipaddress, config.ltacpport, config.mailCommand, config.momRetry, config.ltaRetry, config.srmRetry, config.srmInit)
    standalone.run()
