#!/usr/bin/env python

import os
import sys
import numpy as numpy
import pyrap.tables as pt

# Add demix directory to sys.path before importing demix modules.
sys.path.insert(0, os.path.join(os.path.dirname(sys.argv[0]), "demix"))

import shiftphasecenter as spc
import demixing as dmx
import smoothdemix as smdx
import subtract_from_averaged as sfa


def demix (infile, remove, target='target',
           clusterdesc='/home/diepen/cep2.clusterdesc',
           timestep=10, freqstep=60, half_window=20, threshold=2.5):
#
#  wrapper routine to Bas vdTol's demixing scripts
#
#  problems/complaints: neal.jackson@manchester.ac.uk
#  further modified by R van Weeren, rvweeren@strw.leidenuniv.nl
#  further modified by G van Diepen, diepen@astron.nl
# args: infile  MS to be demixed
#       remove  list of stuff to remove eg ['CygA','CasA'] 
#       target  name of target  (default 'target')
#    half_window=20   integer window size of median filter, 20 is a good choice
#    threshold  =2.5  solutions above/below threshold*rms are smoothed, 2.5 is a good choice
#
# invocation within python: import do_demix
#                           do_demix.demix('infile',['remove1','remove2'],'target_name')
#
#  *** NOTE: assumes that the MS to be demixed has a 'uv' in the middle of it!!!
#  *** NOTE: you need to "use LofIm" before starting (only tcsh working tested!!!)  
#  *** NOTE: do not run two demixing sessions in the same directory in parallel!!
# --------------------------------------------------------------------------
# edit the following two lines only if you know what you're doing !!!
# OTHERWISE DO NOT TOUCH THE STUFF BELOW
#
    demixdir = '/home/weeren/scripts/demixingfast/'
    skymodel = '/home/weeren/scripts/Ateam_LBA_CC.skymodel'

# Note that sys.path contains the directory from which the main module is loaded.
# That directory also contains the other scripts/modules used further down,
# so python will find them.
# Extend sys.path with '.' to make sure local python scripts are found.
    sys.path.append('.')

    (dirname, filename) = os.path.split(infile)
    key = os.path.join(dirname, 'key_' + filename)
    mixingtable = os.path.join(dirname, 'mixing_' + filename)
    basename = infile.replace('_uv.MS', '') + '_'

#  If needed, run NDPPP to preflag input file out to demix.MS

    t = pt.table(infile)
    shp = t.getcell("DATA", 0).shape
    t = 0
    mstarget = infile.replace('uv',target)
    os.system ('rm -f -r ' + mstarget)
    if (shp[0] == 64  or  shp[0] == 128  or  shp[0] == 256):
        f=open(basename + 'NDPPP_dmx.parset','w')
        f.write('msin = %s\n' % infile)
        f.write('msin.autoweight = True\n')
        f.write('msin.startchan = nchan/32\n')
        f.write('msin.nchan = 30*nchan/32\n')
        f.write('msout = %s\n' % mstarget)
        f.write('steps=[preflag]\n')
        f.write('preflag.type=preflagger\n')
        f.write('preflag.corrtype=auto\n')
        f.close()
        os.system ('NDPPP ' + basename + 'NDPPP_dmx.parset')
    else:
        os.system ('cp -r ' + infile + ' ' + mstarget)


    print 'Removing targets '+str(remove)+' from '+mstarget
    spc.shiftphasecenter (mstarget, remove, freqstep, timestep)

