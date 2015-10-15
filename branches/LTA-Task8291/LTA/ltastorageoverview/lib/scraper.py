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

import subprocess
import logging
import cPickle as pickle
import time
import datetime
import sys
import os
import os.path
import tempfile
import shutil
import threading
import multiprocessing
from random import random
from collections import deque

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


logging.basicConfig(filename='get_srm2_' + time.strftime("%Y-%m-%d_%HH%M") + '.log', level=logging.DEBUG, format="%(asctime)-15s %(levelname)s %(message)s")
#logging.basicConfig(level=logging.DEBUG, format="%(asctime)-15s %(levelname)s %(message)s")
logger = logging.getLogger()


class FileInfo:
    '''Simple struct to hold filename and size'''
    def __init__(self, filename, size, modified_at):
        '''
        Parameters
        ----------
        filename : string
        size : int
        '''
        self.filename = filename
        self.size = size
        self.modified_at = modified_at

    def __str__(self):
        return self.filename + " " + humanreadablesize(self.size) + " " + str(self.modified_at)


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
        if p.returncode == 0 and len(loglines) > 1 and "FAILURE" not in logs[0]:
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
                    logger.error("path line shorter than expected: %s" % pathLine)
                    continue

                if entryType.lower() == 'directory':
                    dirname = pathLineItems[1]
                    if dirname == (self.directory + '/') or dirname == self.directory: # skip current directory
                        continue
                    else:
                        foundDirectories.append(Location(self.srmurl, dirname))
                elif entryType.lower() == 'file':
                    try:
                        filesize = int(pathLineItems[0])
                        filename = pathLineItems[1]
                        timestampline = [x for x in lines if 'modified at' in x][0]
                        timestamppart = timestampline.split('at:')[1].strip()
                        timestamp = datetime.datetime.strptime(timestamppart + ' UTC', '%Y/%m/%d %H:%M:%S %Z')
                        foundFiles.append(FileInfo(filename, filesize, timestamp))
                    except Exception as e:
                        logger.exception(str(e))
                else:
                    logger.error("Unknown type: %s" % entryType)

        # recurse and ask for more files if we hit the 900 line limit
        if len(entries) >= 900:
            logger.debug('There are more than 900 lines in the results')
            extraResult = self.getResult(offset + 900)
            logger.debug('extraResult %s' % str(extraResult))
            foundDirectories += extraResult.subDirectories
            foundFiles += extraResult.files

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
    def __init__(self, locations, results):
        '''
        Parameters
        ----------
        locations : deque([Location])

        results : deque([LocationResult])
        '''
        threading.Thread.__init__(self)
        self.daemon = True
        self.locations = locations
        self.results = results

    def run(self):
        '''A single location is pop\'ed from the locations deque and the results are queried.
        Resulting subdirectories are appended to the locations deque'''
        try:
            with lock:
                location = self.locations.popleft()

            # get results... long blocking
            result = location.getResult()
            logger.info(result)

            with lock:
                self.results.append(result)
                for subdir in result.subDirectories:
                    self.locations.append(subdir)

        except IndexError:
            pass
        except Exception as e:
            logger.error(str(e))


class LocationResultTreeNode:
    '''Helper class to order results in a tree structure,
    so we can get totals (files and sizes) of the whole (or part of) the tree.'''
    def __init__(self, locationResult, parent=None, children=None):
        '''
        Parameters
        ----------
        locationResult : LocationResult
            the locationResult for this tree node

        parent : LocationResultTreeNode
            the parent LocationResultTreeNode of this tree node

        children : [LocationResultTreeNode]
            the children LocationResultTreeNode's of this tree node
        '''
        self.locationResult = locationResult
        self.parent = parent
        self.children = children if children else []

    def __str__(self):
        return "LocationResultTreeNode: loc=%s total # files=%d in total # subDirs=%d with a total size of %s" % (self.locationResult.location.path(), self.totalNrOfFilesInDirAndSubDirs(), self.totalNrOfSubDirs(), humanreadablesize(self.totalFileSizeOfDirAndSubDirs()))

    def totalNrOfFilesInDirAndSubDirs(self):
        '''returns the total number of files in this dir and the all subdirs of the tree'''
        return self.locationResult.nrOfFiles() + sum([child.totalNrOfFilesInDirAndSubDirs() for child in self.children])

    def totalNrOfSubDirs(self):
        '''returns the total number of subdirs in this dir and the all subdirs of the tree'''
        return self.locationResult.nrOfSubDirs() + sum([child.totalNrOfSubDirs() for child in self.children])

    def totalFileSizeOfDirAndSubDirs(self):
        '''returns the total filesize in this dir and the all subdirs of the tree'''
        return self.locationResult.totalFileSizeOfDir() + sum([child.totalFileSizeOfDirAndSubDirs() for child in self.children])

    def treeString(self, level=0, maxLevel=sys.maxint):
        '''returns a string tree representation of this node and its subtree
        uses recursion to indent each sublevel.
        can be limited to maxLevel levels deep'''
        return "> {path} # files={numFiles} fileSizeOfDir={fileSize} # filesInTree={numFilesInTree} fileSizeOfTree={fileSizeOfTree}{tree}".format(
            path=self.locationResult.location.path(),
            numFiles=self.locationResult.nrOfFiles(),
            fileSize=humanreadablesize(self.locationResult.totalFileSizeOfDir()),
            numFilesInTree=self.totalNrOfFilesInDirAndSubDirs(),
            fileSizeOfTree=humanreadablesize(self.totalFileSizeOfDirAndSubDirs()),
            tree='\n' + '\n'.join([(level+1)*'    ' + child.treeString(level+1, maxLevel) for child in self.children]) if self.children and level < maxLevel else "")


