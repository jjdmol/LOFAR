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

def main(sources, name):
    results = []
    for s in sources:
        result = os.popen('ssh %s "ls %s/%s*"' % (s[0], s[1], name))
        lines  = result.readlines()
        for l in lines:
            if l.find(".MS"):
                if l.find(".cksum") < 0:
                    results.append(s[0] + ':' + s[1] + '/' + name + '/' + l.rstrip())
    return results

if __name__ == "__main__":
    config = {'nrListNodes':4, 'nrLifsNodes':12}
    parser = OptionParser(usage='prog -n<measurement name>')
    parser.add_option("-n", "--name", default="", help="Name of remote host to query")
    options, args = parser.parse_args()
    sources = []
    for i in range(config['nrListNodes']):
        name  = "list%03d" % (i+1)
        sources.append((name,"/data"))
        sources.append((name,"/san/LOFAR"))
    for i in range(config['nrLifsNodes']):
        name  = "lifs%03d" % (i+1)
        sources.append((name,"/data"))

    result = main(sources, options.name)
    print result
    sys.exit(1)