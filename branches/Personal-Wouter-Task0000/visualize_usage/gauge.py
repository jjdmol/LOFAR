"""

The Gauge widget draws a semi-circular gauge. You supply raw_limits,
shaded regions, names and the current value, and invoke it like this:


    from pylab import figure, show

    raw_value = -4.0
    raw_limits = [-1.0,1.0,1,0.1]
    raw_zones = [[-1.0,0.0,'r'],[0.0,0.5,'y'],[0.5,1.0,'g']]
    attribute_name = "Rx MOS (24h)"
  

    graph_height = 1.6
    graph_width  = 2.4
    fig_height   = graph_height
    fig_width    = graph_width

    fig = figure(figsize=(fig_width, fig_height ))
  
    rect = [(0.0/fig_width), (0.2/fig_height),
            (graph_width/fig_width), (graph_height/fig_height)]
  
    gauge = Gauge(fig, rect,
               xlim=( -0.1, graph_width+0.1 ),
               ylim=( -0.4, graph_height+0.1 ),
               xticks=[],
               yticks=[],
               )
    gauge.set_axis_off()
    fig.add_axes(gauge)

    show()
    
    source:
    http://osdir.com/ml/python.matplotlib.devel/2005-07/msg00026.html
    unknown author unknown copyright.
    
    The original code was incorrect and would not run
    Posted on a PUBLIC mailing list for a gnu software package.
    
    Astron
    2013
    Wouter Klijn (klijn@astron.nl)
"""
import matplotlib
matplotlib.use("Agg")  # Do not display the figures in x-window
from matplotlib.figure import Figure
from matplotlib.axes import Axes
import math
import types

from math import pi


