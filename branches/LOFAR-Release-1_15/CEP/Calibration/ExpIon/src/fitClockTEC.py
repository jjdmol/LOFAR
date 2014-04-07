import numpy as np;
import lofar.expion.parmdbmain as parmdbmain
import lofar.expion.baselinefitting as fitting
#import lofar.expion.fitting as fitting
from scipy import optimize as opt
import tables as tab
import sys
from pylab import *
light_speed=299792458.

clockarray=0
tecarray=0
offsetarray=0
residualarray=0


def ClockTECfunc(xarray,par):
    delay=np.array([-1*par[1]*1e-9]).flatten() #in ns, array has dimension 1, even if scalar
    delayfact=2*np.pi*delay[:,np.newaxis]*xarray
    TEC=np.array([par[0]]).flatten();          # dTEC in TECU
    drefract=-8.4479745e9*TEC[:,np.newaxis]/xarray;
    return drefract[:,np.newaxis,:]+delayfact+par[2]; #returns nTEC x nClock x nFreq

def getRM(ion,station,refStationIdx=0,starttime=0,SBselect='all'):
    '''get Rotation Measure vs. time and residual rotation'''
    freqs=ion.freqs[:]
    nF=freqs.shape[0]
    if SBselect=='all':
        SBselect=np.ones(nF,dtype=bool)
    if isinstance(SBselect,list) and len(SBselect)==2:
        SBselect=np.logical_and(freqs>SBselect[0],freqs<SBselect[1])
    ant1 = list(ion.stations[:]).index(station)
    ant2 = refStationIdx
    A = (light_speed/ freqs[SBselect])**2
    A = np.reshape(A, (-1,1))
    rot=ion.rotation[starttime:,:,ant1][:,SBselect]-ion.rotation[starttime:,:,ant2][:,SBselect]
    #remove nans
    rot[np.isnan(rot)]=0.
    while len(rot.shape)>2:
        rot=rot[:,:,0]
    
    RM=np.remainder(rot+np.pi,2*np.pi)-np.pi
    
    RMtime=  np.dot(1./(np.dot(A.T,A)), np.dot(A.T,RM.T)).flatten()
    residuals=RM-RMtime[:,np.newaxis]*A.T
    return RMtime,residuals

def getInitPar(data,dTECArray, dClockArray,freqs,ff=ClockTECfunc):
    '''initialize paramaters and unwraps data for fit'''
    if np.min(np.absolute(dTECArray))<1e-5:
        dTECArray+=0.0001 #just to prevent 0 to be there, since otherwise the fit might get stuck in 0 
    nT=dTECArray.shape[0]
    nD=dClockArray.shape[0]
    par=[dTECArray,dClockArray,0]
    # first check all unwrapping possibilities
    bigdata=ff(freqs,par) # returns array of shape nT,nD,nF
    wraps=np.around(np.divide(bigdata-data,2*np.pi));
    difference=  bigdata-data-wraps*2*np.pi
    offset=np.average(difference,axis=2);
    index=np.unravel_index(
        np.argmin(
            np.sum(np.absolute((difference.T-offset.T).T),axis=2))
        ,(nT,nD));
    OffsetIn=-1*offset[index];
    par=[dTECArray[index[0]],dClockArray[index[1]],OffsetIn];
    estimate=ff(freqs,par).flatten()
    wraps=np.around(np.divide(estimate-data,2*np.pi));
    data[:]=np.add(2*np.pi*wraps,data)
    return par

