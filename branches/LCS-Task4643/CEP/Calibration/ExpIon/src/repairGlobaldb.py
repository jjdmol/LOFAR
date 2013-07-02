import tables
import lofar.parmdb
import pyrap.tables as pt
import lofar.expion.ionosphere as iono
import os
import numpy as np
class dummyion:
    pass

def get_source_list( pdb, source_pattern_list ):
   source_list = []
   for pattern in source_pattern_list :
      parmname_list = pdb.getNames( 'DirectionalGain:?:?:*:*:' + pattern )
      source_list.extend([n.split(':')[-1] for n in parmname_list])
      parmname_list = pdb.getNames( 'RotationAngle:*:' + pattern )
      source_list.extend([n.split(':')[-1] for n in parmname_list])
   print set(source_list)
   return sorted(set(source_list))

def get_station_list( pdb, station_pattern_list, DirectionalGainEnable ):
   station_list = []
   for pattern in station_pattern_list :
      parmname_list = pdb.getNames( { True : 'DirectionalGain:?:?:*:'+pattern+':*', False: 'Gain:?:?:*:' + pattern}[DirectionalGainEnable] )
      station_list.extend(sorted(set([n.split(':')[{True : -2, False : -1}[DirectionalGainEnable]] for n in parmname_list])))
   return station_list

def repair_station_table(myion,globaldbpath,instrumentdb):
    stations = ["*"]
    myion.stations = get_station_list( instrumentdb, stations, myion.DirectionalGainEnable )
    myion.N_stations = len(myion.stations)

    antenna_table_name = os.path.join( globaldbpath, "ANTENNA")
    if not os.path.exists(antenna_table_name) :
        print "ANTENNA table not existing, please copy to globaldb"
        return
    antenna_table = pt.table(antenna_table_name) 
    name_col = antenna_table.getcol('NAME')
    position_col = antenna_table.getcol( 'POSITION' )
    myion.station_positions = [position_col[name_col.index(station_name)] for station_name in myion.stations]
    antenna_table.close()
    station_table = myion.hdf5.createTable(myion.hdf5.root, 'stations', {'name': tables.StringCol(40), 'position':tables.Float64Col(3)})
    row = station_table.row
    for (station, position) in zip(myion.stations, myion.station_positions) : 
        row['name'] = station
        row['position'] = position
        row.append()
        station_table.flush()
    myion.array_center = np.array( myion.station_positions ).mean(axis=0).tolist()
    myion.hdf5.createArray(myion.hdf5.root, 'array_center', myion.array_center)
    


def repair_pointing(myion,globaldbpath):
    field_table_name = os.path.join( globaldbpath, "FIELD" )
    if not os.path.exists(field_table_name) :
        print "FIELD table not existing, please copy to globaldb"
        return
    field_table = pt.table( field_table_name)
    field_table = pt.table( globaldbpath + "/FIELD")
    phase_dir_col = field_table.getcol('PHASE_DIR')
    myion.pointing = phase_dir_col[0,0,:]
    field_table.close()
    myion.hdf5.createArray(myion.hdf5.root, 'pointing', myion.pointing)
    

def repair_sources(myion,globaldb,instrumentdb):
    skydbname = globaldb + "/sky"
    if not os.path.exists(skydbname) : 
        print "No skydb found, copy first to globaldb"
        return
    skydb = lofar.parmdb.parmdb( skydbname )
    sources = ["*"]
    myion.sources = get_source_list( instrumentdb, sources )
    myion.source_positions = []
    for source in myion.sources :
        try:
            RA = skydb.getDefValues( 'Ra:' + source )['Ra:' + source][0][0]
            dec = skydb.getDefValues( 'Dec:' + source )['Dec:' + source][0][0]
        except KeyError:
            # Source not found in skymodel parmdb, try to find components
            RA = np.array(skydb.getDefValues( 'Ra:' + source + '.*' ).values()).mean()
            dec = np.array(skydb.getDefValues( 'Dec:' + source + '.*' ).values()).mean()
        myion.source_positions.append([RA, dec])

def add_to_h5_func(h5file,data,name='test',dtype=None):
    atom = tables.Atom.from_dtype(data.dtype)
    if name in h5file.root:
        h5file.removeNode('/'+name)
    myarray=h5file.createCArray(h5file.root,name,atom,shape=data.shape)
    myarray[:]=data
    myarray.flush()
        
