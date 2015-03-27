#                                                      LOFAR PIPELINE FRAMEWORK
#
#                                                               UsageStat class
#                                                           Wouter Klijn, 2014
#                                                               klijn@astron.nl
#
# TODO:
# 1. The stats are currently collecting using os calls and subprocess.
#    A light weight version might use /proc/pid directly
# 2. Large scale refactor: 
#    Enforce the reading and writing of each recipe to a specific directory
#    In combination with du we could then exactly measure how much file data is
#    written.
#    An idea is to make this part of the recipe. Although I like the idea of a
#    decorator: this makes is explicit
# 3. Creating a decorator version of this stat collected might be a plan anyways
#    Marcel suggested a with-block syntax. When lots of decorated functionality
#    becomes available this might lead to boilerplate eg:
#
#    @logging("scriptname")
#    @usage_stat("scriptname")
#    def recipename(bla):
#
#    vs
#    def recipename(bla):
#        with logging("striptname"):
#        with usage_stat("scriptname"):
#        with xml_loggin("scriptname"):
#     
# 4. I dont like that xml magix is needed to fill the nodes.
#    A number of helper function on the xmllogging might be a good idea.
#    Supporting direct xml should still be possible for lo-tech / api acces
#    should be supported also.
# ------------------------------------------------------------------------------
import threading 
import time
import subprocess
import os
import tempfile

from lofarpipe.support.xmllogging import add_child
import xml.dom.minidom as xml


poller_string = """
#!/bin/bash -e

PID=$1
exe=$(readlink /proc/${PID}/exe) || exit 1
rb=$(grep ^read_bytes /proc/${PID}/io | awk '{print $2}')
wb=$(grep ^write_bytes /proc/${PID}/io | awk '{print $2}')
cwb=$(grep ^cancelled_write_bytes /proc/${PID}/io | awk '{print $2}')
TIME=$(date +%s%3N) 
PSOUT=($(ps -p ${PID} --no-header -o %cpu,%mem))  # could be gotten from proc. This works for now ( Optionally use resident -  man ps)
# print executable, TIME, readbytes, writebytes, cancelled bytes, %cpu, %memory
echo "['${exe}','${TIME}','${rb}','${wb}','${cwb}','${PSOUT[0]}','${PSOUT[1]}']"
"""


