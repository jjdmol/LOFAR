#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                          Parameterset Handling
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os
from tempfile import mkstemp
from contextlib import contextmanager

from lofar.parameterset import parameterset

class Parset(parameterset):
    """
    This wraps lofar.parameterset to provide a convenient means of iterating
    over the parameterset's keys.

    It should be replaced (eventually) by rolling this functionality directly
    into the C++ implementation.
    """
    def __init__(self, filename=None, caseInsensitive=False):
        super(Parset, self).__init__(filename, caseInsensitive)
        self.keys = []
        if filename:
            self._append_file(filename)

    def add(self, key, value):
        super(Parset, self).add(key, value)
        self.keys.append(key)

    def adoptFile(self, filename, prefix=''):
        super(Parset, self).adoptFile(filename, prefix)
        self._append_file(filename, prefix)

    def clear(self):
        super(Parset, self).clear()
        self.keys = []

    def remove(self, key):
        super(Parset, self).remove(key)
        self.keys.remove(key)

    def replace(self, key, value):
        super(Parset, self).replace(key, value)
        if not key in self.keys:
            self.keys.append(key)

    def subtractSubset(self, baseKey):
        super(Parset, self).subtractSubset(baseKey)
        self.keys = filter(
            lambda key: False if key[:len(baseKey)] == baseKey else True,
            self.keys
        )

    #def makeSubset(self, baseKey, prefix=None):
        #newps = Parset()
        #for key in self.keys:
            #if key[:len(baseKey)] == baseKey:
                #if prefix:
                    #newkey = key.replace(baseKey, prefix)
                #else:
                    #newkey = key
                #newps.add(newkey, self[key].get())
        #return newps

    def addStringVector(self, key, vector):
        super(Parset, self).add(key, "[ %s ]" % ", ".join(vector))
        self.keys.append(key)

    def _append_file(self, filename, prefix=''):
        file = open(filename, 'r')
        for line in file:
            key = line.split("=")[0].strip()
            if key:
                self.keys.append(prefix + key)
        file.close()

    def __iter__(self):
        return iter(self.keys)

    @classmethod
    def fromDict(cls, kvm):
        """
        Create a parameterset object from the given dict `kvm`.
        
        Caution: although any value that can be converted to a string will be
        written to the Parset, some values cannot be interpreted correctly by
        the C++ Parameterset class (e.g., a python dict).
        """
        if not isinstance(kvm, dict):
            raise TypeError("Input argument must be a dictionary")
        obj = Parset()
        for k in kvm:
            obj.add(k, str(kvm[k]))
        return obj

def get_parset(parset_filename):
    """
    Returns an instance of Parset with the given file loaded.
    """
    return Parset(parset_filename)

def patch_parset(parset, data, output_dir=None):
    """
    Generate a parset file by adding the contents of the data dictionary to
    the specified parset object. Write it to file, and return the filename.

    `parset` may either be the filename of a parset-file or an instance of
    `lofar.parameterset.parameterset`.
    """
    if isinstance(parset, str):
        temp_parset = parameterset(parset)
    else:
        temp_parset = parset.makeSubset('')  # a sneaky way to copy the parset
    for key, value in data.iteritems():
        temp_parset.replace(key, str(value))
    fd, output = mkstemp(dir=output_dir)
    temp_parset.writeFile(output)
    os.close(fd)
    return output

@contextmanager
def patched_parset(parset, data, output_dir=None, unlink=True):
    """
    Wrap patch_parset() in a contextmanager which removes the generated parset
    when it finishes.

    The never_unlink flag is to facilitate debugging -- one can leave a
    patched parset in place for manual checking if required.
    """
    filename = patch_parset(parset, data, output_dir)
    try:
        yield filename
    finally:
        if unlink: os.unlink(filename)
