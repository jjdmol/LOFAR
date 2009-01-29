import numpy
import pylab
import lofar.parmdb

def fetch(db, stations, phasors=False, parm="Gain:11", direction=""):
    print stations

    suffix = ""
    if direction:
        suffix = ":%s" % direction

    ampl = []
    phase = []
    for station in stations:
        if phasors:
            fqname = "%s:Ampl:%s%s" % (parm, station, suffix)
            ampl.append(numpy.array(db.getValuesGrid(fqname)[fqname]))
            fqname = "%s:Phase:%s%s" % (parm, station, suffix)
            phase.append(numpy.array(db.getValuesGrid(fqname)[fqname]))
        else:        
            fqname = "%s:Real:%s%s" % (parm, station, suffix)
            re = numpy.array(db.getValuesGrid(fqname)[fqname])
            fqname = "%s:Imag:%s%s" % (parm, station, suffix)
            im = numpy.array(db.getValuesGrid(fqname)[fqname])
            
            tmp = numpy.sqrt(re * re + im * im)
            tmp.transpose()
            ampl.append(tmp)
            
            tmp = numpy.arctan2(im, re)
            tmp.transpose()
            phase.append(tmp)

    return (ampl, phase)
    
def plot(sol, stack=True, sep=5.0, fig=None):
    pylab.figure(fig)
    pylab.clf()
    
    offset = 0.0
    for i in range(0,len(sol)):
        pylab.plot(sol[i] + offset)
        if stack:
            offset += sol[i].mean() + sep * sol[i].std()
        
    pylab.show()
