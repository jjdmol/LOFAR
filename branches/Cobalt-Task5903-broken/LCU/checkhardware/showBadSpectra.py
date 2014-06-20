#!/usr/bin/python

import os
import numpy as np
import matplotlib.pyplot as plt
import time

obs_id_to_plot  = ''
station_to_plot = ''

spectraPath  = r'/localhome/stationtest/bad_spectra'

def main():
    files = full_listdir(spectraPath)
    for file_name in files:
        file_data = file_name[file_name.rfind('/')+1:].split('_')
        station_id = file_data[0]
                
        if station_to_plot != '' and station_to_plot != station_id:
            continue
        
        f = open(file_name, 'r')
        file_data = f.read()
        for line in file_data.splitlines():
                      
            line_data = line.strip().split('=')
            if len(line_data) < 2:
                continue
            val_name = line_data[0].strip()
            val_data = line_data[1].strip()
            if val_name == 'timestamp':
                dumpTime = time.gmtime(float(val_data))
                timestr = time.strftime("%Y-%m-%d %H:%M:%S", dumpTime)
            if val_name == 'ObsID':
                obs_id = val_data
            if val_name == 'check':
                fault = val_data    
            if val_name == 'rcumode':
                rcumode = val_data
            if val_name == 'rcu':
                rcu = val_data
            if val_name == 'frequency':
                freq = str2array(val_data[1:-1])
                #print len(freq)
            if val_name == 'mean-spectra':
                mean_spectra = str2array(val_data[1:-1])
                #print len(mean_spectra)
            if val_name == 'rcu-spectra':
                bad_rcu = str2array(val_data[1:-1])
                #print len(bad_rcu)

        plt.plot(freq,mean_spectra,'k',freq,bad_rcu,'r')
        plt.legend(('median-spectra', 'rcu-%s' %(rcu)), fancybox=True)
        plt.title("%s    RCU %s    %s" %\
                 (station_id, rcu, fault))
        plt.xlabel('MHz')         
        plt.ylabel('dBm')         
        x1 = freq[0]+2.0
        x2 = freq[0]+12.0
        plt.text(x1,117, "ObsID", fontsize=10)
        plt.text(x2,117, ": %s" %(obs_id), fontsize=10) 
        plt.text(x1,115, "RecTime", fontsize=10) 
        plt.text(x2,115, ": %s" %(timestr), fontsize=10) 
        plt.text(x1,113, "RcuMode", fontsize=10) 
        plt.text(x2,113, ": %s" %(rcumode), fontsize=10) 
        plt.ylim(58,120)
        plt.grid()        
        plt.show()
            

def str2array(data):
    return (np.array([float(val) for val in data.split()]))
    


def full_listdir(dir_name):
     return sorted([os.path.join(dir_name, file_name) for file_name in os.listdir(dir_name)])


if __name__ == "__main__":
    main()