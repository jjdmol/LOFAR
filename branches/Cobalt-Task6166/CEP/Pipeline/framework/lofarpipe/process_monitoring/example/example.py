#! /usr/bin/python
"""Example of the monitoring class usage. This example basically shows all functionality currently implemented. """
import subprocess
import os
import time
import socket

print "Best started in background so that you can observe the monitor files."

mypid = os.getpid()
mp = subprocess.Popen(["../monitor.py",str(mypid)]) # Use PID of caller to identify which monitor to talk with

time.sleep(3) # wait till it started. This time is fully is arbitrary.

sp = subprocess.Popen("./script.py", stdout=subprocess.PIPE) # start the external script we want to monitor and ask for the PID.
monpid = sp.pid

print "monitoring started. See for output monitor_{0} (and for error messages error_{0}).".format(mypid)
 
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("testscript {0}".format(monpid)) # We want to monitor the application with PID monpid and write lines to the logs using the name testscript
time.sleep(60) # generate some data to observe

s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("stop") # stop the running monitoring service. NB: service currently stays alive if not explicitly killed in this setup.

print "monitoring stopped. First demo done. Proceeding to second one."

mypid += 1 # Just to change the name of the monitoring file we create a fake 'PID'. The communication hook can of course be anything anyhow.

mp2 = subprocess.Popen(["../monitor.py",str(mypid)])  # Start a new monitor. NB: we could have kept the previous one alive of course.
time.sleep(3)

print "monitoring started. See for output monitor_{0} (and for error messages error_{0}.".format(mypid)
# now let's monitor two scripts at the same time, wait untill they end and clean up.

sp2 = subprocess.Popen("./script.py", stdout=subprocess.PIPE)
monpid2 = sp2.pid
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("testscript_2 {0}".format(monpid2))
print "Started script 2 and added to monitoring"
time.sleep(60) # Simulate some stuff happening here by just waiting a bit.

sp3 = subprocess.Popen("./script.py", stdout=subprocess.PIPE)
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

time.sleep(5) # just give it some time to generate data points with zeros.
print "All done. Stopping monitoring..."

s=socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.connect("/tmp/{0}_pipesock".format(mypid))
s.send("stop") # stop monitoring!
