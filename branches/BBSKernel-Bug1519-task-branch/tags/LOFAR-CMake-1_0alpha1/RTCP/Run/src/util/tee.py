from threading import Thread
from select import select
import os,fcntl
import sys

def setNonBlock( fd ):  
  # get the file's current flag settings
  fl = fcntl.fcntl(fd, fcntl.F_GETFL)
  
  # update the file's flags to put the file into non-blocking mode.
  fcntl.fcntl(fd, fcntl.F_SETFL, fl | os.O_NONBLOCK)


class Tee(Thread):
  def __init__(self,inputfd,outputfds):
    Thread.__init__(self)

    self.inputfd = inputfd
    self.outputfds = outputfds[:]

    setNonBlock( self.inputfd )

    self.setDaemon( True )
    self.start()

  def run(self):
    while True:
      rlist,_,xlist = select( [self.inputfd], [], [self.inputfd] )

      if self.inputfd in xlist:
        # exceptional condition
        break

      if self.inputfd in rlist:
        data = os.read( self.inputfd, 4096 )

        if len(data) == 0:
          # eof
          break

        for f in self.outputfds:
          try:
            os.write( f, data )
          except OSError,e:
            pass
