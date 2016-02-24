#!/usr/bin/env python

##Only internal in the Ingest
JobRetry     = -2
##Below are hardcoded defines for communicating with MoM!
JobError     = -1
JobHold      =  0
JobScheduled =  1
JobProducing =  2
JobProduced  =  3

def jobState2String(jobstate):
  if jobstate == JobRetry:
    return "%d (JobRetry)" % jobstate
  elif jobstate == JobError:
    return "%d (JobError)" % jobstate
  elif jobstate == JobHold:
    return "%d (JobHold)" % jobstate
  elif jobstate == JobScheduled:
    return "%d (JobScheduled)" % jobstate
  elif jobstate == JobProducing:
    return "%d (JobProducing)" % jobstate
  elif jobstate == JobProduced:
    return "%d (JobProduced)" % jobstate
  return str(jobstate)

## Job should probably be refactored into a class at some point.
## Right now it's just a big dict.

##------------------ Job keys --------------------------
## job['Status'] : JobRetry, JobError, JobHold, JobScheduled, JobProducing, JobProduced
## job['ExportID'] : nodeName == 'exportjob'
## job['Location'] : <input name="Location">locus029:/data/L202708/L202708_SB243_uv.dppp.MS</input>
## job['host'] : job['Location'].split(':')[0]
## job['filename'] : SOAP call, filename argument in new_job
## Project = job['Project'] : <input name="Project">LC1_055</input>
## DataProduct = job['DataProduct'] : <input name="DataProduct">L202708_SB243_uv.dppp.MS</input>
## FileName = job['DataProduct'] (+ '.tar')
## JobId = job['JobId'] : <input name="JobId">A_1134_1134_3767569_10318605_L202708_SB243_uv.dppp.MS</input>
## ArchiveId = int(job['ArchiveId']) : <input name="ArchiveId">10318605</input>
## ObsId = int(job['ObservationId']) : <input name="ObservationId">202708</input>
## unused : <input name="Subband">-1</input>
## Source = job['Source'] : <input name="Source">L201198_red</input>
## Source = self.DataProduct + ' ' + self.DataProduct[:-3] + '.raw'
## Source = self.DataProduct
## Type = job["Type"] : <input name="Type">tier0-ingest</input>
## Type = "MoM"
## HostLocation = job['Location'].split(':')[0] 
## Location = job['Location'].split(':')[1] 
## jobfile.split('_')[1] == exportId
## suffix = self.job['filename'].split('.')[-1]
## job['retry'] = 0
## job['job_group'] = self.job['ExportID'].split('_')[1] 
## job['errors'] = []
## job['destination'] = self.job_groups[jg].get_destination()
##
## N.B. HostLocation == job['host']

class parser():
    def __init__(self, logger):
      self.logger = logger

    ## Code to generate results ---------------------------------------------
    def parse(self, job):
        self.job = {}
        try:
            from xml.dom import minidom, Node
            doc = minidom.parse(job)
            if doc.documentElement.nodeName == 'exportjob':
                self.job['ExportID'] = str(doc.documentElement.attributes.get('exportID').nodeValue)
                for node in doc.documentElement.childNodes:
                    if node.nodeName == 'inputlist':
                        name  = "'" + node.attributes.get('name').nodeValue + "'"
                        exec(eval("'self.job[%s] = []' % (name)"))
                        for itemnode in node.childNodes:
                            if itemnode.nodeName == 'listitem':
                                value = itemnode.childNodes[0].nodeValue
                                exec(eval("'self.job[%s].append(%s)' % (name, value)"))
                    elif node.nodeName == 'input':
                        name  = "'" + node.attributes.get('name').nodeValue + "'"
                        value = node.childNodes[0].nodeValue
                        if value == 'True' or value == 'False':
                            exec(eval("'self.job[%s] = %s' % (name, value)"))
                        else:
                            value = "'''" + value + "'''" ## tripple quotes because a value could be "8 O'clock" for example
                            exec(eval("'self.job[%s] = %s' % (name, value)"))
            if self.job['ExportID']: ## we need an export ID to identify the job
                if self.job['ObservationId'][0] == 'L':
                    self.job['ObservationId'] = self.job['ObservationId'][1:]
                test = int(self.job['ObservationId']) ## check if it can be converted to an int
                test = int(self.job['ArchiveId']) ## check if it can be converted to an int
                self.job['host'] = self.job['Location'].split(':')[0]
                self.job['Status'] = JobScheduled
                self.job['retry'] = 0
                self.job['errors'] = []
                if not "Type" in self.job:
                    self.job["Type"] = "MoM"

                if not "JobId" in self.job:
                    self.job["JobId"] = self.job['ExportID']

                if self.job["Type"].lower() == "eor":
                    self.job['job_group'] = self.job['IngestGroupId']
                else:
                    self.job['job_group'] = int(self.job['ExportID'].split('_')[1])
                return self.job
        except:
            self.logger.exception('Failed importing job: ' + job)
        self.job['Status'] = JobError
        return self.job
