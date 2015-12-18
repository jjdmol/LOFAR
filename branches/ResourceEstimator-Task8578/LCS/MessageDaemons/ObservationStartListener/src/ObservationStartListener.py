#!/usr/bin/env python
# ObservationStartListener.py: Receive observation messages to dispatch tasks
#
# Copyright (C) 2015
# ASTRON (Netherlands Institute for Radio Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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
#
# $Id$

"""
LOFAR daemon that listens to incoming messages. If a message concerns our
cluster, run a program and pass it the message and the affected (matched) hosts.
This can e.g. be used to start observation processes via a cluster job manager.

Pass -h or --help to print the usage description.
"""

from sys import argv
import os
import signal
import logging
from socket import gethostname
from time import sleep
from subprocess import Popen
from optparse import OptionParser
daemon_exc = None
try:
  from daemon import DaemonContext
except ImportError as exc:
  daemon_exc = exc

import lofar.parameterset as lofParset
import lofar.messagebus.messagebus as lofMess

__version__ = "1.0"

def runProcess(execPath, parsetFilename, hosts):
    """
    Run command execPath with arguments parsetFilename and the sequence hosts.
    The command's stderr is hooked up to our file logger.
    Raises OSError on error.
    """
    cmd = [execPath, parsetFilename]
    cmd.extend(hosts)
    logFileStream = logger.handlers[0].stream  # logfile was set up as handler 0
    logger.info('Executing command: %s', cmd)
    proc = Popen(cmd, stderr=logFileStream, close_fds=True)  # SIGCHLD = SIG_IGN

def getOutputHosts(paramset, matchPrefix):
    """
    Returns list of output hostnames. May contain duplicates.
    paramset must be a lofar.parameterset with the LOFAR observation key/value
    pairs populated.
    matchPrefix must be a str, unicode, or a tuple of these (no list!) that
    contains the host prefix. Hostnames starting with matchPrefix are returned.
    """
    hosts = []

    # Pass suitable default values to all paramset.getXXX() functions,
    # so we don't have to try/except RuntimeError.
    uvOutputEnabled = paramset.getBool('ObsSW.Observation.DataProducts.Output_Correlated.enabled', False)
    if uvOutputEnabled:
        uvOutputLocations = paramset.getStringVector('ObsSW.Observation.DataProducts.Output_Correlated.locations', [], True)

        # E.g. 'node' in ['node1:/a/b', 'nope:/x/y'] -> ['node1']
        hosts.extend([loc.split(':')[0] for loc in uvOutputLocations if loc.startswith(matchPrefix)])

    cohStokesOutputEnabled = paramset.getBool('ObsSW.Observation.DataProducts.Output_CoherentStokes.enabled', False)
    if cohStokesOutputEnabled:
        cohStokesOutputLocations = paramset.getStringVector('ObsSW.Observation.DataProducts.Output_CoherentStokes.locations', [], True)
        hosts.extend([loc.split(':')[0] for loc in cohStokesOutputLocations if loc.startswith(matchPrefix)])

    incohStokesOutputEnabled = paramset.getBool('ObsSW.Observation.DataProducts.Output_IncoherentStokes.enabled', False)
    if incohStokesOutputEnabled:
        incohStokesOutputLocations = paramset.getStringVector('ObsSW.Observation.DataProducts.Output_IncoherentStokes.locations', [], True)
        hosts.extend([loc.split(':')[0] for loc in incohStokesOutputLocations if loc.startswith(matchPrefix)])

    return hosts

def saveData(filename, data):
    with file(filename, 'wb') as f:  # binary: write as-is; no conv!
        f.write(data)

def uniq(seq):
   """ Returns non-duplicate elements of sequence seq. Not order preserving. """
   return type(seq)(set(seq))

def processMessages(receiver, matchPrefix, execPath, msgSaveDir):
    while True:
        msg = None
        try:
            msg = receiver.get()  # blocking
            if msg is None:
                continue

            content = msg.content()
            # payload type can be unicode, but parameterset only converts str to std::string
            message = str(content.payload)
            ps = lofParset.parameterset()
            ps.adoptBuffer(message)
            hosts = getOutputHosts(ps, matchPrefix)
            if hosts:
                logger.info('Received message is applicable to us, so act on it')

                obsId = content.sasid
                messageFilename = msgSaveDir + 'L' + obsId + '.parset.xml'

                try:
                    saveData(messageFilename, message)

                    hosts = uniq(hosts)
                    hosts.sort()

                    runProcess(execPath, messageFilename, hosts)
                except IOError as exc:  # saveData()
                    logger.error('Skipped running executable: failed to save message to %s: %s',
                                 exc.filename, exc.strerror)
                except OSError as exc:  # runProcess()
                    logger.error('Failed to run executable: %s', exc.strerror)

            logger.info('Done with message')

        except lofMess.message.MessageException as exc:  # XMLDoc(), _get_data()
            logger.error('Failed to parse or retrieve node from XML message: %s', exc.message)

        finally:
            if msg is not None:
                receiver.ack(msg)  # optional for topics, needed for queues

