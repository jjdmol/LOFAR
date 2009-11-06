import numpy
import pylab
import lofar.parmdb

def fetch(db, stations, phasors=False):
    print stations

    ampl = []
    phase = []
    for station in stations:
        if phasors:
            ampl.append(numpy.array(db.getValuesGrid("Gain:11:Ampl:%s" % station)["Gain:11:Ampl:%s" % station]))
            phase.append(numpy.array(db.getValuesGrid("Gain:11:Phase:%s" % station)["Gain:11:Phase:%s" % station]))
        else:        
            re = numpy.array(db.getValuesGrid("Gain:11:Real:%s" % station)["Gain:11:Real:%s" % station])
            im = numpy.array(db.getValuesGrid("Gain:11:Imag:%s" % station)["Gain:11:Imag:%s" % station])
            
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
    
