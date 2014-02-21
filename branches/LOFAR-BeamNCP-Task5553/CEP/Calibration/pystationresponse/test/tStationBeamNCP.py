try:
    import lofar.stationresponse as st
    import pyrap.tables as tab
    import numpy as np
    import sys

    MSNAME='tStationBeamNCP_tmp.MS'

    myt=tab.table(MSNAME,ack=False)
    mys=st.stationresponse( msname=MSNAME, inverse=False, useElementResponse=True, useArrayFactor=False, useChanFreq=False)
    times=myt.getcol('TIME')
    mys.setDirection(0.01,0.5*np.pi)

    a=[mys.evaluateStation(time=times[0],station=st) for st in range(20)]

    for a1 in a:
        for a2 in a:
            if np.linalg.norm(a1-a2)>1.e-6:
                sys.exit(1)

    sys.exit(0)
except:
    sys.exit(3)