def getClockTECAll(ph,amp,freqs,stationname,stIdx,polIdx):
    global tecarray
    global clockarray
    global offsetarray

    maxTimesteps=500
    #first unwrap data and get initial guess
    nT=ph.shape[0]
    nF=freqs.shape[0]

    stepDelay=.03
    if 'CS' in stationname:
        iTEC1=-0.1
        iTEC2=0.1
        iD1=-2
        iD2=2
    else:
        iTEC1=-0.4
        iTEC2=0.4
        iD1=-200
        iD2=200

    if "HBA" in stationname:
        stepdTEC=0.0005
    else:
        stepdTEC=0.0001  # faster wrapping for LBA 

    tmsteps=nT/maxTimesteps
    for istep in range(tmsteps):
     bigshape=[0,2*min(maxTimesteps,nT-maxTimesteps*istep)+1]
     alldata=np.array([])
     allfreqs=[]
     for itm in range(istep*maxTimesteps,min(nT,(istep+1)*maxTimesteps)):
        
        if itm%100==0 and itm>0:
            sys.stdout.write(str(itm)+'... '+str(par[0])+' '+str(par[1])+' '+str(par[2])+' ')
            sys.stdout.flush()
        if itm>0:
            iTEC1=par[0]-0.1
            iTEC2=par[0]+0.1
            iD1=par[1]-10
            iD2=par[1]+10
        dTECArray=np.arange(iTEC1,iTEC2,stepdTEC)
        dClockArray=np.arange(iD1,iD2,stepDelay)

        flags=(amp[itm,:]!=1);
        nrFlags=np.sum(np.logical_not(flags))
        data=ph[itm,:][flags]
        tmpfreqs=freqs[flags]
        par = getInitPar(data,dTECArray, dClockArray,tmpfreqs,ClockTECfunc,useOffset=False,plot_debug=(itm%100==0))

        alldata=np.append(alldata,data)
        bigshape[0]+=tmpfreqs.shape[0]
        allfreqs.append(tmpfreqs)
        
        
     print "got bigmatrix",bigshape
     bigmatrix=np.zeros(bigshape)
     idx=0
     
     for itm in range(min(nT-istep*maxTimesteps,maxTimesteps)):
        nextidx=allfreqs[itm].shape[0]
        bigmatrix[idx:idx+nextidx,2*itm]=-8.4479745e9/allfreqs[itm]
        bigmatrix[idx:idx+nextidx,2*itm+1]=-2.e-9*np.pi*allfreqs[itm]
        idx+=nextidx
     bigmatrix[:,-1]+=1
     print "fitting",bigmatrix.shape,alldata.shape
     sol=np.linalg.lstsq(bigmatrix,alldata)
     finalpar=sol[0]
     print "result",finalpar[0],finalpar[1],finalpar[-1]
     offsetarray[istep*maxTimesteps:(istep+1)*maxTimesteps,stIdx,polIdx]=finalpar[-1]
     tecarray[istep*maxTimesteps:(istep+1)*maxTimesteps,stIdx,polIdx]=finalpar[:-1].reshape(-1,2)[:,0]
     clockarray[istep*maxTimesteps:(istep+1)*maxTimesteps,stIdx,polIdx]=finalpar[:-1].reshape(-1,2)[:,1]

def getClockTEC(ph,amp,freqs,SBselect,stationname,stIdx,polIdx,fixedOffset=False):
    global tecarray
    global clockarray
    global offsetarray
    global residualarray

    errorf= lambda par,x,data:  (-1*par[0]*x*2*np.pi*1e-9+par[1]-data).flatten() #delay function+ offset
    #remove nans
    ph[np.isnan(ph)]=0.
    ph=np.unwrap(np.remainder(ph,2*np.pi)) # unwrap last (=freq) axis
    par=[0.,0.]
    avg_delay=0.
    if ("CS" in stationname) or ("HBA" in stationname):
        #do not do this step for RS stations LBA,because ionospheric fluctuations
        result=opt.leastsq(errorf,par,args=(freqs,ph)) #get average delay
        avg_delay=result[0][0]

    print "avg_delay",stationname,polIdx,avg_delay
    # define the function we want to fit, for core stations keep delay fixed
    stepDelay=.3
    if 'CS' in stationname:
        fixedDelay=True
        if fixedOffset:
            ff=lambda x,par:ClockTECfunc(x,[par[0],avg_delay,0.])
        else:
            ff=lambda x,par:ClockTECfunc(x,[par[0],avg_delay,par[1]])
        initTEC1=-0.5
        initTEC2=0.5
        initD1=avg_delay
        initD2=avg_delay+stepDelay
    else:
        fixedDelay=False
        if fixedOffset:
            ff=lambda x,par:ClockTECfunc(x,[par[0],par[1],0.])
        else:
            ff=lambda x,par:ClockTECfunc(x,par)
        par=[0.,0.,0.]
        initTEC1=-2
        initTEC2=2
        if "HBA" in stationname:
            initD1=avg_delay-30
            initD2=avg_delay+30
        else:
            initD1=-200
            initD2=200

    if "HBA" in stationname:
        stepdTEC=0.01
    else:
        stepdTEC=0.008  # faster wrapping for LBA 

    errorf = lambda par,x,y: (ff(x,par)-y).flatten()

    nTimes=ph.shape[0]
    nF=ph.shape[1]

    success=False
    iTEC1=initTEC1
    iTEC2=initTEC2
    iD1=initD1
    iD2=initD2
    finalpar=[0]*nTimes
    print stationname,polIdx,"tm:",
    for tm in range(0,nTimes):
        if tm%100==0:
            sys.stdout.write(str(tm)+'...')
            sys.stdout.flush()
        if tm>0 and success:
            iTEC1=finalpar[tm-1][0]-5*stepdTEC
            iTEC2=finalpar[tm-1][0]+6*stepdTEC
            if not fixedDelay:
                iD1=finalpar[tm-1][1]-1*stepDelay
                iD2=finalpar[tm-1][1]+2*stepDelay
        else:
            iTEC1=max(iTEC1-5*stepdTEC,min(initTEC1,iTEC2-5))
            iTEC2=min(iTEC2+6*stepdTEC,max(initTEC2,iTEC2+5))
            if not fixedDelay:
                iD1=max(iD1-1*stepDelay,min(initD1,iD2-60))
                iD2=min(iD2+2*stepDelay,max(initD2,iD1+60))
        itm=tm
        dTECArray=np.arange(iTEC1,iTEC2,stepdTEC)
        dClockArray=np.arange(iD1,iD2,stepDelay)

        flags=(amp[itm,:]!=1);
        nrFlags=np.sum(np.logical_not(flags))
        data=ph[itm,:][flags]
        tmpfreqs=freqs[flags]
        
        if nrFlags>0.5*nF:
            print "TOO many data points flagged:",tm,tmpfreqs.shape[0],"remaining"
            if tm>0:
                finalpar[tm]=np.array(finalpar[tm-1])
            else:
                finalpar[tm]=np.array(par)
            success=False
            continue
        par = getInitPar(data,dTECArray, dClockArray,tmpfreqs,ClockTECfunc)
        if fixedDelay:
            par=par[:1]+par[2:]
        if fixedOffset:
            par=par[:-1]
        (finalpar[tm],success)=opt.leastsq(errorf,par,args=(tmpfreqs,data))
        #print "fitted",tm,(finalpar[tm],success)
        if not hasattr(finalpar[tm],'__len__'):
            finalpar[tm]=[finalpar[tm]]
        chi2 = np.average(np.power(errorf(par,tmpfreqs,data), 2))
        if chi2>10:
            print "got a Fail",stationname,itm,chi2,finalpar[tm]
            success=False
        else:
            residualarray[itm,SBselect,stIdx,polIdx][flags]=errorf(finalpar[tm],tmpfreqs,data)
         
    print 'finished'
    #acquire lock?, store data
    finalpar=np.array(finalpar)
    tecarray[:,stIdx,polIdx]=np.array(finalpar)[:,0]
    if fixedDelay:
        clockarray[:,stIdx,polIdx]=avg_delay*np.ones(clockarray.shape[0])
        if not fixedOffset:
            offsetarray[:,stIdx,polIdx]=np.array(finalpar)[:,1]
    else:
        clockarray[:,stIdx,polIdx]=np.array(finalpar)[:,1]
        if not fixedOffset:
            offsetarray[:,stIdx,polIdx]=np.array(finalpar)[:,2]


