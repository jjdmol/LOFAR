#!/usr/bin/python

# Copyright (C) 2012-2015    ASTRON (Netherlands Institute for Radio Astronomy)
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.

# $Id$

# TODO: add comments to methods
# TODO: code cleanup
# TODO: scraper should be able to process each directory more than once. Requires changes in store.py

import subprocess
import logging
import time
import datetime
import sys
import os
import os.path
import threading
import multiprocessing
from ltastorageoverview import store
from ltastorageoverview.utils import humanreadablesize
from random import random

#logging.basicConfig(filename='scraper.' + time.strftime("%Y-%m-%d") + '.log', level=logging.DEBUG, format="%(asctime)-15s %(levelname)s %(message)s")
logging.basicConfig(level=logging.DEBUG, format="%(asctime)-15s %(levelname)s %(message)s")
logger = logging.getLogger()


class FileInfo:
    '''Simple struct to hold filename and size'''
    def __init__(self, filename, size, created_at):
        '''
        Parameters
        ----------
        filename : string
        size : int
        '''
        self.filename = filename
        self.size = size
        self.created_at = created_at

    def __str__(self):
        return self.filename + " " + humanreadablesize(self.size) + " " + str(self.created_at)

class SrmlsException(Exception):
    def __init__(self, command, exitcode, stdout, stderr):
        self.command = command
        self.exitcode = exitcode
        self.stdout = stdout
        self.stderr = stderr

    def __str__(self):
        return "%s failed with code %d.\nstdout: %s\nstderr: %s" % \
                (self.command, self.exitcode, self.stdout, self.stderr)

class ParseException(Exception):
    def __init__(self, message):
        self.message = message

    def __str__(self):
        return self.message


