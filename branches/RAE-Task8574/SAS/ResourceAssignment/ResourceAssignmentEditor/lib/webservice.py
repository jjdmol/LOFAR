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

# $Id:

'''ResourceAssignementEditor webservice serves a interactive html5 website for
viewing and editing lofar resources.'''

import sys
from flask import Flask
from flask import render_template
from flask import url_for
import os

__root_path = os.path.dirname(os.path.abspath(__file__))
print '__root_path=%s' % __root_path

'''The flask webservice app'''
app = Flask('ResourceAssignementEditor',
            instance_path=__root_path,
            template_folder=os.path.join(__root_path, 'templates'),
            static_folder=os.path.join(__root_path, 'static'),
            instance_relative_config=True)

print 'app.template_folder= %s' % app.template_folder
print 'app.static_folder= %s' % app.static_folder

# Load the default configuration
app.config.from_object('resourceassignementeditor.config.default')

@app.route('/')
@app.route('/index.htm')
@app.route('/index.html')
def index():
    '''Serves the ResourceAssignementEditor's index page'''
    return render_template('index.html', title='Resource Assignement Editor')

def main(argv=None, debug=False):
    '''Start the webserver'''
    app.run(debug=debug, port=5001)

if __name__ == '__main__':
    main(sys.argv[1:], True)
