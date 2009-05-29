import math
import numpy
import pylab
import lofar.parmdb

def fetch(db, stations, phasors=False, parm="Gain:11", direction=None):
    print stations

    suffix = ""
    if not direction is None:
        suffix = ":%s" % direction

    ampl = []
    phase = []
    for station in stations:
        if phasors:
            fqname = "%s:Ampl:%s%s" % (parm, station, suffix)
            ampl.append(numpy.squeeze(numpy.array(db.getValuesGrid(fqname)[fqname])))
            fqname = "%s:Phase:%s%s" % (parm, station, suffix)
            phase.append(numpy.squeeze(numpy.array(db.getValuesGrid(fqname)[fqname])))
        else:        
            fqname = "%s:Real:%s%s" % (parm, station, suffix)
            re = numpy.squeeze(numpy.array(db.getValuesGrid(fqname)[fqname]))
            fqname = "%s:Imag:%s%s" % (parm, station, suffix)
            im = numpy.squeeze(numpy.array(db.getValuesGrid(fqname)[fqname]))
            
            tmp = numpy.sqrt(re * re + im * im)
            tmp.transpose()
            ampl.append(tmp)
            
            tmp = numpy.arctan2(im, re)
            tmp.transpose()
            phase.append(tmp)

    return (numpy.array(ampl), numpy.array(phase))
    

def phase_unwrap(phase, tol=0.5, delta_tol=1.0):
    """
    Unwrap phase by restricting phase[n] to fall within a range [-tol, tol]
    around phase[n - 1].
    
    If this is impossible, the closest phase (modulo 2*pi) is used and tol is
    increased by delta_tol (tol is capped at pi).
    """

    assert(tol < math.pi)

    # Effective tolerance.
    eff_tol = tol

    ref = phase[0]
    for i in range(0, len(phase)):
        delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

        if delta < -math.pi:
            delta += 2.0 * math.pi
        elif delta > math.pi:
            delta -= 2.0 * math.pi

        phase[i] = ref + delta

        if abs(delta) <= eff_tol:
            # Update reference phase and reset effective tolerance.
            ref = phase[i]
            eff_tol = tol
        elif eff_tol < math.pi:
            # Increase effective tolerance.
            eff_tol += delta_tol * tol
            if eff_tol > math.pi:
                eff_tol = math.pi


def phase_unwrap2(phase, window_size=5):
    """
    Unwrap phase by estimating the trend of the phase signal.
    """

    windowl = numpy.array([math.fmod(phase[0], 2.0 * math.pi)] * window_size)
    
    delta = math.fmod(phase[1] - windowl[0], 2.0 * math.pi)
    if delta < -math.pi:
        delta += 2.0 * math.pi
    elif delta > math.pi:
        delta -= 2.0 * math.pi
    windowu = numpy.array([windowl[0] + delta] * window_size)

    phase[0] = windowl[0]
    phase[1] = windowu[0]

    meanl = windowl.mean()
    meanu = windowu.mean()
    slope = (meanu - meanl) / float(window_size)

    for i in range(2, len(phase)):
        ref = meanu + (1.0 + (float(window_size) - 1.0) / 2.0) * slope
        delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

        if delta < -math.pi:
            delta += 2.0 * math.pi
        elif delta > math.pi:
            delta -= 2.0 * math.pi
        
        phase[i] = ref + delta
        
        windowl[:-1] = windowl[1:]
        windowl[-1] = windowu[0]
        windowu[:-1] = windowu[1:]
        windowu[-1] = phase[i]

        meanl = windowl.mean()
        meanu = windowu.mean()
        slope = (meanu - meanl) / float(window_size)
    
    
def phase_normalize(phase):
    """
    Normalize input phase to the range [-pi, pi].
    """

    # Convert to range [-2*pi, 2*pi].
    numpy.fmod(phase, 2.0 * numpy.pi)

    # Convert to range [-pi, pi]
    phase[phase < -numpy.pi] += 2.0 * numpy.pi
    phase[phase > numpy.pi] -= 2.0 * numpy.pi


def plot(sol, stack=True, sep=5.0, fig=None, sub=None):
    """
    Plot a list of signals.
    
    If 'fig' is equal to None, a new figure will be created. Otherwise, the
    specified figure number is used. The 'sub' argument can be used to create
    subplots. The 'sep' and 'stack' argument can be used to control placement
    of the plots in the list.
    
    The figure number of the figure used to plot in is returned.
    """    
    
    cf = pylab.figure(fig)
    
    if fig is not None and sub is None:
        pylab.clf()

    if sub is not None:    
        pylab.subplot(sub)
    
    offset = 0.0
    for i in range(0,len(sol)):
        pylab.plot(sol[i] + offset)
        if stack:
            offset += sol[i].mean() + sep * sol[i].std()

    return cf.number
    