def doRepair(globaldbpath,
             GainEnable = False, DirectionalGainEnable = False,
             PhasorsEnable = False, RotationEnable = False, CommonRotationEnable = False,polarizations=[0,1],tablename='instrument-0'):
    if os.path.isfile(globaldbpath+'/ionmodel.hdf5'):
        myion=iono.IonosphericModel([globaldbpath])
    else:
        myion=dummyion()
        myion.hdf5=tables.openFile(globaldbpath+'/ionmodel.hdf5','w')

    myion.GainEnable = GainEnable
    myion.DirectionalGainEnable = DirectionalGainEnable
    myion.PhasorsEnable =PhasorsEnable 
    myion.RotationEnable =RotationEnable 
    myion.CommonRotationEnable =CommonRotationEnable 
    myion.polarizations = polarizations
    myion.N_pol = len(polarizations)
    myion.instrument_db_list=[globaldbpath+'/%s-%i'%(tablename,idx) for idx in range(500) if os.path.isdir(globaldbpath+'/%s-%i'%(tablename,idx))]
    
    instrumentdb=lofar.parmdb.parmdb(myion.instrument_db_list[0])

    if hasattr(myion,'stations'):
        stations=myion.stations
    else:
        repair_station_table(myion,globaldbpath,instrumentdb)


    if not hasattr(myion,'pointing'):
        repair_pointing(myion,globaldbpath)


    if not hasattr(myion,'sources'):
        if DirectionalGainEnable or myion.RotationEnable:
            print "getting source names from instrumentdb"
            repair_sources(myion,globaldbpath,instrumentdb)
        
        else:
            myion.sources = ["Pointing"]
            myion.source_positions = [list(myion.pointing)]
        
        source_table = myion.hdf5.createTable(myion.hdf5.root, 'sources', {'name': tables.StringCol(40), 'position':tables.Float64Col(2)})
        row = source_table.row
        for (source, position) in zip(myion.sources, myion.source_positions) :
            row['name'] = source
            row['position'] = position
            row.append()
        source_table.flush()
    myion.N_sources = len(myion.sources)
           
    # First collect all frequencies 
    # We need them beforehand to sort the frequencies (frequencies are not necessarily given in sorted order)
    if PhasorsEnable:
        infix = ('Ampl', 'Phase')
    else:
        infix = ('Real', 'Imag')
    if myion.GainEnable :
        parmname0 = ':'.join(['Gain', str(myion.polarizations[0]), str(myion.polarizations[0]), infix[1], myion.stations[0]])
    else:
        if myion.DirectionalGainEnable :
            parmname0 = ':'.join(['DirectionalGain', str(myion.polarizations[0]), str(myion.polarizations[0]), infix[1], myion.stations[0], myion.sources[0]])
    myion.freqs = []
    myion.freqwidths = []
    newdblist=[]
    for instrumentdb_name in myion.instrument_db_list:
        print "opening",instrumentdb_name,parmname0
        try:
            instrumentdb = lofar.parmdb.parmdb( instrumentdb_name )
            v0 = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]
            freqs = v0['freqs']
        except:
            print "Error opening " + instrumentdb_name,"removing from list"
        else:
            myion.freqs = np.concatenate([myion.freqs, freqs])
            myion.freqwidths = np.concatenate([myion.freqwidths, v0['freqwidths']])
            newdblist.append(instrumentdb_name)
    myion.instrument_db_list=newdblist
    # Sort frequencies, find both the forward and inverse mapping
    # Mappings are such that
    #    sorted_freqs = unsorted_freqs[sorted_freq_idx]
    #    sorted_freqs[inverse_sorted_freq_idx] = unsorted_freqs
    # We will use the following form
    #    sorted_freqs[inverse_sorted_freq_idx[selection]] = unsorted_freqs[selection]
    # to process chunks (=selections) of unsorted data and store them in sorted order
    sorted_freq_idx = sorted(range(len(myion.freqs)), key = lambda idx: myion.freqs[idx])
    inverse_sorted_freq_idx = sorted(range(len(myion.freqs)), key = lambda idx: sorted_freq_idx[idx])
      
    myion.freqs = myion.freqs[sorted_freq_idx]
    myion.freqwidths = myion.freqwidths[sorted_freq_idx]
    add_to_h5_func(myion.hdf5,np.array(myion.freqs),name='freqs',dtype=tables.Float64Atom())
    add_to_h5_func(myion.hdf5,np.array(myion.freqwidths),name='freqwidths',dtype=tables.Float64Atom())
    myion.N_freqs = len(myion.freqs)
    
    myion.times = v0['times']
    myion.timewidths = v0['timewidths']
    add_to_h5_func(myion.hdf5,myion.times,name='times',dtype=tables.Float64Atom())
    add_to_h5_func(myion.hdf5,myion.timewidths,name='timewidths',dtype=tables.Float64Atom())

    myion.N_times = len( myion.times )
    add_to_h5_func(myion.hdf5,  np.array(myion.polarizations),name='polarizations',dtype=tables.Float32Atom())
      

    if GainEnable or DirectionalGainEnable:
        if hasattr(myion,'phases'):
            myion.hdf5.removeNode('/phases')
        chunkshape = (1024 , 32, 1, 1, 1)
        ph=myion.hdf5.createCArray(myion.hdf5.root, 'phases', tables.Float64Atom(), shape=(myion.N_times, myion.N_freqs, myion.N_stations, myion.N_sources, myion.N_pol), chunkshape = chunkshape)
            
        if hasattr(myion,'amplitudes'):
            myion.hdf5.removeNode('/amplitudes')
        chunkshape = (1024 , 32, 1, 1, 1)
        amp = myion.hdf5.createCArray(myion.hdf5.root, 'amplitudes', tables.Float64Atom(), shape=(myion.N_times, myion.N_freqs, myion.N_stations, myion.N_sources, myion.N_pol), chunkshape = chunkshape) 
        if not hasattr(myion,'flags'):   
            myion.flags = myion.hdf5.createCArray(myion.hdf5.root, 'flags', tables.Float32Atom(), shape=(myion.N_times, myion.N_freqs))

    if RotationEnable or CommonRotationEnable:
        if not hasattr(myion,'rotation'):
            chunkshape = (1024 , 32, 1, 1)
            rotation = myion.hdf5.createCArray(myion.hdf5.root, 'rotation', tables.Float64Atom(), shape=ph.shape[:4], chunkshape = chunkshape)
        else:
            rotation =  myion.rotation

    freq_idx = 0

    for instrumentdb_name in myion.instrument_db_list:
        print "processing",instrumentdb_name
        instrumentdb = lofar.parmdb.parmdb( instrumentdb_name )
        v0 = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]
        freqs = v0['freqs']
        N_freqs = len(freqs)
        sorted_freq_selection = inverse_sorted_freq_idx[freq_idx:freq_idx+N_freqs]
        for pol_idx,pol in enumerate(myion.polarizations[:]):
            for station_idx,station in enumerate(list(myion.stations[:])):
                
                if GainEnable:
                    parmname0 = ':'.join(['Gain', str(pol), str(pol), infix[0], station])
                    parmname1 = ':'.join(['Gain', str(pol), str(pol), infix[1], station])
                    if PhasorsEnable:
                        gain_phase = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']
                        if gain_phase.shape != ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, 0, pol_idx].shape:
                            print "wrong shape",gain_phase.shape,parmname1
                            continue;
                        
                        ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, 0, pol_idx] = gain_phase
                        gain_amplitude = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                        amp[:,sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, 0, pol_idx] = gain_ampitude
                    else:
                        gain_real = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                        gain_imag = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']

                        cdata=gain_real+1.j*gain_imag
                        if cdata.shape != ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, 0, pol_idx].shape:
                            print "wrong shape",cdata.shape,parmname1
                            continue;

                        ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, 0, pol_idx] =np.angle(cdata)

                        amp[:,sorted_freq_selection[0]:sorted_freq_selection[-1]+1 , station_idx, 0, pol_idx] = np.absolute(cdata)

                if myion.DirectionalGainEnable:
                    for source_idx,source in enumerate(myion.sources):
                        parmname0 = ':'.join(['DirectionalGain', str(pol), str(pol), infix[0], station, source])
                        parmname1 = ':'.join(['DirectionalGain', str(pol), str(pol), infix[1], station, source])
                        hasAmpl=False
                        hasPhase=False
                        if parmname0 in instrumentdb.getNames():
                            hasAmpl=True
                        if parmname1 in instrumentdb.getNames():
                            hasPhase=True
                            
                        if myion.PhasorsEnable:
                            
                            if hasPhase:
                                gain_phase = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']
                                if gain_phase.shape != ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, source_idx, pol_idx].shape:
                                    print "wrong shape",gain_phase.shape,parmname1
                                    continue;
                        
                                ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, source_idx, pol_idx] = gain_phase
                            if hasAmpl:
                                gain_amplitude = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                                amp[:,sorted_freq_selection[0]:sorted_freq_selection[-1]+1 , station_idx, source_idx, pol_idx] = gain_ampitude
                        else:
                            gain_real = instrumentdb.getValuesGrid( parmname0 )[ parmname0 ]['values']
                            gain_imag = instrumentdb.getValuesGrid( parmname1 )[ parmname1 ]['values']

                            cdata=gain_real+1.j*gain_imag
                            if cdata.shape != ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, source_idx, pol_idx].shape:
                                print "wrong shape",cdata.shape,parmname1
                                continue;
                        
                            ph[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, source_idx, pol_idx] =np.angle(cdata)

                            amp[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, source_idx, pol_idx] = np.absolute(cdata)
                     
        if myion.CommonRotationEnable:
            for station_idx,station in enumerate(myion.stations):
                parmname = ':'.join(['CommonRotationAngle', station])
                rot = instrumentdb.getValuesGrid( parmname )[ parmname ]['values']
                rotation[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx,0] = rot

        if myion.RotationEnable:
            for station_idx,station in enumerate(myion.stations):
                for source_idx,source in enumerate(myion.sources):
                    parmname = ':'.join(['RotationAngle', station,source])
                    rot = instrumentdb.getValuesGrid( parmname )[ parmname ]['values']
                    if rot.shape != rotation[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx, source_idx].shape:
                        print "wrong shape",rot.shape,parmname
                        continue;
                    rotation[:, sorted_freq_selection[0]:sorted_freq_selection[-1]+1, station_idx,source_idx] = rot



        freq_idx += N_freqs
