import sys
print "Script for generating DPPP config file"
sys.stdout.write("Give the number of processing nodes [18]: ")
nodes = sys.stdin.readline()
if len(nodes) <= 1: nodes = 18
else: nodes = int(nodes)

sys.stdout.write("Give the number of storage nodes[(9]: ")
storage = sys.stdin.readline()
if len(storage) <= 1: storage = 9
else: storage = int(storage)

sys.stdout.write("Give the number of bands[36]: ")
bands = sys.stdin.readline()
if len(bands) <= 1: bands = 36
else: bands = int(bands)

sys.stdout.write("Give the name/location of the dataset in /data [L2008_05199]: ")
location = sys.stdin.readline()
if len(location) <= 1: location = "L2008_05199"

print "Using " + str(nodes) + ',' + str(storage) + ',' + str(bands)

output = """## Example usage
# Destination host is the name of the machine where you want to copy the individual subbands and then process them
# destination_directory is the directory where the MS will be copied into
# ms_location_host is the machine where the original measurement is located (from where you are going to copy it).
# full ms name is the full name of the measurement set with full path
# Remember that if you want to run more than one subband on one node
##  Destination_hostname:destination_directory   ms_location_host:full_ms_name\n"""

#listfen variant
#for b in range(bands):
  #output += "lioff%(n)03d:/lifs%(s)03d/pipeline lifs%(s)03d:/data/%(l)s/SB%(b)d.MS\n" % \
   #{'n':b%nodes+1, 's':b%storage+1, 'l':location[:-1], 'b':b}

#lioffen variant
for b in range(bands):
  output += "lioff%(n)03d:/data/scratch /lifs%(s)03d/%(l)s/%(l)s_SB%(b)d.MS\n" % \
   {'n':(b%nodes+1) + 21, 's':b%storage+1, 'l':location[:-1], 'b':b}

print output + "\n\n"

sys.stdout.write("If the output above is correct then give filename to write to [empty name will cancel]: ")
filename = sys.stdin.readline()
if len(filename) <= 1: exit()
print "Writing to " + filename
ofile = open(filename[:-1], 'w')
ofile.write(output)
ofile.close()
