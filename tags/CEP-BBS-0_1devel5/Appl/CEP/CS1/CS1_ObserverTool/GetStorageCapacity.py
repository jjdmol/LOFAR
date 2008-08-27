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

import os, sys
from optparse import OptionParser

def main(name, ldir):
    result = os.popen('ssh %s "df %s"' % (name, ldir))
    lines  = result.readlines()
    try:
        capacity = int(lines[1].split()[-2][:-1])
    except:
        capacity = -1
    return capacity

if __name__ == "__main__":
    parser = OptionParser(usage='prog -r<remote host>, -d<directory>')
    parser.add_option("-r", "--remote_host", default="", help="Name of remote host to query")
    parser.add_option("-d", "--directory", default="", help="Name of remote directory to query")
    options, args = parser.parse_args()
    result = main(options.remote_host, options.directory)
    print "The system %s is at %d%% capacity." % (options.remote_host, result)
    sys.exit(result)