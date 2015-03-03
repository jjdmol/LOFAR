import threading 
import time
import subprocess
import os
import tempfile

from lofarpipe.support.xmllogging import add_child
import xml.dom.minidom as xml


poller_string = """
#! /bin/bash

PID=$(echo $1)
uid=`ls -ld /proc/${PID} | awk '{ print $3 }' `
exe=` ls -l /proc/${PID}/exe | awk '{print $10}'`
rb=`cat /proc/${PID}/io | grep read_bytes | awk '{print $2}'`
wb=`cat /proc/${PID}/io | grep write_bytes | grep -v cancelled | awk '{print $2}'`
cwb=`cat /proc/${PID}/io | grep cancelled_write_bytes | awk '{print $2}'`
TIME=$(($(date +%s%3N))) 
PSOUTPUT=($(ps -p ${PID} -o %cpu,%mem | tail -n 1))  # could be gotten from proc. This works for now ( Optionally use resident -  man ps)
# print the PID, TIME, executable, readbytes,writebytes,cancelled bytes, the cpu% , memory%
echo "['${exe}', '$TIME','${rb}','${wb}','${cwb}','${PSOUTPUT[0]}','${PSOUTPUT[1]}']" 
"""


class UsageStats(threading.Thread):
    def __init__(self, logger,  poll_interval=10.0):
        threading.Thread.__init__(self)
        self.logger = logger
        self.stopFlag = threading.Event()
        self.lock = threading.Lock()
        self.owner_pid = os.getpid()
        self.pid_in = [self.owner_pid]
        self.pid_tracked = []
        self.pid_stats = {}
        self.poll_interval = poll_interval

    def run(self):
        while not self.stopFlag.isSet():
            # *************************************
            # first add new to track pids to the active list
            # in a lock to assure correct functioning
            self.lock.acquire()
            if self.pid_in:           
                self.pid_tracked.extend(self.pid_in)
               
                
                # initiate the location for save stat information
                for pid in self.pid_in:
                    self.pid_stats[pid] = []
                
                self.pid_in = []

            self.lock.release()
            
            (temp_file, temp_path) = tempfile.mkstemp()
            temp_file = open(temp_path, "w")
            temp_file.write(poller_string)
            temp_file.close()

            # now get stats for each tracked pid
            try:
                for pid in self.pid_tracked:
                    pps = subprocess.Popen(["bash", temp_path, str(pid)],
                               stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
                    out, err = pps.communicate()

                    parset_output = eval(out.rstrip()) # remove trailing white space
                    self.pid_stats[pid].append(parset_output)
            finally:
                os.remove(temp_path)

            time.sleep(self.poll_interval)


    def addPID(self, pid):
        self.lock.acquire()
        self.pid_in.append(pid)
        self.lock.release()

    def setStopFlag(self):
        self.stopFlag.set()
     
    def getStatsAsXmlString(self):
        local_document = xml.Document()
        resource_stat_xml = local_document.createElement("resource_usage")

        if not self.pid_stats:  # if there are no entries in the stats dict
            resource_stat_xml.setAttribute("noStatsRecorded", "true")
            return resource_stat_xml.toxml(encoding = "ascii")
        
        try:
            for idx,(key,value) in enumerate(self.pid_stats.iteritems()):
                #if there are entries
                if value:  
                    child_pid = add_child(resource_stat_xml, "process")
                    child_pid.setAttribute("idx", str(idx))          
                    #The first entry should contain the exec name. only needed once
                    child_pid.setAttribute("executable", str(value[0][0]))
                    child_pid.setAttribute("pid", str(key))
                    for entry in value:
                        if "MEM" in str(entry[6]):  # this is the default value
                            continue
                        data_point = add_child(child_pid, "data_point")
                        data_point.setAttribute("timestamp", str(entry[1]))
                        data_point.setAttribute("read_bytes", str(entry[2]))
                        data_point.setAttribute("write_bytes", str(entry[3]))
                        data_point.setAttribute("cancelled_bytes", str(entry[4]))
                        data_point.setAttribute("cpu", str(entry[5]))
                        data_point.setAttribute("mem", str(entry[6]))
        except:
            self.logger.error("monitoring statistic recording failed")
            resource_stat_xml.setAttribute("noStatsRecorded", "Exception")
            return resource_stat_xml.toxml(encoding = "ascii")

        return resource_stat_xml.toxml(encoding = "ascii")

