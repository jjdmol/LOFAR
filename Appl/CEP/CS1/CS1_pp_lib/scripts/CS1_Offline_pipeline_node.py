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

options, args = parser.parse_args()

add_log(options, 'Started processing:')

os.system('rm -rf *.MS *.parset *.debug *.log')
add_log(options, 'Cleaning up old data:')

result = os.system('scp -r ' + options.remote_host + ':' + options.ms + ' .')
add_log(options, 'Completed copying data:')

print "Preparations done"
sys.stdout.flush

assert result == 0

MS=os.path.split(options.ms)[1]

print 'Start processing: ' + MS
sys.stdout.flush
add_log(options, 'Start processing: ' + MS)

if MS == "":
    print "No Measurement set given, exiting"
    sys.exit(1)
else:
    if not os.path.exists(MS):
        print "MS does not exist, exiting"
        sys.exit(2)

try:
    os.environ['AIPSPATH']
except:
    print "AIPSPATH not set, exiting"
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

os.system("/app/LOFAR/stable/CS1_BandpassCorrector")
add_log(options, 'CS1_BandpassCorrector finished')
os.system("/app/LOFAR/stable/CS1_FrequencyFlagger")
add_log(options, 'CS1_FrequencyFlagger finished')
os.system("/app/LOFAR/stable/CS1_DataSquasher")
add_log(options, 'CS1_DataSqasher finished')
#os.system('rm -rf ' +  MS)
add_log(options, 'Deleting MS finished')
