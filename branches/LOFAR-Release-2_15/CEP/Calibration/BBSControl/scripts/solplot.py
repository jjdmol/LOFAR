import math
import numpy
import pylab

__styles = ["%s%s" % (x, y) for y in ["-", ":"] for x in ["b", "g", "r", "c",
    "m", "y", "k"]]

def unwrap(phase, tol=0.25, delta_tol=0.25):
    """
    Unwrap phase by restricting phase[n] to fall within a range [-tol, tol]
    around phase[n - 1].

    If this is impossible, the closest phase (modulo 2*pi) is used and tol is
    increased by delta_tol (tol is capped at pi).

    The "phase" argument is assumed to be a 1-D array of phase values.
    """

    assert(tol < math.pi)

    # Allocate result.
    out = numpy.zeros(phase.shape)

    # Effective tolerance.
    eff_tol = tol

    ref = phase[0]
    for i in range(0, len(phase)):
        delta = math.fmod(phase[i] - ref, 2.0 * math.pi)

        if delta < -math.pi:
            delta += 2.0 * math.pi
        elif delta > math.pi:
            delta -= 2.0 * math.pi

        out[i] = ref + delta

        if abs(delta) <= eff_tol:
            # Update reference phase and reset effective tolerance.
            ref = out[i]
            eff_tol = tol
        elif eff_tol < math.pi:
            # Increase effective tolerance.
            eff_tol += delta_tol * tol
            if eff_tol > math.pi:
                eff_tol = math.pi

    return out

def unwrap_windowed(phase, window_size=5):
    """
    Unwrap phase by estimating the trend of the phase signal.

    The "phase" argument is assumed to be a 1-D array of phase values.
    """

    # Allocate result.
    out = numpy.zeros(phase.shape)

    windowl = numpy.array([math.fmod(phase[0], 2.0 * math.pi)] * window_size)

    delta = math.fmod(phase[1] - windowl[0], 2.0 * math.pi)
    if delta < -math.pi:
        delta += 2.0 * math.pi
    elif delta > math.pi:
        delta -= 2.0 * math.pi
    windowu = numpy.array([windowl[0] + delta] * window_size)

    out[0] = windowl[0]
    out[1] = windowu[0]

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

        out[i] = ref + delta

        windowl[:-1] = windowl[1:]
        windowl[-1] = windowu[0]
        windowu[:-1] = windowu[1:]
        windowu[-1] = out[i]

        meanl = windowl.mean()
        meanu = windowu.mean()
        slope = (meanu - meanl) / float(window_size)

    return out

def normalize(phase):
    """
    Normalize phase to the range [-pi, pi].
    """

    # Convert to range [-2*pi, 2*pi].
    out = numpy.fmod(phase, 2.0 * numpy.pi)

    # Convert to range [-pi, pi]
    out[out < -numpy.pi] += 2.0 * numpy.pi
    out[out > numpy.pi] -= 2.0 * numpy.pi

    return out

def plot(sol, fig=None, sub=None, scatter=False, stack=False, sep=5.0,
    sep_abs=False, labels=None, show_legend=False):
    """
    Plot a sequence of 1-D real valued signals.

    If 'fig' is equal to None, a new figure will be created. Otherwise, the
    specified figure number is used. The 'sub' argument can be used to create
    subplots.

    The 'scatter' argument selects between scatter and line plots.

    The 'stack', 'sep', and 'sep_abs' arguments can be used to control placement
    of the plots in the list. If 'stack' is set to True, each plot will be
    offset by the mean plus sep times the standard deviation of the previous
    plot. If 'sep_abs' is set to True, 'sep' is used as is.

    The 'labels' argument can be set to a list of labels and 'show_legend' can
    be set to True to show a legend inside the plot.

    The figure number of the figure used to plot in is returned.
    """
    global __styles

    cf = pylab.figure(fig)

    if fig is not None and sub is None:
        pylab.clf()

    if sub is not None:
        pylab.subplot(sub)

    offset = 0.0
    for i in range(0,len(sol)):
        if labels is None:
            if scatter:
                pylab.scatter(range(0, len(sol[i])), sol[i] + offset,
                    edgecolors="None", c=__styles[i % len(__styles)][0],
                    marker="o")
            else:
                pylab.plot(sol[i] + offset, __styles[i % len(__styles)])
        else:
            if scatter:
                pylab.scatter(range(0, len(sol[i])), sol[i] + offset,
                    edgecolors="None", c=__styles[i % len(__styles)][0],
                    marker="o", label=labels[i])
            else:
                pylab.plot(sol[i] + offset, __styles[i % len(__styles)],
                    label=labels[i])

        if stack:
            if sep_abs:
                offset += sep
            else:
                offset += sol[i].mean() + sep * sol[i].std()

    if not labels is None and show_legend:
        pylab.legend()

    return cf.number
