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

# $Id$

# TODO: add comment to methods
# TODO: code cleanup
# TODO: where to store the sqlite database?

import sys
import os
import os.path
from datetime import datetime
from flask import Flask
from flask import render_template
from flask import json
from ltastorageoverview import store
from lofar.common.util import humanreadablesize
from lofar.common.datetimeutils import monthRanges

app = Flask('LTA storage overview')
app.config.root_path = os.path.dirname(__file__)
db = None

@app.route('/')
@app.route('/index.html')
def index():
    # TODO: serve html first, and let client request data via ajax
    usages = {}

    sites = db.sites()
    sites2 = [x for x in sites if x[1] != 'nikhef']
    sites = [sites2[0], sites2[2], sites2[1]]

    total = 0.0
    numFiles = 0L
    for site in sites:
        site_usage = float(db.totalFileSizeInSite(site[0]))
        usages[site[1]] = site_usage
        total += site_usage
        numFiles += db.numFilesInSite(site[0])

    storagesitedata='[' + ', '.join(['''{name: "%s %s", y: %.1f}''' % (site[1], humanreadablesize(usages[site[1]]), 100.0*usages[site[1]]/total) for site in sites]) + ']'

    min_date, max_date = db.datetimeRangeOfFilesInTree()
    min_date = datetime(2012, 1, 1)
    month_ranges = monthRanges(min_date, max_date)

    # convert end-of-month timestamps to milliseconds since epoch
    epoch = datetime.utcfromtimestamp(0)
    datestamps=[('%d' % ((x[1] - epoch).total_seconds()*1000,)) for x in month_ranges]

    usage_per_month_series='['
    deltas_per_month_series='['
    for site in sites:
        cumulatives = [db.totalFileSizeInSite(site[0], to_date=mr[1]) for mr in month_ranges]

        data = ', '.join(['[%s, %s]' % (x[0], str(x[1])) for x in zip(datestamps, cumulatives)])
        usage_per_month_series += '''{name: '%s', data: [%s]},\n''' % (site[1], data)

        deltas = [0]
        for i in range(1, len(cumulatives)):
            delta = cumulatives[i] - cumulatives[i-1]
            deltas.append(delta)

        data = ', '.join(['[%s, %s]' % (x[0], str(x[1])) for x in zip(datestamps, deltas)])
        deltas_per_month_series += '''{name: '%s', data: [%s]},\n''' % (site[1], data)


    usage_per_month_series+=']'
    deltas_per_month_series+=']'

    return render_template('index.html',
                           title='LTA storage overview',
                           storagesitetitle='LTA Storage Site Usage',
                           storagesitesubtitle='Total: %s #dataproducts: %s' % (humanreadablesize(total, 'B', 1000), humanreadablesize(numFiles, '', 1000)),
                           storagesitedata=storagesitedata,
                           usage_per_month_series=usage_per_month_series,
                           deltas_per_month_series=deltas_per_month_series,
                           data_gathered_timestamp=db.mostRecentVisitDate().strftime('%Y/%m/%d %H:%M:%S'))

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
            usage = long(db.totalFileSizeInTree(rootDir[0]))
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
    dbpath = argv[0] if argv else 'ltastorageoverview.sqlite'

    if not os.path.exists(dbpath):
        print 'No database file found at \'%s\'' % (dbpath,)
        sys.exit(-1)

    print 'Using database at \'%s\'' % (dbpath,)

    global db
    db = store.LTAStorageDb(dbpath)

    app.run(debug=True,host='0.0.0.0')

if __name__ == '__main__':
    main(sys.argv[1:])

