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

def main(files, name):
    config = {'nrListNodes':4, 'nrLifsNodes':12}
    for f in files: ## host:/path/file
        filename  = os.path.basename(f) ##file
        start     = filename.find("SB")
        stop      = filename.find(".MS")
        if (start == -1 or stop == -1):
            continue
        number    = int(filename[start+2:stop])
        host, loc = os.path.dirname(f).split(':')
        target    = 'lifs%03d' % (number % config['nrLifsNodes'] + 1) # hardcoded number of lifs nodes!
        if host == target and name == loc:
            print "It's the same file:" + host + ':' + name
            continue
        result    = os.system('ssh %s "mkdir %s"' % (target, name))
        if result == 0 or result == 256:
            result = os.system('ssh -A %s "scp -r %s %s:%s"' % (host, loc + '/' + filename, target, name))
            if result: print "Error:" + str(result)
            else:
                result = os.system('ssh -A %s "rm -rf %s"' % (host, loc + '/' + filename))
                if result: print "Error:" + str(result)

if __name__ == "__main__":
    parser = OptionParser(usage='prog -f<files>, -n<name>')
    parser.add_option("-f", "--files", default="", help="Files to copy")
    parser.add_option("-n", "--name", default="", help="Name of dataset")
    options, args = parser.parse_args()
    result = main(options.files.split(','), options.name)
