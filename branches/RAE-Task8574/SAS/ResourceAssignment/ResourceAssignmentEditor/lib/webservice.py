#!/usr/bin/python

# Copyright (C) 2012-2015  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

'''ResourceAssignementEditor webservice serves a interactive html5 website for
viewing and editing lofar resources.'''

import sys
from flask import Flask

'''The flask webservice app'''
app = Flask('ResourceAssignementEditor')


@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignementEditor's index page'''
    return "Hello World!"


def main(argv=None, debug=False):
    '''Start the webserver'''
    app.run(use_debugger=debug)

if __name__ == '__main__':
    main(sys.argv[1:], True)
