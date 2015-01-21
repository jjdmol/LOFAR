#                                                       LOFAR PIPELINE FRAMEWORK
#
#                     Handle data-map file containing Data Product descriptions
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

"""
This module contains methods to load and store so-called data-map file and
to iterate over these maps. Data-map file contain a description of the
different input- and output- data products in a human readable form.
"""

from lofarpipe.support.lofarexceptions import DataMapError
from lofarpipe.support.utilities import deprecated

class DataProduct(object):
    """
    Class representing a single data product.
    """
    def __init__(self, host, file, skip=True):

        self.host = str(host)
        self.file = str(file)
        self.skip = bool(skip)

    def __repr__(self):
        """Represent an instance as a Python dict"""
        return (
            "{'host': '%s', 'file': '%s', 'skip': %s}" %
            (self.host, self.file, self.skip)
        )

    def __str__(self):
        """Print an instance as 'host:file'"""
        return ':'.join((self.host, self.file))

    def __eq__(self, other):
        """Compare for equality"""
        return (
            self.host == other.host and
            self.file == other.file and
            self.skip == other.skip
        )

    def __ne__(self, other):
        """Compare for non-equality"""
        return not self.__eq__(other)


class DataMap(object):
    """
    Class representing a data-map, which basically is a collection of data
    products.
    """
    class TupleIterator(object):
        """
        Iterator returning data-map entries as tuple (host, file). Use this
        iterator for backward compatibility.
        """
        def __init__(self, data):
            self.data = data
            self.index = 0

        def __iter__(self):
            return self

        def next(self):
            try:
                value = self.data[self.index]
            except IndexError:
                raise StopIteration
            self.index += 1
            return (value.host, value.file)

    class SkipIterator(object):
        """
        Iterator returning only data-map entries whose `skip` attribute is
        False.
        """
        def __init__(self, data):
            self.data = data
            self.index = 0

        def __iter__(self):
            return self

        def next(self):
            while(True):
                try:
                    value = self.data[self.index]
                except IndexError:
                    raise StopIteration
                self.index += 1
                if not value.skip:
                    return value

    def __init__(self, data=list(), iterator=iter):

        self._data = list()
        self.data = data
        self.iterator = iterator

    def __repr__(self):
        return repr(self.data)

    def __iter__(self):
        return self.iterator(self.data)

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.data[index]

    def __eq__(self, other):
        if type(other) != type(self):
            # comparison of DataMap and MultiDataMap should always fail.
            # ( We do engineering and not set logic)
            return False
        return all(x == y for (x, y) in zip(self.data, other.data))

    def __ne__(self, other):
        return not self.__eq__(other)

    @classmethod
    def load(cls, filename):
        """Load a data map from file `filename`. Return a DataMap instance."""
        with open(filename) as f:
            return cls(eval(f.read()))

    def save(self, filename):
        """Save a data map to file `filename` in human-readable form."""
        with open(filename, 'w') as f:
            f.write(repr(self.data))

    @property
    def data(self):
        """Property to get self.data"""
        return self._data

    @data.setter
    def data(self, data):
        """Property to set self.data, allows do input validation."""
        self._set_data(data, dtype=DataProduct)

    def _set_data(self, data, dtype=DataProduct):
        try:
            if all(isinstance(item, dtype) for item in data):
                self._data = data
            elif all(isinstance(item, dict) for item in data):
                self._data = [dtype(**item) for item in data]
            elif all(isinstance(item, tuple) for item in data):
                self._data = [dtype(*item) for item in data]
            else:
                raise TypeError
        except TypeError:
            raise DataMapError("Failed to validate data map: %s" % repr(data))

class MultiDataMap(DataMap):
    """
    Class representing a specialization of data-map, a collection of data
    products located on the same node, skippable as a set and individually
    """
    @DataMap.data.setter
    def data(self, data):
        self._set_data(data, dtype=MultiDataProduct)



