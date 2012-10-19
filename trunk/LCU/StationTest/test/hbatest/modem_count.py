#!/usr/bin/env python
# Python script to test the HBA modem communication (modem_count.py)
# uses beamduur.sh to generate the test files
# M.J.Norden, V 1.1, 29 April 2010

# import section
import os
import sys

if len(sys.argv) == 1:
    print '--------------------------------------------'
    print 'Error: no arguments found'
    print '--------------------------------------------'
    print ''
    print 'usage modem_count.py directory'
    print '    directory = ./Beamdata'
    print ''
    print '--------------------------------------------'
    print ''
    exit(0)

# Fill dictonairy values with all possible delays values
def create_values(values) :
        n=128
        while n<255:
        	values[str(n)]=0
        	n=n+4
	return values	
	
# Read directory with the files to processs	
def open_dir(dirname) :
	files = filter(os.path.isfile, os.listdir('.'))
	#files.sort(key=lambda x: os.path.getmtime(x))
 	return files

def main() :
	counter=0
	qcounter=0
	values={}
	tiles={}
	g=""
	# read in arguments
	dir_name=sys.argv[1]
	print 'Dir name is ' + dir_name
        os.chdir(dir_name)
	files = open_dir(dir_name)
	create_values(values)
	for filename in files:
		f=file(filename, "r")	
		lines=0
		for line in f:
   			lines+=1
   			a = line.split()
			for pos in a:
				if pos in values :
					# count the valid delay values
					# dictonairy values
					values[pos]+=1
					counter+=1
				elif pos == '???':
					# count the questions mark elements
					# dictonairy tiles
					qcounter+=1
					g=g.join(["HBA[",str(lines-2),"]"])
					tiles[g]=a.index(pos)-1
	print "The ??? counter is ",qcounter
	print "The element counter is ",counter
	h=tiles.keys()
	h.sort()
	for k in h:
		print k,"element",tiles[k],"is ???"
	
	# count how often a delay value is used
	b=values.keys()
	b.sort()
	for k in b:
		print "delay value",k,"exist",values[k],"times"			
main()






