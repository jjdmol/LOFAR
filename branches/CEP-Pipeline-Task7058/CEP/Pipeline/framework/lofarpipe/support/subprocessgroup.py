import subprocess
from lofarpipe.support.lofarexceptions import PipelineException


class SubProcessGroup(object):
        """
        A wrapper class for the subprocess module: allows fire and forget
        insertion of commands with a an optional sync/ barrier/ return
        """
        def __init__(self, logger=None, usageStats = None,
                     max_concurrent_processes = 8):
            self.process_group = []
            self.logger = logger
            self.usageStats = usageStats
            self.running_process_count = 0
            self.max_concurrent_processes = max_concurrent_processes

            # list of command vdw pairs not started because the maximum
            # number of processes was reached
            self.processes_waiting_for_execution = []

        def _start_process(self, cmd, cwd):
            """
            Helper function collection all the steps needed to start a process
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

            if type(cmd_in) == type(""): #todo ugly
                cmd = cmd_in.split()
            elif type(cmd_in) == type([]):
                cmd = cmd_in
            else:
                raise Exception("SubProcessGroup.run() expects a string or" +
                    "list[string] as arguments suplied: {0}".format(type(cmd)))

            # We need to be able to limit the maximum number of processes
            if running_process_count >= max_concurrent_processes: 
                # Save the command string and cwd
                self.processes_waiting_for_execution.append((cmd, cwd))
            else:
                self._start_process(cmd, cwd)


        def wait_for_finish(self):
            """
            Wait for all the processes started in the current group to end.
            Return the return status of a processes in an dict (None of no 
            processes failed 
            This is a Pipeline component: Of an logger is supplied the 
            std out and error will be suplied to the logger
            """
            collected_exit_status = []
            while (true):
                for idx, (completed, (cmd, process)) in \
                                enumerate(self.process_group):
                    # this process is complete continue
                    if completed:
                        continue

                    # poll returns null if the process is not completed
                    if process.poll() == None:
                        continue

                    # We have a completed process
                    # communicate with the process
                    # TODO: This would be the best place to create a
                    # non mem caching interaction with the processes!
                    # TODO: should a timeout be introduced here to prevent never ending
                    # runs?
                    (stdoutdata, stderrdata) = process.communicate()
                    exit_status = process.returncode

                    # get the exit status
                    if  exit_status != 0:
                        collected_exit_status.append((cmd, exit_status))

                    # log the std out and err
                    if self.logger != None:
                        self.logger.info(cmd)
                        self.logger.debug(stdoutdata)
                        self.logger.warn(stderrdata)
                    else:
                        print cmd
                        print stdoutdata
                        print stderrdata

                    # Now update the state of the internal state
                    self.process_group[idx] = (True, (cmd, process))
                    self.running_process_count -= 1
                    # Finished process, decrease check if there are pending
                    # subprocesses, empthy string evaluates to False
                    if not self.processes_waiting_for_execution:  
                        # Get the last process
                        cmd , cwd = self.processes_waiting_for_execution.pop()
                        # start it
                        self._start_process(cmd, cwd)

            if len(collected_exit_status) == 0:
                collected_exit_status = None
            return collected_exit_status