def readPickle(path):
    '''read and return the pickled (locations,results) tuple from a file'''
    with open(path, 'rb') as pickleFile:
        return pickle.load(pickleFile)


def writePickle(path, locations,results):
    ''' Write the locations and results to a file.
    can be used for intermediate storage in case the script is interrupted or killed
    Parameters
    ----------
    path : string
        name of the file in which to save the pickled data

    locations : deque([Location])
        the deque of Location which still need to be queried

    results : deque([c])
        the deque of LocationResults which we already found
        '''
    try:
        # since pickling can take a long time
        # and the program can be interupted while pickling
        # write pickle to tmp file and move it to the given path later
        tmpfile = os.path.join(tempfile.gettempdir(), "srm.pickle")
        with open(tmpfile , 'wb') as pickleFile:
                pickle.dump((locations,results), pickleFile, 2)

        if not os.path.isabs(path):
            path = os.path.join(os.getcwd(), path)

        # move tmpfile to given path, but make intermediate backup
        if os.path.exists(path):
            shutil.move(path, path + ".old")

        # actual move
        shutil.move(tmpfile, path)

        # remove intermediate backup
        if os.path.exists(path + ".old"):
            os.remove(path + ".old")

    except Exception as e:
        logger.error(e)


def main(argv):
    '''the main function scanning all locations and gathering the results'''

    # results are stored in a deque which is thread safe
    results = deque()

    # the big dict containing per site a deque of Locations which need to be scanned
    # the deque's in this dict are filled and emptied using parrellel running ResultGetterThreads
    locations = { 'target' : deque( [    Location("srm://srm.target.rug.nl:8444", "/lofar/ops/disk") ] ) ,
                                'nikhef' : deque( [    Location("srm://tbn18.nikhef.nl:8446", "/dpm/nikhef.nl/home/lofar") ] ),
                                'sara' : deque( [    Location("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/ops"),
                                                                Location("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/user"),
                                                                Location("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/software"),
                                                                Location("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/storage"),
                                                                Location("srm://srm.grid.sara.nl:8443", "/pnfs/grid.sara.nl/data/lofar/pulsar") ] ),
                                'juelich' : deque( [    Location("srm://lofar-srm.fz-juelich.de:8443", "/pnfs/fz-juelich.de/data/lofar/ops") ] ) }

    # for each site we want one or more ResultGetterThreads
    # so make a dict with a list per site based on the locations
    getters = dict([(k,[]) for k,v in locations.items()])

    # some helper functions
    def numLocationsInQueues():
        '''returns the total number of locations in the queues'''
        return sum([len(v) for v in locations.values()])

    def totalNumGetters():
        '''returns the total number of parallel running ResultGetterThreads'''
        return sum([len(v) for v in getters.values()])

    # parse arguments for given pickle file
    if len(argv) == 1:
        # if a pickle file name was supplied, try to read it
        # and use the contents as locations and getters dicts/queues
        outputFile = argv[0]

        if os.path.exists(outputFile):
            logger.info('Trying to load srm data from %s' % outputFile)
            # overwrite initial locations and results with stored results from file
            (locations, results) = readPickle(outputFile)
            [logger.info("%s: %d locations in queue" % (k,len(v))) for k,v in locations.items()]
            logger.info('Total: %d locations in queue and %d results gathered' % (numLocationsInQueues(), len(results)))
            logger.info('Loaded srm data from %s' % outputFile)
        else:
            writePickle(outputFile, locations, results)
    else:
        # else make a default pickle file name and store inital locations and results
        outputFile = "srm_data_"+ time.strftime("%Y-%m-%d") + ".pickle"
        writePickle(outputFile, locations, results)

    # only enter main loop if there is anything to process
    if numLocationsInQueues() > 0:
        # keep track when last pickle was done
        lastPickleTimeStamp = time.time()

        # the main loop
        # loop over the locations and spawn ResultGetterThreads to get the results parallel
        # use load balancing over the different sites and with respect to queue lengths
        # do not overload this host system
        while numLocationsInQueues() > 0 or totalNumGetters() > 0:

            # get rid of old finished ResultGetterThreads
            finishedGetters = dict([(site, [getter for getter in getterList if not getter.isAlive()]) for site,getterList in getters.items()])
            for site,finishedGetterList in finishedGetters.items():
                for finishedGetter in finishedGetterList:
                    getters[site].remove(finishedGetter)

            # save intermediate data to pickle every thirty minutes
            if time.time() - lastPickleTimeStamp > 1800:
                with lock:
                    writePickle(outputFile, locations, results)
                lastPickleTimeStamp = time.time()

            # spawn new ResultGetterThreads
            # do not overload this host system
            while numLocationsInQueues() > 0 and (totalNumGetters() <= 4 or
                                                  (os.getloadavg()[0] < 3*multiprocessing.cpu_count() and
                                                  totalNumGetters() < 2*multiprocessing.cpu_count())):

                # in order to pick random locationqueue from all sites....
                # gather 'stats' per site
                # construct a weight based on these stats
                siteStats = {}
                for site in locations:
                    numGetters = len(getters[site])
                    locationQueueLength = len(locations[site])
                    weight = float(locationQueueLength) / float(20 * (numGetters + 1))
                    if numGetters == 0 and locationQueueLength > 0:
                        weight = 1e6 # make getterless sites extra important, so each site keeps flowing
                    siteStats[site] = {'# get': numGetters, '# queue': locationQueueLength, 'weight' : weight }

                totalWeight = sum([stat['weight'] for stat in siteStats.values()])

                if len(results) % 10 == 0:
                    logger.debug("siteStats:\n%s" % str('\n'.join([str((k,v)) for k,v in siteStats.items()])))

                # now pick a random site using the weights
                site = None
                cumul = 0.0
                r = random()
                for s,stat in siteStats.items():
                    ratio = stat['weight']/totalWeight
                    cumul += ratio

                    if r <= cumul:
                        site = s
                        break

                if not site:
                    break

                # make and start a new ResultGetterThread the location deque of the chosen site
                newGetter = ResultGetterThread(locations[site], results)
                newGetter.start()
                getters[site].append(newGetter)

                # small sleep between starting multiple getters
                time.sleep(0.25)

            # sleep before main loop next iteration
            # to wait for some results
            # and some getters to finis
            time.sleep(1)

        # all locations were processed
        # write final pickle
        writePickle(outputFile, locations, results)


    # process results into tree structure
    # first make a helper lookup dict with an empy tree node for each result location
    loc2res = {}
    for r in results:
        loc2res[r.location.path()] = LocationResultTreeNode(r)

    # then try to link each tree node to it's parent and child nodes using the helper dict loc2res
    for r in results:
        treeNode = loc2res[r.location.path()]

        parentLocationPath = r.location.parentLocation().path()

        if parentLocationPath in loc2res:
            parentTreeNode = loc2res[parentLocationPath]
            treeNode.parent = parentTreeNode

        for subDirLocation in r.subDirectories:
            if subDirLocation.path() in loc2res:
                subDirTreeNode = loc2res[subDirLocation.path()]
                treeNode.children.append(subDirTreeNode)


    # filter the root tree nodes
    rootTreeNodes = [x for x in loc2res.values() if not x.parent]

    # and finally log the resuling tree info for each root tree node.
    logger.info('---------------------------')
    logger.info('----------RESULTS----------\n' +
                            '\n\n'.join([x.treeString(maxLevel = 1) for x in rootTreeNodes]))


if __name__ == "__main__":
    main(sys.argv[1:])