def getResidualPhaseWraps(avgResiduals,freqs):
    flags=avgResiduals!=0.
    nSt=avgResiduals.shape[1]
    nF=freqs.shape[0]
    wraps=np.zeros((nSt,),dtype=np.float)
    for ist in range(nSt):
        print ist
        tmpfreqs=freqs[flags[:,ist]]
        nF=tmpfreqs.shape[0]
        if nF<10:
            print "too many flagged",ist
            continue
        basef,steps=getPhaseWrapBase(tmpfreqs)

        data=avgResiduals[flags[:,ist],ist]
        wraps[ist]=np.dot(1./(np.dot(basef.T,basef)), np.dot(basef,data))
    return wraps,steps

def getPhaseWrapBase(freqs):
    nF=freqs.shape[0]
    A=np.zeros((nF,2),dtype=np.float)
    A[:,1] = freqs*2*np.pi*(-1e-9)
    A[:,0] = -8.44797245e9/freqs
    steps=np.dot(np.dot(np.linalg.inv(np.dot(A.T,A)),A.T),2*np.pi*np.ones((nF,),dtype=np.float))
    basef=np.dot(A,steps)-2*np.pi
    return basef,steps

def getResidualPhaseWraps2(avgResiduals,freqs):
    flags=avgResiduals[:,10]==0.
    nSt=avgResiduals.shape[1]
    nF=freqs.shape[0]
    wraps=np.zeros((nSt,),dtype=np.float)
    #tmpflags=np.sum(flags[:,np.sum(flags,axis=0)<(nF*0.5)],axis=1)
    tmpflags=flags
    tmpfreqs=freqs[np.logical_not(tmpflags)]
    tmpbasef,steps=getPhaseWrapBase(tmpfreqs)
    basef=np.zeros(freqs.shape)
    basef[np.logical_not(tmpflags)]=tmpbasef
    basef=basef.reshape((-1,1))
    
    data=avgResiduals[:,:]
        

    wraps=fitting.fit(data,basef,wraps,flags).flatten()
    return wraps,steps

