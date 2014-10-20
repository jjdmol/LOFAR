#! /usr/bin/python
"""Example of the monitoring class usage. This example basically shows all functionality currently implemented. """
import subprocess
import os
import time
import socket

#
# ****************************************************************************
# Start the monitoring software
print "Best started in background so that you can observe the monitor files."
# is this the preferred way to start the monitor?
# We might make it part of the pipeine framework as an include?
mypid = os.getpid()

# Use PID of caller to identify which monitor to talk with
mp = subprocess.Popen(["monitor.py",str(mypid)]) 

# This wait would not be needed if imported in the the class
time.sleep(1) # wait till it started. This time is fully is arbitrary
print "monitoring started. See for output monitor_{0} ".format(mypid) + \
      "(and for error messages error_{0}).".format(mypid)


# ****************************************************************************
# start the external script we want to monitor and ask for the PID.
sp = subprocess.Popen("example/script.py", stdout=subprocess.PIPE) 
monpid = sp.pid

# create connection to the monitoring software
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))

# We want to monitor the application with PID monpid and write lines to the logs using the name testscript
s.send("testscript {0}".format(monpid)) 
time.sleep(60) # generate some data to observe

# stop the running monitoring service. NB: service currently stays alive if not explicitly killed in this setup.
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("stop") 
print "monitoring stopped. First demo done. Proceeding to second one."


# ****************************************************************************
# Start a new monitor
mypid += 1 # Just to change the name of the monitoring file we create a fake 'PID'. The communication hook can of course be anything anyhow.
# what happens if we reuse the old monitor pid??

mp2 = subprocess.Popen(["monitor.py",str(mypid)])  # Start a new monitor. NB: we could have kept the previous one alive of course.
time.sleep(3)

print "monitoring started. See for output monitor_{0} (and for error messages error_{0}.".format(mypid)
# now let's monitor two scripts at the same time, wait untill they end and clean up.

sp2 = subprocess.Popen("example/script.py", stdout=subprocess.PIPE)
monpid2 = sp2.pid
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("testscript_2 {0}".format(monpid2))
print "Started script 2 and added to monitoring"
time.sleep(60) # Simulate some stuff happening here by just waiting a bit.

sp3 = subprocess.Popen("example/script.py", stdout=subprocess.PIPE)
monpid3 = sp3.pid
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("testscript_3 {0}".format(monpid3))
print "Started script 3 and added to monitoring"

sp2.communicate()
print "script 2 is done now. Removing it from monitoring..."

s=socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("del {0}".format(monpid2)) # stop monitoring script 2 but keep going on monitoring anything else.
sp3.communicate()
print "script 3 is done now. Removing it from monitoring..."

s=socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("del {0}".format(monpid3)) # stop monitoring script 3 but keep going on monitoring anything else.

# just give it some time to generate data points with zeros.
# why??
time.sleep(5) 
print "All done. Stopping monitoring..."

s=socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("stop") # stop monitoring!
