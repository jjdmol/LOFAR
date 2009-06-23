import os
import fcntl
import socket
from subprocess import Popen,STDOUT,call
from Hosts import ropen
from tee import Tee

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

    def __init__(self, cmd, outfiles=[], infile=None, killcmd=os.kill ):
        """ Run command `cmd', with I/O optionally redirected.

            cmd:      command to execute.
            outfiles: filenames for output, or [] to use stdout.
            infile:   filename for input, or None to use stdin.
            killcmd:  function used to abort, called as killcmd( pid, signal ). """

        if DRYRUN:
          self.cmd = "echo %s" % (cmd,)
        else:
          self.cmd = cmd

        debug("RUN %s: %s > %s" % (self.__class__.__name__,cmd,", ".join(outfiles)))

        if outfiles == []:
          stdout = None
        else:
          # open all outputs, remember them to prevent the files
          # from being closed at the end of __init__
          self.outputs = [ropen( o, "w", 1 ) for o in outfiles]

          if len(self.outputs) == 1:
            # one output, no need to multiplex
            stdout = self.outputs[0]
          else:
            # create a pipe to multiplex everything through
            r,w = os.pipe()

            # feed all file descriptors to Tee
            Tee( r, [o.fileno() for o in self.outputs] )

            # keep the pipe input
            stdout = w

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

    def mergeOutputs( outputs ):
      """ Merges outputs (a list of strings) into one file descriptor. """


      return w

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

        try:
          self.success = self.popen.wait() == 0
        except OSError:
          # process died prematurely or never started?
          self.success = False

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

