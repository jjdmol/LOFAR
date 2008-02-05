from optparse import OptionParser
from LOFAR_Parset import Parset
import sys, os

parser = OptionParser(usage='%prog [options] -m<MS name>')
parser.add_option("-m", "--msin", dest="msin", default="",
                  help="Measurementset name")

options, args = parser.parse_args()

MS = options.msin

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
Bandpass_parset['ms'] = MS
Bandpass_parset['fixed']   = 5
Bandpass_parset['window'] = 1
Bandpass_parset.writeToFile("CS1_BandpassCorrector.parset")
fd = open("CS1_BandpassCorrector.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

Flagger_parset = Parset()
Flagger_parset['ms'] = MS
Flagger_parset['existing'] = False
Flagger_parset['threshold']= 1.1
Flagger_parset.writeToFile("CS1_frequencyFlagger.parset")
fd = open("CS1_FrequencyFlagger.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

Squasher_parset  = Parset()
(head, tail) = os.path.split(MS)
Squasher_parset['inms']          = MS
Squasher_parset['outms']         = head + "/s" + tail
Squasher_parset['start']         = 0
Squasher_parset['step']          = 64
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
os.system("/app/LOFAR/stable/CS1_FrequencyFlagger")
os.system("/app/LOFAR/stable/CS1_DataSquasher")