class Location:
    '''A Location is a directory at a storage site which can be queried with getResult()'''
    def __init__(self, srmurl, directory):
        '''
        Parameters
        ----------
        srmurl : string
            the srm url of the storage site. for example: srm://srm.grid.sara.nl:8443
        directory : int
            a directory at the storage site. for example: /pnfs/grid.sara.nl/data/lofar/storage
        '''
        self.srmurl = srmurl
        self.directory = directory

    def path(self):
        '''returns the full path srmurl + directory'''
        return self.srmurl + self.directory

    def isRoot(self):
        '''is this a root directory?'''
        return self.directory == '/'

    def parentDir(self):
        '''returns parent directory path'''
        if self.isRoot():
            return '/'
        stripped = self.directory.rstrip('/')
        ridx = stripped.rindex('/')
        if ridx == 0:
            return '/'
        return stripped[:ridx]

    def parentLocation(self):
        '''returns a Location object for the parent directory'''
        return Location(self.srmurl, self.parentDir())

    def __str__(self):
        '''returns the full path'''
        return self.path()

    def getResult(self, offset=0):
        '''Returns LocationResult with the subdirectries and files in at this location'''
        foundFiles = []
        foundDirectories = []

        logger.info("Scanning %s", self.path())

        # the core command: do an srmls call and parse the results
        # srmls can only yield max 900 items in a result, hence we can recurse for the next 900 by using the offset
        cmd = ["bash", "-c", "source %s;srmls -l -count=900 -offset=%d %s%s" % ('/globalhome/ingest/service/bin/init.sh', offset, self.srmurl, self.directory)]
        # logger.debug(' '.join(cmd))
        p = subprocess.Popen(cmd, stdin=open('/dev/null'), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        logs = p.communicate()
        # logger.debug('Shell command for %s exited with code %s' % (self.path(), p.returncode))
        loglines = logs[0].split('\n')

        # parse logs from succesfull command
        if p.returncode == 0 and len(loglines) > 1:
            entries = []
            entry = []

            for line in loglines:
                entry.append(line)

                if 'Type:' in line:
                    entries.append(entry)
                    entry = []

            for lines in entries:
                if len(lines) < 2:
                    continue
                pathLine = lines[0].strip()
                pathLineItems = [x.strip() for x in pathLine.split()]
                entryType = lines[-1].strip().split('Type:')[-1].strip()

                if len(pathLineItems) < 2:
                    raise ParseException("path line shorter than expected: %s" % pathLine)

                if entryType.lower() == 'directory':
                    dirname = pathLineItems[1]
                    if dirname.rstrip('/') == self.directory.rstrip('/'):
                        # skip current directory
                        continue

                    if len(dirname) < 1 or not dirname[0] == '/':
                        raise ParseException("Could not parse dirname from line: %s\nloglines:\n%s"
                            % (pathLineItems[1], logs[0]))

                    foundDirectories.append(Location(self.srmurl, dirname))
                elif entryType.lower() == 'file':
                    try:
                        filesize = int(pathLineItems[0])
                        filename = pathLineItems[1]
                        timestamplines = [x for x in lines if 'ed at:' in x]
                        timestampline = None
                        for line in timestamplines:
                            if 'created' in line:
                                timestampline = line
                                break
                            timestampline = line
                        timestamppart = timestampline.split('at:')[1].strip()
                        timestamp = datetime.datetime.strptime(timestamppart + ' UTC', '%Y/%m/%d %H:%M:%S %Z')
                        foundFiles.append(FileInfo(filename, filesize, timestamp))
                    except Exception as e:
                        raise ParseException("Could not parse fileproperies:\n%s\nloglines:\n%s"
                            % (str(e), logs[0]))
                else:
                    logger.error("Unknown type: %s" % entryType)

            # recurse and ask for more files if we hit the 900 line limit
            if len(entries) >= 900:
                logger.debug('There are more than 900 lines in the results')
                extraResult = self.getResult(offset + 900)
                logger.debug('extraResult %s' % str(extraResult))
                foundDirectories += extraResult.subDirectories
                foundFiles += extraResult.files
        else:
            raise SrmlsException(' '.join(cmd), p.returncode, logs[0], logs[1])

        return LocationResult(self, foundDirectories, foundFiles)


class LocationResult:
    '''Holds the query result for a Location: a list of subDirectories and/or a list of files'''
    def __init__(self, location, subDirectories = None, files = None):
        '''
        Parameters
        ----------
        location : Location
            For which location this result was generated. (i.e. it is the parent of the subdirectories)

        subDirectories : [Location]
            A list of subdirectories

        files : [FileInfo]
            A list of files in this location
        '''
        self.location = location
        self.subDirectories = subDirectories if subDirectories else []
        self.files = files if files else []

    def __str__(self):
        return "LocationResult: path=%s # subdirs=%d # files=%d totalFileSizeOfDir=%s" % (self.location.path(), self.nrOfSubDirs(), self.nrOfFiles(), humanreadablesize(self.totalFileSizeOfDir()))

    def nrOfSubDirs(self):
        return len(self.subDirectories)

    def nrOfFiles(self):
        return len(self.files)

    def totalFileSizeOfDir(self):
        return sum([fileinfo.size for fileinfo in self.files])


# our lock for safe guarding locations and results
# which will be queried in parallel
lock = threading.Lock()

class ResultGetterThread(threading.Thread):
    '''Helper class to query Locations asynchronously for results.
    Gets the result for the first Location in the locations deque and appends it to the results deque
    Appends the subdirectory Locations at the end of the locations deque for later processing'''
    def __init__(self, db, dir_id):
        threading.Thread.__init__(self)
        self.daemon = True
        self.db = db
        self.dir_id = dir_id

    def run(self):
        '''A single location is pop\'ed from the locations deque and the results are queried.
        Resulting subdirectories are appended to the locations deque'''
        try:
            with lock:
                dir = self.db.directory(self.dir_id)

                if not dir:
                    return

                dir_id = dir[0]
                dir_name = dir[1]
                self.db.updateDirectoryLastVisitTime(dir_id, datetime.datetime.utcnow())

                site_id = dir[2]
                site = self.db.site(site_id)
                srm_url = site[2]

            location = Location(srm_url, dir_name)

            try:
                # get results... long blocking
                result = location.getResult()
                logger.info(result)

                with lock:
                    self.db.insertFileInfos([(file.filename, file.size, file.created_at, dir_id) for file in result.files])

                    # skip empty nikhef dirs
                    filteredSubDirectories = [loc for loc in result.subDirectories
                                            if not ('nikhef' in loc.srmurl and 'generated' in loc.directory) ]

                    # filteredSubDirectories = [loc for loc in filteredSubDirectories
                    #                        if not 'lc3_007' in loc.directory ]

                    subDirectoryNames = [loc.directory for loc in filteredSubDirectories]

                    if subDirectoryNames:
                        self.db.insertSubDirectories(subDirectoryNames, dir_id,
                                                    datetime.datetime.utcnow() - datetime.timedelta(days=1000))

            except (SrmlsException, ParseException) as e:
                logger.error('Error while scanning %s\n%s' % (location.path(), str(e)))

                logger.info('Rescheduling %s for new visit.' % (location.path(),))
                self.db.updateDirectoryLastVisitTime(self.dir_id, datetime.datetime.utcnow() - datetime.timedelta(days=1000))

        except Exception as e:
            logger.error(str(e))

            logger.info('Rescheduling dir_id %d for new visit.' % (self.dir_id,))
            self.db.updateDirectoryLastVisitTime(self.dir_id, datetime.datetime.utcnow() - datetime.timedelta(days=1000))

def main(argv):
    '''the main function scanning all locations and gathering the results'''

    db = store.LTAStorageDb('/data2/ltastorageoverview.sqlite')

    if not db.sites():
        db.insertSite('target', 'srm://srm.target.rug.nl:8444')
        db.insertSite('nikhef', 'srm://tbn18.nikhef.nl:8446')
        db.insertSite('sara', 'srm://srm.grid.sara.nl:8443')
        db.insertSite('juelich', 'srm://lofar-srm.fz-juelich.de:8443')

        db.insertRootDirectory('target', '/lofar/ops')
        db.insertRootDirectory('target', '/lofar/ops/disk')
        db.insertRootDirectory('nikhef', '/dpm/nikhef.nl/home/lofar')
        db.insertRootDirectory('sara', '/pnfs/grid.sara.nl/data/lofar/ops')
        db.insertRootDirectory('sara', '/pnfs/grid.sara.nl/data/lofar/user')
        db.insertRootDirectory('sara', '/pnfs/grid.sara.nl/data/lofar/software')
        db.insertRootDirectory('sara', '/pnfs/grid.sara.nl/data/lofar/storage')
        db.insertRootDirectory('sara', '/pnfs/grid.sara.nl/data/lofar/pulsar')
        db.insertRootDirectory('juelich', '/pnfs/fz-juelich.de/data/lofar/ops')

        for dir_id in [x[0] for x in db.rootDirectories()]:
            db.updateDirectoryLastVisitTime(dir_id, datetime.datetime.utcnow() - datetime.timedelta(days=1000))

    # for each site we want one or more ResultGetterThreads
    # so make a dict with a list per site based on the locations
    getters = dict([(site[1],[]) for site in db.sites()])

    # some helper functions
    def numLocationsInQueues():
        '''returns the total number of locations in the queues'''
        return db.numDirectoriesNotVisitedSince(datetime.datetime.utcnow() - datetime.timedelta(days=1))

    def totalNumGetters():
        '''returns the total number of parallel running ResultGetterThreads'''
        return sum([len(v) for v in getters.values()])

    # only enter main loop if there is anything to process
    if numLocationsInQueues() > 0:

        # the main loop
        # loop over the locations and spawn ResultGetterThreads to get the results parallel
        # use load balancing over the different sites and with respect to queue lengths
        # do not overload this host system
        while numLocationsInQueues() > 0 or totalNumGetters() > 0:

            # get rid of old finished ResultGetterThreads
            finishedGetters = dict([(site_name, [getter for getter in getterList if not getter.isAlive()]) for site_name, getterList in getters.items()])
            for site_name,finishedGetterList in finishedGetters.items():
                for finishedGetter in finishedGetterList:
                    getters[site_name].remove(finishedGetter)

            # spawn new ResultGetterThreads
            # do not overload this host system
            while numLocationsInQueues() > 0 and (totalNumGetters() <= 4 or
                                                  (os.getloadavg()[0] < 3*multiprocessing.cpu_count() and
                                                  totalNumGetters() < 2.5*multiprocessing.cpu_count())):

                with lock:
                    sitesStats = db.visitStats(datetime.datetime.utcnow() - datetime.timedelta(days=1))

                for site_name, site_stats in sitesStats.items():
                    numGetters = len(getters[site_name])
                    queue_length = site_stats['queue_length']
                    weight = float(queue_length) / float(20 * (numGetters + 1))
                    if numGetters == 0 and queue_length > 0:
                        weight = 1e6 # make getterless sites extra important, so each site keeps flowing
                    site_stats['# get'] = numGetters
                    site_stats['weight'] = weight

                totalWeight = sum([site_stats['weight'] for site_stats in sitesStats.values()])

                #logger.debug("siteStats:\n%s" % str('\n'.join([str((k, v)) for k, v in sitesStats.items()])))

                # now pick a random site using the weights
                chosen_site_name = None
                cumul = 0.0
                r = random()
                for site_name,site_stats in sitesStats.items():
                    ratio = site_stats['weight']/totalWeight
                    cumul += ratio

                    if r <= cumul and site_stats['queue_length'] > 0:
                        chosen_site_name = site_name
                        break

                if not chosen_site_name:
                    break

                chosen_dir_id = sitesStats[chosen_site_name]['least_recent_visited_dir_id']

                # make and start a new ResultGetterThread the location deque of the chosen site
                newGetter = ResultGetterThread(db, chosen_dir_id)
                newGetter.start()
                getters[chosen_site_name].append(newGetter)

                logger.info('numLocationsInQueues=%d totalNumGetters=%d' % (numLocationsInQueues(), totalNumGetters()))

                # small sleep between starting multiple getters
                time.sleep(0.25)

            # sleep before main loop next iteration
            # to wait for some results
            # and some getters to finis
            time.sleep(1)

        # all locations were processed

if __name__ == "__main__":
    main(sys.argv[1:])

