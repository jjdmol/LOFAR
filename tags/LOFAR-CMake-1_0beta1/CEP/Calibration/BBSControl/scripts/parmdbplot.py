#!/usr/bin/env python
#
# Copyright (C) 2007
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

import sys
import lofar.parmdb as parmdb
import copy
import math
import numpy

import matplotlib
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure
from matplotlib.font_manager import FontProperties

from PyQt4.QtCore import *
from PyQt4.QtGui import *

__styles = ["%s%s" % (x, y) for y in ["-", ":"] for x in ["b", "g", "r", "c",
    "m", "y", "k"]]

def contains(container, item):
    try:
        return container.index(item) >= 0
    except ValueError:
        return False

def common_domain(parms):
    if len(parms) == 0:
        return None

    domain = [-1e30, 1e30, -1e30, 1e30]
    for parm in parms:
        tmp = parm.domain()
        domain = [max(domain[0], tmp[0]), min(domain[1], tmp[1]), max(domain[2], tmp[2]), min(domain, tmp[3])]

    if domain[0] >= domain[1] or domain[2] >= domain[3]:
        return None

    return domain

def parseFloat(text, lower, upper):
    value = float(text)
    if value < lower:
        value = lower
    elif value > upper:
        value = upper

    return value

def unwrap(phase, tol=0.25, delta_tol=0.25):
    """
    Unwrap phase by restricting phase[n] to fall within a range [-tol, tol]
    around phase[n - 1].

    If this is impossible, the closest phase (modulo 2*pi) is used and tol is
    increased by delta_tol (tol is capped at pi).
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

def plot(sol, fig, clf=True, sub=None, scatter=False, stack=False, sep=5.0,
    sep_abs=False, labels=None, show_legend=False, title=None, xlabel=None,
    ylabel=None):
    """
    Plot a list of signals.

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

    if clf:
        fig.clf()

    if sub is not None:
        fig.add_subplot(sub)

    axes = fig.gca()
    if not title is None:
        axes.set_title(title)
    if not xlabel is None:
        axes.set_xlabel(xlabel)
    if not ylabel is None:
        axes.set_ylabel(ylabel)

    offset = 0.0
    for i in range(0,len(sol)):
        if labels is None:
            if scatter:
                axes.scatter(range(0, len(sol[i])), sol[i] + offset,
                    edgecolors="None", c=__styles[i % len(__styles)][0],
                    marker="o")
            else:
                axes.plot(sol[i] + offset, __styles[i % len(__styles)])
        else:
            if scatter:
                axes.scatter(range(0, len(sol[i])), sol[i] + offset,
                    edgecolors="None", c=__styles[i % len(__styles)][0],
                    marker="o", label=labels[i])
            else:
                axes.plot(sol[i] + offset, __styles[i % len(__styles)],
                    label=labels[i])

        if stack:
            if sep_abs:
                offset += sep
            else:
                offset += sol[i].mean() + sep * sol[i].std()

    if not labels is None and show_legend:
        axes.legend(prop=FontProperties(size="x-small"), markerscale=0.5)

class Parm:
    def __init__(self, db, name, elements=None, isPolar=False):
        self._db = db
        self._name = name
        self._elements = elements
        self._isPolar = isPolar
        self._value = None
        self._value_domain = None
        self._value_resolution = None

        self._readDomain()

    def name(self):
        return self._name

    def isPolar(self):
        return self._isPolar

    def empty(self):
        return self._empty

    def domain(self):
        return self._domain

    def value(self, domain=None, resolution=None, asPolar=True, unwrap_phase=False):
        if self.empty():
            assert(False)
            return (numpy.zeros((1,1)), numpy.zeros((1,1)))

        if self._value is None or self._value_domain != domain or self._value_resolution != resolution:
            self._readValue(domain, resolution)

        if asPolar:
            if self.isPolar():
                ampl = self._value[0]
                phase = normalize(self._value[1])
            else:
                ampl = numpy.sqrt(numpy.power(self._value[0], 2) + numpy.power(self._value[1], 2))
                phase = numpy.arctan2(self._value[1], self._value[0])

            if unwrap_phase:
                for i in range(0, phase.shape[1]):
                    phase[:, i] = unwrap(phase[:, i])

            return (ampl, phase)

        if not self.isPolar():
            re = self._value[0]
            im = self._value[1]
        else:
            re = self._value[0] * numpy.cos(self._value[1])
            im = self._value[0] * numpy.sin(self._value[1])

        return (re, im)

    def _readDomain(self):
        if self._elements is None:
            self._domain = self._db.getRange(self.name())

        domain_el0 = self._db.getRange(self._elements[0])
        domain_el1 = self._db.getRange(self._elements[1])
        self._domain = [max(domain_el0[0], domain_el1[0]), min(domain_el0[1], domain_el1[1]), max(domain_el0[2], domain_el1[2]), min(domain_el0[3], domain_el1[3])]

        self._empty = (self._domain[0] >= self._domain[1]) or (self._domain[2] >= self._domain[3])

    def _readValue(self, domain=None, resolution=None):