def run(broker, address, matchPrefix, execPath, msgSaveDir):
    # Receiver test: qpid-receive -b broker_hostname -a queue_or_exchange_name -f -m 1
    # Sender test:   qpid-send -a queue_or_exchange_name --content-string "`/bin/cat L12345-task-specif.xml`"
    timeout = 0.1
    while True:
        receiver = None
        try:
            logger.info('Listening to broker %s at address %s', broker, address)
            receiver = lofMess.FromBus(address, broker = broker)
            timeout = 0.2  # sec

            processMessages(receiver, matchPrefix, execPath, msgSaveDir)

        except lofMess.BusException as exc:
            # FromBus() failed: fatal iff it has never worked, else retry.
            # FromBus' Session also retries, but wrt conn, not wrt addr/queue.
            if timeout == 0.1:
                logger.exception(exc)  # e.g. no such queue: q.x.y.z
                break
            logger.error('%s; sleeping %.1f seconds', exc.message, timeout)
            sleep(timeout)
            if timeout < 8.0:  # capped binary backoff
                timeout *= 2.0

        finally:
            if receiver is not None:
                logger.info('closing connection to broker')
                receiver.close()  # optional; FromBus (actually Session) does a delayed close

    return 1

def sigterm_handler(signr, stack_frame):
    logger.info('Received SIGTERM')

    # Easiest way to unblock blocking reads and execute __del__() and 'finally:'
    global sigterm_seen
    sigterm_seen = True
    raise KeyboardInterrupt('received SIGTERM')

def getModuleName():
    if __package__ is not None:
        name = __package__
    else:
        try:
            name = os.path.splitext(os.path.basename(__file__))[0]
        except NameError as exc:
            name = 'ObservationStartListener'  # poor, but __file__ is N/A interactively
    return name

def initLogger(logfilename, quiet):
    # Log to file. Optional to stderr. File-only is better for a system service.
    global logger
    logger = logging.getLogger(getModuleName())
    logger.setLevel(logging.INFO)
    fmt = logging.Formatter('%(asctime)s %(levelname)s %(message)s')

    fh = logging.FileHandler(logfilename)  # appends or creates, may raise
    fh.setLevel(logging.INFO)
    fh.setFormatter(fmt)
    logger.addHandler(fh)  # created process logic expects file logger first
    if not quiet:
        sh = logging.StreamHandler()  # stderr by default
        sh.setLevel(logging.INFO)
        sh.setFormatter(fmt)
        logger.addHandler(sh)

    return fh

def registerCmdOptions(parser):
    # already supported options: -h, --help, --version, --
    parser.add_option('-b', '--broker', dest='broker',
                      help='qpid broker hostname')
    parser.add_option('-a', '--address', dest='address',
                      help='qpid address (i.e. queue or topic name) on BROKER to subscribe to')
    parser.add_option('-p', '--match-prefix', dest='matchPrefix',
                      default=gethostname(), metavar='MATCH_PREFIX',
                      help='One or more comma-separated prefix(es) in a single argument value to match message values to. Default: node\'s hostname. (E.g. pass "cbt" for COBALT, "drg,drag" for DRAGNET.)')
    parser.add_option('-m', '--msg-save-dir', dest='msgSaveDir', metavar='MSG_SAVE_DIR',
                      help='directory where matched messages are saved. To pass filename to EXEC and for debugging.')
    parser.add_option('-x', '--exec', dest='execPath', metavar='EXEC',
                      help='executable to run for each matched message received. The message filename and a list of matched hostnames are passed to the executable.')
    parser.add_option('-l', '--logfile', dest='logfilename',
                      default='/dev/null', metavar='LOG_FILE',
                      help='(also) log to LOGFILE')
    parser.add_option('-q', '--quiet', action='store_true', dest='quiet',
                      default=False,
                      help='suppress logging stream to stderr. Useful with -l and when run from systemd to keep system log clean.')
    daemon_help = 'run this program as a daemon. Use absolute paths in other options.'
    if daemon_exc is not None:
        daemon_help += ' (N/A: ImportError: ' + daemon_exc.message + ')'
    parser.add_option('-d', '--daemon', action='store_true', dest='daemonize',
                      default=False, help=daemon_help)

