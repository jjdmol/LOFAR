#!/usr/bin/env python

__all__ = ["ropen","rmkdir","rexists","runlink","rsymlink"]

import os
import subprocess
import socket

HOSTNAME = os.environ.get("HOSTNAME")

def split( filename ):
  """ Internally used: split a filename into host,file. Host == '' if pointing to localhost. """

  if ":" not in filename:
    return "",filename
  else:
    host,file = filename.split(":",1)

    if host in ["localhost",HOSTNAME]:
      host = ""

    return host,file

def ropen( filename, mode = "r", buffering = -1 ):
  """ Open a local or a remote file for reading or writing. A remote file
      has the syntax host:filename or tcp:<ip>:<port>. """

  assert mode in "rwa", "Invalid mode: %s" % (mode,)

  host,file = split( filename )

  if host == "":
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

  host,dir = split( dirname )

  if host == "":
    # a local file
    return os.path.exists( dir ) or os.mkdir( dir )

  # only create directory if it does not exist
  subprocess.call( ["ssh",host,"[ ! -e %s ] && mkdir %s" % (dir,dir)] )

def rexists( filename ):
  """ Checks for the availability of a local or a remote file. A remote
      file has the syntax host:filename. """

  host,file = split( filename )

  if host == "":
    # a local file
    return os.path.exists( file )

  return int(subprocess.Popen( ["ssh",host,"[ ! -e %s ]; echo $?" % (file,)], stdout=subprocess.PIPE ).stdout.read()) == 1

def runlink( filename ):
  """ Deletes a local or a remote file. A remote
      file has the syntax host:filename. """

  host,file = split( filename )

  if host == "":
    # a local file
    return os.unlink( file )

  return int(subprocess.Popen( ["ssh",host,"rm -f '%s'" % (file,)], stdout=subprocess.PIPE ).stdout.read()) == 1

def rsymlink( src, dest ):
  """ Create a symlink at src, pointing to dest.
      src/dest have the syntax host:filename, but dest should not point at a different host. """

  srchost,srcfile = split( src )    
  desthost,destfile = split( dest )    

  assert srchost == desthost, "rsymlink( %s, %s ) requires a link across machines" % (src,dest)

  if srchost == "":
    # a local file
    return os.symlink( dest, src )

  return int(subprocess.Popen( ["ssh",srchost,"ln -s '%s' '%s'" % (destfile,srcfile,)], stdout=subprocess.PIPE ).stdout.read()) == 1

