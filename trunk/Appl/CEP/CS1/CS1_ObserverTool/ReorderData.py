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
    for f in files: ## host:/path/file
        filename = os.path.basename(f) ##file
        if filename[0] == 'L': ##L2008_xxxxx_SBxx.MS
            number = int(filename[14:-3])
        else: ## SBxx.MS
            number = int(filename[2:-3])
        host = os.path.dirname(f).split(':')
        target = 'lifs%03d' % number
        result = os.system('ssh %s mkdir /data/%s' % (target, name))
        if result == 0 or result == 256:
            os.system('ssh %s scp -r %s %s:/data/%s' % (host, f, target, name))

if __name__ == "__main__":
    parser = OptionParser(usage='prog -f<files>, -n<name>')
    parser.add_option("-f", "--files", default="", help="Files to copy")
    parser.add_option("-n", "--name", default="", help="Name of dataset")
    options, args = parser.parse_args()
    result = main(options.files, options.name)
