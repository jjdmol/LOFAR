from optparse import OptionParser
from LOFAR_Parset import Parset
import sys, os, time

def add_log(options, line):
    logfile = open(options.log, 'a+')
    logfile.write(line + ' ' + time.ctime() + '\n')
    logfile.close()

parser = OptionParser(usage='%prog [options] -m<MS name> -r<remote_host>')
parser.add_option("-r", "--remote_host", dest="remote_host", default="",
                  help="Host name")
parser.add_option("-m", "--ms", dest="ms", default="",
                  help="Measurementset name")
parser.add_option("-l", "--log", dest="log", default="",
                  help="Logfile name")
parser.add_option("-d", "--dir", dest="directory", default="",
                  help="Directory name")


options, args = parser.parse_args()

print 'Started processing:'

if not os.path.exists(options.directory):
  os.mkdir(options.directory)
os.chdir(options.directory)

os.system('rm -rf *.MS *.parset *.debug *.log')
add_log(options, 'Cleaned up old data:')

add_log(options, 'scp -r ' + options.remote_host + ':' + options.ms + ' .')
result = os.system('scp -r ' + options.remote_host + ':' + options.ms + ' .')
add_log(options, 'Completed copying data:')

add_log(options, "Preparations done")
sys.stdout.flush

print str(result == 0) + str(result)

MS=os.path.split(options.ms)[1]

print 'Start processing: ' + MS
sys.stdout.flush
add_log(options, 'Start processing: ' + MS)

if MS == "":
    add_log(options, "No Measurement set given, exiting")
    sys.exit(1)
else:
    if not os.path.exists(MS):
        add_log(options, "MS does not exist, exiting")
        sys.exit(2)

try:
    os.environ['AIPSPATH']
except:
    add_log(options, "AIPSPATH not set, exiting")
    sys.exit(3)

Bandpass_parset = Parset()
Bandpass_parset['ms']     = MS
Bandpass_parset['fixed']  = 5
Bandpass_parset['window'] = 1
Bandpass_parset.writeToFile("CS1_BandpassCorrector.parset")
fd = open("CS1_BandpassCorrector.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

Flagger_parset = Parset()
Flagger_parset['ms']        = MS
Flagger_parset['existing']  = False
Flagger_parset['threshold'] = 2.0
Flagger_parset.writeToFile("CS1_FrequencyFlagger.parset")
fd = open("CS1_FrequencyFlagger.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

Squasher_parset  = Parset()
(head, tail) = os.path.split(MS)
Squasher_parset['inms']          = MS
Squasher_parset['outms']         = tail + "s"
Squasher_parset['start']         = 0
Squasher_parset['step']          = 32
Squasher_parset['nchan']         = 256
Squasher_parset['threshold']     = 0
Squasher_parset['useflags']      = True
Squasher_parset['allcolumns']    = True
Squasher_parset.writeToFile("CS1_DataSquasher.parset")
fd = open("CS1_DataSquasher.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

# Unsure if these are usefull imager parameters, probably onlt around 60 MHz
Imager_parset  = Parset()
Imager_parset['ms']            = MS
Imager_parset['compress']      = "False"
Imager_parset['datamode']      = "channel"
Imager_parset['imagemode']     = "mfs"
Imager_parset['spwid']         = [0,1]
Imager_parset['nchan']         = 256
Imager_parset['start']         = 0
Imager_parset['step']          = 1
Imager_parset['nx']            = 512
Imager_parset['ny']            = 512
Imager_parset['cellx']         = 750
Imager_parset['celly']         = 750
Imager_parset['stokes']        = "I"
Imager_parset['weighttype']    = "natural"
Imager_parset['weightnpixels'] = 1024
Imager_parset['tile']          = 32
Imager_parset['padding']       = 1.0
Imager_parset['gridfunction']  = "SF"
Imager_parset['imagetype']     = "observed"
Imager_parset['imagename']     = "observed.image"
Imager_parset.writeToFile("CS1_Imager.parset")
fd = open("CS1_Imager.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()


os.system("/app/LOFAR/stable/CS1_BandpassCorrector")
add_log(options, 'CS1_BandpassCorrector finished')
os.system("/app/LOFAR/stable/CS1_FrequencyFlagger")
add_log(options, 'CS1_FrequencyFlagger finished')
os.system("/app/LOFAR/stable/CS1_DataSquasher")
add_log(options, 'CS1_DataSqasher finished')

os.system("glish -l /app/LOFAR/stable/flag_auto.g " + MS)
add_log(options, "Flagging auto correlations done")

os.system("/app/LOFAR/stable/CS1_Imager")
add_log(options, "Imager done")

os.system('rm -rf ' +  MS)
add_log(options, 'Deleting MS finished')
