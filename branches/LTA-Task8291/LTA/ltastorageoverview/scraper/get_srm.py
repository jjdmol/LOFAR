#!/usr/bin/python

import subprocess, logging, cPickle, time, sys, os.path

logging.basicConfig(filename='get_srm'+ time.strftime("%Y-%m-%d_%H:%M") + '.log', level=logging.DEBUG, format="%(asctime)-15s %(levelname)s %(message)s")
logger =logging.getLogger()
srminit = '/globalhome/ingest/service/bin/init.sh'

if len(sys.argv) < 2:
  output = "files"+ time.strftime("%Y-%m-%d") + ".pickle"
else:
  output = sys.argv[1]

def GetDir(srmroot, path):
  files = []
  offset = 0
  while True:
    cmd = ["bash", "-c", "source %s;srmls -count=900 -offset=%d %s%s" % (srminit, offset, srmroot, path)]
    logger.debug(cmd)
    p       = subprocess.Popen(cmd, stdin=open('/dev/null'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    logs    = p.communicate()
    lines   = logs[0].split('\n')
    logger.debug('Shell command for %s exited with code %s' % (path, p.returncode))
    if len(lines) < 2:
      logger.error("Response shorter than expected for %s, %d: %s" % (path, offset, lines))
      return files
    if p.returncode > 0:
      logger.exception("Unexpected return code for %s, %d: %s" % (path, offset, lines))
      logger.debug("Found %d files for %s", len(files), path)
      raise Exception("Oopz") ## we should add some retry function here at some point?
    if "FAILURE" in lines[1]:
      logger.warning("Unexpected FAILURE for %s, %d: %s" % (path, offset, lines[1]))
      logger.debug("Found %d files for %s", len(files), path)
      return files
    for l in lines:
      line = l.split()
      if len(line) < 2:
        if l: #otherwise it's just an empty trailing line
          logger.error("Line shorter than expected: %s" % l)
        continue
      if line[1] == (path + '/') or  line[1] == path: #Current path
        continue
      if line[1][-1] == '/' or (int(line[0]) < 1): #Directory, size=0 for Target/StoRM, but no trailing slash
        logger.debug("Found directory: %s", line[1])
        files.append((line[1], GetDir(srmroot, line[1])))
      else:
        try:
          files.append((line[1], int(line[0])))
        except:
          logger.exception(line)
    if not len(lines) > 900:
      logger.debug("Found %d files for %s" % (len(files), path))
      return files
    offset += 900

#"""ingest@lexar001:~$ srmls srm://srm.grid.sara.nl:8443/pnfs/grid.sara.nl/data/lofar/
#  512 /pnfs/grid.sara.nl/data/lofar//
#      512 /pnfs/grid.sara.nl/data/lofar/lotest/
#      512 /pnfs/grid.sara.nl/data/lofar/trans/
#      512 /pnfs/grid.sara.nl/data/lofar/eor/
#      512 /pnfs/grid.sara.nl/data/lofar/surveys/
#      512 /pnfs/grid.sara.nl/data/lofar/ops/
#      512 /pnfs/grid.sara.nl/data/lofar/cosmics/
#      512 /pnfs/grid.sara.nl/data/lofar/storage/
#      512 /pnfs/grid.sara.nl/data/lofar/pulsar/
#      512 /pnfs/grid.sara.nl/data/lofar/software/
#      512 /pnfs/grid.sara.nl/data/lofar/proc/
#      512 /pnfs/grid.sara.nl/data/lofar/user/"""

def updatePickle(foundfiles, outputfile):
  if len(foundfiles) or not os.path.exists(outputfile):
    output = open(outputfile, 'wb')
    cPickle.dump(foundfiles, output, 2)
    output.close()
  else:
    output = open(outputfile, 'rb')
    foundfiles = cPickle.load(output)
    output.close()
  return foundfiles

files = []
files = updatePickle(files, output)
if len(files) < 1: files.append(("srm://srm.target.rug.nl:8444", GetDir("srm://srm.target.rug.nl:8444", "/lofar/ops/disk")))
files = updatePickle(files, output)
if len(files) < 2: files.append(("srm://tbn18.nikhef.nl:8446", GetDir("srm://tbn18.nikhef.nl:8446", "/dpm/nikhef.nl/home/lofar")))
files = updatePickle(files, output)
if len(files) < 3: files.append(("srm://srm.grid.sara.nl:8443", GetDir("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/ops")))
files = updatePickle(files, output)
if len(files) < 4: files.append(("srm://srm.grid.sara.nl:8443", GetDir("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/user")))
files = updatePickle(files, output)
if len(files) < 5: files.append(("srm://srm.grid.sara.nl:8443", GetDir("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/software")))
files = updatePickle(files, output)
if len(files) < 6: files.append(("srm://srm.grid.sara.nl:8443", GetDir("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/storage")))
files = updatePickle(files, output)
if len(files) < 7: files.append(("srm://srm.grid.sara.nl:8443", GetDir("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/pulsar")))
files = updatePickle(files, output)
if len(files) < 8: files.append(("srm://lofar-srm.fz-juelich.de:8443", GetDir("srm://lofar-srm.fz-juelich.de:8443", "/pnfs/fz-juelich.de/data/lofar/ops")))
files = updatePickle(files, output)
logger.debug("get_srm is done")
print "Done" 