#        print "fetching:", self.name()

        if self._elements is None:
            value = numpy.array(self.__fetch_value(self.name(), domain, resolution))
            self._value = (value, numpy.zeros(value.shape))
        else:
            el0 = numpy.array(self.__fetch_value(self._elements[0], domain, resolution))
            el1 = numpy.array(self.__fetch_value(self._elements[1], domain, resolution))
            self._value = (el0, el1)

        self._value_domain = domain
        self._value_resolution = resolution

    def __fetch_value(self, name, domain=None, resolution=None):
        if domain is None:
            tmp = self._db.getValuesGrid(name)[name]
        else:
            if resolution is None:
                tmp = self._db.getValues(name, domain[0], domain[1], domain[2], domain[3])[name]
            else:
                tmp = self._db.getValuesStep(name, domain[0], domain[1], resolution[0], domain[2], domain[3], resolution[1])[name]

        if type(tmp) is dict:
            return tmp["values"]

        # Old parmdb interface.
        return tmp

class PlotWindow(QFrame):
    def __init__(self, parms, resolution=None, parent=None):
        QFrame.__init__(self, parent)

        self.parms = parms
        self.resolution = resolution

        self.fig = Figure((5, 4), dpi=100)

        self.canvas = FigureCanvas(self.fig)
        self.canvas.setParent(self)

        self.toolbar = NavigationToolbar(self.canvas, self)

        self.axis = 0
        self.index = 0
        axisSelector = QComboBox()
        axisSelector.addItem("Frequency")
        axisSelector.addItem("Time")
        self.connect(axisSelector, SIGNAL('activated(int)'), self.handle_axis)

        self.show_legend = False
        legendCheck = QCheckBox("Legend")
        self.connect(legendCheck, SIGNAL('stateChanged(int)'), self.handle_legend)

        self.polar = True
        polarCheck = QCheckBox("Polar")
        polarCheck.setChecked(True)
        self.connect(polarCheck, SIGNAL('stateChanged(int)'), self.handle_polar)

        self.unwrap_phase = False
        unwrapCheck = QCheckBox("Unwrap phase")
        self.connect(unwrapCheck, SIGNAL('stateChanged(int)'), self.handle_unwrap)

#        self.slider = QSlider(Qt.Horizontal)
#        self.slider.setMinimum(0)
#        self.slider.setMaximum(159)
#        self.connect(self.slider, SIGNAL('sliderReleased()'), self.handle_slider)

        self.spinner = QSpinBox()
        self.connect(self.spinner, SIGNAL('valueChanged(int)'), self.handle_spinner)
        hbox = QHBoxLayout()
        hbox.addWidget(axisSelector)
        hbox.addWidget(self.spinner)
        hbox.addWidget(legendCheck)
        hbox.addWidget(polarCheck)
        hbox.addWidget(unwrapCheck)
        hbox.addStretch(1)
        hbox.addWidget(self.toolbar)

        layout = QVBoxLayout()
        layout.addWidget(self.canvas, 1)
        layout.addLayout(hbox);
        self.setLayout(layout)

        self.domain = common_domain(self.parms)

        self.shape = (0, 0)
        if not self.domain is None:
            self.shape = (self.parms[0].value(self.domain, self.resolution)[0].shape)
            assert(len(self.shape) == 2)

        self.spinner.setRange(0, self.shape[1 - self.axis] - 1)

        self.plot()

    def plot(self):
        el0 = []
        el1 = []
        labels = []

        if not self.domain is None:
            for parm in self.parms:
                value = parm.value(self.domain, self.resolution, self.polar, self.unwrap_phase)

                if value[0].shape != self.shape or value[1].shape != self.shape:
                    print "warning: non-consistent result shape; will skip parameter:", parm.name()
                    continue

                if self.axis == 0:
                    el0.append(value[0][:, self.index])
                    el1.append(value[1][:, self.index])
                else:
                    el0.append(value[0][self.index, :])
                    el1.append(value[1][self.index, :])
                labels.append(parm.name())


        legend = self.show_legend and len(labels) > 0
        if self.polar:
            plot(el0, self.fig, sub="211", labels=labels, show_legend=legend, title="Amplitude")
            plot(el1, self.fig, clf=False, sub="212", stack=True, scatter=True, labels=labels, show_legend=legend, title="Phase", ylabel="Angle (rad)")
        else:
            plot(el0, self.fig, sub="211", labels=labels, show_legend=legend, title="Real")
            plot(el1, self.fig, clf=False, sub="212", labels=labels, show_legend=legend, title="Imaginary")

        self.canvas.draw()

    def handle_spinner(self, index):
        self.index = index
        self.plot()

    def handle_axis(self, axis):
        if axis != self.axis:
            self.axis = axis
            self.spinner.setRange(0, self.shape[1 - self.axis] - 1)
            self.spinner.setValue(0)
            self.plot()

    def handle_legend(self, state):
        self.show_legend = (state == 2)
        self.plot()

    def handle_unwrap(self, state):
        self.unwrap_phase = (state == 2)
        self.plot()

    def handle_polar(self, state):
        self.polar = (state == 2)
        self.plot()

