#!/usr/bin/python
#
# Look for RCUs in ON mode and set EC to observing
#
# 2012 P.Donker

from isEcLib import *
import time
import subprocess
import os,sys

def main():
    host = getIP()
    if host == None:
        print "==============================================="
        print "ERROR, this script can only run on a IS station"
        print "==============================================="
        
    ec = EC(host)
    ec.printInfo(False)    
   
    ec.connectToHost()
    time.sleep(1.0)
    # version is used to check if function is available in firmware
    version,versionstr  = ec.getVersion()  
    ec.disconnectHost()
    
    # run each minute
    while True:
    	RSPobserving = 0
    	TBBobserving = 0
    	
    	response = cmd('rspctl', '--rcu').splitlines()
    	for line in response:
    		pos = line.find('RCU[') 
    		if pos != -1:
    			data = line.strip().split(',')[0].split()[2]
    			if data == 'ON':
    				RSPobserving = 1
    	
    	ec.connectToHost()
    	time.sleep(1.0)
        ec.setObserving(RSPobserving)
        ec.disconnectHost()
        time.sleep(60.0)

# excecute commandline cmd
def cmd(cmd, args=''):
	if len(args) > 0:
		proc = subprocess.Popen([cmd, args], stdout=subprocess.PIPE, stderr = subprocess.STDOUT )
	else:
		proc = subprocess.Popen([cmd], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
	(so, se) = proc.communicate()
	return(so)

# start main()
if __name__ == "__main__":

	# Fork the process, so we can run it as a daemon
	# using /etc/init.d/ecSetObserving [start/stop/status]	
	fpid = os.fork()
	if fpid!=0:
  	# Running as daemon now. PID is fpid
  		sys.exit(0)

	main()