class UsageStats(threading.Thread):
    """
    Class used for monitoring resource usage of processes.
    Each poll_interval mem, cpu and disk activity of trackeds pid is gathered

    usage:
    After initiation it must be started using the start() method
    Pids to monitor can be added with addPID
    setStopFlag stops the monitor
    The recorded data can be retrieved using the getStatsAsXmlString

    Should be thread save (adding of pids is done in a lock)
    Created temp files should be cleaned on keyboard intterupt
    """
    # TODO: Make class "with-block" aware by defining __enter__ and __exit__
    # This make code using this class cleaner.
    def __init__(self, logger=None,  poll_interval=10.0):
        """
        Create the usage stat object. Create events for starting and stopping.
        By default the Process creating the object is tracked.
        Default polling interval is 10 seconds
        """
        threading.Thread.__init__(self)
        self.logger = logger
        self.stopFlag = threading.Event()
        self.lock = threading.Lock()
        self.owner_pid = os.getpid()
        self.pid_in = [self.owner_pid]
        self.pid_tracked = []
        self.pid_stats = {}
        self.poll_interval = poll_interval
        self.poll_counter = poll_interval
        self.temp_path = None

        # Write a bash file which will retrieve the needed info from proc
        (temp_file, self.temp_path) = tempfile.mkstemp()
        temp_file = open(self.temp_path, "w")
        temp_file.write(poller_string)
        temp_file.close()
            
    def __del__(self):
        """
        Clean up the temp file after the file is not in use anymore
        """
        os.remove(self.temp_path)
            

    def run(self):
        """
        Run function. 

        While no stopflag is set:
        Add the new PIDs that might have been added during the sleep
        Create the bash script used for collecting data from os
        Parse data and add to storage member
        Remove the bash script 
        sleep for poll_interval
        """
        while not self.stopFlag.isSet():
            # If the timer waits for the full 10 minutes using scripts
            # appear halting for lnog durations after ending
            # poll in a tight wait loop to allow quick stop
            if self.poll_counter < self.poll_interval:
                self.poll_counter += 1
                time.sleep(1)
                continue

            # reset the counter to zero
            self.poll_counter = 0           

            # *************************************
            # first add new to track pids to the active list
            # in a lock to assure correct functioning
            # TODO: use "with self.lock:" instead of manual acquire/release
            self.lock.acquire()
            if self.pid_in:
                self.pid_tracked.extend(self.pid_in)
               
                
                # initiate the location for save stat information
                for pid in self.pid_in:
                    self.pid_stats[pid] = []
                
                self.pid_in = []
            self.lock.release()
           
            # now get stats for each tracked pid           
            pid_out = []
            for pid in self.pid_tracked:
                pps = subprocess.Popen(["bash", self.temp_path, str(pid)],
                            stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
                out, err = pps.communicate()
                # Store pids that are not active anymore (process has closed)
                if not pps.returncode == 0:
                    pid_out.append(pid)
                    continue                    # nothing to store continue
                    
                # remove trailing white space
                parset_output = eval(out.rstrip()) 
                self.pid_stats[pid].append(parset_output)
                
            # in the previous loop we stored al the pid that are invalid and 
            # can be cleared
            for pid in pid_out:
                try:
                    self.pid_tracked.remove(pid)
                except ValueError:  # key does not exist (should not happen)
                    pass

    def addPID(self, pid):
        """
        Threadsave add a process id to track. It will be added at the next 
        poll_interval
        """
        # TODO: use "with self.lock:" instead of manual acquire/release
        self.lock.acquire()
        self.pid_in.append(pid)
        self.lock.release()

    def setStopFlag(self):
        """
        Stop the monitor
        """
        self.stopFlag.set()
     
    def getStatsAsXmlString(self):
        """
        returns the collected data as a xml file
        Data is cleaned and labeled according to the metric used.
        """
        local_document = xml.Document()
        resource_stat_xml = local_document.createElement("resource_usage")
        resource_stat_xml.setAttribute("node_recipe_pid",str(self.owner_pid))

        if not self.pid_stats:  # if there are no entries in the stats dict
            resource_stat_xml.setAttribute("noStatsRecorded", "true")
            return resource_stat_xml.toxml(encoding = "ascii")
        
        try:
            # TODO: The returned values are not in order and the owner PID
            # might not be printed with idx 0. Maybee print seperately
            for idx,(key,value) in enumerate(self.pid_stats.iteritems()):
                # if there are entries
                if value:  
                    child_pid = add_child(resource_stat_xml, "process")
                    child_pid.setAttribute("idx", str(idx))          
                    # The first entry should contain the exec name.
                    #  only needed once
                    child_pid.setAttribute("executable", str(value[0][0]))
                    child_pid.setAttribute("pid", str(key))
                    for entry in value:
                        # TODO: probably no longer needed with updated bash 
                        # script
                        if "MEM" in str(entry[6]):  # this is the default value
                            continue
                        data_point = add_child(child_pid, "data_point")
                        data_point.setAttribute("timestamp", str(entry[1]))
                        data_point.setAttribute("read_bytes", str(entry[2]))
                        data_point.setAttribute("write_bytes", str(entry[3]))
                        data_point.setAttribute("cancelled_bytes", 
                                                str(entry[4]))
                        data_point.setAttribute("cpu", str(entry[5]))
                        data_point.setAttribute("mem", str(entry[6]))
        except:
            self.logger.warn("monitoring statistic recording failed")
            resource_stat_xml.setAttribute("noStatsRecorded", "Exception")
            # TODO: coalesce these two returns in one "finally:"
            return resource_stat_xml.toxml(encoding = "ascii")

        return resource_stat_xml.toxml(encoding = "ascii")

