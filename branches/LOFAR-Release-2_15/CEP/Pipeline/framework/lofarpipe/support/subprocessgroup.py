import subprocess
import time
from lofarpipe.support.lofarexceptions import PipelineException


class SubProcessGroup(object):
        """
        A wrapper class for the subprocess module: allows fire and forget
        insertion of commands with a an optional sync/ barrier/ return
        """
        def __init__(self, logger=None, 
                     usageStats = None,
                     # Default CEP2 is max 8 cpu used
                     max_concurrent_processes = 8,  
                     # poll each 10 seconds: we have a mix of short and long 
                     # running processes
                     polling_interval = 10):        
            self.process_group = []
            self.logger = logger
            self.usageStats = usageStats
            self.running_process_count = 0
            self.max_concurrent_processes = max_concurrent_processes

            # list of command vdw pairs not started because the maximum
            # number of processes was reached
            self.processes_waiting_for_execution = []
            self.polling_interval = polling_interval

        def _start_process(self, cmd, cwd):
            """
            Helper function collection all the coded needed to start a process
            """
            # About to start a process, increase the counter
            self.running_process_count += 1

            # Run subprocess
            process = subprocess.Popen(
                        cmd,
                        cwd=cwd,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
            
            # save the process
            self.process_group.append((False, (cmd, process)))

            # add to resource monitor if available
            if self.usageStats:
                self.usageStats.addPID(process.pid)

            if self.logger == None:
                print "Subprocess started: {0}".format(cmd)
            else:
                self.logger.info("Subprocess started: {0}".format(cmd))


        def run(self, cmd_in, unsave=False, cwd=None):
            """
            Add the cmd as a subprocess to the current group: The process is
            started!
            cmd can be suplied as a single string (white space seperated)
            or as a list of strings
            """

            if isinstance(cmd_in, str): 
                cmd = cmd_in.split()
            elif isinstance(cmd_in, list):
                cmd = cmd_in
            else:
                raise Exception("SubProcessGroup.run() expects a string or" +
                    "list[string] as arguments suplied: {0}".format(type(cmd)))

            # We need to be able to limit the maximum number of processes
            if self.running_process_count >= self.max_concurrent_processes: 
                # Save the command string and cwd
                self.processes_waiting_for_execution.append((cmd, cwd))
            else:
                self._start_process(cmd, cwd)


        def wait_for_finish(self):
            """
            Wait for all the processes started in the current group to end.
            Return the return status of a processes in an dict (None of no 
            processes failed 
            This is a Pipeline component: If an logger is supplied the 
            std out and error will be suplied to the logger
            """
            collected_exit_status = []

            while (True):
                # The exit test
                if (self.running_process_count == 0 and 
                   len(self.processes_waiting_for_execution) == 0):
                    break # the while loop

                # start with waiting 
                time.sleep(self.polling_interval)

                # check all the running processes for completion
                for idx, (completed, (cmd, process)) in \
                                enumerate(self.process_group):
                    if completed:
                        continue

                    # poll returns null if the process is not completed
                    if process.poll() == None:
                        continue

                    # We have a completed process
                    # communicate with the process
                    # TODO: This would be the best place to create a
                    # non mem caching interaction with the processes!
                    (stdoutdata, stderrdata) = process.communicate()
                    exit_status = process.returncode

                    # get the exit status
                    if  exit_status != 0:
                        collected_exit_status.append((cmd, exit_status))

                    # log the std out and err
                    if self.logger != None:
                        self.logger.info(
                             "Subprocesses group, completed command: ")
                        self.logger.info(cmd)
                        self.logger.debug(stdoutdata)
                        self.logger.warn(stderrdata)
                    else:
                        print "Subprocesses group, completed command: "
                        print cmd
                        print stdoutdata
                        print stderrdata

                    # Now update the state of the internal state
                    self.process_group[idx] = (True, (cmd, process))
                    self.running_process_count -= 1
                                
                   
                # if there are less then the allowed processes running and
                # we have waiting processes start another on
                while (self.running_process_count < self.max_concurrent_processes 
                    and
                    len(self.processes_waiting_for_execution) != 0): 
                    # Get the last process
                    cmd , cwd = self.processes_waiting_for_execution.pop()
                    # start it
                    self._start_process(cmd, cwd)

            # If none of the processes return with error status
            if len(collected_exit_status) == 0:
                collected_exit_status = None
            return collected_exit_status
