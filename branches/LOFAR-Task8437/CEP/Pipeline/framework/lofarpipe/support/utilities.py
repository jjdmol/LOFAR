#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                               Utility routines
#                                                         John Swinbank, 2009-10
#                                                      swinbank@transientskp.org
#                                                          Marcel Loose, 2011-12
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

from __future__ import with_statement

from itertools import islice, repeat, chain, izip
from contextlib import closing, contextmanager
from time import sleep
from random import randint
import warnings

import os
import errno
import shutil
import sys

try:
    import subprocess27 as subprocess
    print >> sys.stderr, __file__, ": Using Python 2.7 subprocess module!"
except ImportError:
    import subprocess
    print >> sys.stderr, __file__, ": Using default subprocess module!"

from lofarpipe.support.pipelinelogging import log_process_output

def deprecated(func):
    """
    This is a decorator which can be used to mark functions as deprecated.
    It will result in a warning being emmitted when the function is used.
    Ref.: http://code.activestate.com/recipes/391367-deprecated/
    """
    def new_func(*args, **kwargs):
        warnings.warn("Call to deprecated function %s." % func.__name__,
                      category=DeprecationWarning,
                      stacklevel=2)
        return func(*args, **kwargs)
    new_func.__name__ = func.__name__
    new_func.__doc__ = func.__doc__
    new_func.__dict__.update(func.__dict__)
    return new_func

#                                                                  Compatibility
#                               The following used to be defined in this module;
#                        they are now included so as not to break existing code.
# ------------------------------------------------------------------------------
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.parset import get_parset, patch_parset

#                                                File and Directory Manipulation
# ------------------------------------------------------------------------------

def get_mountpoint(path):
    """
    Return the path to the mount point containing the given path.

    :param path: Path to check
    """
    return path if os.path.ismount(path) else get_mountpoint(
        os.path.abspath(os.path.join(path, os.pardir))
    )

def create_directory(dirname):
    """
    Recursively create a directory, without failing if it already exists.
    """
    try:
        if dirname:
            os.makedirs(dirname)
    except OSError, failure:
        if failure.errno != errno.EEXIST:
            raise failure

def delete_directory(dirname):
    """
    Recursively delete a directory tree: Without failing if the dir does not
    exist
    """
    try:
        shutil.rmtree(dirname)
    except OSError, e:
        if not e.errno == errno.ENOENT:
            raise e

def disk_usage(*paths):
    """
    Return the disk usage in bytes by the file(s) in ``paths``.
    """
    cmd = ['du', '-s', '-b']
    proc = subprocess.Popen(cmd + list(paths), stdout = subprocess.PIPE)
    sout = proc.communicate()[0]
    if sout:
        return sum([int(s.split('\t')[0]) for s in sout.strip().split('\n')])
    else:
        return 0

#                                                    IPython Dependency Checking
# ------------------------------------------------------------------------------

def build_available_list(listname):
    """
    This can be pushed to an IPython engine and run there to generate a list
    of data which is locally stored (and hence available for processing).
    """
    import os
    from IPython.kernel.engineservice import get_engine
    available = [
        filename for filename in filenames if os.access(filename, os.R_OK)
    ]
    properties = get_engine(id).properties
    properties[listname] = available

def clear_available_list(listname):
    """
    Delete lists of locally stored data to free up resources on the engine.
    """
    from IPython.kernel.engineservice import get_engine
    del(get_engine(id).properties[listname])

def check_for_path(properties, dependargs):
    """
    Run on IPython engine to check for the existence of a given path in the
    locally available data, as recorded by build_available_list().

    Used for dependency checking when scheduling jobs.
    """
    try:
        return dependargs[0] in properties[dependargs[1]]
    except NameError:
        return False

#                                                       Iterators and Generators
# ------------------------------------------------------------------------------

def is_iterable(obj):
    """
    Return True if the given object is iterable, else False.
    """
    try:
        iter(obj)
    except:
        return False
    else:
        return True

try:
    from itertools import izip_longest
except ImportError:
    def izip_longest(*args, **kwds):
        """
        This code is lifted from the Python 2.6 manual, since izip_longest
        isn't available in the 2.5 standard library.
        izip_longest('ABCD', 'xy', fillvalue='-') --> Ax By C- D-
        """
        fillvalue = None
        def sentinel(counter = ([fillvalue] * (len(args) - 1)).pop):
            yield counter()         # yields the fillvalue, or raises IndexError
        fillers = repeat(fillvalue)
        iters = [chain(it, sentinel(), fillers) for it in args]
        try:
            for tup in izip(*iters):
                yield tup
        except IndexError:
            pass

