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
from flask import Flask
from flask import Config
from flask import render_template
from flask import json
import threading
from ltastorageoverview import store

app = Flask('LTA storage overview')
print str(app.config)
print __file__
print os.path.dirname(__file__)
app.config.root_path = os.path.dirname(__file__)
print
print str(app.config)
db = store.LTAStorageDb('../ltastorageoverview.sqlite')

def humanreadablesize(num, suffix='B'):
    """ converts the given size (number) to a human readable string in powers of 1024"""
    try:
        for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
            if abs(num) < 1024.0:
                return "%3.1f%s%s" % (num, unit, suffix)
            num /= 1024.0
        return "%.1f%s%s" % (num, 'Y', suffix)
    except TypeError:
        return str(num)

@app.route('/')
@app.route('/index.html')
def index():
    return render_template('index.html',
                           title='LTA storage overview',
                           storagesitedata='''[{name: "Microsoft Internet Explorer",y: 56.33}, {name: "Chrome",y: 24.03,sliced: true,selected: true}, {name: "Firefox",y: 10.38}, {name: "Safari",y: 4.77}, {name: "Opera",y: 0.91}, {name: "Proprietary or Undetectable",y: 0.2}]''')

@app.route('/rest/sites/')
def get_sites():
    sites = {'sites': [{'id': x[0], 'name': x[1], 'url': x[2]} for x in db.sites()]}
    return json.jsonify(sites)

@app.route('/rest/sites/<int:site_id>')
def get_site(site_id):
    site = db.site(site_id)
    site_dict = {'id': site[0], 'name': site[1], 'url': site[2]}
    return json.jsonify(site_dict)

@app.route('/rest/sites/usages')
def get_sites_usages():
    sites = {'sites_usages': [{'id': x[0],
                               'name': x[1]} for x in db.sites()]}

    for site in sites['sites_usages']:
        rootDirs = db.rootDirectoriesForSite(site['id'])

        site_usage = 0L
        for rootDir in rootDirs:
            usage = db.totalFileSizeInTree(rootDir[0])
            print site['name'] + " " + rootDir[1] + " " + str(usage)
            site_usage += usage
        site['usage'] = site_usage
        site['usage_hr'] = humanreadablesize(site_usage)

    return json.jsonify(sites)

@app.route('/rest/rootdirectories/',)
def get_rootDirectories():
    rootDirs = {'rootDirectories': [{'id': x[0], 'name': x[1], 'site_id': x[2], 'site_name': x[3]} for x in db.rootDirectories()]}
    return json.jsonify(rootDirs)

@app.route('/rest/directory/<int:dir_id>/subdirectories/',)
def get_directoryTree(dir_id):
    subDirsList = {'subdirectories': [{'id': x[0], 'name': x[1], 'parent_dir_id': x[2]} for x in db.subDirectories(dir_id, 1, False)]}
    return json.jsonify(subDirsList)

@app.route('/rest/directory/<int:dir_id>/files')
def get_filesInDirectory(dir_id):
    files = {'files': [{'id': x[0], 'name': x[1], 'size': x[2], 'creation_date': x[3]} for x in db.filesInDirectory(dir_id)]}
    return json.jsonify(files)


def main(argv):
    #db = store.LTAStorageDb(argv[0] if argv else 'ltastoragedb.sqlite')
    app.run(debug=True)

if __name__ == '__main__':
    main(sys.argv[1:])

