import numpy as np
import format
import cStringIO
import os
import itertools
import sys

from cPickle import load as _cload, loads

_file = file

def seek_gzip_factory(f):
    """Use this factory to produce the class so that we can do a lazy
    import on gzip.

    """
    import gzip, new

    def seek(self, offset, whence=0):
        # figure out new position (we can only seek forwards)
        if whence == 1:
            offset = self.offset + offset

        if whence not in [0, 1]:
            raise IOError, "Illegal argument"

        if offset < self.offset:
            # for negative seek, rewind and do positive seek
            self.rewind()
            count = offset - self.offset
            for i in range(count // 1024):
                self.read(1024)
            self.read(count % 1024)

    def tell(self):
        return self.offset

    if isinstance(f, str):
        f = gzip.GzipFile(f)

    f.seek = new.instancemethod(seek, f)
    f.tell = new.instancemethod(tell, f)

    return f

def zipfile_factory(*args, **kwargs):
    """Allow the Zip64 extension, which is supported in Python >= 2.5.

    """
    from zipfile import ZipFile
    if sys.version_info > (2, 4):
        kwargs['allowZip64'] = True
    return ZipFile(*args, **kwargs)

class BagObj(object):
    """A simple class that converts attribute lookups to
    getitems on the class passed in.
    """
    def __init__(self, obj):
        self._obj = obj
    def __getattribute__(self, key):
        try:
            return object.__getattribute__(self, '_obj')[key]
        except KeyError:
            raise AttributeError, key

class NpzFile(object):
    """A dictionary-like object with lazy-loading of files in the zipped
    archive provided on construction.

    The arrays and file strings are lazily loaded on either
    getitem access using obj['key'] or attribute lookup using obj.f.key

    A list of all files (without .npy) extensions can be obtained
    with .files and the ZipFile object itself using .zip
    """
    def __init__(self, fid):
        # Import is postponed to here since zipfile depends on gzip, an optional
        # component of the so-called standard library.
        import zipfile
        _zip = zipfile_factory(fid)
        self._files = _zip.namelist()
        self.files = []
        for x in self._files:
            if x.endswith('.npy'):
                self.files.append(x[:-4])
            else:
                self.files.append(x)
        self.zip = _zip
        self.f = BagObj(self)

    def __getitem__(self, key):
        # FIXME: This seems like it will copy strings around
        #   more than is strictly necessary.  The zipfile
        #   will read the string and then
        #   the format.read_array will copy the string
        #   to another place in memory.
        #   It would be better if the zipfile could read
        #   (or at least uncompress) the data
        #   directly into the array memory.
        member = 0
        if key in self._files:
            member = 1
        elif key in self.files:
            member = 1
            key += '.npy'
        if member:
            bytes = self.zip.read(key)
            if bytes.startswith(format.MAGIC_PREFIX):
                value = cStringIO.StringIO(bytes)
                return format.read_array(value)
            else:
                return bytes
        else:
            raise KeyError, "%s is not a file in the archive" % key

def load(file, mmap_mode=None):
    """
    Load a pickled, ``.npy``, or ``.npz`` binary file.

    Parameters
    ----------
    file : file-like object or string
        The file to read.  It must support ``seek()`` and ``read()`` methods.
    mmap_mode: {None, 'r+', 'r', 'w+', 'c'}, optional
        If not None, then memory-map the file, using the given mode
        (see `numpy.memmap`).  The mode has no effect for pickled or
        zipped files.
        A memory-mapped array is stored on disk, and not directly loaded
        into memory.  However, it can be accessed and sliced like any
        ndarray.  Memory mapping is especially useful for accessing
        small fragments of large files without reading the entire file
        into memory.

    Returns
    -------
    result : array, tuple, dict, etc.
        Data stored in the file.

    Raises
    ------
    IOError
        If the input file does not exist or cannot be read.

    Notes
    -----
    - If the file contains pickle data, then whatever is stored in the
      pickle is returned.
    - If the file is a ``.npy`` file, then an array is returned.
    - If the file is a ``.npz`` file, then a dictionary-like object is
      returned, containing ``{filename: array}`` key-value pairs, one for
      each file in the archive.

    Examples
    --------
    Store data to disk, and load it again:

    >>> np.save('/tmp/123', np.array([[1, 2, 3], [4, 5, 6]]))
    >>> np.load('/tmp/123.npy')
    array([[1, 2, 3],
           [4, 5, 6]])

    Mem-map the stored array, and then access the second row
    directly from disk:

    >>> X = np.load('/tmp/123.npy', mmap_mode='r')
    >>> X[1, :]
    memmap([4, 5, 6])

    """
    import gzip

    if isinstance(file, basestring):
        fid = _file(file,"rb")
    elif isinstance(file, gzip.GzipFile):
        fid = seek_gzip_factory(file)
    else:
        fid = file

    # Code to distinguish from NumPy binary files and pickles.
    _ZIP_PREFIX = 'PK\x03\x04'
    N = len(format.MAGIC_PREFIX)
    magic = fid.read(N)
    fid.seek(-N,1) # back-up
    if magic.startswith(_ZIP_PREFIX):  # zip-file (assume .npz)
        return NpzFile(fid)
    elif magic == format.MAGIC_PREFIX: # .npy file
        if mmap_mode:
            return format.open_memmap(file, mode=mmap_mode)
        else:
            return format.read_array(fid)
    else:  # Try a pickle
        try:
            return _cload(fid)
        except:
            raise IOError, \
                "Failed to interpret file %s as a pickle" % repr(file)

def save(file, arr):
    """
    Save an array to a binary file in NumPy format.

    Parameters
    ----------
    f : file or string
        File or filename to which the data is saved.  If the filename
        does not already have a ``.npy`` extension, it is added.
    x : array_like
        Array data.

    See Also
    --------
    savez : Save several arrays into an .npz compressed archive
    savetxt : Save an array to a file as plain text

    Examples
    --------
    >>> from tempfile import TemporaryFile
    >>> outfile = TemporaryFile()

    >>> x = np.arange(10)
    >>> np.save(outfile, x)

    >>> outfile.seek(0)
    >>> np.load(outfile)
    array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9])

    """
    if isinstance(file, basestring):
        if not file.endswith('.npy'):
            file = file + '.npy'
        fid = open(file, "wb")
    else:
        fid = file

    arr = np.asanyarray(arr)
    format.write_array(fid, arr)

def savez(file, *args, **kwds):
    """
    Save several arrays into a single, compressed file with extension ".npz"

    If keyword arguments are given, the names for variables assigned to the
    keywords are the keyword names (not the variable names in the caller).
    If arguments are passed in with no keywords, the corresponding variable
    names are arr_0, arr_1, etc.

    Parameters
    ----------
    file : Either the filename (string) or an open file (file-like object)
        If file is a string, it names the output file.  ".npz" will be appended
        if it is not already there.
    args : Arguments
        Any function arguments other than the file name are variables to save.
        Since it is not possible for Python to know their names outside the
        savez function, they will be saved with names "arr_0", "arr_1", and so
        on.  These arguments can be any expression.
    kwds : Keyword arguments
        All keyword=value pairs cause the value to be saved with the name of
        the keyword.

    See Also
    --------
    save : Save a single array to a binary file in NumPy format
    savetxt : Save an array to a file as plain text

    Notes
    -----
    The .npz file format is a zipped archive of files named after the variables
    they contain.  Each file contains one variable in .npy format.

    """

    # Import is postponed to here since zipfile depends on gzip, an optional
    # component of the so-called standard library.
    import zipfile

    if isinstance(file, basestring):
        if not file.endswith('.npz'):
            file = file + '.npz'

    namedict = kwds
    for i, val in enumerate(args):
        key = 'arr_%d' % i
        if key in namedict.keys():
            raise ValueError, "Cannot use un-named variables and keyword %s" % key
        namedict[key] = val

    zip = zipfile_factory(file, mode="w")

    # Place to write temporary .npy files
    #  before storing them in the zip
    import tempfile
    direc = tempfile.gettempdir()
    todel = []

    for key, val in namedict.iteritems():
        fname = key + '.npy'
        filename = os.path.join(direc, fname)
        todel.append(filename)
        fid = open(filename,'wb')
        format.write_array(fid, np.asanyarray(val))
        fid.close()
        zip.write(filename, arcname=fname)

    zip.close()
    for name in todel:
        os.remove(name)

