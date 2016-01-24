""" script for determing the peak in the spectrum
Andre 10 July 2009
Usage python ./determinepeak.py [# of RCUs]

"""
# INIT

import array
import operator
import os
import time
import sys
import math
import numpy

# Read directory with the files to processs 
def open_dir(dirname) :
    files = filter(os.path.isfile, os.listdir('.'))
    #files.sort(key=lambda x: os.path.getmtime(x))
    return files

def rm_files(dir_name,file) :
    cmdstr = 'rm ' + file
    os.popen3(cmdstr)
    return

def rec_stat(dirname,num_rcu) :
    os.popen("rspctl --statistics --duration=10 --integration=10 --select=0:" + str(num_rcu-1) + " 2>/dev/null")
    return

# Open file for processsing
def open_file(files, file_nr) :
    # check if file is data file, no junk
    if files[file_nr][-3:] == 'dat':
        file_name = files[file_nr]
        fileinfo = os.stat(file_name)
        size = int(fileinfo.st_size)
        f=open(file_name,'rb')
        max_frames = size/(512*8)
        frames_to_process=max_frames
        rcu_nr = int(files[file_nr][-6:-4])
        #print 'File nr ' + str(file_nr) + ' RCU nr ' + str(rcu_nr) + '  ' + files[file_nr][-6:-4]
    else :
        frames_to_process=0
        f=open(files[file_nr],'rb')
        rcu_nr = 0
    return f, frames_to_process, rcu_nr 

# Read single frame from file   
def read_frame(f):
    sst_data = array.array('d')     
    sst_data.fromfile(f,512)
    sst_data = sst_data.tolist()
    return sst_data

# switch on HBA tiles gentle
def switchon_hba() :
	
	try:
           os.popen3("rspctl --rcumode=5 --sel=0:31")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=32:63")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=64:95")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=96:127")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=128:159")
           time.sleep(1)
	   os.popen3("rspctl --rcumode=5 --sel=160:191")
           time.sleep(1)
	except:
	   print"NL station"	
        os.popen("rspctl --rcuenable=1")
        return 

# Main loop
def main() :
    sub_time=[]
    sub_file=[]
    dir_name = './hbadatatest/' #Work directory will be cleaned
    if not(os.path.exists(dir_name)):
        os.mkdir(dir_name)
    rmfile = '*.log'
    hba_elements=16
    sleeptime=1
    ctrl_string='='
    # read in arguments
    if len(sys.argv) < 2 :
        num_rcu=96
    else :
        num_rcu = int(sys.argv[2])
    print ' Number of RCUs is ' + str(num_rcu)
    max_subband=range(0,num_rcu)
    max_rfi=range(0,num_rcu)
    os.chdir(dir_name)
    #os.popen("rspctl --clock=200")
    #print 'Clock is set to 200 MHz'
    #time.sleep(10)
    #---------------------------------------------
    # capture reference data (all HBA elements off)
    rm_files(dir_name,'*')
    switchon_hba()
    #os.popen("rspctl --rcumode=5 2>/dev/null")
    #os.popen("rspctl --rcuenable=1 2>/dev/null")
    for ind in range(hba_elements) :
        ctrl_string=ctrl_string + '128,'
    strlength=len(ctrl_string)
    ctrl_string=ctrl_string[0:strlength-1]
    cmd_str='rspctl --hbadelay' + ctrl_string + ' 2>/dev/null'
    os.popen(cmd_str)
    print 'Setting all HBA elements on (128)'
    time.sleep(sleeptime)
    print 'Capture data'
    rec_stat(dir_name,num_rcu)
    #rm_files(dir_name,rmfile)
    # get list of all files in dir_name
    files = open_dir(dir_name)
    # start searching for maxima for each RCU
    for file_cnt in range(len(files)) :
        f, frames_to_process, rcu_nr  = open_file(files, file_cnt)
        if frames_to_process > 0 : 
            sst_data = read_frame(f)
            [maxval,subband_nr] = max((x,i) for i,x in enumerate(sst_data[1:]))
            max_rfi[rcu_nr]=10*numpy.log10(maxval)
            max_subband[rcu_nr]=subband_nr+1   
        f.close
        for rcuind in range(num_rcu) :
                print 'RCU ' + str(rcuind) + ' has max. RFI (' + str(round(max_rfi[rcuind],1)) + ' dB) in subband ' + str(max_subband[rcuind])
main()
