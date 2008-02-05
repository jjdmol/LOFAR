from optparse import OptionParser
from LOFAR_Parset import Parset
import sys, os, time

parser = OptionParser(usage='%prog [options] -m<Obs name>')
parser.add_option("-o", "--obs", dest="obs", default="",
                  help="Observation name")

options, args = parser.parse_args()

Observation = options.obs

##settings
node_dir = '/local/renting'
run_id   = 'test01'
nodes    = [('lioff001','SB0.MS'),('lioff002','SB1.MS'),('lioff003','SB2.MS'),
('lioff004','SB3.MS'),('lioff005','SB4.MS'),('lioff006','SB5.MS'),
('lioff007','SB6.MS'),('lioff008','SB7.MS'),('lioff009','SB8.MS'),
('lioff010','SB9.MS'),('lioff011','SB10.MS'),('lioff012','SB11.MS'),
('lioff013','SB12.MS'),('lioff014','SB13.MS'),('lioff015','SB14.MS'),
('lioff016','SB15.MS'),('lioff017','SB16.MS'),('lioff018','SB17.MS')
]


if Observation == "":
    print "No Observation set given, exiting"
    sys.exit(1)
else:
    if not os.path.exists(Observation):
        print "Observation does not exist, exiting"
        sys.exit(2)

try:
    os.environ['AIPSPATH']
except:
    print "AIPSPATH not set, exiting"
    sys.exit(3)

try:
    os.environ['SSH_AGENT_PID']
except:
    print "SSH-AGENT not active, exiting"
    sys.exit(3)

pidlist = []

def spawn(host, MS):
        try:
            pid = os.fork()
        except OSError, e:
            print 'Unable to fork:' + str(e)
            os._exit(1)
        if pid == 0: ## the new client
            ##Copy node script so we are sure we are running the right version
            os.system('scp CS1_Offline_pipeline_node.py ' + host + ':' + node_dir +
                      '/CS1_Offline_pipeline_node.py')
            ##Copy the subband MS
            os.system('scp -r ' + Observation + '/' + MS +
                      ' ' + host + ':' + node_dir + '/' + MS)
            ##Run the node script on every node
            os.system('ssh -t ' + host +
                      ' "setenv PYTHONPATH /app/LOFAR/stable;'  +
                      'source /app/scripts/doUnstableAIPS++;cd '+node_dir + ';python ' + node_dir +
                      '/CS1_Offline_pipeline_node.py -m' +
                      node_dir + '/' + MS +
                      ' >> ' + run_id + '.log"')
            os._exit(0)
        else: ## parent process
            pidlist.append(pid)

for node in nodes:
    spawn(node[0], node[1])

time.sleep(10) ## wait for childs to spawn
for p in pidlist: ## wait for the clients to finish
    (pid, status) = os.waitpid(p, 0)
    print 'result: ' + str(nodes[pidlist.index(pid)]) + ' ' + str(pid) + ' ' + str(status)

MSlist = []
for node in nodes:
    ## get the squashed measurementsets back
    os.system('scp -r ' + node[0] + ':' + node_dir + '/s' + node[1] + ' .')
    MSlist.append('s' + node[1])

MS = 'result.MS'

Combiner_parset = Parset()
Combiner_parset['inms']  = MSlist
Combiner_parset['out']   = MS
Combiner_parset.writeToFile("CS1_SPWCombine.parset")
fd = open("CS1_SPWComine.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

## Unsure if these are usefull imager parameters, probably onlt around 60 MHz
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
Flagger_parset.writeToFile("CS1_Imager.parset")
fd = open("CS1_Imager.debug", 'w')
fd.write("Global 20\n")
fd.write("Everything 20\n")
fd.close()

## some processing on the final MS
os.system("/app/LOFAR/stable/CS1_SPWCombine")
os.system("glish -l /app/LOFAR/stable/flag_auto.g " + MS)
#os.system("/app/LOFAR/stable/CS1_Imager")
