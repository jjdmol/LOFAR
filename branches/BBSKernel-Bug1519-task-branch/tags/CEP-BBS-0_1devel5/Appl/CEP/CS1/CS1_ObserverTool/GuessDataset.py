#!/usr/bin/env python
############################################################################
#    Copyright (C) 2008 by Adriaan Renting   #
#    renting@astron.nl   #
#                                                                          #
#    This program is free software; you can redistribute it and#or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
#    This program is distributed in the hope that it will be useful,       #
#    but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#    GNU General Public License for more details.                          #
#                                                                          #
#    You should have received a copy of the GNU General Public License     #
#    along with this program; if not, write to the                         #
#    Free Software Foundation, Inc.,                                       #
#    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
############################################################################

import sys
from optparse import OptionParser

def main(stations, bands, inttime, duration):
    if stations == "":
        sys.stdout.write("Give the number of (micro)stations [16]: ")
        stations = sys.stdin.readline()
    if len(stations) <= 1: stations = 16
    else: stations = int(stations)

    if bands == "":
        sys.stdout.write("Give the number of bands [36]: ")
        bands = sys.stdin.readline()
    if len(bands) <= 1: bands = 36
    else: bands = int(bands)

    if inttime == "":
        sys.stdout.write("Give the integration time [10]: ")
        inttime = sys.stdin.readline()
    if len(inttime) <= 1: inttime = 10
    else: inttime = int(inttime)

    if duration == "":
        sys.stdout.write("Give the duration of the measurement [1:23] hh:mm: ")
        duration = sys.stdin.readline()[:-1]
    if len(duration) <= 1: duration = "1:23"

    baselines = stations *(stations+1) /2
    return str(256*32*baselines * bands * (int(duration[:-3]) *60 + int(duration[-2:])) * 60/inttime)

if __name__ == "__main__":
    parser = OptionParser(usage='prog -r<remote host>')
    parser.add_option("-s", "--stations", default="", help="Number of stations")
    parser.add_option("-b", "--bands", default="", help="Nnumber of bands")
    parser.add_option("-i", "--integration_time", default="", help="Integration time of one sample")
    parser.add_option("-d", "--duration", default="", help="Total duration of measurement [hh:mm]")
    options, args = parser.parse_args()
    result = main(options.stations, options.bands, options.integration_time, options.duration)
    print "Total datasize is %s" % result
    sys.exit(result)