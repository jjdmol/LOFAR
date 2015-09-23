#!/usr/bin/python
# script to automatically generate NDPPP script

import os, optparse, subprocess, sys, signal, time

##----------------------input options----------------------------
parser = optparse.OptionParser()
parser.add_option("-O", "--Observation", dest="Observation",
                  help="Observation name (L2010_12345)")
parser.add_option("-T", "--Type", dest="Type", choices = ["lse", "lce"],
                  help="Where to look for Observation (lse or lce)")
parser.add_option("-r", "--rundir", dest="Rundir",
                  help="Runtime directory (/home/renting/ltacp)")
parser.add_option("-d", "--debug", dest="Debug", default = False,
                  help="Verbose (more detailed output)")
parser.add_option("-S", "--Subdir", dest="Subdir",
                  help="Non-default sub directory (e.g. /data/scratch/pizzo instead of /data1 or /data/scratch)")
parser.add_option("-E", "--Exclude", dest="Exclude", default = [],
                  help="Nodes to exclude (e.g. 1,14,19)")
parser.add_option("-L", "--Location", dest="Location", default = "/data4",
                  help="Where to store the files")
parser.add_option("-H", "--Host", dest="Host", choices = ["lexar001", "lexar002"],
                  help="Host on which to store the files")

(options, args) = parser.parse_args()
if not options.Observation:
  parser.error("Observation not set")
if not (options.Type == "lse" or options.Type == "lce"):
  parser.error("Not a valid type: " + str(options.Type))
if not options.Location:
  parser.error("Location not set")
if not options.Host:
  parser.error("Host not set")
if not options.Rundir:
  parser.error("Runtime directory not set")

print options

exclude = []
try:
  if len(options.Exclude) > 0:
    inputs = options.Exclude.split(',')
    for i in inputs:
      exclude.append(int(i))
except ValueError:
  print "Exclude values can not be parsed"
  exit(-1)

obs   = options.Observation
node  = options.Type
debug = options.Debug
lexar = options.Host
location = options.Location

runtime_location = options.Rundir

print "Processing started for " + obs + " on " + node + " nodes."
if debug: print "excluded nodes for searching: " + str(exclude)

if node == "lse":
  nodes = range(1,25)
  locations = ["/data1", "/data2", "/data3", "/data4"]
else:
  nodes = range(1,73)
  locations = ['/data/scratch']
if options.Subdir:
  locations = [options.Subdir]
  if debug: print "Using non-standard directory: " + options.Subdir

##-----------------find the files -------------------------------
def find_files():
  for n in nodes:
    if n in exclude:
      if debug: print "Skipping: " + str(n)
      continue
    else:
      for location in locations:
        command = ["ssh", "-T", "%s%03i" % (node, n), 'python %s/find_files.py %s' % (runtime_location, location + '/' + obs)]
        file_finder = subprocess.Popen(command, stdout=subprocess.PIPE)
        if file_finder.returncode:
          print "error: %i" % (file_finder.return_code)
        else:
          output    = file_finder.communicate()[0]
          file_list = output.split()
          for f in file_list:
            files.append((n, location, f))
        sys.stdout.write('.')
        sys.stdout.flush()

files = []
find_files()
if debug: print files
print "\nFound " + str(len(files)) + " datasets to process"
server = 0

#-------------------spawn------------------------
def spawn(command):
        try:
            pid = os.fork()
        except OSError, e:
            print 'Unable to fork:' + str(e)
            os._exit(1)
        if pid == 0: ## the new client
            os.system(command)
            os._exit(0)
        else: ## parent process
            server = pid

print "Starting server on " + lexar
if lexar == "lexar001":
  command = "ssh -T lexar001.offline.lofar 'java -cp %s/ltacp.jar nl.astron.ltacp.server.LtaCpServer 10.178.1.1 2011 8 50 > /data4/ltacp-server.out'" % runtime_location
  if debug: print command
  spawn(command)
if debug: print "Server: " + str(server)
time.sleep(5) ##wait a few seconds for the server to start

#------------processing the files-----------------------
print "processing the files"

checksums = []
for f in files:
  command = "ssh -T %s%03i 'java -jar %s/ltacp.jar %s 2011 %s/%s/%s.tar %s/%s/%s'" % (node, f[0], runtime_location, lexar, location, obs, f[2], f[1], obs, f[2])
  if debug: print command
  comm = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  if comm.returncode:
    print "error: %i" % (comm.return_code)
  else:
    output = comm.communicate()[0]
    lines  = output.split()
    for l in lines:
      if l[0:11] == "<checksums>":
        checksums.append("%s: %s" % (f[2], l))
  sys.stdout.write('.')
  sys.stdout.flush()

outfile = open("%s/logs/%s.log" % (runtime_location, obs), 'w+')
for c in checksums:
  outfile.write(c + "\n")
outfile.close()

print "Finished with processing files"

#os.kill(server, signal.SIGTERM)
print "Stopped server"
os.system("scp %s/logs/%s.log %s:%s/%s/" % (runtime_location, obs, lexar, location, obs))

print "Done"