def getClockTECBaselineFit(ph,amp,freqs,SBselect,polIdx,stIdx,useOffset=False,stations=[],initSol=[],chi2cut=300.,fixedClockforCS=False,timeIdx=0):
    global tecarray
    global clockarray
    global offsetarray
    global residualarray
    amp[np.isnan(ph)]=1
    ph[np.isnan(ph)]=0.
    ph=np.unwrap(ph,axis=0) 
    #first unwrap data and get initial guess
    nT=ph.shape[0]
    nF=freqs.shape[0]
    nSt=ph.shape[2]
    nparms=2+(useOffset>0)
    #sol = np.zeros((nSt,nparms),dtype=np.float)
    sol = np.zeros((nSt,nparms),dtype=np.float)
    print sol.shape,nparms,nSt
    A=np.zeros((nF,nparms),dtype=np.float)
    A[:,1] = freqs*2*np.pi*(-1e-9)
    A[:,0] = -8.44797245e9/freqs
    if useOffset:
        A[:,2] = np.ones((nF,))

    constant_parms=np.zeros(sol.shape,dtype=bool)
    if fixedClockforCS:
        for ist,st in enumerate(stations):
            if 'CS' in st:
                constant_parms[ist,1]=True
    stepDelay=1

    if "HBA" in stations[0]:
        stepdTEC=0.005
    else:
        stepdTEC=0.001  # faster wrapping for LBA 

 
    succes=False
    initprevsol=False
    nrFail=0
    for itm in range(nT):
        
        if itm%100==0 and itm>0:
            sys.stdout.write(str(itm)+'... '+str(sol[-1,0]-sol[0,0])+' '+str(sol[-1,1]-sol[0,1])+' '+str(sol[-1,-1]-sol[0,-1])+' ')
            sys.stdout.flush()
 
        flags=(amp[itm,:]==1);
        nrFlags=np.sum(flags,axis=0)
        if itm==0 or not succes:
         for ist in range(1,nSt):
            if (nF-nrFlags[ist])<10:
                print "Too many data points flagged",itm,ist
                continue;
            if itm==0 or not initprevsol:
                if hasattr(initSol,'__len__') and len(initSol)>ist:
                    iTEC1=initSol[ist,0]
                    iTEC2=initSol[ist,0]+stepdTEC
                    iD1=initSol[ist,1]
                    iD2=initSol[ist,1]+stepDelay
                else:
                 if 'CS' in stations[ist]:
                    iTEC1=-0.2
                    iTEC2=0.2
                    iD1=-4
                    iD2=4
                 else:
                    iTEC1=-1
                    iTEC2=1
                    iD1=-200
                    iD2=200
                print "First",iTEC1,iTEC2,iD1,iD2
                    

            else:
                
                iTEC1=prevsol[ist,0]-stepdTEC*nrFail
                iTEC2=prevsol[ist,0]+stepdTEC*(nrFail+1)
                if not fixedClockforCS or not 'CS' in stations[ist]: 
                    iD1=prevsol[ist,1]-stepDelay*nrFail
                    iD2=prevsol[ist,1]+stepDelay*(nrFail+1)
                else:
                    iD1=sol[ist,1]
                    iD2=sol[ist,1]+stepDelay
                    
                print "Failure",iTEC1,iTEC2,iD1,iD2,nrFail

            dTECArray=np.arange(iTEC1,iTEC2,stepdTEC)
            dClockArray=np.arange(iD1,iD2,stepDelay)
            data=ph[itm,:,ist][np.logical_not(np.logical_or(flags[:,ist],flags[:,0]))]-ph[itm,:,0][np.logical_not(np.logical_or(flags[:,ist],flags[:,0]))]
            tmpfreqs=freqs[np.logical_not(np.logical_or(flags[:,ist],flags[:,0]))]
            print "getting init",ist,
            par = getInitPar(data,dTECArray, dClockArray,tmpfreqs,ClockTECfunc)
            print par
            sol[ist,:]=par[:nparms]
        if not succes:
            #reset first station
            sol[0,:] = np.zeros(nparms)
        #sol=fitting.fit(ph[itm],A,sol.T,flags).T
        #sol=fitting.fit(ph[itm],A,sol,flags)
        sol=fitting.fit(ph[itm],A,sol.T,flags,constant_parms.T).T
        tecarray[itm+timeIdx,stIdx,polIdx]=sol[:,0]
        clockarray[itm+timeIdx,stIdx,polIdx]=sol[:,1]
        if useOffset:
            offsetarray[itm+timeIdx,stIdx,polIdx]=sol[:,2]
        residual = ph[itm] - np.dot(A, sol.T)
        residual = residual - residual[:, 0][:,np.newaxis]
        residual = np.remainder(residual+np.pi, 2*np.pi) - np.pi       
        residual[flags]=0
        residualarray[np.ix_([itm+timeIdx],SBselect,stIdx,[polIdx])]=residual.reshape((1,nF,nSt,1))
        chi2=np.sum(np.square(np.degrees(residual)))/(nSt*nF)
        if chi2>chi2cut:
            print "failure",chi2,sol
            succes=False
            nrFail=0
        else:
            prevsol=np.copy(sol)
