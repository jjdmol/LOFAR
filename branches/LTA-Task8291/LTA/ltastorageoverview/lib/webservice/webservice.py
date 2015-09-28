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

import sys
import os
import os.path
import flask
import threading
from ltastorageoverview import store

app = flask.Flask('LTA storage overview')
db = None

def run():
    app.run()

def runThreaded():
    serviceThread = threading.Thread(target=run)
    serviceThread.daemon = True
    serviceThread.start()

@app.route('/')
def hello_world():
    return 'Hello World!'

@app.route('/json/sites/')
def get_sites():
    sites = {'sites' : [ {'id':x[0], 'name':x[1], 'url':x[2] } for x in db.sites()]}
    return flask.json.jsonify(sites)

@app.route('/json/sites/<int:site_id>')
def get_site(site_id):
    site = db.site.site(site_id)
    site_dict = {'id':site[0], 'name':site[1], 'url':site[2] }
    return flask.json.jsonify(site_dict)

def main(argv):
    db = store.LTAStorageDb(argv[0] if argv else 'ltastoragedb.sqlite')
    run()

if __name__ == '__main__':
    main(sys.argv[1:])

