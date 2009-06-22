import os
import fcntl
from subprocess import Popen,STDOUT,call
from Hosts import ropen

DRYRUN = False

def debug( str ):
  """ Override with custom logging function. """

  pass

def mpikill( pid, signal ):
  SyncCommand( "mpikill -s %s %s" % (signal,pid) )

class AsyncCommand(object):
    """
    Executes an external command in the background
    """

    def __init__(self, cmd, outfile=None, infile=None, killcmd=os.kill ):
        """ Run command `cmd', with I/O optionally redirected.

            cmd:     command to execute.
            outfile: filename for output, or None.
            infile:  filename for input, or None.
            killcmd: function used to abort, called as killcmd( pid, signal ). """

        if DRYRUN:
          self.cmd = "echo %s" % (cmd,)
        else:
          self.cmd = cmd

        debug("RUN %s: %s > %s" % (self.__class__.__name__,cmd,outfile))

        if outfile is None:
          stdout = None
        else:
          # Line buffer stdout to get lower latency logging info.
          stdout = ropen( outfile, "w", 1 )

        if infile is None:
          stdin = None
        else:
          # Line buffer stdin to satisfy MPIRUN on the BG/P. It will not provide output without it.
          stdin  = ropen( infile, "w", 1 )

        self.done = False
        self.reaped = False
        self.success = False
        self.killcmd = killcmd
        self.popen = Popen( self.cmd.split(), stdin=stdin, stdout=stdout, stderr=STDOUT )
        self.run()

    def run(self):
        """ Will be called when the command has just been started. """

        pass

    def isDone(self):
        """ Return whether the command has finished execution, but do not block. """

        self.done = (self.popen.poll() is not None)

        return self.done

    def wait(self):
        """ Block until the command finishes execution. """

        if self.reaped:
          # already wait()ed before
          return

        self.success = self.popen.wait() == 0
        self.done = True
        self.reaped = True

    def abort(self, signal = 2):
        """ Abort the command, if it has not finished yet. """

        if not self.isDone():
          debug( "Sending signal %s to PID %s" % (signal,self.popen.pid) )

          self.killcmd( self.popen.pid, signal )

    def isSuccess(self):
        """ Returns whether the command returned succesfully. """

        self.wait()
        return self.success

class SyncCommand(AsyncCommand):
    """
    Executes an external command immediately.
    """

    def run(self):
      self.wait()

