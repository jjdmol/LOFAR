#!/usr/bin/env python

# This script does a basic check of the validity of a component file.

import sys


# Split the line into its individual parts which are separated by whitespace.
# A part can be enclosed in single or double quotes.
def split (line):
    parts = []
    st = 0
    while st < len(line):
        if line[st] in [' ', '\t', '\r']:
            st += 1
        elif line[st] == '"'  or  line[st] == "'":
            quote = line[st]
            st += 1
            end = st
            while end < len(line)  and  line[end] != quote:
                end += 1
            if end > len(line):
                print "Missing quote in line:", line
            parts.append (line[st:end])
            st = end+1
        else:
            end = st
            while end < len(line)  and  line[end] not in [' ', '\t', '\r']:
                end += 1
            parts.append (line[st:end])
            st = end
    return parts

# Check if a node line is correct.
def checknode (parts, nodes):
    if parts[1] in nodes:
        print 'Node', parts1[1], 'is defined multiple times'
    else:
        nodes.append (parts[1])

#par	type		I		text	-	10		0	#"preflagger"	-	"Type of the flagger, do not change"

def checkpar (parts, parNames, node):
    (x0,name,io,dtype,x1,x2,x3,defval,x3,x4) = parts
    if len(node) == 0:
        print 'Par', name, 'used before a node line is given'
    if name in parNames:
        print 'Par', name, 'is defined multiple times in node', node
    else:
        parNames.append (name)
    if io not in ['I','O']:
        print 'Incorrect io type', dtype, 'for par', name, 'in node', node
    if dtype not in ['bool','vbool','int','vint','pint','uint','vuint','flt','vflt','dbl','vdbl','text','vtext','ptext','time']:
        print 'Incorrect data type', dtype, 'for par', name, 'in node', node
    if io == 'O'  and  len(defval) > 0:
        print 'Output par', name, 'in node', node, 'cannot have a default value'
    if dtype[0:1] == 'v'  and  defval == '':
        print 'Vector valued par', name, 'has empty string as default'

def checkuses (parts, nodes):
    if parts[1] not in nodes:
        print 'Used node', parts[1], 'is undefined; probably external'

def checkcomp (fileName):
    nodes = []
    node = ''
    parNames = []
    f = open(fileName)
    for line in f:
        if len(line) > 0  and line[0] != '#':
            # Ignore newline char at end
            parts = split(line[:-1])
##            print len(parts),parts
            if len(parts) > 0:
                if len(parts) == 1:
                    parts.append ('')
                if parts[0] == 'node':
                    if len(parts) != 6:
                        print 'node line', parts[1],'should consists of 6 parts'
                    else:
                        checknode (parts, nodes)
                        node = parts[1]
                        parNames = []
                elif parts[0] == 'par':
                    if len(parts) != 10:
                        print 'par line', parts[1],'should consists of 10 parts'
                    else:
                        checkpar (parts, parNames, node)
                elif parts[0] == 'uses':
                    if len(parts) != 6:
                        print 'uses line', parts[1],'should consists of 6 parts'
                    else:
                        checkuses (parts, nodes)
                else:
                    print 'unknown line type', parts[0]


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print 'Insufficient arguments; run as:'
        print '   checkcomp.py componentfile'
        sys.exit(1)
    checkcomp (sys.argv[1])
