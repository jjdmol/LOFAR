#                                                       LOFAR PIPELINE FRAMEWORK
#
#                              Group data into appropriate chunks for processing
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from collections import defaultdict
import os
import subprocess

from lofar.mstools import findFiles

import lofarpipe.support.utilities as utilities
from lofarpipe.support.clusterdesc import get_compute_nodes
from lofarpipe.support.parset import Parset

def group_files(logger, clusterdesc, node_directory, group_size, filenames):
    """
    Group a list of files into blocks suitable for simultaneous
    processing, such that a limited number of processes run on any given
    host at a time.

    All node_directory on all compute nodes specified in clusterdesc is
    searched for any of the files listed in filenames. A generator is
    produced; on each call, no more than group_size files per node
    are returned.
    """
    # Given a limited number of processes per node, the first task is to
    # partition up the data for processing.
    logger.debug('Listing data on nodes')
    data = {}
    for node in get_compute_nodes(clusterdesc):
        logger.debug("Node: %s" % (node))
        exec_string = ["ssh", node, "--", "find",
            node_directory,
            "-maxdepth 1",
            "-print0"
            ]
        logger.debug("Executing: %s" % (" ".join(exec_string)))
        my_process = subprocess.Popen(
            exec_string, stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
        sout = my_process.communicate()[0]
        data[node] = sout.split('\x00')
        data[node] = utilities.group_iterable(
            [element for element in data[node] if element in filenames],
            group_size,
        )

    # Now produce an iterator which steps through the various chunks of
    # data to image, and image each chunk
    data_iterator = utilities.izip_longest(*list(data.values()))
    for data_chunk in data_iterator:
        to_process = []
        for node_data in data_chunk:
            if node_data: 
                to_process.extend(node_data)
        yield to_process

def gvds_iterator(gvds_file, nproc=4):
    """
    Reads a GVDS file.

    Provides a generator, which successively returns the contents of the GVDS
    file in the form (host, filename), in chunks suitable for processing
    across the cluster. Ie, no more than nproc files per host at a time.
    """
    parset = Parset(gvds_file)

    data = defaultdict(list)
    for part in range(parset.getInt('NParts')):
        host = parset.getString("Part%d.FileSys" % part).split(":")[0]
        file = parset.getString("Part%d.FileName" % part)
        vds  = parset.getString("Part%d.Name" % part)
        data[host].append((file, vds))

    for host, values in data.iteritems():
        data[host] = utilities.group_iterable(values, nproc)

    while True:
        yieldable = []
        for host, values in data.iteritems():
            try:
                for filename, vds in values.next():
                    yieldable.append((host, filename, vds))
            except StopIteration:
                pass
        if len(yieldable) == 0:
            raise StopIteration
        else:
            yield yieldable

#def load_data_map(filename):
    #"""
    #Load a mapping of filename <-> compute node from a parset on disk.
    #"""
    #datamap = Parset(filename)
    #data = []
    #for host in datamap:
        #for filename in datamap.getStringVector(host):
            #data.append((host, filename))
    #return data

#def store_data_map(filename, data):
    #"""
    #Store a mapping of filename <-> compute node as a parset on disk.
    #"""
    #datamap = defaultdict(list)
    #for (host,file) in data:
        #datamap[host].append(file)
    #outfile = open(filename, 'w')
    #for key in sorted(datamap):
        #outfile.write('%s = %s\n' % (key, datamap[key]))
    #outfile.close()


def validate_data_maps(*args):
    """
    Validate the IO product specifications in the data maps `args`. Each data
    map must be a list of tuples (hostname, filepath). 
    
    Requirements imposed on product specifiations:
    - Length of all product lists must be equal.
    - All data-products must reside on the same node.
    
    Return True if all requirements are met, otherwise return False.
    """
    # Precondition check on `args`. All arguments must be lists; and all
    # lists must contains tuples of length 2.
    for arg in args:
        assert(
            isinstance(arg, list) and
            all(isinstance(item, tuple) and len(item) == 2 for item in arg)
        )

    # Check if all lists have equal length. We do this by creating a set
    # from a tuple of lenghts of `args`. The set must have length 1.
    if len(set(len(arg) for arg in args)) != 1: 
        return False
    
    # Next, check if the data products in `args`, when matched by index,
    # reside on the same node. We can use the same trick as before, by
    # checking the size of a set created from a tuple of hostnames.
    for i in xrange(len(args[0])):
        if len(set(arg[i][0] for arg in args)) != 1: 
            return False
    
    return True
    

def load_data_map(filename):
    """
    Load map-file `filename` containing tuples of (host,filepath)
    """
    file = open(filename)
    data = eval(file.read())
    file.close()
    if not validate_data_maps(data):
        raise TypeError("Map-file data validation failed")
    return data


def store_data_map(filename, data):
    """
    Store tuples of (host,filepath) in a map-file `filename`.
    """
    if not validate_data_maps(data):
        raise TypeError("Map-file data validation failed")
    file = open(filename, 'w')
    file.write(repr(data))
    file.close()


def tally_data_map(data, glob, logger=None):
    """
    Verify that the files specified in the data map `data` exist on the cluster.
    The glob pattern `glob` should contain the pattern to be used in the search.
    This function will return a list of booleans: True for each item in `data`
    that is present on the cluster; False otherwise.
    """
    # Check that `data` is in the correct format
    validate_data_maps(data)
    
    # Determine the directories to search. Get unique directory names from
    # `data` by creating a set first.
    dirs = list(set(os.path.dirname(d[1]) for d in data))

    # Compose the filename glob-pattern.
    glob = ' '.join(os.path.join(d, glob) for d in dirs)

    # Search the files on the cluster using the glob-pattern; turn them into a
    # list of tuples.
    if logger:
        logger.debug("Searching for files: %s" % glob)
    found = zip(*findFiles(glob, '-1d'))
    
    # Return a mask containing True if file exists, False otherwise
    return [f in found for f in data]

