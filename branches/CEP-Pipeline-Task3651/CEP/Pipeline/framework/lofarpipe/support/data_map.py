#                                                       LOFAR PIPELINE FRAMEWORK
#
#                     Handle data-map files containing Data Product descriptions
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

from lofarpipe.support.lofarexceptions import DataMapError

"""
This module contains methods to load and store so-called data-map files and
to iterate over these maps. Data-map files contain a description of the
different input- and output- data products in a human readable form.
"""

class DataProduct(object):
    """
    Class representing a single data product.
    """
    def __init__(self, host, file, skip):
        self.host = host
        self.file = file
        self.skip = skip

    def __repr__(self):
        "Represent an instance as a Python dict"
        return (
            "{'host': '%s', 'file': '%s', 'skip': %s}" %
            (self.host, self.file, self.skip)
        )

    @classmethod
    def fromDict(cls, item):
        "Create a DataProduct from a dict"
        try:
            return cls(item['host'], item['file'], item['skip'])
        except KeyError, err:
            raise DataMapError("Missing key %s in dict: %s" % (err, item))


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
        self.data = data
        self.iterator = iterator
        print "type(self.data) =", type(self.data)
        print "self.data =", self.data

    def __iter__(self):
        return self.iterator(self.data)

    def __repr__(self):
        return repr(self.data)

    @classmethod   
    def load(cls, filename):
        """
        Load a data map from file `filename`. Return a DataMap instance.
        """
        with open(filename) as f:
            data = eval(f.read())
        return cls([DataProduct.fromDict(item) for item in data])

    def save(self, filename):
        """
        Save a data map to file `filename` in human-readable form.
        """
        with open(filename, 'w') as f:
            f.write(repr(self.data))

    @property
    def data(self):
        return self._data
        
    @data.setter
    def data(self, data):
        if not(all(isinstance(item, DataProduct) for item in data)): 
            raise DataMapError("Validation failed for data map: %s" % data)
        self._data = data
        

def load_data_map(filename):
    """
    Load a list of dict -- containing items host, file, and skip --
    from map-file `filename` and return it as a DataMap object.
    """
    return DataMap.load(filename)


def store_data_map(filename, data):
    """
    Store a list of dict -- containing items host, file, and skip --
    in map-file `filename`.
    """
    DataMap(data).save(filename)


def validate_data_maps(*args):
    """
    Validate the IO product specifications in the data maps `args`. Each data
    map must be a list of dict containing items host, file and skip.
    
    Requirements imposed on product specifications:
    - Length of all product lists must be equal.
    - All data-products must reside on the same host.
    
    Return True if all requirements are met, otherwise return False.
    """
    # Precondition check on `args`. All arguments must be lists; and all
    # lists must contains dicts of length 3, containing keys 'host', 'file',
    # and 'skip'.
    for arg in args:
        assert(
            isinstance(arg, list) and
            all(isinstance(item, dict) and 
                len(item) == 3 and 
                item.has_key('host') and 
                item.has_key('file') and 
                item.has_key('skip')
                for item in arg
            )
        ), "Precondition check failed for data map: %s" % arg

    # Check if all lists have equal length. We do this by creating a set
    # from a tuple of lenghts of `args`. The set must have length 1.
    if len(set(len(arg) for arg in args)) != 1:
        return False
    
    # Next, check if the data products in `args`, when matched by index,
    # reside on the same host. We can use the same trick as before, by
    # checking the size of a set created from a tuple of hostnames.
    for i in xrange(len(args[0])):
        if len(set(arg[i]['host'] for arg in args)) != 1:
            return False
    
    return True
    

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
    dirs = list(set(os.path.dirname(d['file']) for d in data))

    # Compose the filename glob-pattern.
    glob = ' '.join(os.path.join(d, glob) for d in dirs)

    # Search the files on the cluster using the glob-pattern; turn them into a
    # list of tuples.
    if logger:
        logger.debug("Searching for files: %s" % glob)
    found = zip(*findFiles(glob, '-1d'))
    
    # Return a mask containing True if file exists, False otherwise
    return [f in found for f in data]


# Self test.
if __name__ == '__main__':
    data_map = DataMap()
    print "Empty DataMap"
    for item in data_map:
        print item

    print "\nLoading foo"
    data_map = load_data_map("foo")
    print "In raw format:"
    print data_map.data

    print "Default iterator"
    for item in data_map:
        print item

    print "TupleIterator"
    data_map.iterator = DataMap.TupleIterator
    for item in data_map:
        print item

    print "SkipIterator"
    data_map.iterator = DataMap.SkipIterator
    for item in data_map:
        print item
        
    print "Saving to foo2"
    store_data_map("foo2", data_map)

    print "\nLoading bar"
    data_map = DataMap.load("bar")
    print "Default iterator"
    print data_map.data

    print "TupleIterator"
    data_map.iterator = DataMap.TupleIterator
    for item in data_map:
        print item

    print "SkipIterator"
    data_map.iterator = DataMap.SkipIterator
    for item in data_map:
        print item

    print "Saving to bar2"
    data_map.save("bar2")
    
