import subprocess
import select
import os
import signal
import fcntl
import time
from lofarpipe.support.lofarexceptions import PipelineException

class SubProcess(object):
    STDOUT = 1
    STDERR = 2

    def __init__(self, logger, cmd, cwd):
        """
          Start a subprocess for `cmd' in working directory `cwd'.

          Output is sent to `logger', or stdout if logger is None.
        """

        def print_logger(line):
            print line

        self.cmd       = cmd
        self.killed    = False
        self.completed = False
        self.logger    = logger.info if logger else print_logger

        # report we are starting
        self.logger("Subprocess starting: %s" % self.cmd)

        # start process
        self.process = subprocess.Popen(
                       cmd,
                       cwd=cwd,

                       # Set buffering parameters
                       bufsize=1, # 1 = line buffering
                       universal_newlines=True, # translate ^M output by ssh -tt

                       # Don't inherit our fds after fork()
                       close_fds=True,

                       # I/O redirection: block stdin, read stdout/stderr separately
                       stdin=file("/dev/null"),
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE)
        self.pid     = self.process.pid
        self.exit_status = None

        # output source streams
        self.output_streams = { self.STDOUT: self.process.stdout,
                                self.STDERR: self.process.stderr }

        # output buffer
        self.output_buffers = { self.STDOUT: "",
                                self.STDERR: "" }

        # output sink
        self.output_loggers = { self.STDOUT: logger.debug if logger else print_logger,
                                self.STDERR: logger.warn  if logger else print_logger }


        # Set fds to non-blocking to allow <4k reads. This is needed if the process
        # alternates between stdout and stderr output.
        for f in self.output_streams.values():
            flag = fcntl.fcntl(f, fcntl.F_GETFL)
            fcntl.fcntl(f, fcntl.F_SETFL, flag | os.O_NONBLOCK)

    def done(self):
        if self.completed:
            return True

        if self.process.poll() is None:
            return False

        # Process is finished, read remaining data and exit code
        (stdout, stderr) = self.process.communicate()
        self.exit_status = self.process.returncode

        self._addoutput(self.STDOUT, stdout, flush=True)
        self._addoutput(self.STDERR, stderr, flush=True)

        self.completed = True

        self.logger("Subprocess completed: %s" % self.cmd)

        return True

    def kill(self):
        if self.killed:
            return

        os.signal(self.pid, signal.SIGTERM)
        self.killed = True

    def fds(self):
        return self.output_streams.values()

    def read(self, fds):
        for fd in fds:
            if fd == self.process.stdout:
                self._addoutput(self.STDOUT, fd.read(4096))
            if fd == self.process.stderr:
                self._addoutput(self.STDERR, fd.read(4096))

    def _addoutput(self, stdtype, output, flush=False):
        buf = self.output_buffers[stdtype] + output
        lines = buf.split("\n")
        remainder = lines.pop() if lines else ""

        for l in lines:
            self.output_loggers[stdtype](l)

        if flush:
            self.output_loggers[stdtype](remainder)
            remainder = ""

        self.output_buffers[stdtype] = remainder

        # 0-byte read means they closed the fd
        if not output:
            if stdtype in self.output_streams:
                # Don't close (subprocess doesn't like that),
                # but do registera to prevent further select()s.
                del self.output_streams[stdtype]

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
                     polling_interval = 10,
                     killSwitch = None):
            self.process_group = []
            self.logger = logger
            self.usageStats = usageStats
            self.running_process_count = 0
            self.max_concurrent_processes = max_concurrent_processes

            # list of command vdw pairs not started because the maximum
            # number of processes was reached
            self.processes_waiting_for_execution = []
            self.polling_interval = polling_interval

            self.killSwitch = killSwitch

        def _start_process(self, cmd, cwd):
            """
            Helper function collection all the coded needed to start a process
            """

            # Do nothing if we're stopping
            if self.killSwitch and self.killSwitch.isSet():
                return

            # About to start a process, increase the counter
            self.running_process_count += 1

            # Run subprocess
            process = SubProcess(self.logger, cmd, cwd)
            
            # save the process
            self.process_group.append(process)

            # add to resource monitor if available
            if self.usageStats:
                self.usageStats.addPID(process.pid)

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

            while self.running_process_count or self.processes_waiting_for_execution:
                # collect all unfinished processes
                processes = [p for p in self.process_group if not p.completed]

                # check whether we're stopping
                if self.killSwitch and self.killSwitch.isSet():
                    for process in processes:
                        process.kill()

                # collect fds we need to poll
                fds = []
                for process in processes:
                    fds.extend(process.fds())

                # poll for data
                rlist, _, _ = select.select(fds, [], [], self.polling_interval)

                # let processed read their data
                for process in processes:
                    process.read(rlist)

                # check all the running processes for completion
                for process in self.process_group:
                    if process.completed:
                        # process completed earlier
                        continue

                    if not process.done():
                        # process still running
                        continue

                    # We have a completed process
                    exit_status = process.exit_status

                    # get the exit status
                    if exit_status != 0:
                        collected_exit_status.append((process.cmd, exit_status))

                    # Now update the state of the internal state
                    self.running_process_count -= 1

                # if there are less then the allowed processes running and
                # we have waiting processes start another on
                while self.running_process_count < self.max_concurrent_processes and self.processes_waiting_for_execution: 
                    # Get the last process
                    cmd, cwd = self.processes_waiting_for_execution.pop()

                    # start it
                    self._start_process(cmd, cwd)

            # If none of the processes return with error status
            if len(collected_exit_status) == 0:
                collected_exit_status = None

            return collected_exit_status