class Gauge(Axes):
    def __init__(self, raw_value, raw_limits, raw_zones, attribute_name, field_names, file_name, resolution, x_length, y_length, *args, **kwargs):
        Axes.__init__(self, *args, **kwargs)

        #Perform Checking
        if(raw_limits[0] == raw_limits[1]):
            raise ValueError('identical_raw_limits_exception: %s' % raw_limits)
        if(raw_limits[1] > raw_limits[0]):
            self.graph_positive = True
        else:    #Swap the raw_limits around
            self.graph_positive = False
            raw_limits[0], raw_limits[1] = raw_limits[1] = raw_limits[0]

        #There must be an integer number of minor ticks for each major tick
        if not(((raw_limits[2] / raw_limits[3]) % 1.0) * raw_limits[3] == 0):
            raise ValueError('bad_tick_spacing_exception')

        if(raw_limits[2] <= 0 or
            raw_limits[3] <= 0 or
            raw_limits[2] < raw_limits[3] or
            raw_limits[3] > abs(raw_limits[1] - raw_limits[0])):
            raise ValueError('bad_raw_limits_exception:%s' % raw_limits)
        for zone in raw_zones:
            if(zone[0] > zone[1]):    #Swap the zones so zone[1] > zone[0]
                zone[0], zone[1] = zone[1], zone[0]
            if(zone[1] < raw_limits[0] or zone[0] > raw_limits[1]):
                raise ValueError('bad_zone_exception' % zone)
            if(zone[0] < raw_limits[0]):
                zone[0] = raw_limits[0]
            if(zone[1] > raw_limits[1]):
                zone[1] = raw_limits[1]

        #Stuff all of the variables into self.
        self.raw_value = raw_value
        self.raw_limits = raw_limits
        self.raw_zones = raw_zones
        self.attribute_name = attribute_name
        self.field_names = field_names
        self.file_name = file_name
        self.resolution = resolution
        self.x_length = x_length
        self.y_length = y_length


        #Draw the gauge
        for zone in raw_zones:
            self.draw_arch(zone, False)
        self.draw_arch(None, True)
        self.draw_ticks()
        self.draw_needle()
        self.draw_bounding_box()
        self.text(0.0, 0.2, self.attribute_name, size=10, va='bottom', ha='center')

        #The black dot
        p = self.plot([0.0], [0.0], '.', color='#000000')


    def draw_arch(self, zone, border):
        if(border):
            start = self.raw_limits[0]
            end = self.raw_limits[1]
        else:
            start = zone[0]
            end = zone[1]
            colour = zone[2]

        x_vect = []
        y_vect = []
        if(self.graph_positive):
            start_value = int(180 - (start - self.raw_limits[0]) * (180.0 / (self.raw_limits[1] - self.raw_limits[0])))
            end_value = int(180 - (end - self.raw_limits[0]) * (180.0 / (self.raw_limits[1] - self.raw_limits[0])))
        else:
            start_value = int((end - self.raw_limits[0]) * (180.0 / (self.raw_limits[1] - self.raw_limits[0])))
            end_value = int((start - self.raw_limits[0]) * (180.0 / (self.raw_limits[1] - self.raw_limits[0])))

        #Draw the arch
        theta = start_value
        radius = 0.85
        while (theta >= end_value):
            x_vect.append(radius * math.cos(theta * (pi / 180)))
            y_vect.append(radius * math.sin(theta * (pi / 180)))
            theta -= 1

        theta = end_value
        radius = 1.0
        while (theta <= start_value):
            x_vect.append(radius * math.cos(theta * (pi / 180)))
            y_vect.append(radius * math.sin(theta * (pi / 180)))
            theta += 1

        if(border):
            #Close the loop
            x_vect.append(-0.85)
            y_vect.append(0.0)

            p = self.plot(x_vect, y_vect, 'b-', color='black', linewidth=1.0)
        else:
            p = self.fill(x_vect, y_vect, colour, linewidth=0.0, alpha=0.4)


    def draw_needle(self):
        x_vect = []
        y_vect = []

        if self.raw_value == None:
            self.text(0.0, 0.4, "N/A", size=10, va='bottom', ha='center')
        else:
            self.text(0.0, 0.4, "%.2f" % self.raw_value, size=15, va='bottom', ha='center')

            #Clamp the value to the raw_limits
            if(self.raw_value < self.raw_limits[0]):
                self.raw_value = self.raw_limits[0]
            if(self.raw_value > self.raw_limits[1]):
                self.raw_value = self.raw_limits[1]

            theta = 0
            length = 0.95
            if(self.graph_positive):
                angle = 180.0 - (self.raw_value - self.raw_limits[0]) * (180.0 / abs(self.raw_limits[1] - self.raw_limits[0]))
            else:
                angle = (self.raw_value - self.raw_limits[0]) * (180.0 / abs(self.raw_limits[1] - self.raw_limits[0]))

            while (theta <= 270):
                x_vect.append(length * math.cos((theta + angle) * (pi / 180)))
                y_vect.append(length * math.sin((theta + angle) * (pi / 180)))
                length = 0.05
                theta += 90

            p = self.fill(x_vect, y_vect, 'b', alpha=0.4)



    def draw_ticks(self):
        if(self.graph_positive):
            angle = 180.0
        else:
            angle = 0.0
        i = 0
        j = self.raw_limits[0]

        while(i * self.raw_limits[3] + self.raw_limits[0] <= self.raw_limits[1]):
            x_vect = []
            y_vect = []
            if(i % (self.raw_limits[2] / self.raw_limits[3]) == 0):
                x_pos = 1.1 * math.cos(angle * (pi / 180.0))
                y_pos = 1.1 * math.sin(angle * (pi / 180.0))
                if(type(self.raw_limits[2]) is types.FloatType):
                    self.text(x_pos, y_pos, "%.2f" % j, size=10, va='center', ha='center', rotation=(angle - 90))
                else:
                    self.text(x_pos, y_pos, "%d" % int(j), size=10, va='center', ha='center', rotation=(angle - 90))
                tick_length = 0.15
                j += self.raw_limits[2]
            else:
                tick_length = 0.05
            i += 1
            x_vect.append(1.0 * math.cos(angle * (pi / 180.0)))
            x_vect.append((1.0 - tick_length) * math.cos(angle * (pi / 180.0)))
            y_vect.append(1.0 * math.sin(angle * (pi / 180.0)))
            y_vect.append((1.0 - tick_length) * math.sin(angle * (pi / 180.0)))
            p = self.plot(x_vect, y_vect, 'b-', linewidth=1, alpha=0.4, color="black")
            if(self.graph_positive):
                angle -= self.raw_limits[3] * (180.0 / abs(self.raw_limits[1] - self.raw_limits[0]))
            else:
                angle += self.raw_limits[3] * (180.0 / abs(self.raw_limits[1] - self.raw_limits[0]))
        if(i % (self.raw_limits[2] / self.raw_limits[3]) == 0):
            x_pos = 1.1 * math.cos(angle * (pi / 180.0))
            y_pos = 1.1 * math.sin(angle * (pi / 180.0))
            if(type(self.raw_limits[2]) is types.FloatType):
                self.text(x_pos, y_pos, "%.2f" % j, size=10, va='center', ha='center', rotation=(angle - 90))
            else:
                self.text(x_pos, y_pos, "%d" % int(j), size=10, va='center', ha='center', rotation=(angle - 90))


    def draw_bounding_box(self):
        x_vect = [
            self.x_length / 2,
            self.x_length / 2,
            - self.x_length / 2,
            - self.x_length / 2,
            self.x_length / 2,
            ]

        y_vect = [
            - 0.1,
            self.y_length,
            self.y_length,
            - 0.1,
            - 0.1,
            ]

        p = self.plot(x_vect, y_vect, 'r-', linewidth=0)