#            print "succes",chi2,dTECArray.shape,dClockArray.shape
            succes=True
            initprevsol=True
            nrFail+=1

def add_to_h5_func(h5file,data,name='test'):
    if name in h5file.root:
        h5file.removeNode('/'+name)
    myarray=h5file.createCArray(h5file.root,name,tab.Float32Atom(),shape=data.shape)
    myarray[:]=data
    myarray.flush()

    

def getAll(ionmodel,refstIdx=0,doClockTEC=True,doRM=False,add_to_h5=True,stationSelect='',label='fit',SBselect='all',allBaselines=True,useOffset=False,initFromPrevious=False,flagBadChannels=False,flagcut=1.5,chi2cut=300.,removePhaseWraps=False,combine_pol=False,fixedClockforCS=False,timerange='all'):
    global tecarray
    global clockarray
    global offsetarray
    global residualarray
    if allBaselines and not doClockTEC:
        doClockTEC=True

    polshape=ionmodel.phases.shape[-1]
    nT=ionmodel.times.shape[0]
    if timerange=='all':
        timerange=[0,nT]
    nT=timerange[1]-timerange[0]
    nF=ionmodel.freqs.shape[0]
    freqs=ionmodel.freqs[:]
    if SBselect=='all':
        SBselect=np.ones(nF,dtype=bool)
    if isinstance(SBselect,list) and len(SBselect)==2:
        SBselect=np.logical_and(freqs>SBselect[0],freqs<SBselect[1])
    if flagBadChannels:
        rms=lambda x,y: np.sqrt(np.mean(np.square(x-np.mean(x,axis=y)),axis=y))
        ph=ionmodel.phases[timerange[0]:timerange[1],:,1,0,0]-ionmodel.phases[timerange[0]:timerange[1],:,0,0,0]
        myrms1=rms(ph[:,SBselect],0)
        freqselect=myrms1<flagcut*np.average(myrms1)
        cutlevel=flagcut*np.average(rms(ph[:,SBselect][:,freqselect],0))
        myrms1=rms(ph,0)
        SBselect=np.logical_and(SBselect,myrms1<cutlevel)
        print "flagging",np.sum(np.logical_not(SBselect)),"channels"
    freqs=freqs[SBselect]
        
    if isinstance(stationSelect,str): 
        stations=[st for st in list(ionmodel.stations[:]) if stationSelect in st]   
    else:
        stations=list(ionmodel.stations[:][stationSelect])
    print "stations",stations
    if doClockTEC:
        clockarray=np.zeros(ionmodel.times.shape+ionmodel.stations[:].shape+(2,))
        tecarray=np.zeros(ionmodel.times.shape+ionmodel.stations[:].shape+(2,))
        offsetarray=np.zeros(ionmodel.times.shape+ionmodel.stations[:].shape+(2,))
        residualarray=np.zeros((len(ionmodel.times),len(ionmodel.freqs),len(ionmodel.stations),2))
        ph=ionmodel.phases
        amp=ionmodel.amplitudes
    if doRM:
        rmarray=np.zeros((len(ionmodel.times),len(ionmodel.stations)))
        rotation_resarray=np.zeros((len(ionmodel.times),len(freqs),len(ionmodel.stations)))
    if allBaselines:
        #stationIndices=[list(ionmodel.stations[:]).index(st) for st in stations]
        stationIndices=np.array([idxst in stations for idxst in ionmodel.stations[:]])
        for pol in range(2):
            if combine_pol:
                phdata=ph[timerange[0]:timerange[1],:,:,0,(polshape-1)][:,SBselect][:,:,stationIndices]+ph[timerange[0]:timerange[1],:,:,0,0][:,SBselect][:,:,stationIndices]
                ampdata=amp[timerange[0]:timerange[1],:,:,0,(polshape-1)][:,SBselect][:,:,stationIndices]+amp[timerange[0]:timerange[1],:,:,0,0][:,SBselect][:,:,stationIndices]
                #return phdata
            else:
                phdata=ph[timerange[0]:timerange[1],:,:,0,pol*(polshape-1)][:,SBselect][:,:,stationIndices]
                ampdata=amp[timerange[0]:timerange[1],:,:,0,pol*(polshape-1)][:,SBselect][:,:,stationIndices]
            if hasattr(ionmodel,'TEC') and initFromPrevious:
                initSol=np.zeros((len(stations),2),dtype=np.float)
                initSol[:,0]=ionmodel.TEC[:][timerange[0],stationIndices,pol]
                initSol[:,1]=ionmodel.Clock[:][timerange[0],stationIndices,pol]
            else:
                initSol=False

            kwargs={'ph':phdata,
                    'amp':ampdata,
                    'freqs':freqs,
                    'SBselect':SBselect,
                    'polIdx':pol,
                    'stIdx':stationIndices,
                    'stations':stations,
                    'useOffset':useOffset,
                    'initSol':initSol,
                    'chi2cut':chi2cut,
                    'timeIdx':timerange[0]}
            getClockTECBaselineFit(**kwargs)
            if removePhaseWraps:
                avgResiduals=np.average(residualarray[timerange[0]:timerange[1],:,:,pol],axis=0)
                wraps,steps=getResidualPhaseWraps(avgResiduals,ionmodel.freqs[:])

                #halfwraps=np.remainder(np.round(np.absolute(wraps[stationIndices]*2)),2)==1
                #print "found halfwraps for",np.array(stations)[halfwraps]

                #offsets=-1*np.average(tecarray[:,stationIndices,pol]-tecarray[:,[0],pol],axis=0)*2.*np.pi/steps[0]
                #remainingwraps=wraps-np.round(wraps)
                #phdata[:,:,:]+=remainingwraps[stationIndices]
                #offsetarray[:,stationIndices,pol]+=remainingwraps[stationIndices]
                #phdata[:,:,:]+=offsets
                #offsetarray[:,stationIndices,pol]+=offsets
                #clockarray[:,stationIndices,pol]+=(np.remainder(offsets+np.pi,2*np.pi)-np.pi)*steps[1]/(2*np.pi)
                #!!!!!!!!!!!!!!!!TESTESTESTSETTE!!!
                #phdata[:,:,np.arange(1,46,2)]+=0.01*np.arange(1,46,2)
                initSol=np.zeros((len(stations),2),dtype=np.float)
                initSol[:,0]=tecarray[timerange[0],stationIndices,pol]+steps[0]*np.round(wraps[stationIndices])
                initSol[:,1]=clockarray[timerange[0],stationIndices,pol]+steps[1]*np.round(wraps[stationIndices])
                #initSol[:,1]=np.average(clockarray[:,stationIndices,pol]-clockarray[:,[0],pol],axis=0)+steps[1]*np.round(wraps[stationIndices])
                print "init Clock with", initSol[:,1]

                kwargs={'ph':phdata,
                        'amp':ampdata,
                        'freqs':freqs,
                        'SBselect':SBselect,
                        'polIdx':pol,
                        'stIdx':stationIndices,
                        'stations':stations,
                        'useOffset':useOffset,
                        'initSol':initSol,
                        'chi2cut':chi2cut,
                        'timeIdx':timerange[0],
                        'fixedClockforCS':fixedClockforCS}
                getClockTECBaselineFit(**kwargs)
            if combine_pol:
                tecarray/=2.
                clockarray/=2.
                break;
            
    else:            
      for ist,st in enumerate(stations):
        print "getting values for station",st
        if doClockTEC:
            if ist==refstIdx:
                continue
            for pol in range(2):
                kwargs={'ph':ph[:,:,ist,0,pol*(polshape-1)][:,SBselect]-ph[:,:,refstIdx,0,pol*(polshape-1)][:,SBselect],
                        'amp':amp[:,:,ist,0,pol*(polshape-1)][:,SBselect],
                        'freqs':freqs,
                        'SBselect':SBselect,
                        'stationname':st,
                        'stIdx':ist,
                        'polIdx':pol}
                getClockTEC(**kwargs)
        if doRM:
            if st==refstIdx:
                continue;
            rmarray[:,ist],rotation_resarray[:,:,ist]=getRM(ionmodel,st,refstIdx,SBselect=SBselect)


    if add_to_h5:
        if hasattr(ionmodel,'hdf5'):
            h5file=ionmodel.hdf5
        else:
            h5file=ionmodel
        if doClockTEC:
            add_to_h5_func(h5file,clockarray,name='Clock')
            add_to_h5_func(h5file,tecarray,name='TEC')
            add_to_h5_func(h5file,offsetarray,name='clock_tec_offsets')
            add_to_h5_func(h5file,residualarray,name='clock_tec_residuals')
        if doRM:
            add_to_h5_func(h5file,rmarray,name='rmtime')
            add_to_h5_func(h5file,rotation_resarray,name='rm_residuals')
    else:
        if doClockTEC:
            np.save('dclock_%s.npy'%(label), clockarray)
            np.save('dTEC_%s.npy'%(label), tecarray)
            np.save('offset_%s.npy'%(label), offsetarray)
            np.save('residual_%s.npy'%(label), residualarray)
        if doRM:
            np.save('rm_%s.npy'%(label), rmarray)
            np.save('rm_residuals_%s.npy'%(label), rotation_resarray)



