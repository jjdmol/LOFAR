import numpy as np
import tables as tab
import os
from subprocess import call

def getClusters(clusterf,skymodel,max_nr_clusters=1000):
    #get clusters
    sky=open(skymodel)
    sources={}
    for line in sky:
        if line.strip()[0]=='#':
            continue;
        splitted=line.split()
        sources[splitted[0].strip()]={}
        sources[splitted[0].strip()]['Ra']=np.pi/12.*(float(splitted[1])
                                                      +float(splitted[2])/60.
                                                      +float(splitted[3])/3600.)
        sources[splitted[0].strip()]['Dec']=np.pi/180.*(float(splitted[4])
                                                        +float(splitted[5])/60.
                                                        +float(splitted[6])/3600.)
        sources[splitted[0].strip()]['I']=float(splitted[7])
        sources[splitted[0].strip()]['sp']=float(splitted[11])
        sources[splitted[0].strip()]['freq0']=float(splitted[-1])

    clusterfile=open(clusterf)
    clusters=[]
    count=0
    tot_nr_sol=0
    nrSB=0
    for line in clusterfile:
        print "adding cluster",line
        if line.strip()[0]=='#':
            continue;
        splitted=line.split()
        if count>=max_nr_clusters:
            tot_nr_sol+=int(splitted[1])
            continue
        clusters.append({})
        clusters[count]['id']=int(splitted[0])
        clusters[count]['nrsol']=int(splitted[1])
        clusters[count]['real']=[]
        clusters[count]['imag']=[]
        clusters[count]['sources']={}
        clusters[count]['store_data']=True
        avg_ra=0
        avg_dec=0
        sum_weight=0
        for src in splitted[2:]:
            clusters[count]['sources'][src.strip()]=sources[src.strip()]
            weight=sources[src.strip()]['I']
            avg_ra+=sources[src.strip()]['Ra']*weight                
            avg_dec+=sources[src.strip()]['Dec']*weight
            sum_weight+=weight
        clusters[count]['Ra']=avg_ra/sum_weight
        clusters[count]['Dec']=avg_dec/sum_weight
        tot_nr_sol+=clusters[count]['nrsol']
        count+=1

    return clusters,tot_nr_sol

def get_freq_data(sol,clusters,tot_nr_sol):
    data=[]
    indices=[]
    for line in sol:
        splitted=line.split()
        indices.append(int(splitted[0]))
        data.append(np.array([float(i) for i in splitted[1:]]))
    data=np.array(data);
    nrStations=(max(indices)+1)/8
    nrTimes=data.shape[0]/(8*nrStations)
    print data.shape,nrTimes,nrStations,8
    if data.shape[0]!=nrTimes*nrStations*8:
        print "wrong shape"
        return -1
    data=data.reshape(nrTimes,nrStations,8,tot_nr_sol)
    start=0
    for icl,cluster in enumerate(clusters):
        if cluster['store_data']==True:
            cluster['real'].append(data[:,:,0:8:2,start:start+cluster['nrsol']])
            cluster['imag'].append(data[:,:,1:8:2,start:start+cluster['nrsol']])
            start+=cluster['nrsol']
    return 1

def remove_unitary(clusters,freqs,store_intermediate=False):
    for idxc,cluster in enumerate(clusters):
      if cluster['store_data']:
        if store_intermediate:
            first=True
            for isb in freqs:
                data=np.load('tmp_store_real_%d_%d.npy'%(idxc,isb))
                if first:
                    cluster['real']=data
                else:
                    cluster['real']=np.concatenate((cluster['real'],data))
                data=np.load('tmp_store_imag_%d_%d.npy'%(idxc,isb))
                if first:
                    cluster['imag']=data
                    first=False
                else:
                    cluster['imag']=np.concatenate((cluster['imag'],data))
                call("rm tmp_store_real_%d_%d.npy"%(idxc,isb),shell=True)
                call("rm tmp_store_imag_%d_%d.npy"%(idxc,isb),shell=True)

                    
        else:
            cluster['real']=np.array(cluster['real'])
            cluster['imag']=np.array(cluster['imag'])
        nrTimes=cluster['real'].shape[1]
        nrSB=cluster['real'].shape[0]
        nrStations=cluster['real'].shape[2]
        cdata=cluster['real']+1.j*cluster['imag']
        cluster['real']=[]
        cluster['imag']=[]
        print cdata.shape
        cdata=np.swapaxes(cdata,0,1)
        print cdata.shape
        cdata=np.swapaxes(cdata,3,4)
        print cdata.shape
        cdata=np.swapaxes(cdata,2,3)
        print cdata.shape
        cdata=np.swapaxes(cdata,1,2)
        print cdata.shape,nrTimes*cluster['nrsol'],nrSB,nrStations,4
        cdata=cdata.reshape(nrTimes*cluster['nrsol'],nrSB,nrStations,4)


        #multiply with unitary matrix to get something more constant in time/freq
        
        J0=cdata[0,0].reshape(nrStations*2,2)
        for ntime in range(nrTimes*cluster['nrsol']):
            for nfreq in range(nrSB):
                if ntime==0 and nfreq==0:
                    continue;
                J1=cdata[ntime,nfreq].reshape(nrStations*2,2)
                u,s,v=np.linalg.svd(np.dot(np.conjugate(J1.T),J0))
                U1=np.dot(u,v)
                J0=np.dot(J1,U1)
                cdata[ntime,nfreq]=J0.reshape(nrStations,4)
            J0=cdata[ntime,0].reshape(nrStations*2,2)
        if store_intermediate:
            np.save('tmp_store_cdata_%d.npy'%(idxc),cdata)
        else:
            cluster['cdata']=cdata