# for each source to remove, and the target, do a freq/timesquash NDPPP

    removeplustarget = numpy.append (remove, target)
    avgoutnames = []

    for rem in removeplustarget:
        os.system ('rm -f ' + basename + 'dmx_avg.parset')
        f=open(basename + 'dmx_avg.parset','w')
        msin = infile.replace('uv',rem)
        f.write('msin = %s\n' % msin)
        msout = msin.replace ('.MS','_avg.MS')
        f.write('msout = %s\n' % msout)
        f.write('steps=[avg]\n')
        f.write('avg.type = averager\n')
        f.write('avg.timestep = %d\n' % timestep)
        f.write('avg.freqstep = %d\n' % freqstep)
        f.close()
        print 'Squashing '+msin+' to '+msout
        os.system ('rm -f -r '+msout)
        os.system ('NDPPP '+ basename + 'dmx_avg.parset')
        # Form avg output names.
        msin = infile.replace('uv',rem)
        msout = msin.replace ('.MS','_avg.MS')
        avgoutnames.append (msout)
        msdem = msin.replace ('.MS','_avg_dem.MS')
        os.system ('rm -f -r '+msdem)

    print '****************************************************\n\n'
    print '     Running the demixing algorithm\n'
    print '\n\n****************************************************\n'

    dmx.demixing (mstarget, mixingtable, avgoutnames, freqstep, timestep, 4)

    print '****************************************************\n\n'
    print '     Finished the demixing algorithm, beginning BBS... \n'
    print '\n\n****************************************************\n'

#
#  run BBS on the demixed measurement sets
#
    for i in remove:
        print 'Doing: ', i
        msin = infile.replace('uv', i)
        msout = msin.replace ('.MS','_avg_dem.MS')  
#
# nb not _avg.MS as in instructions
#
        os.system ('rm -f -r '+ basename + i +'.vds')
        os.system ('rm -f -r '+ basename + i +'.gds')
        print 'Create vds & gds files....'
        os.system ('makevds '+clusterdesc+' '+msout+' '+ basename + i +'.vds')
        os.system ('combinevds '+ basename + i +'.gds '+ basename + i +'.vds')
        command='calibrate -f --key '+ key + ' --cluster-desc '+clusterdesc+' --db ldb001 --db-user postgres '+ basename + i+'.gds '+demixdir+'bbs_'+i+'.parset '+skymodel+' $PWD'
        print command
        os.system(command)

        input_parmdb = msout + '/instrument'
        output_parmdb= msout + '/instrument_smoothed'
        smdx.smoothparmdb(input_parmdb,output_parmdb, half_window, threshold)
        command='calibrate --clean --skip-sky-db --skip-instrument-db --instrument-name instrument_smoothed --key '+ key + ' --cluster-desc '+clusterdesc+' --db ldb001 --db-user postgres '+ basename + i +'.gds '+demixdir+'bbs_'+i+'_smoothcal.parset '+skymodel+' $PWD'
        print command
        os.system(command)


# Form the list of input files and subtract.
    demfiles = [infile.replace('uv',rem+'_avg_dem') for rem in remove]
    sfa.subtract_from_averaged (mstarget.replace('.MS','_avg.MS'),
                                mixingtable,
                                demfiles,
                                mstarget.replace('.MS','_sub.MS'))
# We're done.


# This is the main entry.
if __name__ == "__main__":
    if len(sys.argv) < 3:
        print 'Insufficient arguments; run as:'
        print '   do_demixing.py ms remove [target] [clusterdesc] [timestep] [freqstep]'
        print '      ms           MS to be demixed (its name must contain "uv")'
        print '      remove       list of stuff to remove e.g. CygA,CasA'
        print '      target       replaces "uv" in output MS name (default "target")'
        print '      clusterdesc  name of clusterdesc file to use'
        print '                   default is /home/diepen/cep2.clusterdesc'
        print '      timestep     timestep for averaging (default 10)'
        print '      freqstep     freqstep for averaging (default 60)'
        sys.exit(1)
    msin = sys.argv[1]
    remove = sys.argv[2].split(',')
    target = 'target'
    if len(sys.argv) > 3:
        target = sys.argv[3]
        if target == 'uv':
            print 'Name of target cannot be "uv"'
            sys.exit(1)
    clusterdesc = '/home/diepen/cep2.clusterdesc'
    if len(sys.argv) > 4:
        clusterdesc = sys.argv[4]
    timestep = 10
    if len(sys.argv) > 5:
        timestep = int(sys.argv[5])
    freqstep = 60
    if len(sys.argv) > 6:
        freqstep = int(sys.argv[6])
    msout = msin.replace('uv', target)
    if msin == msout:
        print 'Name of input MS must contain "uv"'
        sys.exit(1)
    demix (msin, remove, target, clusterdesc, timestep, freqstep)