def checkArgs(parser, options, leftOverArgs):
    # Mandatory option is contradictory, but these as positional args is unclear.
    if options.broker is None:
        parser.error('--broker (-b) is required (or pass -h for usage)')
    if options.address is None:
        parser.error('--address (-a) is required (or pass -h for usage)')
    if options.msgSaveDir is None:
        parser.error('--msg-save-dir (-m) is required (or pass -h for usage)')
    if options.execPath is None:
        parser.error('--exec (-x) is required (or pass -h for usage)')
    if options.daemonize and daemon_exc is not None:
        parser.error('--daemon (-d) is N/A: ImportError: ' + daemon_exc.message)

    options.matchPrefix = tuple(options.matchPrefix.split(','))  # for str.startswith()

    options.execPath = os.path.abspath(options.execPath)  # for -d or systemd
    if not os.path.isfile(options.execPath) or not os.access(options.execPath, os.X_OK):
        parser.error('--exec (-x): No such executable file at %s', options.execPath)

    options.msgSaveDir = os.path.abspath(options.msgSaveDir)  # for -d or systemd
    if options.msgSaveDir and options.msgSaveDir[-1] != os.path.sep:
        options.msgSaveDir += os.path.sep
    try:
        os.makedirs(options.msgSaveDir)  # exist_ok=True only since Python 3.2
    except OSError as exc:
        if exc.errno != os.errno.EEXIST or not os.path.isdir(options.msgSaveDir):
            parser.error('--msg-save-dir (-m): Failed to create %s: %s',
                         exc.filename, exc.strerror)

    if leftOverArgs:
        parser.error('unused command line arguments: %s', leftOverArgs)

def main(args):
    """
    Start the program, possibly as a daemon depending on args.
    The parameter args is like argv[1:].
    Returns an exit status which is nearly always 1, since the program is a
    service in an infinite loop except for fatal errors.
    SIGINT (KeyboardInterrupt) leads to return with exit status 1.
    SIGTERM ends the program. It is properly re-raised after internal cleanup.
    """
    usageStr = 'Usage: %prog -b BROKER -a ADDR [-p MATCH_PREFIX] -m MSG_SAVE_DIR -x EXEC [-l LOGFILE] [-q] [-d]'
    versionStr = getModuleName() + ' ' + __version__
    parser = OptionParser(usage=usageStr, version=versionStr)
    registerCmdOptions(parser)
    (options, leftOverArgs) = parser.parse_args(args)
    checkArgs(parser, options, leftOverArgs)
    logFileHandler = initLogger(options.logfilename, options.quiet)  # may raise

    status = 1
    try:
        # systemd stops us via SIGTERM (& SIGKILL). Deal via KeyboardInterrupt.
        global sigterm_seen
        sigterm_seen = False
        signal.signal(signal.SIGTERM, sigterm_handler)
        signal.signal(signal.SIGPIPE, signal.SIG_IGN)  # maintain service
        signal.signal(signal.SIGHUP, signal.SIG_IGN)   # maintain service
        signal.signal(signal.SIGCHLD, signal.SIG_IGN)  # auto-reap child procs

        if options.daemonize:
            with DaemonContext(files_preserve = [logFileHandler.stream]):
                logger.info('%s v%s pid %d on %s. Args: %s', argv[0], __version__,
                            os.getpid(), gethostname(), args)
                status = run(options.broker, options.address, options.matchPrefix,
                             options.execPath, options.msgSaveDir)
        else:
            logger.info('%s %s pid %d on %s. Args: %s', argv[0], __version__,
                        os.getpid(), gethostname(), args)
            status = run(options.broker, options.address, options.matchPrefix,
                         options.execPath, options.msgSaveDir)
    except KeyboardInterrupt:
        if sigterm_seen:
            # after cleanup re-raise SIGTERM for parent; exit(val) is inadequate
            signal.signal(signal.SIGTERM, signal.SIG_DFL)
            logger.info('Exiting due to SIGTERM (status was %d)', status)
            os.kill(os.getpid(), signal.SIGTERM)
        else:
            logger.info('KeyboardInterrupt')
            status = 1  # maybe need os.kill() too, but Python exits 1 on kbint

    logger.info('Exiting with status %d', status)
    return status

if __name__ == '__main__':
    from sys import exit
    exit(main(argv[1:]))

