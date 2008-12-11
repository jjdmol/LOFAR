SHM_WEBSERVER = '10.144.0.1'
SHM_WEB_PREF = 'http://'+ SHM_WEBSERVER + '/shm/'

def diagnosis_to_set(diag):
    '''flatten a diagnosis to a set of strings'''
    if (len(diag['component']) == 0):
        return set()
    else:
        s = set()
        for i in range(len(diag['component'])):
            s.add("%s@%1d@%04d"%(diag['component'][i],diag['state'][i],diag['confidence'][i]))
        return s

def set_to_diagnosis(dset):
    '''unflatten set to diagnosis'''
    if (len(dset) == 0):
        return {}
    else:
        diagnosis = {'time':0,
                     'num_faults':0,
                     'datum_epoch':[],
                     'component':[],
                     'state':[],
                     'confidence':[],
                     'reported':'false',
                     'si_id': 0}
        for s in dset:
            v = s.split('@')
            diagnosis['component'].append(v[0])
            diagnosis['state'].append(v[1])
            diagnosis['confidence'].append(v[2])
        diagnosis['num_faults'] = len(diagnosis['component'])

        return diagnosis
    
def db_to_diagnosis(dbinst):
    '''SHM DB diagnosis query result to diagnosis dict'''
    #if (len(dbinst) == 0):
    #    return {}
    #else:
    diagnosis = {'time'        :dbinst.time,
                 'num_faults'  :dbinst.num_faults,
                 'datum_epoch' :dbinst.datum_epoch,
                 'component'   :dbinst.component,
                 'state'       :dbinst.state,
                 'confidence'  :dbinst.confidence,
                 'reported'    :dbinst.reported_to_mis,
                 'si_id'       :dbinst.si_id}
    return diagnosis

def url(db_url):
    '''add the unchanging prefix to urls from the SHM database'''
    return SHM_WEB_PREF + db_url