class MainWindow(QFrame):
    def __init__(self, db):
        QFrame.__init__(self)
        self.db = db

#        self.setWindowTitle("parmdbplot")

        layout = QVBoxLayout()

        self.list = QListWidget()
        self.list.setSelectionMode(QAbstractItemView.ExtendedSelection)
        layout.addWidget(self.list, 1)

        self.useResolution = True
        checkResolution = QCheckBox("Use resolution")
        checkResolution.setChecked(True)
        self.connect(checkResolution, SIGNAL('stateChanged(int)'), self.handle_resolution)

        self.resolution = [QLineEdit(), QLineEdit()]
#        validator = QDoubleValidator(self.resolution[0])
#        validator.setRange(1.0, 2.0)
#        self.resolution[0].setValidator(validator)
        self.resolution[0].setAlignment(Qt.AlignRight)
        self.resolution[1].setAlignment(Qt.AlignRight)

        hbox = QHBoxLayout()
        hbox.addWidget(checkResolution)
        hbox.addWidget(self.resolution[0])
        hbox.addWidget(QLabel("Hz"))
        hbox.addWidget(self.resolution[1])
        hbox.addWidget(QLabel("s"))
        layout.addLayout(hbox)

        self.button = QPushButton("Plot")
        layout.addWidget(self.button)
        self.connect(self.button, SIGNAL('clicked()'), self.handle_plot)

        self.button = QPushButton("Close all figures")
        layout.addWidget(self.button)
        self.connect(self.button, SIGNAL('clicked()'), self.handle_close)

        self.setLayout(layout)

        self.figures = []
        self.parms = []
        self.populate()

    def populate(self):
        for parm in self.db.getNames():
            split = parm.split(":")

            if contains(split, "Real") or contains(split, "Imag"):
                if contains(split, "Real"):
                    idx = split.index("Real")
                    split[idx] = "Imag"
                    elements = [parm, ":".join(split)]
                else:
                    idx = split.index("Imag")
                    split[idx] = "Real"
                    elements = [":".join(split), parm]

                split.pop(idx)
                name = ":".join(split)

                found = False
                for i in range(len(self.parms)):
                    if self.parms[i].name() == name and not self.parms[i].isPolar():
                        found = True
                        break

                if not found:
                    self.parms.append(Parm(self.db, name, elements))
            elif contains(split, "Ampl") or contains(split, "Phase"):
                if contains(split, "Ampl"):
                    idx = split.index("Ampl")
                    split[idx] = "Phase"
                    elements = [parm, ":".join(split)]
                else:
                    idx = split.index("Phase")
                    split[idx] = "Ampl"
                    elements = [":".join(split), parm]

                split.pop(idx)
                name = ":".join(split)

                found = False
                for i in range(len(self.parms)):
                    if self.parms[i].name() == name and self.parms[i].isPolar():
                        found = True
                        break

                if not found:
                    self.parms.append(Parm(self.db, name, elements, True))
            else:
                self.parms.append(Parm(self.db, name))

        self.parms = [parm for parm in self.parms if not parm.empty()]
        self.parms.sort(cmp=lambda x, y: cmp(x.name(), y.name()))

        domain = common_domain(self.parms)
        if not domain is None:
            self.resolution[0].setText("%.6f" % ((domain[1] - domain[0]) / 100.0))
            self.resolution[1].setText("%.6f" % ((domain[3] - domain[2]) / 100.0))

        for parm in self.parms:
            name = parm.name()
            if parm.isPolar():
                name = "%s (polar)" % name

            QListWidgetItem(name, self.list)

    def handle_resolution(self, state):
        self.useResolution = (state == 2)

    def handle_plot(self):
        parms = []
        tmp = self.list.selectedItems()
        tmp.sort()
        for item in tmp:
            idx = self.list.row(item)
            parms.append(copy.copy(self.parms[idx]))

        resolution = None
        domain = common_domain(parms)

        if domain is not None and self.useResolution:
            resolution = [parseFloat(self.resolution[0].text(), 1.0, domain[1] - domain[0]),
                parseFloat(self.resolution[1].text(), 1.0, domain[3] - domain[2])]

        self.figures.append(PlotWindow(parms, resolution))
        self.figures[-1].show()

    def handle_close(self):
        for fig in self.figures:
            fig.close()

if __name__ == "__main__":
    if len(sys.argv) <= 1 or sys.argv[1] == "--help":
        print "usage: parmdbplot.py <parmdb>"
        sys.exit(1)

    db = parmdb.parmdb(sys.argv[1])

    app = QApplication(sys.argv)
    window = MainWindow(db)
    window.show()

#    app.connect(app, SIGNAL('lastWindowClosed()'), app, SLOT('quit()'))
    app.exec_()