def make_widget(raw_value, raw_limits, raw_zones, attribute_name, field_names,
                file_name, resolution=72, size_multiplier=4, disp=False):
    from pylab import figure, show, savefig, xlim

    x_length = 1.4 * size_multiplier # Length of the Primary axis
    y_length = 1.6 * size_multiplier # Length of the Secondary axis

    fig_height = y_length
    fig_width = x_length
    fig = figure(figsize=(fig_width, fig_height))
    rect = [(0.0 / fig_width), (0.2 / fig_height), (x_length / fig_width), (y_length / fig_height)]
    gauge = Gauge(raw_value,
        raw_limits, raw_zones,
        attribute_name, field_names,
        file_name, resolution,
        x_length, y_length,
        fig, rect,
        xlim=(-x_length, x_length),
        ylim=(-0.2, y_length),
        xticks=[],
        yticks=[],
        )

    gauge.set_axis_off()
    fig.add_axes(gauge)

    xlimmits = gauge.get_xlim()
    gauge.set_xlim(xlimmits[0] , xlimmits[1])
    ylimmits = gauge.get_ylim()
    gauge.set_ylim(ylimmits[0] / size_multiplier, (ylimmits[1] * 0.8) / size_multiplier)

    fig.canvas.print_figure(file_name, dpi=resolution)
    if disp:
        show()



def create_and_save_gauge(value=50.0,
                          file_name="gauge.png",
                          attribute_name="Gauge title",
                          draw=False,
                          limits=[0.0, 80.0, 90.0, 100.0],
                          value_print_step=10,
                          axis_tick_step=5,
                          size_multiplier=2,
                          resolution=200):
    """
    Simple accessor function for creating an image with a gauge dail    
    """
    min_gauge_value = limits[0]
    green_limit = limits[1]
    yellow_limit = limits[2]
    max_gauge_value = limits[3]
    raw_limits = [min_gauge_value, max_gauge_value, value_print_step, axis_tick_step]
    raw_zones = [[min_gauge_value, green_limit, 'g'],
                 [green_limit, yellow_limit, 'y'],
                 [yellow_limit, max_gauge_value, 'r']]

    field_names = ['None', 'None', 'None']


    make_widget(value, raw_limits, raw_zones, attribute_name, field_names,
                file_name, resolution, size_multiplier, draw)


if __name__ == '__main__':
    file_name = 'gauge.png'
    attribute_name = "% CPU usage\n (5 Min. Av.)"
    value = 80.0
    create_and_save_gauge(value, file_name, attribute_name, draw=False)