def SwapClockTECAxes(ionmodel):
    print "swap axes will reshape your Clock and TEC solutions. The order of Clock is now times  x stations x polarizations and of TEC: times x stations x sources x polarizations"
    TEC =ionmodel.TEC;
    TECshape=TEC.shape
    Clock =ionmodel.Clock;
    Clockshape=Clock.shape
    nT=ionmodel.times.shape[0]
    nst=ionmodel.stations[:].shape[0]
    nsources=ionmodel.N_sources
    newshape=(nT,nsources,nst,2)
    if TECshape==newshape:
        print "nothing to be done for TEC"
    else:
        TEC=TEC[:]
        indices=range(4) #nT,st,nsources,pol
        tmaxis=TECshape.index(nT)
        indices[tmaxis]=0
        staxis=TECshape.index(nst)
        indices[staxis]=1
        if len(TECshape)==3 or (nsources!=2):
            polaxis=TECshape.index(2)
            indices[polaxis]=3
            if len(TECshape)>3:
               nsaxis=TECshape.index(nsources) 
            else:
                TEC=TEC.reshape(TECshape+(1,))
                nsaxis=3

            indices[nsaxis]=2
        else:
            print "ambigous shape of TEC, try swapping by hand"
        while tmaxis>0:
            TEC=TEC.swapaxes(tmaxis,tmaxis-1)
            indices[tmaxis]=indices[tmaxis-1]
            tmaxis-=1
            indices[tmaxis]=0
        staxis=indices.index(1)
        
        while staxis>1:
            TEC=TEC.swapaxes(staxis,staxis-1)
            indices[staxis]=indices[staxis-1]
            staxis-=1
            indices[staxis]=1
        srcaxis=indices.index(2)
        
        while srcaxis>2:
            TEC=TEC.swapaxes(srcaxis,srcaxis-1)
            indices[srcaxis]=indices[srcaxis-1]
            srcaxis-=1
            indices[srcaxis]=2
        add_to_h5_func(ionmodel.hdf5,TEC,name='TEC')
    newshape=(nT,nst,2)
    if Clockshape==newshape:
        print "nothing to be done for Clock"
    else:
        Clock=Clock[:]
        indices=range(3) #nT,st,pol
        tmaxis=Clockshape.index(nT)
        indices[tmaxis]=0
        staxis=Clockshape.index(nst)
        indices[staxis]=1
        polaxis=Clockshape.index(2)
        indices[polaxis]=2
        while tmaxis>0:
            Clock=Clock.swapaxes(tmaxis,tmaxis-1)
            indices[tmaxis]=indices[tmaxis-1]
            tmaxis-=1
            indices[tmaxis]=0
        staxis=indices.index(1)
        
        while staxis>1:
            Clock=Clock.swapaxes(staxis,staxis-1)
            indices[staxis]=indices[staxis-1]
            staxis-=1
            indices[staxis]=1
       
        add_to_h5_func(ionmodel.hdf5,Clock,name='Clock')
      
    
