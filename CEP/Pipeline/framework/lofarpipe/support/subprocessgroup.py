import subprocess
from lofarpipe.support.lofarexceptions import PipelineException


class SubProcessGroup(object):
        """
        A wrapper class for the subprocess module: allows fire and forget
        insertion of commands with a an optional sync/ barrier/ return
        """
        def __init__(self, logger=None):
            self.process_group = []
            self.logger = logger


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

            # Run subprocess
            process = subprocess.Popen(
                        cmd,
                        cwd=cwd,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)
            # save the process
            self.process_group.append((cmd, process))

            # TODO: SubProcessGroup could saturate a system with to much 
            # concurent calss: artifical limit to 20 subprocesses
            if not unsave and (len(self.process_group) > 20):
                self.logger.error("Subprocessgroup could hang with more"
                    "then 20 concurent calls, call with unsave = True to run"
                     "with more than 20 subprocesses")
                raise PipelineException("Subprocessgroup could hang with more"
                    "then 20 concurent calls. Aborting")

            if self.logger == None:
                print "Subprocess started: {0}".format(cmd)
            else:
                self.logger.info("Subprocess started: {0}".format(cmd))

        def wait_for_finish(self):
            """
            Wait for all the processes started in the current group to end.
            Return the return status of a processes in an dict (None of no 
            processes failed 
            This is a Pipeline component: Of an logger is supplied the 
            std out and error will be suplied to the logger
            """
            collected_exit_status = []
            for cmd, process in self.process_group:
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

            if len(collected_exit_status) == 0:
                collected_exit_status = None
            return collected_exit_status
