#!/usr/bin/env python

__all__ = ["ropen"]

import os
import subprocess

HOSTNAME = os.environ.get("HOSTNAME")

def ropen( filename, mode = "r", buffering = -1 ):
  """ Open a local or a remote file for reading or writing. A remote file
      has the syntax host:filename. """

  assert mode in "rwa", "Invalid mode: %s" % (mode,)

  if ":" not in filename:
    # a local file
    return open(filename, mode, buffering)

  host,file = filename.split(":",2)

  if host in ["","localhost",HOSTNAME]:
    # a local file
    return open(file, mode)

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