def writeClocktoParmdb(ionmodel,average=False,create_new = True):
    '''if average the average of both polarizations is used, snice BBS can handle only on value at the moment'''
    if not hasattr(ionmodel,'Clock'):
        print "No Clock solutions found, maybe you forgot to run the fit?"
        return
    Clock=ionmodel.Clock[:] # times x stations x pol
    parms = {}
    parm = {}
    parm[ 'freqs' ] = np.array( [ .5e9 ] )
    parm[ 'freqwidths' ] = np.array( [ 1.0e9 ] )
    parm[ 'times' ] = ionmodel.times[:].ravel()
    parm[ 'timewidths' ] = ionmodel.timewidths[:].ravel()
    
    stations=list(ionmodel.stations[:])
    pol=ionmodel.polarizations[:]
    for ist,st in enumerate(stations):
        if average:
            Clock_parm = parm.copy()
            parmname = ':'.join(['Clock', st])
            value=0.5*ionmodel.Clock[:, ist,0]+0.5*ionmodel.Clock[:, ist]
            value*=1.e-9
            Clock_parm[ 'values' ] = value
            parms[ parmname ] = Clock_parm
        else:
            for  n_pol,ipol in enumerate(pol):
                Clock_parm = parm.copy()
                parmname = ':'.join(['Clock', str(ipol), st])
                Clock_parm[ 'values' ] = 1.e-9*ionmodel.Clock[:, ist,n_pol]
                parms[ parmname ] = Clock_parm
    #return parms        
    parmdbmain.store_parms( ionmodel.globaldb + '/ionosphere', parms, create_new = create_new)