def fill_sb(clusters,solpath,solpath_end,subbandlist,tot_nr_sol,store_intermediate=False):
    freqs=[]
    for isb,sb in enumerate(subbandlist):
        print "opening",solpath+str(sb)+solpath_end
        if not os.path.isfile(solpath+str(sb)+solpath_end):
            print "skipping",sb
            continue;
        sol=open(solpath+str(sb)+solpath_end)
        if get_freq_data(sol,clusters,tot_nr_sol)>0:
            freqs.append(isb)
            # store intermediate results....to save memory?
            if store_intermediate:
                for idxc,cluster in enumerate(clusters):
                    if cluster['store_data']:
                        np.save('tmp_store_real_%d_%d.npy'%(idxc,isb),np.array(cluster['real']))
                        np.save('tmp_store_imag_%d_%d.npy'%(idxc,isb),np.array(cluster['imag']))
                        cluster['real']=[]
                        cluster['imag']=[]
            
    return freqs
        
def addToH5File(h5file,clusters,freqs,store_intermediate=False):
    #group into nrsolutions
    poss_nr_sol=[]
    groups=[]
    for clusteridx,cluster in enumerate(clusters):
        if not cluster['nrsol'] in poss_nr_sol:
            poss_nr_sol.append(cluster['nrsol'])
            groups.append([])
        idx=poss_nr_sol.index(cluster['nrsol'])
        groups[idx].append(clusteridx)

    if 'sagefreqIdx' in h5file.root:
        h5file.removeNode('/sagefreqIdx')

    h5file.createArray(h5file.root, 'sagefreqIdx',freqs)
    for igrp,grp in enumerate(groups):
        # create arrays:
        if store_intermediate:
            cdata=np.load('tmp_store_cdata_%d.npy'%(grp[0]))
        else:
            cdata=clusters[grp[0]]['cdata']
            
        arrayshape=cdata.shape[:-1]+(len(grp),4)
        for name in ['sageradec%d'%igrp,'sagephases%d'%igrp,'sageamplitudes%d'%igrp]:
            if name in h5file.root:
                h5file.removeNode('/'+name)
        
        srcarray = h5file.createCArray(h5file.root, 'sageradec%d'%igrp, tab.Float32Atom(), shape=(len(grp),2))
        pharray = h5file.createCArray(h5file.root, 'sagephases%d'%igrp, tab.Float32Atom(), shape=arrayshape)
        amparray = h5file.createCArray(h5file.root, 'sageamplitudes%d'%igrp, tab.Float32Atom(), shape=arrayshape)
        for idx,clusteridx  in enumerate(grp):
            if store_intermediate:
                cdata=np.load('tmp_store_cdata_%d.npy'%(clusteridx))
            else:
                cdata=clusters[clusteridx]['cdata']
            pharray[:,:,:,idx,:]=np.angle(cdata)
            amparray[:,:,:,idx,:]=np.absolute(cdata)
            srcarray[idx,:]=np.array([clusters[clusteridx]['Ra'],clusters[clusteridx]['Dec']])
            clusters[clusteridx]['cdata']=[]
            if store_intermediate:
                call("rm tmp_store_cdata_%d.npy"%(clusteridx),shell=True)
        pharray.flush();
        amparray.flush();
        srcarray.flush();

def UpdateIonmodel(h5filename,clusterf,skymodel,solpath,subbands,max_nr_clusters=1000,store_intermediate=True):
    '''Add sagecal solutions to your ionmodel.hdf5.Use store_intermediate if you are trying to store many solutions, since otherwise the program will run out of memory. subbands is a list with subband indices. The sagecal solutions are stored per group with the same timestep. THe list of valid subband indices are also stored in the hdf5 file.''' 
    
    file_example=os.listdir(solpath)[0]
    pos=file_example.find('SB')
    start_name=solpath+'/'+file_example[:pos+2]
    end_name=file_example[pos+5:]
    subbandlist=['%03d'%(sb) for sb in subbands]
    clusters,solshape=getClusters(clusterf=clusterf,skymodel=skymodel,max_nr_clusters=max_nr_clusters)
    freqs=fill_sb(clusters,solpath=start_name,solpath_end=end_name,subbandlist=subbandlist,tot_nr_sol=solshape,store_intermediate=store_intermediate)
    remove_unitary(clusters,freqs,store_intermediate)
    h5file = tab.openFile(h5filename, mode = "r+")
    addToH5File(h5file,clusters,np.array(subbands)[freqs],store_intermediate=store_intermediate)
    
