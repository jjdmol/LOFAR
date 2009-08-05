#!/usr/bin/env python

__all__ = ["ropen","rmkdir","rexists"]

import os
import subprocess
import socket

HOSTNAME = os.environ.get("HOSTNAME")

def ropen( filename, mode = "r", buffering = -1 ):
  """ Open a local or a remote file for reading or writing. A remote file
      has the syntax host:filename or tcp:<ip>:<port>. """

  assert mode in "rwa", "Invalid mode: %s" % (mode,)

  if ":" not in filename:
    # a local file
    return open(filename, mode, buffering)

  host,file = filename.split(":",1)

  if host in ["","localhost",HOSTNAME]:
    # a local file
    return open(file, mode, buffering)

  if host == "tcp":
    # create a TCP socket
    s = socket.socket()
    ip,port = file.split(":")
    s.connect( (ip,int(port)) )
    return s.makefile(mode, buffering)

  modelist = {
    "r": "cat %s" % (file,),
    "w": "cat - > %s" % (file,),
    "a": "cat - >> %s" % (file,),
  }

  primitive = {
    "r": "r",
    "w": "w",
    "a": "w",
  }

  assert mode in modelist, "Invalid mode: %s" % (mode,)

  if mode in "wa":
    # writing
    return subprocess.Popen( ["ssh",host,modelist[mode]], bufsize=buffering, stdin=subprocess.PIPE ).stdin
  else:
    # reading
    return subprocess.Popen( ["ssh",host,modelist[mode]], bufsize=buffering, stdout=subprocess.PIPE ).stdout

def rmkdir( dirname ):
  """ Make a local or a remote directory. A remote directory name
      has the syntax host:filename. """

  if ":" not in dirname:
    # a local file
    return os.path.exists( dirname ) or os.mkdir( dirname )

  host,dir = dirname.split(":",2)

  if host in ["","localhost",HOSTNAME]:
    # a local file
    return os.path.exists( dir ) or os.mkdir( dire )

  # only create directory if it does not exist
  subprocess.call( ["ssh",host,"[ ! -e %s ] && mkdir %s" % (dir,dir)] )

def rexists( filename ):
  """ Checks for the availability of a local or a remote file. A remote
      file has the syntax host:filename. """

  if ":" not in filename:
    # a local file
    return os.path.exists( filename )

  host,file = filename.split(":",2)

  if host in ["","localhost",HOSTNAME]:
    # a local file
    return os.path.exists( file )

  return int(subprocess.Popen( ["ssh",host,"[ ! -e %s ]; echo $?" % (file,)], stdout=subprocess.PIPE ).stdout.read()) == 1

def runlink( filename ):
  """ Deletes a local or a remote file. A remote
      file has the syntax host:filename. """

  if ":" not in filename:
    # a local file
    return os.path.exists( filename )

  host,file = filename.split(":",2)

  if host in ["","localhost",HOSTNAME]:
    # a local file
    return os.unlink( file )

  return int(subprocess.Popen( ["ssh",host,"rm -f %s" % (file,)], stdout=subprocess.PIPE ).stdout.read()) == 1
