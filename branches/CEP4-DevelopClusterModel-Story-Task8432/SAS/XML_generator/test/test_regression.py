#! /usr/bin/env python
import sys, os, subprocess, difflib

# diff should only be something like:
# 3,5c3,5
# <           <version>2.10.3</version>
# <           <template version="2.10.3" author="Alwin de Jong" changedBy="Alwin de Jong">
# <           <description>XML Template generator version 2.10.3</description>
# ---
# >   <version>2.12.0</version>
# >   <template version="2.12.0" author="Alwin de Jong,Adriaan Renting" changedBy="Adriaan Renting">
# >   <description>XML Template generator version 2.12.0</description>
def checkDiff(diff):
  if len(diff) == 8 or len(diff) == 0:
    return True
  return False

def main():
  infiles = os.listdir("txt")
  results = []
  for infile in infiles:
    if infile.startswith("old"):
      continue # pre 2.6 files that no longer have valid syntax
    name, ext = os.path.splitext(infile)
    outfile = name + ".xml"
    print "\n"
    print "*** Processing %s ***" % infile
    cmd  = ["../src/xmlgen.py", "-i", "./txt/%s" % infile, "-o", "test.xml"]
    p    = subprocess.Popen(cmd, stdin=open('/dev/null'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    logs = p.communicate()[0].splitlines()  #stdout
    print "xmlgen ran with return code: %s" % p.returncode
    xmlgen = p.returncode
    if p.returncode:
      for l in logs: print l
      results.append((infile, xmlgen, -1, False))
      continue
    else:
      #import fileinput
      #for line in fileinput.FileInput("test.xml",inplace=1):
      #  if line.rstrip():
      #    print line,
      #cmd   = ["sed", "-i", "'/^$/d'", "test.xml"]
      #p     = subprocess.Popen(cmd, stdin=open('/dev/null'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      cmd   = ["diff", "-w", "-I", r"^[[:space:]]*$", "./xml/%s.xml" % name, "test.xml"]
      #cmd   = ["diff -w -I '^[[:space:]]*$' ./xml/%s.xml test.xml" % name]
      ## -w ignores differences in whitespace
      ## -I '^[[:space:]]*$' because -B doesn't work for blank lines (on OSX?)
      p     = subprocess.Popen(cmd, stdin=open('/dev/null'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      logs  = p.communicate()
      diffs = logs[0].splitlines() #stdout
      print "diff reply was %i lines long" % len(diffs)
      check = checkDiff(diffs) and len(logs[1]) == 0
      if not check:
        for l in diffs: print l
        print logs[1]
      results.append((infile, xmlgen, p.returncode, check))
    os.remove("test.xml")
  print "\nResults:"
  success = True
  for r in results:
    print "%s: xmlgen: %i diff: %i, %s" % r
    success = success and r[3]
  if success:
    print "success"
    return 0
  else:
    print "failure"
    return 1

if __name__ == "__main__":
  sys.exit(main())