import numpy
import pylab
import lofar.parmdb

def fetch(db,stations, phasors=False, parmname="Gain:11"):
    print stations

    ampl = []
    phase = []
    for station in stations:
        if phasors:
            ampl.append(numpy.array(db.getValuesGrid("%s:Ampl:%s" % (parmname,station))["%s:Ampl:%s" % (parmname,station)]))
            phase.append(numpy.array(db.getValuesGrid("%s:Phase:%s" % (parmname,station))["%s:Phase:%s" % (parmname,station)]))
        else:        
            re = numpy.array(db.getValuesGrid("%s:Real:%s" % (parmname,station))["%s:Real:%s" % (parmname,station)])
            im = numpy.array(db.getValuesGrid("%s:Imag:%s" % (parmname,station))["%s:Imag:%s" % (parmname,station)])
            
            tmp = numpy.sqrt(re * re + im * im)
            tmp.transpose()
            ampl.append(tmp)
            
            tmp = numpy.arctan2(im, re)
            tmp.transpose()
            phase.append(tmp)

    return (ampl, phase)
    
def plot(sol):
    pylab.clf()
    
    offset = 0.0
    for i in range(0,len(sol)):
        pylab.plot(sol[i] + offset)
        offset += sol[i].mean() + 5.0 * sol[i].std()
        
    pylab.show()
    