def writePhaseScreentoParmdb(ionmodel,create_new = True):
    N_sources=ionmodel.N_sources
    if not hasattr(ionmodel,'globaldb'):
        ionmodel.globaldb='./'
    for n_source in range(N_sources):
       if ionmodel.DirectionalGainEnable:
          source = ionmodel.sources[n_source]
          identifier = ':'.join([str(pol), station, source])
       else:
          identifier = ':'.join([str(pol), station])

       # TEC
       TEC_parm = parm.copy()
       parmname = ':'.join(['TEC', identifier])
       TEC_parm[ 'values' ] = ionmodel.TEC[:,n_station,n_source,n_pol]
       parms[ parmname ] = TEC_parm

       #TECfit
       TECfit_parm = parm.copy()
       parmname = ':'.join(['TECfit', identifier])
       TECfit_parm[ 'values' ] = ionmodel.TECfit[:,n_station,n_source,n_pol]
       parms[ parmname ] = TECfit_parm

       #TECfit_white
       TECfit_white_parm = parm.copy()
       parmname = ':'.join(['TECfit_white', identifier])
       TECfit_white_parm[ 'values' ] = ionmodel.TECfit_white[:,n_station,n_source,n_pol]
       parms[ parmname ] = TECfit_white_parm

    # Piercepoints
      
    for n_station in range(len(ionmodel.stations)):
     station = ionmodel.stations[n_station]
     for n_source in range(N_sources):
        if ionmodel.DirectionalGainEnable:
           source = ionmodel.sources[n_source]
           identifier = ':'.join([station, source])
        else:
           identifier = station
        PiercepointX_parm = parm.copy()
        parmname = ':'.join(['Piercepoint', 'X', identifier])
        print n_source, n_station
        x = ionmodel.piercepoints[:]['positions_xyz'][:,n_source, n_station,0]
        PiercepointX_parm['values'] = x
        parms[ parmname ] = PiercepointX_parm

        PiercepointY_parm = parm.copy()
        parmname = ':'.join(['Piercepoint', 'Y', identifier])
        y = ionmodel.piercepoints[:]['positions_xyz'][:,n_source, n_station,1]
        PiercepointY_parm['values'] = array(y)
        parms[ parmname ] = PiercepointY_parm

        PiercepointZ_parm = parm.copy()
        parmname = ':'.join(['Piercepoint', 'Z', identifier])
        z = ionmodel.piercepoints[:]['positions_xyz'][:,n_source, n_station,2]
        PiercepointZ_parm['values'] = z
        parms[ parmname ] = PiercepointZ_parm

        Piercepoint_zenithangle_parm = parm.copy()
        parmname = ':'.join(['Piercepoint', 'zenithangle', identifier])
        za = ionmodel.piercepoints[:]['zenith_angles'][:,n_source, n_station]
        Piercepoint_zenithangle_parm['values'] = za
        parms[ parmname ] = Piercepoint_zenithangle_parm

     time_start = ionmodel.times[0] - ionmodel.timewidths[0]/2
     time_end = ionmodel.times[-1] + ionmodel.timewidths[-1]/2


     parm[ 'times' ] = numpy.array([(time_start + time_end) / 2])
     parm[ 'timewidths' ] = numpy.array([time_end - time_start])

     height_parm = parm.copy()
     height_parm[ 'values' ] = numpy.array( ionmodel.piercepoints.attrs.height )
     parms[ 'height' ] = height_parm

     beta_parm = parm.copy()
     beta_parm[ 'values' ] = numpy.array( ionmodel.TECfit_white.attrs.beta )
     parms[ 'beta' ] = beta_parm

     r_0_parm = parm.copy()
     r_0_parm[ 'values' ] = numpy.array(  ionmodel.TECfit_white.attrs.r_0 )
     parms[ 'r_0' ] = r_0_parm

     parmdbmain.store_parms( ionmodel.globaldb + '/ionosphere', parms, create_new = create_new)


def writePhaseScreenInfo(ionmodel,filename="clocktec.xmmlss.send"):
    if not hasattr(ionmodel,'TEC'):
        print 'no fitted TEC information in you model, maybe you forgot to fit?'
        return
    

    np.savez(filename,
             clock=ionmodel.Clock[:],
             tec=ionmodel.TEC[:],
             offset=ionmodel.hdf5.root.clock_tec_offsets[:],
             freqs=ionmodel.freqs[:],
             radec=ionmodel.pointing[:],
             antenna_names =ionmodel.stations[:],
             antenna_positions=ionmodel.station_positions[:],
             radeccal=ionmodel.source_positions[:])
    