class MultiDataProduct(object):
    """
    Class representing a single data product.
    """
    def __init__(self, host, file, skip=True, file_skip=None):
        self.host = str(host)
        try:
            # Try parsing as a list
            file_list = eval(file)
            if isinstance(file_list, list):
                self.file = file_list
        except:
            self.file = file

        # Create a default skip field with the same truth value is the 'parent'
        if file_skip == None:
            self.file_skip = [bool(skip)] * len(self.file)
        else:
            # All entries in the boollist should be a bool
            for entry in file_skip:
                if not isinstance(entry, bool):
                    raise TypeError
            # The length of the boollist should 
            if len(file_skip) != len(self.file):
                raise TypeError
            self.file_skip = file_skip

        self.skip = bool(skip)

    def __repr__(self):
        """Represent an instance as a Python dict"""
        return (
            "{'host': '%s',  'skip': %s, 'file': \"%s\", 'file_skip' : %s}" %
            (self.host, self.skip, self.file, self.file_skip)
        )

    def __str__(self):
        """Print an instance as 'host:file'"""
        return ':'.join((self.host, self.file))

    def __eq__(self, other):
        """Compare for equality"""
        return (
            self.host == other.host and
            self.file == other.file and
            self.file_skip == other.file_skip and
            self.skip == other.skip
        )

    def __ne__(self, other):
        """Compare for non-equality"""
        return not self.__eq__(other)

@deprecated
def load_data_map(filename):
    """
    Load a data map from file `filename` and return it as a DataMap object.
    The file should either contain a list of dict (new-style), containing items
    host, file, and skip; or a list of tuple (old-style), containing host and
    file.
    This method is for backward compatibility. New code should use 
    `DataMap.load` instead. The iterator of the returned DataMap object is set
    to TupleIterator, so that existing code that expects tuples of (host,file)
    when iterating over a data map's contents won't break.
    """
    data_map = DataMap.load(filename)
    data_map.iterator = DataMap.TupleIterator
    return data_map


@deprecated
def store_data_map(filename, data):
    """
    Store a data map in map-file `filename`. Assume the argument is a new-style
    DataMap object. If that fails, assume data is old-style list of tuples of
    (host, filepath). In either case, the data is written in the new format:
    a list of dict containing items host, file, and skip.
    This method is for backward compatibility. New code should use the method
    `DataMap.save` instead.
    """
    try:
        # Assume `data` is a DataMap object.
        data.save(filename)
    except AttributeError:
        # Assume `data` can be used to construct a DataMap object.
        DataMap(data).save(filename)


def validate_data_maps(*args):
    """
    Validate the IO product specifications in the data maps `args`. 
        
    Requirements imposed on product specifications:
    - Length of all product lists must be equal.
    - All data-products must reside on the same host.
    
    Return True if all requirements are met, otherwise return False.
    """
    # Check if all data maps have equal length. We do this by creating a set
    # from a tuple of lenghts of `args`. The set must have length 1.
    if len(set(len(arg) for arg in args)) != 1:
        return False

    # Next, check if the data products in `args`, when matched by index,
    # reside on the same host. We can use the same trick as before, by
    # checking the size of a set created from a tuple of hostnames.
    for i in xrange(len(args[0])):
        if len(set(arg[i].host for arg in args)) != 1:
            return False

    return True


def align_data_maps(*args):
    """
    Aligns the skip fields in the supplied datamaps.
    If a single skip is set for an entrie, all other entries in the other
    datamaps will be set.
    
    Raises a DataMapError if the datamaps are of different size and
    alignment is not possible.
    """
    if len(args) < 2:
        raise DataMapError("At least two datamaps are needed to perform align.")

    if not validate_data_maps(*args):
        raise DataMapError(
            "Validation of data maps failed in align_data_map"
            " Supplied datamaps: {0}".format(repr(args)))

    # for all the input maps get the entries in a single tuple
    for entries in zip(*args):
        skip = False
        # For each entrie perform an or of the skip fields
        for entrie in entries:
            skip = skip or entrie.skip
        # Assign the resulting skip field to all the entries
        for entrie in entries:
            entrie.skip = skip



@deprecated
def tally_data_map(data, glob, logger=None):
    """
    Verify that the file specified in the data map `data` exist on the cluster.
    The glob pattern `glob` should contain the pattern to be used in the search.
    This function will return a list of booleans: True for each item in `data`
    that is present on the cluster; False otherwise.
    This method is deprecated, because the new data-map file keep track of the
    `skip` attribute of each data product in the data-map.
    """
    import os
    from lofar.mstools import findFiles

    # Determine the directories to search. Get unique directory names from
    # `data` by creating a set first.
    dirs = list(set(os.path.dirname(d.file) for d in data))

    # Compose the filename glob-pattern.
    glob = ' '.join(os.path.join(d, glob) for d in dirs)

    # Search the file on the cluster using the glob-pattern; turn them into a
    # list of tuples.
    if logger:
        logger.debug("Searching for file: %s" % glob)
    found = zip(*findFiles(glob, '-1d'))

    # Return a mask containing True if file exists, False otherwise
    return [(f.host, f.file) in found for f in data]

