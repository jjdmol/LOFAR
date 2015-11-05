#!/usr/bin/env python

import argparse
import glob, os
import re

def findnewestpatch(rev, branch='trunk', patchdir='.'):
  os.chdir(patchdir)
  patches=[]
  patchfiles={}
  branchname=branch.rstrip('/').split('/')[-1]
  for file in glob.glob("*"+branchname+"*.patch"):
    m = re.search('r([0-9]+)', file)
    if m:
      patches+=[int(m.group(1))]
      patchfiles[int(m.group(1))]=file 

  patches=[patchrev for patchrev in patches if patchrev<=rev]
  patches=sorted(patches)

  return patchfiles[patches[-1]]

if __name__ == '__main__':
  parser = argparse.ArgumentParser(description = "Find most recent patch for a casa repository")
  parser.add_argument("rev", help="Revision you want to patch", type=int)
  parser.add_argument("-b", "--branch", help="Branch you want to patch (default is trunk)", default="trunk")
  parser.add_argument("-p", "--patchdir", help="Directory containing patches (default is .)", default=".")

  args = parser.parse_args()

  patchname=findnewestpatch(args.rev, branch=args.branch, patchdir=args.patchdir)
  print patchname