def group_iterable(iterable, size):
    """
    Group the iterable into tuples of given size.  Returns a generator.

    Example:
    >>> for x in group([1,2,3,4,5], 3): print x
    (1, 2, 3)
    (4, 5)
    """
    return (
        filter(lambda x: x is not None, x)
        for x in izip_longest(
            *[islice(iterable, n, None, size) for n in xrange(size)]
        )
    )

#                                                                  Miscellaneous
# ------------------------------------------------------------------------------

def read_initscript(logger, filename, shell = "/bin/sh"):
    """
    Return a dict of the environment after sourcing the given script in a shell.
    """
    if not os.path.exists(filename):
        logger.warn("Environment initialisation script not found!")
        return {}
    else:
        logger.debug("Reading environment from %s" % filename)
        p = subprocess.Popen(
            ['. %s ; env' % (filename)],
            shell = True,
            executable = shell,
            stdout = subprocess.PIPE,
            stderr = subprocess.PIPE,
            close_fds = True
        )
        so, se = p.communicate()
        environment = [x.split('=', 1) for x in so.strip().split('\n')]
        environment = filter(lambda x: len(x) == 2, environment)
        return dict(environment)

def string_to_list(my_string):
    """
    Convert a list-like string (as in pipeline.cfg) to a list of values.
    """
    return [x.strip() for x in my_string.strip('[] ').split(',') if x.strip()]

def spawn_process(cmd, logger, cwd = None, env = None, max_tries = 2, max_timeout = 30):
    """
    DEPRECATED -- spawn_process leads to custom, and thus bad, output handling. Use
                  support.subprocessgroup.SubProcessGroup instead.

    Tries to spawn a process.

    If it hits an OSError due to lack of memory or too many open files, it
    will keep trying max_tries times, waiting timeout seconds between each.

    If successful, the process object is returned. Otherwise, we eventually
    propagate the exception.
    """

    logger.error("support.utilities.spawn_process is DEPRECATED. Please use support.subprocessgroup.SubProcessGroup")

    # Make sure the working directory exists.
    create_directory(cwd);

    trycounter = 0
    while True:
        logger.debug(
            "Spawning subprocess: cmd=%s, cwd=%s, env=%s" % (cmd, cwd, env))
        try:
            process = subprocess.Popen(
                cmd, cwd = cwd, env = env, stdin = subprocess.PIPE, stdout = subprocess.PIPE, stderr = subprocess.PIPE
            )
        except OSError, e:
            logger.warn(
                "Failed to spawn external process %s (%s)" % (" ".join(cmd), str(e))
            )
            if trycounter < max_tries:
                timeout = randint(1, max_timeout)
                logger.warn(
                    "Retrying in %d seconds (%d more retries)." %
                    (timeout, max_tries - trycounter - 1)
                )
                trycounter += 1
                sleep(timeout)
            else:
                raise
        else:
            break
    return process

def catch_segfaults(cmd, cwd, env, logger, max = 1, cleanup = lambda: None,
                    usageStats = None):
    """
    Run cmd in cwd with env, sending output to logger.

    If it segfaults, retry upto max times.
    """
    tries = 0
    while tries <= max:
        if tries > 0:
            logger.debug("Retrying...")
        logger.debug("Running: %s" % (' '.join(cmd),))
        process = spawn_process(cmd, logger, cwd, env)
        #add the created process to the usageStat object
        if usageStats:
            usageStats.addPID(process.pid)

        if 'casa' in cmd[0]:
	    import time
            while process.returncode is None:
                process.poll()
                time.sleep(1)
        sout, serr = process.communicate()
        log_process_output(cmd[0], sout, serr, logger)
        if process.returncode == 0:
            break
        elif process.returncode == -11:
            logger.warn("%s process segfaulted!" % cmd[0])
            cleanup()
            tries += 1
            continue
        else:
            raise subprocess.CalledProcessError(
                process.returncode, cmd[0]
            )
    if tries > max:
        logger.error("Too many segfaults from %s; aborted" % (cmd[0]))
        raise subprocess.CalledProcessError(process.returncode, cmd[0])
    return process
