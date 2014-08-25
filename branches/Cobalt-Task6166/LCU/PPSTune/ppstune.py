#!/usr/bin/env python2
# -*- python -*-

r'''
Script to determine new PPS delays at LOFAR stations. Its only
external dependencies are ``numpy`` and
``/opt/stationtest/verify.py``. The standard Python ``logging`` module
is used for log file handling.
'''

# Note that optparse is deprecated in Python 2.7 in favour of argparse...
from optparse import OptionParser
import os, sys, time, traceback
import signal, inspect
import subprocess
import logging
import pwd
from math import log


try:
    all([])
except NameError: # Python 2.4 does not yet know of all()
    def all(iterable):
        r'''
        Return True if bool(x) is True for all values x in the iterable.

        **Parameters**

        iterable : something iterable such as a list or sequence.

        **Examples**

        >>> all([True, True, True])
        True
        >>> all([1, 1, 1])
        True
        >>> all([1, 1, 0])
        False
        >>> all([False, False, False])
        False
        '''
        for element in iterable:
            if not element:
                return False
        return True
    

def version_string():
    r'''
    Returns version number as a string.

    **Example**

    >>> version_string()
    '1.1'
    '''
    return '1.1'


#######################
#                     #
#      Utilities      #
#                     #
#######################


def flatten_list(list_of_lists):
    r'''
    Takes a list of lists and spits out one list with all sub lists
    concatenated. [[1, 2, 3], [4, 5]] -> [1, 2, 3, 4, 5]

    **Parameters**

    list_of_lists : list of lists
        The list to flatten.

    **Returns**

    A one dimensional list.

    **Examples**

    >>> flatten_list([[1, 2, 3], ['a', True], ['b', ['c', 4]]])
    [1, 2, 3, 'a', True, 'b', ['c', 4]]
    '''
    return [element for sub_list in list_of_lists for element in sub_list]




def transpose_lists(matrix):
    r'''
    Transpose a matrix represented as a list of lists.

    **Parameters**

    matrix : list of lists

    **Returns**

    Transposed list of lists.

    **Examples**

    >>> transpose_lists([[1,2,3,4], [5,6,7,8], [9,9,9,9]])
    [[1, 5, 9], [2, 6, 9], [3, 7, 9], [4, 8, 9]]
    '''
    logging.debug('transpose_lists(%r)', matrix)
    cols = max([len(row) for row in matrix])
    transposed = []
    for col_id in range(cols):
        row = []
        for row_id in range(len(matrix)):
            row.append(matrix[row_id][col_id])
        transposed.append(row)
    return transposed
    



def check_output(args, stderr = None, execute = True, timeout_s = None):
    r'''
    Call an external program with command line arguments, and return
    its output as a byte string.

    **Parameters**

    args : list of strings
        The command and its argument as a list of strings.

    stderr : None or subprocess.STDOUT
        If None: ignore stderr output, if subprocess.STDOUT, redirect
        standard error output to standard output.

    execute : bool
        If True, execute the command. If False, just log the command
        line and return an empty string without executing the command.

    timeout_s : None or float
        If None: wait until child process terminates. If a float: wait
        for at most ``timeout_s`` seconds for the child to
        terminate. If the chilkd does not terminate within the set
        time out, kill the child process with SIGTERM and raise a
        RuntimeError.

    **Returns**

    A string containing all of the program output.

    **Examples**

    >>> check_output(['ls', '-d', '.'], execute = False)
    ''
    >>> check_output(['ls', '-d', '.'])
    '.\n'
    >>> check_output(['sleep', '2'], timeout_s = 3.0)
    ''
    >>> check_output(['sleep', '2'], timeout_s = 1.0)
    Traceback (most recent call last):
    ...
    RuntimeError: sleep 2 killed with signal 15; output:
    ''

    ``slowoutput`` writes five characters per second:
    
    >>> check_output(['test/slowoutput.py', 'short'], timeout_s =1.5)
    'short'
    >>> check_output(['test/slowoutput.py', 'a', 'longish string', 'that interestst no-one'], timeout_s =1.5)
    Traceback (most recent call last):
    ...
    RuntimeError: test/slowoutput.py a longish string that interestst no-one killed with signal 15; output:
    'a longis'

    '''
    logging.debug(' '.join(args))
    if execute:
        if timeout_s is None:
            return subprocess.Popen(args,
                                    shell  = False,
                                    stdout = subprocess.PIPE,
                                    stdin  = subprocess.PIPE,
                                    stderr = stderr).communicate()[0]
        else:
            start_date = time.time()
            process = subprocess.Popen(args,
                shell  = False,
                stdout = subprocess.PIPE,
                stdin  = subprocess.PIPE,
                stderr = stderr)
            stdout = []
            out = ''
            while True:
                out = process.stdout.read(1)
                if out == '' and process.poll() != None:
                    break
                if out != '':
                    stdout.append(out)
                if time.time() - start_date > timeout_s:
                    logging.error('timeout after %6.3f s: terminating command %s ',
                                  timeout_s, ' '.join(args))
                    os.kill(process.pid, signal.SIGTERM)
                    raise RuntimeError('%s killed with signal %d; output:\n%r' %
                                       (' '.join(args), signal.SIGTERM,
                                        ''.join(stdout)))
            logging.debug('process.poll(): %r', process.poll())
            if process.poll() < 0:
                raise RuntimeError('%s killed with signal %d' %
                                   (' '.join(args), process.poll()))
            return ''.join(stdout)
    else:
        return ''




def gmtime_tuple(date_s):
    r'''
    Return the ``date_s`` as a gmtime tuple containing (year, month,
    day, hour, minute, second).

    **Parameters**

    date_s : float
        Number of seconds elapsed since 1970.0.

    **Returns**

    (int, int, int, int, int, int) tuple containing (year, month, day,
    hour, minute, second).

    **Examples**

    >>> gmtime_tuple(1332246766.307168)
    (2012, 3, 20, 12, 32, 46)
    '''
    gmtime = time.gmtime(date_s)
    return (gmtime.tm_year, gmtime.tm_mon, gmtime.tm_mday,
            gmtime.tm_hour, gmtime.tm_min, gmtime.tm_sec)



def dew_point_temperature_c(temperature_c, humidity_percent):
    r'''
    Compute the dew point temperature.

    **Parameters**

    temperature_c : float
        Temperature in degrees Celsius.

    humidity_percent : float
        Relative humidity in percentage of saturation humidity.

    **Returns**

    A float containing the dew point temperature in degrees celsius.

    **Examples**

    >>> '%4.1f' % dew_point_temperature_c(12.4, 62.0)
    ' 5.3'
    >>> '%4.1f' % dew_point_temperature_c(19.3, 87.0)
    '17.1'
    >>> '%4.1f' % dew_point_temperature_c(18.3, 84.0)
    '15.5'
    >>> '%4.1f' % dew_point_temperature_c(41.0, 8.5)
    ' 1.0'
    >>> '%4.1f' % dew_point_temperature_c(20.0, 30.2)
    ' 2.0'

    '''
    const_a = 17.271
    const_b = 237.7
    t_c     = temperature_c
    gamma = const_a*t_c/(const_b + t_c) + log(humidity_percent/100.0)
    return const_b*gamma/(const_a - gamma)



###########################################
#                                         #
#       Station control and analysis      #
#                                         #
###########################################

class RSPDriverDownError(RuntimeError):
    pass

class BrokenRSPBoardsError(RuntimeError):
    pass

class MalformedRemoteStationConfError(RuntimeError):
    pass

def swlevel(level, swlevel_cmd = '/opt/lofar/bin/swlevel', timeout_s = 80.0):
    r'''
    Set the swlevel at the station.

    **Parameters**

    level : int
        The level to which one wants to switch.

    swlevel_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    timeout_s : float
        If the swlevel command has not returned after ``timeout_s``
        seconds, raise a RuntimeError. At international stations,
        changing swlevel from 1 to 6 should typically take about 40
        seconds, hence the timeout at 80 seconds to catch seriously
        anomalous cases.

    **Returns**

    A string with the terminal output of swlevel.

    **Example**

    >>> print(swlevel(2, 'test/swlevel'))
    Currently set level is 2
    <BLANKLINE>
    Status of all software level:
    1 : PVSS00pmon                20884
    1 : LogProcessor              20930
    1 : ServiceBroker             20959
    1 : SASGateway                20981
    ---
    2 : RSPDriver                 18372
    2 : TBBDriver                 18391
    ---
    3 : AMCServer                 DOWN
    3 : CalServer                 DOWN
    3 : BeamServer                DOWN
    ---
    4 : SoftwareMonitor           DOWN
    4 : HardwareMonitor           DOWN
    ---
    5 : SHMInfoServer             DOWN
    ---
    6 : CTStartDaemon             DOWN
    6 : StationControl            DOWN
    6 : ClockControl              DOWN
    6 : CalibrationControl        DOWN
    6 : BeamControl               DOWN
    6 : TBBControl                DOWN
    ---
    <BLANKLINE>
    <BLANKLINE>

    >>> print(swlevel(6, 'test/swlevel'))
    Currently set level is 6
    <BLANKLINE>
    Status of all software level:
    1 : PVSS00pmon                20884
    1 : LogProcessor              20930
    1 : ServiceBroker             20959
    1 : SASGateway                20981
    ---
    2 : RSPDriver                 18372
    2 : TBBDriver                 18391
    ---
    3 : AMCServer                 18413
    3 : CalServer                 18416
    3 : BeamServer                18419
    ---
    4 : SoftwareMonitor           18890
    4 : HardwareMonitor           19038
    ---
    5 : SHMInfoServer             19226
    ---
    6 : CTStartDaemon             19251
    6 : StationControl            19281
    6 : ClockControl              19768
    6 : CalibrationControl        23318 [ObsID: 53525]
    6 : BeamControl               23320 [ObsID: 53525]
    6 : TBBControl                DOWN
    ---
    <BLANKLINE>
    <BLANKLINE>
    >>> print(swlevel('6-broken-rsp', 'test/swlevel'))
    Traceback (most recent call last):
    ...
    BrokenRSPBoardsError: Broken RSP Boards:
    No RSP board 4 found
    No RSP board 5 found
    No RSP board 6 found

    '''
    logging.info('Going to swlevel %r', level)
    swlevel_output = check_output([swlevel_cmd, str(level)], timeout_s = timeout_s)
    broken_list = [line.strip() for line in swlevel_output.split('\n')
                   if 'No RSP board' in line]
    if len(broken_list) > 0:
        logging.error('Broken RSP boards:\n%s', '\n'.join(broken_list))
        raise BrokenRSPBoardsError('Broken RSP Boards:\n%s' % '\n'.join(broken_list))
    return swlevel_output



def check_swlevel_output(swlevel_cmd = '/opt/lofar/bin/swlevel'):
    r'''
    Return the current output of ``swlevel``.

    **Parameters**
    
    swlevel_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    **Returns**

    A string.
        
    **Examples**
    
    >>> print(check_swlevel_output('test/swlevel'))
    Currently set level is 6
    <BLANKLINE>
    Status of all software level:
    1 : PVSS00pmon                20884
    1 : LogProcessor              20930
    1 : ServiceBroker             20959
    1 : SASGateway                20981
    ---
    2 : RSPDriver                 18372
    2 : TBBDriver                 18391
    ---
    3 : AMCServer                 18413
    3 : CalServer                 18416
    3 : BeamServer                18419
    ---
    4 : SoftwareMonitor           18890
    4 : HardwareMonitor           19038
    ---
    5 : SHMInfoServer             19226
    ---
    6 : CTStartDaemon             19251
    6 : StationControl            19281
    6 : ClockControl              19768
    6 : CalibrationControl        23318 [ObsID: 53525]
    6 : BeamControl               23320 [ObsID: 53525]
    6 : TBBControl                DOWN
    ---
    <BLANKLINE>
    <BLANKLINE>

    '''
    swlevel_output = None
    if 'CalledProcessError' not in dir(subprocess):
        swlevel_output = check_output([swlevel_cmd])
    else:
        try:
            swlevel_output = check_output([swlevel_cmd])
        except subprocess.CalledProcessError:
            swlevel_output = sys.exc_info()[1].output
         
    return swlevel_output




def get_swlevel(swlevel_cmd = '/opt/lofar/bin/swlevel'):
    r'''
    Return the current ``swlevel`` of a station.

    **Parameters**
    
    swlevel_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    **Returns**

    An int between 0 and 6 inclusive, representing the swlevel.
        
    **Examples**
    
    >>> get_swlevel('test/swlevel')
    6
    '''
    swlevel_output = check_swlevel_output(swlevel_cmd = swlevel_cmd)

    return int(swlevel_output.split('\n')[0].split()[-1])
    





def get_clock_frequency_mhz(rspctl_cmd  = '/opt/lofar/bin/rspctl',
                            swlevel_cmd = '/opt/lofar/bin/swlevel',
                            sleep_s = 1.0):
    r'''
    Return the current clock frequency in MHz by analyzing the output
    of ``rspctl --tdstatus``.

    **Parameters**

    rspctl_cmd : string
        Command to run rspctl. Use a different value for testing.

    swlevel_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.        

    sleep_s : number
        Number of seconds to sleep before calling an rspctl
        command. Set to 0 to speed up unit tests, 1.0 during regular
        operations on a station.

    **Returns**

    An integer value 200 or 160, depending on the clock.

    **Raises**

    RuntimeError
        If abnormal conditions occur at the station

    **Examples**

    >>> get_clock_frequency_mhz(rspctl_cmd = 'test/rspctl', sleep_s = 1.0)
    200
    >>> get_clock_frequency_mhz(rspctl_cmd = 'test/rspctl-160', sleep_s = 0.0)
    160
    >>> get_clock_frequency_mhz(rspctl_cmd = 'test/rspctl-intl', sleep_s = 0.0)
    200
    >>> get_clock_frequency_mhz(rspctl_cmd = 'test/rspctl-not-locked', sleep_s = 0.0)
    Traceback (most recent call last):
    ...
    RuntimeError: No boards locked to clock
    >>> get_clock_frequency_mhz(rspctl_cmd = 'test/rspctl-no-output', swlevel_cmd = 'test/swlevel', sleep_s = 0.0)
    Traceback (most recent call last):
    ...
    RuntimeError: rspctl --tdstatus empty
   
    '''
    time.sleep(sleep_s)
    tdstatus     = check_output([rspctl_cmd, '--tdstatus'], timeout_s = 4.0).split('\n')
    if '\n'.join(tdstatus).strip() == '':
        if rspdriver_down(swlevel_cmd):
            raise RSPDriverDownError('rspctl --tdstatus empty; RSPDRIVER IS DOWN')
        else:
            raise RuntimeError('rspctl --tdstatus empty')
    locked_lines = [line for line in tdstatus if 'LOCKED' in line]
    clocks_mhz   = [int(line.split('|')[2].strip()) for line in locked_lines]
    if len(locked_lines) == 0:
        logging.error('%s --tdstatus:\n%s', rspctl_cmd, '\n'.join(tdstatus))
        raise RuntimeError('No boards locked to clock')
    clock_mhz = clocks_mhz[0]
    if not all([clock == clock_mhz for clock in clocks_mhz]):
        raise RuntimeError('Not all clocks locked to same frequency: %r' %
                           clocks_mhz)
    return clock_mhz




def wait_for_clocks_to_lock(rspctl_cmd  = '/opt/lofar/bin/rspctl',
                            swlevel_cmd = '/opt/lofar/bin/swlevel',
                            sleep_s = 5.0,
                            timeout_s = 120.0):
    r'''
    Block until the clocks are locked according to rspctl --tdstatus.

    **Parameters**

    rspctl_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    swlevel_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.        

    sleep_s : number
        Number of seconds to sleep after calling rspctl --tdstatus to
        see if the clocks are locked.

    timeout_s : number
        Maximum amount of seconds to try if clocks have locked. Raise
        a RuntimeError if clocks are not locked at end of timeout.

    **Returns**

    Clock frequency after locking.

    **Raises**

    RuntimeError
        If anything is out of the ordinary. In this case that is usually
        fatal because it probably means that the clocks did not (all)
        lock to the same frequency.

    **Examples**

    >>> wait_for_clocks_to_lock(rspctl_cmd = 'test/rspctl', sleep_s = 0.0)
    200
    >>> wait_for_clocks_to_lock(rspctl_cmd = 'test/rspctl-160', sleep_s = 0.0)
    160
    >>> wait_for_clocks_to_lock(rspctl_cmd = 'test/rspctl-no-output', swlevel_cmd = 'test/swlevel', sleep_s = 0.0)
    Traceback (most recent call last):
    ...
    RuntimeError: rspctl --tdstatus empty
    >>> wait_for_clocks_to_lock(rspctl_cmd = 'test/rspctl-not-locked', sleep_s = 1.0, timeout_s = 3.0)
    Traceback (most recent call last):
    ...
    RuntimeError: No boards locked to clock
    '''
    logging.info('Waiting for clocks to lock...')
    time_spent_s = 0.0
    while ('?' in check_output([rspctl_cmd, '--tdstatus'], timeout_s = 4.0)
           and time_spent_s <= timeout_s):
        logging.debug('still waiting for clocks to lock ...')
        time.sleep(sleep_s)
        time_spent_s += sleep_s
    if time_spent_s > timeout_s:
        logging.warning('wait_for_clocks_to_lock timed out after %r s',
                        time_spent_s)
    # get_clock_frequency_mhz will raise exception if not locked by now.
    return get_clock_frequency_mhz(rspctl_cmd, swlevel_cmd)





def set_clock_frequency_mhz(clock_mhz, rspctl_cmd = '/opt/lofar/bin/rspctl', timeout_s = 300):
    r'''
    Set the clock frequency in MHz. This function blocks until the
    clocks are properly locked.

    **Parameters**

    clock_mhz : int
        Either 200 or 160.

    rspctl_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    timeout_s : number
        Maximum amount of seconds to try if clocks have locked. Raise
        a RuntimeError if clocks are not locked at end of timeout.

        
    **Returns**

    The actual clock frequency after switching.

    
    **Raises**

    RuntimeError
        If the clock frequency after switching is not equal to the
        requested frequency.

    ValueError
        If clock_mhz not in [160, 200]

        
    **Examples**

    >>> set_clock_frequency_mhz(200, rspctl_cmd = 'test/rspctl', timeout_s = 2.0)
    200
    >>> set_clock_frequency_mhz(16, rspctl_cmd = 'test/rspctl', timeout_s = 2.0)
    Traceback (most recent call last):
    ...
    ValueError: clock_mhz (16) neither 160 nor 200
    
    Unfortunately we can't let an independent test script change
    clocks the next time it's called....
    
    >>> set_clock_frequency_mhz(160, rspctl_cmd = 'test/rspctl', timeout_s = 2.0)
    Traceback (most recent call last):
    ...
    RuntimeError: Clocks locked to 200 MHz instead of 160 MHz

    '''
    if clock_mhz not in [160, 200]:
        raise ValueError('clock_mhz (%d) neither 160 nor 200' % clock_mhz)

    current_clock_mhz = wait_for_clocks_to_lock(rspctl_cmd = rspctl_cmd,
                                                timeout_s  = timeout_s)
    if current_clock_mhz != clock_mhz:
        logging.info('Switching clock to %d MHz', clock_mhz)
        check_output([rspctl_cmd, '--clock', str(clock_mhz)], timeout_s = 1.0)
        time.sleep(1.0)
        current_clock_mhz = wait_for_clocks_to_lock(rspctl_cmd = rspctl_cmd,
                                                    timeout_s  = timeout_s)
        if current_clock_mhz != clock_mhz:
            raise RuntimeError('Clocks locked to %d MHz instead of %d MHz' %
                               (current_clock_mhz, clock_mhz))
        else:
            logging.info('Clocks locked to %d MHz', current_clock_mhz)
    return current_clock_mhz




def rspdriver_down(swlevel_cmd = '/opt/lofar/bin/swlevel'):
    r'''
    '''
    swlevel_lines        = check_swlevel_output(swlevel_cmd).split('\n')
    rsp_driver_pid_lines = [line for line in swlevel_lines
                            if 'RSPDriver' in line and not 'Missing' in line]
    if len(rsp_driver_pid_lines) > 0:
        return rsp_driver_pid_lines[0].split()[-1].strip() == 'DOWN'
    else:
        return True
    




def restart_rsp_driver(lofar_log_dir,
                       rspdriver_cmd = '/opt/lofar/bin/RSPDriver',
                       swlevel_cmd   = '/opt/lofar/bin/swlevel',
                       sudo_cmd      = '/usr/bin/sudo'):
    r'''
    Kill the running RSPDriver (if any), and restart it in the
    background using "sudo -b" if the RSPDriver was already running
    when this function was called. The aim of this function is to have
    the RSPDriver re-initialize itself without reloading the firmware
    images, as is done by ``swlevel``.

    Note the call to sudo -b in the body of this function. It is very
    important to redirect stdin, stdout, and stderr there. If not
    done, remote shells such as ssh and lcurun will hang after
    ppstune.py finishes, because there are still open streams because
    of the RSPDriver in the background.

    **Parameters**

    rspdriver_cmd : string
        Full path to the RSPDriver executable.

    swlevel_cmd : string
        The name of the shell command to run as swlevel. Set a
        different value for local off-station testing.

    sudo_cmd : string
        The name of the shell command to run as sudo. Set a different
        value for local off-station testing.

    **Returns**

    None.

    **Raises**

    OSError
        If it can't find executables.

    RuntimeError
        If the swlevel output is mangled.

    **Examples**

    >>> restart_rsp_driver(lofar_log_dir = 'test',
    ...                    rspdriver_cmd = 'test/RSPDriver', 
    ...                    swlevel_cmd = 'test/swlevel', sudo_cmd = 'test/sudo')
    >>> restart_rsp_driver('test', swlevel_cmd = 'test/swlevel',
    ...                    sudo_cmd = 'test/sudo')
    Traceback (most recent call last):
    ...
    OSError: /opt/lofar/bin/RSPDriver does not exist
    '''

    swlevel_lines        = check_swlevel_output(swlevel_cmd).split('\n')
    rsp_driver_pid_lines = [line for line in swlevel_lines
                            if 'RSPDriver' in line and not 'Missing' in line]
    current_swlevel = get_swlevel(swlevel_cmd)
    if  abs(current_swlevel) < 2:
        logging.warning('swlevel (%r) < 2. Will not start RSPDriver now',
                        current_swlevel)
        return None
    if len(rsp_driver_pid_lines) == 1:
        if rsp_driver_pid_lines[0].split()[-1].strip() != 'DOWN':
            rsp_pid = int(rsp_driver_pid_lines[0].split()[-1])
            logging.info('Killing RSPDriver with PID %d', rsp_pid)
            output = subprocess.Popen([sudo_cmd, 'kill', '-15', str(rsp_pid)],
                        stdin  = subprocess.PIPE,
                        stdout = subprocess.PIPE).communicate(input = '\n')[0]
            time.sleep(1.0)
            if len(output) != 0:
                raise OSError('Failed to kill RSPDriver with PID %d:\n%s' %
                              (rsp_pid, output))
    
        if os.path.exists(rspdriver_cmd):
            logging.info('Starting RSPDriver')
            command = ('%s -b %s < %s 1>> %s 2>&1' %
                       (sudo_cmd, rspdriver_cmd, os.devnull,
                        os.path.join(lofar_log_dir, 'RSPDriver.stdout')))
            logging.info('$ %s', command) 
            subprocess.Popen(command, shell = True)
            time.sleep(1.0)
            return None
        else:
            raise OSError('%s does not exist' % rspdriver_cmd)
    else:
        raise RuntimeError('Wrong RSPDriver line(s) from swlevel: "%s"' %
                           '\n'.join(rsp_driver_pid_lines))
            
                

    

def set_sync_delay(rsp_boards, edge = 'rising', mode = 'reset', execute = True):
    r'''
    Increment or reset the synchronisation delay, using the rising or
    falling flank of the clock pulse.

    **Parameters**

    rsp_boards : string
        Comma separated list of rsp boards to use in the test. Form of
        the list: 'rsp0,rsp1,rsp2'

    edge : string
        'rising' or 'falling'

    mode : string
        'reset' or 'increment'

    execute : bool
        True: execute the command, False: log command line and exit.

    **Returns**

    None

    **Raises**

    ValueError
        In case of problems with the parameters.

    **Examples**

    Only set ``execute`` to ``False`` for examples and test cases. In
    real use, execute must be ``True``.

    >>> set_sync_delay('rsp0,rsp1', edge = 'rising', mode = 'reset', execute = False)
    python2 verify.py --brd rsp0,rsp1 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 0

    >>> set_sync_delay('rsp0,rsp1', edge = 'rising', mode = 'increment', execute = False)
    python2 verify.py --brd rsp0,rsp1 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1

    >>> set_sync_delay('rsp2,rsp4', edge = 'falling', mode = 'reset', execute = False)
    python2 verify.py --brd rsp2,rsp4 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge f --pps_delay 0

    >>> set_sync_delay('rsp2,rsp4', edge = 'falling', mode = 'increment', execute = False)
    python2 verify.py --brd rsp2,rsp4 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge f --pps_delay 1

    >>> set_sync_delay('rsp2,rsp4', edge = 'first', mode = 'increment', execute = False)
    Traceback (most recent call last):
    ...
    ValueError: edge must be 'rising' or 'falling', not 'first'

    >>> set_sync_delay('rsp2,rsp4', edge = 'rising', mode = 'increase', execute = False)
    Traceback (most recent call last):
    ...
    ValueError: mode must be 'reset' or 'increment', not 'increase'

    '''
    if len(edge) == 0 or edge.lower() not in ['rising', 'falling']:
        raise ValueError('edge must be \'rising\' or \'falling\', not %r' %
                         edge)
    if len(mode) == 0 or mode.lower() not in ['reset', 'increment']:
        raise ValueError('mode must be \'reset\' or \'increment\', not %r' %
                         mode)
    
    command_line = ['python2'    , 'verify.py',
                    '--brd'      , rsp_boards,
                    '--fpga'     , 'blp0,blp1,blp2,blp3',
                    '--te'       , 'tc/sync_delay.py',
                    '--pps_edge' , edge.lower()[0],
                    '--pps_delay', str(['reset', 'increment'].index(mode))]
    if not execute:
        print(' '.join(command_line))
    logging.debug(check_output(command_line, execute = execute, timeout_s = 4.0))
    


def parse_rspctl_status_diff_line(line):
    r'''
    Parse ``rspctl --status line`` containing diff information into a
    dict with keywords 'rsp', 'sync', 'diff', 'count', 'samples',
    'slices'. This is a helper function for rspctl_status_diffs().

    **Parameters**

    line : string
        One line containing diff/samples/count/slices info.

    **Returns**

    A dict containing the parsed information from the line.

    **Raises**

    ValueError
        If the line cannot be parsed correctly.

    **Examples**

    >>> parse_rspctl_status_diff_line('RSP[ 5]     1:          0      15174 2553089024     195312\n')
    {'count': 15174, 'slices': 195312, 'sync': 1, 'samples': 2553089024, 'diff': 0, 'rsp': 5}
    >>> parse_rspctl_status_diff_line('RSP[10] Sync         diff      count    samples     slices\n')
    Traceback (most recent call last):
    ...
    ValueError: invalid literal for int() with base 10: 'Sync'

    >>> parse_rspctl_status_diff_line('RSP[10] Sync  What       diff\n')
    Traceback (most recent call last):
    ...
    ValueError: Cannot parse 'RSP[10] Sync  What       diff\n' for diffs and slices

    '''
    sanitized   = line.replace('[', ' ').replace(']', ' ').replace(':', ' ')
    words       = sanitized.strip().split()
    if len(words) != 7 or words[0] != 'RSP':
        raise ValueError('Cannot parse %r for diffs and slices' % line)
    
    return {'rsp'    : int(words[1]),
            'sync'   : int(words[2]),
            'diff'   : int(words[3]),
            'count'  : int(words[4]),
            'samples': int(words[5]),
            'slices' : int(words[6])}



def rspctl_status_diffs(rspctl_cmd = '/opt/lofar/bin/rspctl', sleep_s = 1.0, timeout_s = 3.0):
    r'''
    Analyze the output of ``rspctl --status`` to obtain the diffs and
    determine if we are measuring the diffs in an even or an odd
    second since startup of the boards.

    **Parameters**

    rspctl_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    sleep_s : number
        Number of seconds to sleep before executing the rspctl
        command. When testing, set to zero. The default is 1 second.

    timeout_s : float
        If the rspctl command has not returned after ``timeout_s``
        seconds, raise a RuntimeError. rspctl commands should 
        typically return after at most one second.
    
    **Returns**

    A tuple (string, list of ints), where the string is either 'even',
    or 'odd', and the list of ints contains the diffs for each Antenna
    Processor (AP). The list is therefore four times the number of RSP
    boards long.

    **Raises**

    RuntimeError
        If the slices do not all have the same value.

    **Examples**

    >>> rspctl_status_diffs('test/rspctl-odd', sleep_s = 0.0)
    ('odd', [512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512, 511, 512, 512, 512, 512, 511, 512, 512, 512, 512, 513, 512, 512, 512, 512, 513])
    >>> rspctl_status_diffs('test/rspctl-even', sleep_s = 0.0)
    ('even', [0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])
    '''
    time.sleep(sleep_s)
    rspctl = check_output([rspctl_cmd, '--status'], timeout_s = timeout_s)
    rspctl_lines  = rspctl.split('\n')
    diff_header_line_no = [line_no
                           for line_no, line in enumerate(rspctl_lines)
                           if 'Sync' in line and 'diff' in line]
    diff_lines = flatten_list([rspctl_lines[line_no + 1 : line_no + 5]
                               for line_no in diff_header_line_no])
    records    = [parse_rspctl_status_diff_line(line) for line in diff_lines]
    diffs      = [record['diff']             for record in records]
    even       = [record['slices'] == 195312 for record in records]
    odd        = [record['slices'] == 195313 for record in records]

    if all(even):
        return ('even', diffs)
    elif all(odd):
        return ('odd', diffs)
    else:
        raise RuntimeError('Measurement not in one second')
        

def fan_state(state_byte):
    r'''
    Returns a string indicating the fan state in a LOFAR station.

    **Parameters**

    state_byte : int
    
        The fan state byte. This is a bitmask, where a set bit
        indicates that the fan is on. The bits have the following
        meaning:
        - bit 0 outer fan front
        - bit 1 inner fan front
        - bit 2 inner fan back
        - bit 4 outer fan back

    **Returns**

    A four characyter string representing the state of the fans. From
    left to right outer front, inner front, inner back, and outer back
    fans.

    **Examples**

    >>> fan_state(8)
    '...o'
    >>> fan_state(4)
    '..o.'
    >>> fan_state(2)
    '.o..'
    >>> fan_state(1)
    'o...'
    >>> fan_state(0)
    '....'
    >>> fan_state(4+1)
    'o.o.'
    '''
    return ''.join(['.o'[(state_byte >> bit)&1] for bit in range(4)])
    



def statusdata_command(station):
    r'''
    Returns the script to request cabinet climate data.

    **Parameters**

    station : string
        Name of the station

    **Returns**

    A string containing the command name.

    **Examples**

    >>> statusdata_command('CS101')
    'test/envcontroltest/nlStatusData.py'
    >>> statusdata_command('RS509')
    'test/envcontroltest/nlStatusData.py'
    >>> statusdata_command('FR606')
    'test/envcontroltest/isStatusData.py'
    >>> statusdata_command('brazil_101')
    'test/envcontroltest/isStatusData.py'
    '''
    if is_nl(station):
        return '/opt/lofar/sbin/nlStatusData.py'
    else:
        return '/opt/lofar/sbin/isStatusData.py'


def cabinet_climate(statusdata_cmd):
    r'''
    stdout format::
    
        time [0] data_cab0 [1] data_cab1 [3] data_cab3

    values in  data_cabx::
    
        setpoint temperature humidity fansstate heaterstate
        temperature : actual temperature in cabinet 
        humidity    : actual humidity in cabinet
        fanstate    : which fans are on
                      bit 0 outer fan front
                      bit 1 inner fan front
                      bit 2 inner fan back
                      bit 4 outer fan back
        heaterstate : only available in cabinet 3
                      0 = off
                      1 = on

    example, returned data::
    
        1333702601 [0] 24.71 16.81 4 0 [1] 24.72 43.36 4 0 [3] 14.69 41.73 2 0

    **Example**

    >>> cabinet_climate('test/envcontroltest/nlStatusData.py')
    [{'heater': False, 'temperature': 24.71, 'humidity': 16.81, 'fans': '..o.', 'date': (2012, 4, 6, 8, 56, 41), 'cabinet': 0}, {'heater': False, 'temperature': 24.72, 'humidity': 43.36, 'fans': '..o.', 'date': (2012, 4, 6, 8, 56, 41), 'cabinet': 1}, {'heater': False, 'temperature': 14.69, 'humidity': 41.73, 'fans': '.o..', 'date': (2012, 4, 6, 8, 56, 41), 'cabinet': 3}]
    >>> cabinet_climate('test/envcontroltest/isStatusData.py')
    [{'heater': False, 'temperature': 14.69, 'humidity': 41.73, 'fans': '.o..', 'date': (2012, 4, 6, 8, 56, 41), 'cabinet': 3}]

    '''
    logging.debug('cabinet_climate(%r)', statusdata_cmd)
    if os.path.exists(statusdata_cmd): 
        try:
            line     =  check_output([statusdata_cmd], timeout_s = 10.0).split('\n')[0]
            words    = [word.strip() for word in line.split()]
            logging.debug('cabinet_climate(): words = %r', words)
            cabinets = [int(word.strip('[]')) for word in words
                        if word[0] == '[' and word[-1] == ']']
            date_s   = float(words[0])

            cabinet_info = []
            for index, cabinet in enumerate(cabinets):
                sublist = words[1+index*5:1+(index+1)*5]
                cabinet_info.append({'cabinet'     : cabinet,
                                     'temperature' : float(sublist[1]),
                                     'humidity'    : float(sublist[2]),
                                     'fans'        : fan_state(int(sublist[3])),
                                     'heater'      : bool(int(sublist[4])),
                                     'date'        : gmtime_tuple(date_s)
                                     })
            return sorted(cabinet_info, key = lambda item : item['cabinet'])
        except RuntimeError:
            logging.warning('Caught %s', str(sys.exc_info()[0]))
            logging.warning(str(sys.exc_info()[1]))
            return []

    else:
        logging.warn('cabinet_climate(): %s does not exists',
                     statusdata_cmd)
        return []



def rubidium_log_file(date):
    r'''
    Return the file name of today's Rubidium log file.

    **Parameters**

    date : float
        Seconds since 1970.0. Can be obtained by calling time.time()

    **Examples**

    >>> rubidium_log_file(date = 1358862518.0424139)
    '/var/log/ntpstats/rubidium_log.20130122'
    '''
    
    return os.path.join('/var', 'log', 'ntpstats',
                        'rubidium_log.%4d%02d%02d' % gmtime_tuple(date)[0:3])



def rubidium_temperature_c(rubidium_log):
    r'''
    Return the most recently measured rubidium case temperature in
    degrees Celsius, as given by the AD10 variable, which lists the
    case temperature in degrees Celsius, divided by 100.0

    **Parameters**

    rubidium_log : string
        Rubidium clock statistics log file. Obtain this file name by
        calling rubidium_log_file().

    **Returns**
    
    A (string, string, float) tuple containing the (date, time,
    temperature). A temperature of -270.0 is returned if the rubidium
    log is empty, or non-existent.

    **Examples**

    >>> rubidium_temperature_c('testdata/rubidium_log.20120412')
    ('2012-04-12', '10:50:25.820584', 71.6)
    >>> rubidium_temperature_c('testdata/rubidium_log.20120413')
    ('2012-04-13', '13:05:02.757915', 73.2)
    >>> rubidium_temperature_c('testdata/rubidium_log.empty')[-1]
    -270.0
    '''
    logging.debug('rubidium_case_temperature(%r)', rubidium_log)
    gmtime       = gmtime_tuple(time.time())
    logging.debug('gmtime: %r', gmtime)
    error_output = ('%4d-%02d-%02d'  % gmtime[0:3],
                    '%02d:%02d:%02d' % gmtime[3:],
                    -270.0)
    if not os.path.exists(rubidium_log):
        logging.warning('%s does not exists; No up-to-date Rubidium info',
                        rubidium_log)
        return error_output
        
    ad10_grep = subprocess.Popen('tail -200 '+rubidium_log+'|grep AD10', shell = True,
                                stdout = subprocess.PIPE).communicate()[0].strip()
    logging.debug('ad10_grep:\n%s', ad10_grep)
    
    lines = [line for line in ad10_grep.split('\n') if line.strip() != '']
    logging.debug('lines:\n%s', lines)
    
    if len(lines) == 0 or (len(lines) == 1 and len(lines[0]) == 0):
        logging.warning('No AD10 info in %s', rubidium_log)
        return error_output
    else:
        good_lines = [line for line in lines if 'ad10 fail' not in line.lower()]
        if len(good_lines) == 0:
            logging.warning('AD10 Fail in %s', rubidium_log)
            return error_output
        items = [item.strip() for item in good_lines[-1].split(';')]
        logging.debug('rubidium_case_temperature(): items = %r', items)
        date_utc, time_utc = tuple(items[0].split('T'))
        logging.debug('rubidium_case_temperature(): date_utc = %r', date_utc)
        logging.debug('rubidium_case_temperature(): time_utc = %r', time_utc)
        ad10 = [item.split()[1]
                for item in items if 'AD10' in item][0]
        if ad10.lower() == 'fail':
            logging.warning('AD10 Fail in %s', rubidium_log)
            return error_output
        else:
            temperature_c = 100.0*float(ad10)
        return (date_utc, time_utc, temperature_c)



def log_cabinet_climate(station):
    r'''
    Write cabinet climate info, obtained by calling
    ``cabinet_climate()``, and rubidium case temperatures, obtained by
    calling ``rubidium_temperature_c()``, to the ``logging`` module.

    **Parameters**

    station : string
        Name of the station for which to write the climate info. 

    **Returns**

    None
    '''
    cab_format = ', '.join(['Cabinet %d: temperature %5.2f C',
                            'humidity %5.1f %%',
                            'dew point %5.1f C',
                            'fans %s', 'heat %3s'])
    for cab in cabinet_climate(statusdata_command(station)):
        logging.info(
            cab_format,
            cab['cabinet'], cab['temperature'],
            cab['humidity'],
            dew_point_temperature_c(cab['temperature'], cab['humidity']),
            cab['fans'],
            ['off', 'ON'][int(cab['heater'])])
    logging.info('Rubidum case temperature at %s %s: %5.2f C',
                 *rubidium_temperature_c(rubidium_log_file(date = time.time())))
    return None



####################################
#                                  #
#       Station information        #
#                                  #
####################################

def station_name(hostname = check_output(['hostname', '-s'])):
    r'''
    Determine station_name by calling the external ``hostname``
    utility, and stripping the last character.

    **Returns**

    A string containing an upper case station name.

    **Examples**

    >>> station_name('CS103C')
    'CS103'
    >>> station_name('RS406C')
    'RS406'
    >>> station_name('de603c')
    'DE603'
    >>> station_name('somehost\n')
    'SOMEHOS'
    '''
    return hostname.upper().strip()[:-1]



def is_nl(station):
    r'''
    Return True if ``station`` is Dutch, False otherwise.

    **Parameters**

    station : string
        Name of the station as returned by ``station_name()``

    **Examples**

    >>> is_nl('cs103')
    True
    >>> is_nl('RS205')
    True
    >>> is_nl('UK608')
    False
    >>> is_nl('brazil_101')
    False
    '''
    valid_number = True
    try:
        int(station[2:])
    except ValueError:
        valid_number = False

    return (len(station) == 5 
            and station.upper()[0:2] in ['RS', 'CS']
            and valid_number)




def rsp_boards_in_station(remote_station_conf='/opt/lofar/etc/RemoteStation.conf'):
    r'''
    return number of RSP boards in ``station``; 12 in case of Dutch
    stations, 24 in case of international stations.

    **Parameters**

    remote_station_conf : string
        Name of RemoteStation.conf file.

    **Returns**

    The number of RSP boards as an integer.

    **Raises**
    
    MalformedRemoteStationConfError
        If the number of RSP boards can not be read from RemoteStation.conf.

    **Examples**

    >>> rsp_boards_in_station('test/CS021-RemoteStation.conf')
    12
    >>> rsp_boards_in_station('test/RS406-RemoteStation.conf')
    12
    >>> rsp_boards_in_station('test/DE601-RemoteStation.conf')
    24
    >>> rsp_boards_in_station('test/Broken-RemoteStation.conf')
    Traceback (most recent call last):
    ...
    MalformedRemoteStationConfError: Malformed RemoteStation.conf at test/Broken-RemoteStation.conf
    '''
    if os.path.exists(remote_station_conf):
        n_rsp_line = [int(line.split('=')[1])
                      for line in open(remote_station_conf)
                      if 'N_RSPBOARDS' in line]
        if len(n_rsp_line) == 1:
            return n_rsp_line[0]
        else:
            logging.error('Malformed RemoteStation.conf at %s', remote_station_conf)
            raise MalformedRemoteStationConfError('Malformed RemoteStation.conf at %s' % remote_station_conf)
    else:
        logging.warning('Cannot find %s; using host name to derive number of RSP boards', remote_station_conf)
        station = station_name()
        if is_nl(station):
            return 12
        elif station.upper() == 'FI609':
            return 12
        else:
            return 24



def rsp_list(num_rsp_boards):
    r'''
    Returns a string of the form rsp0,rsp1,rsp....,rsp(n-1), where n
    is ``num_rsp_boards``

    **Parameters**

    num_rsp_boards : int
        Total number of RSP boards in a station. Use 12 for Dutch
        stations and 24 for international stations.

    **Returns**

    A string that can be used in a call to
    ``/opt/stationtest/verify.py``.

    **Examples**

    >>> rsp_list(12)
    'rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11'
    '''
    return ','.join(['rsp%d' % rsp for rsp in range(num_rsp_boards)])




def measure_diff_stability(clock_mhz, repeat = 10,
                           measure_diffs_fn = rspctl_status_diffs):
    r'''
    Measure the number of times that the diffs of a certain AP are
    different from the reference diffs.

    **Parameters**

    clock_mhz : int
        Clock frequency in MHz.

    repeat : int
        Number of attempts to measure stability.

    measure_diffs_fn : function
        The function to call for obtaining measured diffs. Leave alone
        in normal use. This parameter is very useful for testing, as
        shown in the example below.

    **Returns**

    A tuple (list, list) containing

    first) A list of integers indicating how many times a certain AP
           has shown a diff that was not equal to the expected
           ``reference_diffs``.

    second) A list of integers containing the median diff value.
     

    **Examples**

    >>> def test_diffs_fn():
    ...     for i in range(10):
    ...         if i % 2 == 0:
    ...             rspctl = 'test/rspctl-even'
    ...         else:
    ...             rspctl = 'test/rspctl-odd'
    ...         yield(rspctl_status_diffs(rspctl, sleep_s  = 0.0))
    >>> test_iterator = test_diffs_fn()
    >>> measure_diff_stability(clock_mhz = 200, repeat = 10, measure_diffs_fn  = test_iterator.next)
    ([0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5], [0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 199999999, 0, 0, 0, 0, 199999999, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1])
    '''
    diffs        = []
    for cycle in range(repeat):
        (parity, measured_diffs) = measure_diffs_fn()
        if parity == 'odd':
            measured_diffs = [diff - 512 for diff in measured_diffs]
        logging.debug('Attempt %02d meas: %r', cycle, measured_diffs)
        normalized_diffs = []
        for diff in measured_diffs:
            if diff < 0:
                normalized_diffs.append(diff+clock_mhz*1000000)
            else:
                normalized_diffs.append(diff)
        logging.debug('Attempt %02d norm: %r', cycle, normalized_diffs)
        diffs.append(normalized_diffs)

    transposed = transpose_lists(diffs)
    medians    = [sorted(row)[repeat/2] for row in transposed]
    deviating  = [sum([diff != median for diff in row])
                  for median, row in zip(medians, transposed)]
    return deviating, medians




def measure_all_delays(station, clock_mhz,
                       edge = 'rising', repeat = 10,
                       rspctl_cmd = '/opt/lofar/bin/rspctl',
                       first_delay_step = 0,
                       one_past_last_delay_step = 64,
                       remote_station_conf='/opt/lofar/etc/RemoteStation.conf'):
    r'''
    TODO: fix measure_all_delays() docs
    
    Cycle through all 64 delay steps and measure diff failures. The
    function uses the following procedure:

    1) reset sync delays to 0
    2) for each delay step:
        1) measure diff stability ``repeat`` times
        2) increment sync delays by one step
    3) reset sync delays to 0     

    **Parameters**

    station : string
        Station name.

    clock_mhz : int
        Clock frequency in MHz. Must be 160 or 200 MHz.

    edge : string
        Edge of the clock flank at which to trigger. Pick one of
        'rising' or 'falling'.
        
    repeat : int
        Number of attempts to measure stability at each delay setting.

    rspctl_cmd : string
        Command to run rspctl. Use a different value for testing.        

    num_delay_steps : int
        Number of 0.078 ns delay steps to scan. Default is the maximum
        of 64 at LOFAR RSP boards.


    **Returns**

    A list of 64 lists of 48 (or 96) elements each containing the
    total number of failures at each delay for each Antenna Processor
    (ap). Indexing: ``failures[delay_step][antenna_processor]``.


    **Raises**

    ValueError
        In case of problems with the input parameters.

    RuntimeError
        In case of trouble with underlying rspctl calls.
    
    **Examples**

    >>> measure_all_delays('CS021', clock_mhz = 200, edge = 'rising', repeat = 3,
    ...                    rspctl_cmd = 'test/rspctl-odd', first_delay_step = 5,
    ...                    one_past_last_delay_step = 7,
    ...                    remote_station_conf='test/CS021-RemoteStation.conf')
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 0
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 1
    python2 verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/sync_delay.py --pps_edge r --pps_delay 0
    [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0], [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]]

    Of course, one has to take care with the inputs:

    >>> measure_all_delays('CS021', clock_mhz = 20, edge = 'rising', repeat = 3,
    ...                    rspctl_cmd = 'test/rspctl-odd', one_past_last_delay_step = 2,
    ...                    remote_station_conf='test/CS021-RemoteStation.conf')
    Traceback (most recent call last):
    ...
    ValueError: clock_mhz (20) not in [160, 200]
    >>> measure_all_delays('CS021', clock_mhz = 200, edge = 'rinsing', repeat = 3,
    ...                    rspctl_cmd = 'test/rspctl-odd', one_past_last_delay_step = 2,
    ...                    remote_station_conf='test/CS021-RemoteStation.conf')
    Traceback (most recent call last):
    ...
    ValueError: edge ('rinsing') not in ['rising', 'falling']

    '''
    if edge not in ['rising', 'falling']:
        raise ValueError('edge (%r) not in [\'rising\', \'falling\']' % edge)
    if clock_mhz not in [160, 200]:
        raise ValueError('clock_mhz (%r) not in [160, 200]' % clock_mhz)

    def meas_fn():
        return rspctl_status_diffs(rspctl_cmd = rspctl_cmd)

    logging.info('Using %s flank of clock pulse', edge)

    rsp_string = rsp_list(rsp_boards_in_station(remote_station_conf))
    logging.info('Using boards %s', rsp_string)

    logging.info('Resetting sync delay to 0')
    set_sync_delay(rsp_string, edge = edge, mode = 'reset',
                   execute = (rspctl_cmd == '/opt/lofar/bin/rspctl'))
    failure_history = []
    previous_medians = None
    # Go to first delay step.
    for delay_step in range(first_delay_step):
        set_sync_delay(rsp_string, edge = edge, mode = 'increment',
                       execute = (rspctl_cmd == '/opt/lofar/bin/rspctl'))
        
    for delay_step in range(first_delay_step, one_past_last_delay_step):
        logging.info('Delay step: %3d', delay_step)
        
        failed_attempts, medians  = measure_diff_stability(
            clock_mhz        = clock_mhz,
            repeat           = repeat,
            measure_diffs_fn = meas_fn)
        # Penalty if no failures occurred, but diff is not the same as
        # previously.
        if previous_medians is not None:
            for index in range(len(failed_attempts)):
                if medians[index] != previous_medians[index]:
                    fails = failed_attempts[index]
                    failed_attempts[index] = min(repeat, fails + 1 + repeat/2)
        previous_medians = medians
            
        logging.info('Diff errors %3d: [%s]', delay_step,
                     ' '.join(['%2d' % fails 
                               for fails in failed_attempts]))
        logging.debug('Median diff %3d: [%s]', delay_step,
                     ' '.join(['%2d' % median 
                               for median in medians]))
        failure_history.append(failed_attempts)
        logging.debug('Incrementing sync delay')
        set_sync_delay(rsp_string, edge = edge, mode = 'increment',
                       execute = (rspctl_cmd == '/opt/lofar/bin/rspctl'))
    
    set_sync_delay(rsp_string, edge = edge, mode = 'reset',
                   execute = (rspctl_cmd == '/opt/lofar/bin/rspctl'))
    return failure_history


####################################
#                                  #
#      Analysis and reporting      #
#                                  #
####################################


def sync_failure_report(diff_error_counts):
    r'''
    Format an ASCII report of the diff error counts.

    **Parameters**

    diff_error_counts : list of lists
        A list with 64 lists of diff error counts. One list per delay
        step. Each list contains the diff error count for each Antenna
        Processor at a certain delay step.

    **Returns**

    A string containing the report.

    **Examples**

    >>> fails = eval(open('testdata/cs030-fails.txt').read())
    >>> abbreviated = [fail[0:20] for fail in fails[0:48]]
    >>> print(sync_failure_report(abbreviated))
    Delay/AP 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19
       0   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       1   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       2   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       3   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       4   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       5   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       6   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       7   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       8   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
       9   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      10   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      11   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      12   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      13   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      14   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      15   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      16   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0
      17   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  7  0  0  0
      18   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10  0  0  0
      19   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10  0  0  0
      20   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10  0  0  0
      21   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10  7  0  0
      22   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  0  0
      23   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  0  0
      24   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  0  0
      25   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  0  0
      26   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  0  0
      27   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  0  0
      28   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10  6  0
      29   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10 10  0
      30   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10 10  0
      31   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10 10  0
      32   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10 10  0
      33   :  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0 10 10 10  1
      34   :  3  0  0  0  0  0  0  0  1  0  0  0  5  0  0  0 10 10 10 10
      35   : 10  0  0  0  0  0  0  0 10  0  0  0 10  0  0  0 10 10 10 10
      36   : 10  0  0  0  5  0  0  0 10  0  0  0 10  0  0  0 10 10 10 10
      37   : 10  0  0  0 10  0  0  0 10  0  0  0 10  0  0  0 10 10 10 10
      38   : 10  0  0  0 10  1  0  0 10  0  0  0 10  4  0  0 10 10 10 10
      39   : 10  0  0  0 10  8  0  0 10  0  0  0 10 10  0  0 10 10 10 10
      40   : 10  0  0  0 10 10  0  0 10  2  0  0 10 10  0  0 10 10 10 10
      41   : 10  2  0  0 10 10  0  0 10 10  0  0 10 10  0  0 10 10 10 10
      42   : 10  8  0  0 10 10  0  0 10 10  0  0 10 10  0  0 10 10 10 10
      43   : 10 10  0  0 10 10  0  0 10 10  0  0 10 10  0  0 10 10 10 10
      44   : 10 10  0  0 10 10  0  0 10 10  0  0 10 10  0  0 10 10 10 10
      45   : 10 10  0  0 10 10  0  0 10 10  0  0 10 10  0  0 10 10 10 10
      46   : 10 10  0  0 10 10  2  0 10 10  0  0 10 10  1  0 10 10 10 10
      47   : 10 10  3  0 10 10 10  0 10 10  1  0 10 10 10  0 10 10 10 10
    '''
    num_aps = len(diff_error_counts[0])
    result = 'Delay/AP '+(' '.join([ ('%02d' % ap) 
                                     for ap in range(num_aps)]))+'\n'
    
    for delay, errors_at_delay in enumerate(diff_error_counts):
        result += ' %3d   : ' % delay
        result += ' '.join([ ('%2d' % errors) for errors in errors_at_delay])
        result += '\n'
    return result[:-1]



def distance_forward(sequence, item):
    r'''
    Compute the number of elements one has to advance to find ``item``
    in ``sequence`` for each element in the sequence.

    **Parameters**

    sequence : sequence
        The sequence to scan.

    item : object
        The instance to search for.

    **Returns**

    A list of integers with the same length as ``sequence``. If an
    ``item`` is not found in forward direction, substitute the length
    of the ``sequence``.
    
    **Examples**

    >>> distance_forward([0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1], item = 1)
    [3, 2, 1, 0, 0, 4, 3, 2, 1, 0, 0]
    >>> distance_forward([0, 0, 0, 1, 1, 0, 0, 0, 0], item = 1)
    [3, 2, 1, 0, 0, 9, 9, 9, 9]
    >>> distance_forward('elephant', item = 'h')
    [4, 3, 2, 1, 0, 8, 8, 8]
    
    '''
    distance = []
    index    = 0
    length   = len(sequence)
    while index < length:
        if sequence[index] != item:
            try:
                next_false  = sequence[index:].index(item)
                for distance_to_next in range(next_false, 0, -1):
                    distance.append(distance_to_next)
                    index += 1
            except ValueError:
                while index < length:
                    distance.append(length)
                    index += 1
        else:
            distance.append(0)
            index += 1
    return distance
        
    
    
    


def ap_optimal_delay_step(ap_failures, cycle_length = 67):
    r'''
    Determine optimal delay step for a single Antenna Processor. If
    several indices are equally good, it returns the lowest index. If
    the sequence of zeros is longer than half the total length of the
    array, it is assumed that the minimum is roughly half the total
    lenth away from the nearest errors.

    **Parameters**

    ap_failures : list of ints
        Number of diff failures for one AP at each delay step.

    cycle_length : int
        Number of steps in a full clock cycle. 67 in case of BOTH
        LOFAR clocks.

    **Returns**

    An int containing the index of the point that is farthest away
    from any failures.

    **Examples**

    >>> ap_optimal_delay_step([0,0,0,0,0,0,0])
    0

    Here is a very long sequence of zeros:

    >>> ap_optimal_delay_step([0,0,0,0,0,0,0,0,0,0,0,6,10], cycle_length = 17)
    3
    >>> ap_optimal_delay_step([0,0,0,0,0,0,0,0,8,10,10,10,10,10], cycle_length = 8)
    4

    And normal examples again

    >>> ap_optimal_delay_step([10,9,0,0,0,0,0,1,8,10,10,10,10])
    4
    >>> ap_optimal_delay_step([10,9,3,1,0,0,0,1,8,0,0,0,0])
    12
    >>> ap_optimal_delay_step([10,9,3,1,0,0,0,1,8,0,0])
    5
    >>> ap_optimal_delay_step([2,1,3,4,5,7])
    1
    '''
    logging.debug('ap_optimal_delay_step(%r, %r)', ap_failures, cycle_length)
    minimum     = min(ap_failures)
    if minimum != 0:
        logging.warn('Minimum number of fails %d > 0', minimum)
    at_minimum = [fails == minimum for fails in ap_failures]
    
    if all(at_minimum):
        return 0

    d_forward = distance_forward(at_minimum, False)
    d_reverse = distance_forward(at_minimum[::-1], False)[::-1]
    
    distance     = [min(d_for, d_rev)
                    for d_for, d_rev in zip(d_forward, d_reverse)]
    modified_distance = []
    for dist in distance:
        if dist <= cycle_length/2:
            modified_distance.append(dist)
        else:
            modified_distance.append(cycle_length - dist)
    max_distance = max(modified_distance)
    return distance.index(max_distance)




def find_optimal_delays(diff_error_counts):
    r'''
    Determine optimal delay step by computing which delay step has the
    largest distance from any diff errors.

    **Parameters**

    diff_error_counts : list of lists of ints
        Indexing: ``diff_error_counts[delay_step][antenna_processor]``
        is the number of diff failures of an ``antenna_processor`` at
        a certain ``delay_step``.

    **Returns**

    A list of ints with the optimal delay step for each Antenna
    Processor.

    **Examples**

    >>> diff_error_counts = eval(open('testdata/rs106-fails.txt').read())
    >>> find_optimal_delays(diff_error_counts)
    [28, 33, 39, 45, 29, 34, 42, 46, 27, 34, 41, 46, 26, 33, 40, 46, 27, 35, 42, 46, 30, 35, 63, 46, 28, 63, 42, 63, 27, 29, 41, 46, 31, 36, 44, 49, 30, 35, 43, 49, 33, 39, 44, 49, 29, 36, 41, 46]
    '''
    logging.debug('find_optimal_delays(%r)', diff_error_counts)
    diff_errors_t = transpose_lists(diff_error_counts)
    optimal_steps = [ap_optimal_delay_step(ap_failures)
                     for ap_failures in diff_errors_t]
    logging.info('Raw optimum: [%s]',
                 ' '.join([('%3d' % step) for step in optimal_steps]))
    median_optimal_step = sorted(optimal_steps)[len(optimal_steps)/2]
    median_optimal_step = median_optimal_step % 66
    if median_optimal_step > 65:
        median_optimal_step = 0
    elif median_optimal_step > 63:
        median_optimal_step = 63
    logging.info('median_optimal_step = %r', median_optimal_step)

    corrected = []
    for step in optimal_steps:
        corr = step % 66
        # Avoid impossible tuning range
        if corr > 65:
            corr = 0
        elif corr > 63:
            corr = 63
        # Ensure that all optima are in the *same* clock pulse.
        if corr < 10 and median_optimal_step >= 32:
            corr = 63
        if corr > 53 and median_optimal_step < 32:
            corr = 0
        corrected.append(corr)
    logging.info('Cor optimum: [%s]',
                 ' '.join([('%3d' % step) for step in corrected]))
    return corrected
        
        
    





def pps_delays_conf(station, start_date, pps_delays):
    r'''
    Format pps_delays in PPSdelays.conf format.

    **Parameters**

    station : string
        Station name.

    start_date : float
        seconds since 1970.0

    pps_delays : list of ints
        The optimal delay steps for each antenna processor.

    **Returns**

    A string with the contents of PPSdelays.conf.

    **Examples**

    >>> print(pps_delays_conf('CS030', 1332246766.307168,
    ... [36, 26, 21, 36, 17, 53, 49, 9, 50, 7, 51, 48, 60, 42, 11, 37,
    ...  47, 57, 33, 47, 49, 22, 2, 51, 61, 44, 14, 63, 61, 3, 37, 19,
    ...  57, 36, 35, 54, 35, 34, 42, 60, 59, 63, 63, 37, 4, 53, 52, 10]))
    #
    # PPSdelays.conf for CS030
    #
    # 2012-03-20 12:32:46
    #
    <BLANKLINE>
    48 [
     36  26  21  36  17  53  49   9  50   7  51  48  60  42  11  37
     47  57  33  47  49  22   2  51  61  44  14  63  61   3  37  19
     57  36  35  54  35  34  42  60  59  63  63  37   4  53  52  10
    ]

    '''

    header_format = '''#
# PPSdelays.conf for %s
#
# Created by %s
#
# %4d-%02d-%02d %02d:%02d:%02d
#

'''
    user =  pwd.getpwuid(os.getuid()).pw_name
    header = header_format % ((station,user,)+gmtime_tuple(start_date))
    contents = header + str(len(pps_delays)) + ' [\n'
    for subrack in range(len(pps_delays) / 16):
        subrack_delays = pps_delays[subrack*16:(subrack+1)*16]
        contents += ' '.join([('%3d' % step) for step in subrack_delays])
        contents += '\n'
    contents += ']'
    return contents




    
##################################
#                                #
#     Command line interface     #
#                                #
##################################

def parse_command_line(argv):
    r'''
    Parse the command line arguments.

    **Parameters**

    argv : list of strings
        The command line arguments passed to the script. Basically the
        contents of ``sys.argv``.

    **Returns**

    An options record with field ``output_dir``.

    **Raises**

    ValueError
        In case of an error parsing the command line.

    **Examples**

    >>> parse_command_line(['ppstune.py']).output_dir
    '/opt/lofar/etc'
    >>> parse_command_line(['ppstune.py', '--output-dir', '/tmp']).output_dir
    '/tmp'
    >>> parse_command_line(['ppstune.py', '--edge', 'both']).edge
    'both'
    >>> parse_command_line(['ppstune.py', '--output-dir']).output_dir
    Traceback (most recent call last):
    ...
    SystemExit: 2
    >>> parse_command_line(['ppstune.py', '--prefix']).output_dir
    Traceback (most recent call last):
    ...
    SystemExit: 2
    >>> parse_command_line(['ppstune.py', 'logfile.txt']).output_dir
    Traceback (most recent call last):
    ...
    ValueError: Unexpected command line option(s) "logfile.txt"

    >>> parse_command_line(['ppstune.py', '--log-level=None'])
    Traceback (most recent call last):
    ...
    ValueError: Log level ('None') not DEBUG, INFO, WARNING, or ERROR

    >>> parse_command_line(['ppstune.py', '--edge', 'first'])
    Traceback (most recent call last):
    ...
    ValueError: Edge ('first') not 'rising', 'falling', or 'both'

    >>> parse_command_line(['ppstune.py', '--clock=260'])
    Traceback (most recent call last):
    ...
    ValueError: Clock (260) must be 160 or 200 MHz
    
    '''
    prefix  = os.path.join('/localhome', 'ppstune', 'data')
    log_dir = os.path.join('/localhome', 'ppstune','log')
    parser  = OptionParser(usage   = 'python2 %prog [options]',
                           version = '%prog '+version_string())
    parser.add_option('--output-dir', type = 'string',
                dest    = 'output_dir',
                help    = 'Use DIR, not "%default" for PPSdelays.conf',
                metavar = 'DIR',
                default = prefix)

    parser.add_option('--edge', type = 'string',
                dest    = 'edge',
                help    = ' '.join(['Use EDGE flank of clock pulse;',
                                    'One of "rising", "falling", or',
                                    '"both". Default: %default']),
                metavar = 'EDGE',
                default = 'both')

    parser.add_option('--clock', type = 'int',
                      dest    = 'clock_mhz',
                      help    = 'Set clock to CLOCK MHz. Default: %default',
                      metavar = 'CLOCK',
                      default = 200)
                
    parser.add_option('--log-dir', type = 'string',
                dest    = 'log_dir',
                help    = 'Use DIR, not "%default" for logs',
                metavar = 'DIR',
                default = log_dir)

    parser.add_option('--log-level', type = 'string',
                dest    = 'log_level',
                help    = ' '.join(['Set minimum log LEVEL (default %default).',
                                    'Choose DEBUG, INFO, WARNING or ERROR']),
                metavar = 'LEVEL',
                default = 'INFO')

    parser.add_option('--no-conf-output', action = 'store_false',
                      dest    = 'write_conf_file',
                      help    = 'Do not write /opt/lofar/etc/PPSdelays.conf',
                      default = True)

    parser.add_option('--skip-measurements', action = 'store_false',
                      dest    = 'measure_delays',
                      help    = ' '.join(['skip measurement of delay steps to',
                                          'speed up testing of setup and',
                                          'tear-down. This option implies',
                                          '--no-conf-output']),
                      default = True)
                      

    parser.add_option('--repeat', type = 'int',
                dest    = 'repeat',
                help    = 'Repeat stability test N times. Default: %default',
                metavar = 'N',
                default = 5)
    
    (options, args) = parser.parse_args(argv[1:])
    if len(args) > 0:
        raise ValueError('Unexpected command line option(s) "%s"' % 
                         ', '.join(args))
    if options.log_level not in ['DEBUG', 'INFO', 'WARNING', 'ERROR']:
        raise ValueError('Log level (%r) not DEBUG, INFO, WARNING, or ERROR' %
                         options.log_level)

    if options.edge not in ['rising', 'falling', 'both']:
        raise ValueError('Edge (%r) not \'rising\', \'falling\', or \'both\'' %
                         options.edge)

    if options.clock_mhz not in [160, 200]:
        raise ValueError('Clock (%d) must be 160 or 200 MHz' %
                         options.clock_mhz)

    if not options.measure_delays:
        options.write_conf_file = False
    return options






def initialize_logging(station, log_dir, log_level):
    r'''
    Initialize the Python logging system. The log file will be written
    to ``log_dir/pps-tuning-<STATION_NAME>.log``.

    **Parameters**

    station : string
        Station name for which the log file must be written.
        
    log_dir : string
        Directory where the log file must be written.
 
    log_level : string
        Minimum log level of lines to write into the
        log file. Possible values are 'DEBUG', 'INFO', 'WARNING', and
        'ERROR'.

    **Returns**

    A string containing the log file name.

    **Examples**

    >>> initialize_logging('CS103', 'testdata/', log_level = 'INFO')
    'testdata/pps-tuning-CS103.log'
    '''
    log_levels   = {'DEBUG'  : logging.DEBUG,
                    'INFO'   : logging.INFO,
                    'WARNING': logging.WARNING,
                    'ERROR'  : logging.ERROR}
    level = log_levels[log_level]
    
    log_format     = ('ppstune.py@'+station.upper() +
                      ' %(asctime)s %(levelname)8s - %(message)s')
    log_file_name  = os.path.join(log_dir,
                                  'pps-tuning-%s.log' % station)

    logger       = logging.root
    logger.setLevel(level)
    formatter    = logging.Formatter(log_format)

    file_handler = logging.FileHandler(log_file_name)
    logger.addHandler(file_handler)

    if len(logger.handlers) == 1:
        stream_handler = logging.StreamHandler()
        logger.addHandler(stream_handler)
        
    for handler in logger.handlers:
        handler.setFormatter(formatter)
        handler.setLevel(level)

    return log_file_name
    




def prepare_for_tuning(conf_etc_name, start_date, clock_mhz,
                       lofar_log_dir,
                       swlevel_cmd = '/opt/lofar/bin/swlevel',
                       rspctl_cmd = '/opt/lofar/bin/rspctl',
                       sudo_cmd    = '/usr/bin/sudo',
                       rspdriver_cmd = '/opt/lofar/bin/RSPDriver',
                       timeout_s   = 300):
    r'''
    Change swlevel to 1, backup existing PPSdelays.conf. It is not removed.
    Change swlevel to 2, and wait until clocks are locked.

    Check for RSP error status at the end.

    **Parameters**

    conf_etc_name : string
        Original PPSdelays.conf file name.

    start_time : float
        Seconds since 1970.0

    clock_mhz : int
        Requested clock frequency in MHz.

    swlevel_cmd : string
        The name of the shell command to run. Set a different value
        for local off-station testing.

    rspctl_cmd : string
        Command to run rspctl. Use a different value for testing.

    sudo_cmd : string
        The name of the shell command to run as sudo. Set a different
        value for local off-station testing.

    rspdriver_cmd : string
        Full path to the RSPDriver executable.

    timeout_s : number
        Maximum amount of seconds to try if clocks have locked. Raise
        a RuntimeError if clocks are not locked at end of timeout.
        
    **Returns**

    A string containing the name of the backup file

    
    **Raises**
 
    IOError
        If backing up fails.
 
    RuntimeError
        In case of problems with the RSP boards, such as clocks
        failing to lock to the 10 MHz signals, or the wrong clocks to
        lock to the 10 MHz.


    **Examples**

    >>> backup_name = prepare_for_tuning('testdata/PPSdelays-test.conf', start_date = 1332246766.307168,
    ...            clock_mhz = 200, lofar_log_dir = 'test',
    ...            swlevel_cmd = 'test/swlevel', rspctl_cmd = 'test/rspctl',
    ...            sudo_cmd = 'test/sudo', timeout_s = 1.0,
    ...            rspdriver_cmd = 'test/RSPDriver')
    >>> backup_name
    'testdata/PPSdelays-test.conf.2012-03-20_1232'
    >>> os.path.exists(backup_name)
    True
    >>> removed = [os.remove('testdata/'+name) for name in os.listdir('testdata/')
    ...  if name[0:20] == 'PPSdelays-test.conf.']
    
    Test some safeguards against problems:

    >>> backup_name = prepare_for_tuning('testdata/PPSdelays-test.conf', start_date = 1332246766.307168,
    ...            clock_mhz = 160, lofar_log_dir = 'test',
    ...            swlevel_cmd = 'test/swlevel', rspctl_cmd = 'test/rspctl',
    ...            sudo_cmd = 'test/sudo', timeout_s = 1.0,
    ...            rspdriver_cmd = 'test/RSPDriver')
    Traceback (most recent call last):
    ...
    RuntimeError: Clocks locked to 200 MHz instead of 160 MHz
    
    >>> backup_name = prepare_for_tuning('/usr/bin/md5sum', start_date = 1332246766.307168,
    ...            clock_mhz = 200, lofar_log_dir = 'test',
    ...            swlevel_cmd = 'test/swlevel', rspctl_cmd = 'test/rspctl',
    ...            sudo_cmd = 'test/sudo', timeout_s = 1.0,
    ...            rspdriver_cmd = 'test/RSPDriver')
    Traceback (most recent call last):
    ...
    IOError: [Errno 13] Permission denied: '/usr/bin/md5sum.2012-03-20_1232'
    >>> removed = [os.remove('testdata/'+name) for name in os.listdir('testdata')
    ...  if name[0:20] == 'PPSdelays-test.conf.']
    
    
    '''
    backup_name = (conf_etc_name+'.%4d-%02d-%02d_%02d%02d' %
                   gmtime_tuple(start_date)[:-1])

    if os.path.exists(conf_etc_name):
        logging.info('Backing up %s to %s', conf_etc_name, backup_name)
        open(backup_name, 'w').write(open(conf_etc_name, 'r').read())
        if open(backup_name, 'r').read() != open(conf_etc_name, 'r').read():
            logging.error('Could not backup %s to %s',
                          conf_etc_name, backup_name)
            raise IOError('Failed to backup %s' % conf_etc_name)

    
    swlevel(2, swlevel_cmd = swlevel_cmd, timeout_s = 80.0)
    old_clock_mhz = wait_for_clocks_to_lock(rspctl_cmd = rspctl_cmd,
                                            timeout_s  = timeout_s)
    logging.info('Clocks locked to %d MHz', old_clock_mhz)

    restart_rsp_driver(swlevel_cmd = swlevel_cmd, sudo_cmd = sudo_cmd,
                       rspdriver_cmd = rspdriver_cmd,
                       lofar_log_dir = lofar_log_dir)
    old_clock_mhz = wait_for_clocks_to_lock(rspctl_cmd = rspctl_cmd,
                                            timeout_s  = timeout_s)
    logging.info('Clocks locked to %d MHz', old_clock_mhz)

    if old_clock_mhz != clock_mhz:
        set_clock_frequency_mhz(clock_mhz, rspctl_cmd = rspctl_cmd,
                                timeout_s = timeout_s)
    time.sleep(1.0)
    rspctl_status = check_output([rspctl_cmd, '--status'], timeout_s = 3.0)
    if 'ERROR' in rspctl_status:
        logging.error('RSP boards in ERROR state:\n%s', rspctl_status)
        raise RuntimeError('RSP boards in ERROR state')
    
    return backup_name




def install_sig_term_handler(start_date, initial_swlevel, lofar_log_dir):
    r'''
    Install a signal handler for SIGTERM, SIGHUP, SIGQUIT, and
    SIGABRT, that switches back to the ``initial_swlevel``.

    **Parameters**

    start_date : float
        start date of the program in seconds since 1970.0. Typically
        the output of ``time.time()``.

    initial_swlevel : int
        swlevel at which the station was operating when the program
        started. The signal handler will restore the swlevel to this
        value.

    **Returns**

    None

    **Example**

    >>> install_sig_term_handler(start_date = 1332246766.307168,
    ...                          initial_swlevel = 6,
    ...                          lofar_log_dir   = 'test')
    '''
    def handler(signal_number, stack_frame):
        r'''
        Handle signals that request the program to terminate
        nicely. The handler will

        1) switch to swlevel 1;
        2) switch to ``initial_swlevel``
        3) exit the program with error code -1
        
        **Parameters**

        signal_number : int
            The signal number.

        stack_frame : frame
            A frame object or None

        **Returns**

        Nothing
        '''
        remove_sig_term_handlers()
        logging.error('Received signal %d; terminating', signal_number)
        logging.error('Stack frame:\n%s',
                      str(inspect.getframeinfo(stack_frame)))
        restart_rsp_driver(lofar_log_dir = lofar_log_dir)
        clock_mhz = wait_for_clocks_to_lock()
        logging.info('Clocks locked to %d MHz', clock_mhz)
        try:
            swlevel(initial_swlevel)
            if initial_swlevel >= 2:
                clock_mhz = wait_for_clocks_to_lock()
                logging.info('Clocks locked to %d MHz', clock_mhz)
                end_date   = time.time()
                logging.error('Execution time %8.3f seconds', end_date - start_date)
            sys.exit(-1)
        except BrokenRSPBoardsError:
            logging.error('Caught %s', str(sys.exc_info()[0]))
            logging.error(str(sys.exc_info()[1]))
            swlevel(1)
            logging.error('Aborting NOW')
            sys.exit(-1)
        except RSPDriverDownError:
            logging.error('Caught %s', str(sys.exc_info()[0]))
            logging.error(str(sys.exc_info()[1]))
            swlevel(1)
            logging.error('Aborting NOW')
            sys.exit(-1)
        
        
    logging.info('Installing termination signal handlers')
    signal.signal(signal.SIGTERM, handler)
    signal.signal(signal.SIGHUP , handler)
    signal.signal(signal.SIGQUIT, handler)
    signal.signal(signal.SIGABRT, handler)
    return None



def remove_sig_term_handlers():
    r'''
    '''
    logging.info('Restoring default termination signal handlers')
    signal.signal(signal.SIGTERM, signal.SIG_DFL)
    signal.signal(signal.SIGHUP , signal.SIG_DFL)
    signal.signal(signal.SIGQUIT, signal.SIG_DFL)
    signal.signal(signal.SIGABRT, signal.SIG_DFL)
    return None



def read_pps_delays(conf_etc_name):
    r'''
    Read contents of ``conf_etc_name`` and logs its findings.

    **Parameters**

    conf_etc_name : string
        Filename of a PPSdelays.conf configuration
        file. Operationally, typically '/opt/lofar/etc/PPSdelays.conf'

    **Returns**

    Contents of the file or None if not found.

    **Examples**

    >>> print(read_pps_delays('testdata/PPSdelays-test.conf'))
    #
    # PPSdelays.conf for CS001
    #
    # 2012-03-30 07:48:24
    #
    <BLANKLINE>
    48 [
     47  51  56  60  47  51  56  60  47  52  58  62  46  50  56  60
     41  45  52  55  42  47  51  56  43  48  53  58  45  48  54  58
     47  51  56  60  44  49  56  60  47  49  58  61  47  52  60  63
    ]
    <BLANKLINE>
    >>> print(read_pps_delays('testdata/PPSdelays-test-nonexistent.conf'))
    None
    
    '''
    pps_delays_contents = None
    if os.path.exists(conf_etc_name):
        pps_delays_contents = open(conf_etc_name, 'r').read()
        logging.info('Reading %s:\n%s', conf_etc_name, pps_delays_contents)
    else:
        logging.info('%s not found', conf_etc_name)
    return pps_delays_contents


def parse_pps_delays(pps_delays_file_contents):
    r'''
    Parse the contents of a PPSdelays.conf file into a list of integers.
    
    **Parameters**
    
    pps_delays_file_contents : string
        The contents of the PPSdelays.conf file.

    **Returns**
    
    A list of integers with the settings form this file.

    **Examples**

    >>> input = read_pps_delays('testdata/PPSdelays-test.conf')
    >>> print(input)
    #
    # PPSdelays.conf for CS001
    #
    # 2012-03-30 07:48:24
    #
    <BLANKLINE>
    48 [
     47  51  56  60  47  51  56  60  47  52  58  62  46  50  56  60
     41  45  52  55  42  47  51  56  43  48  53  58  45  48  54  58
     47  51  56  60  44  49  56  60  47  49  58  61  47  52  60  63
    ]
    <BLANKLINE>
    >>> parse_pps_delays(input)
    [47, 51, 56, 60, 47, 51, 56, 60, 47, 52, 58, 62, 46, 50, 56, 60, 41, 45, 52, 55, 42, 47, 51, 56, 43, 48, 53, 58, 45, 48, 54, 58, 47, 51, 56, 60, 44, 49, 56, 60, 47, 49, 58, 61, 47, 52, 60, 63]
    '''
    if pps_delays_file_contents is None:
        raise ValueError('pps_delays_file_contents is None')
    lines_with_contents = [line.split('#')[0].strip()
                           for line in pps_delays_file_contents.split('\n')
                           if line.split('#')[0].strip() != '']
    array_string = ' '.join(lines_with_contents)
    settings     = [int(word) for word in (array_string.split('[')[1].split(']')[0]).split()]
    return settings


def write_pps_delays_conf(etc_name, temp_name, pps_delays):
    r'''
    Write contents of ``pps_delays`` to ``temp_name``

    **Parameters**

    etc_name : string
        Final name of config file. Most likely
        '/opt/lofar/etc/PPSdelays.conf' 

    temp_name :string
        File to which ``pps_delays`` is written first. If this
        succeeds, this file is renamed to ``etc_name``.

    **Raises**

    IOError or OSError
        In case of problems writing the file.

    **Returns**
    
       True if successful.

    **Examples**

    First we clean up in case of failed previous tests..
    
    >>> etc_name  = 'testdata/PPSdelays-output-test.conf'
    >>> temp_name = 'testdata/PPSdelays-output-test-temp.conf'
    >>> if os.path.exists(etc_name):
    ...     os.remove(etc_name)
    >>> if os.path.exists(temp_name):
    ...     os.remove(temp_name)

    Now we can write the file.
    
    >>> write_pps_delays_conf(etc_name, temp_name, 'contents of the file')
    True
    >>> os.path.exists(etc_name)
    True
    >>> os.path.exists(temp_name)
    False
    >>> open(etc_name).read()
    'contents of the file\n'

    A pre-existing ``etc_name`` should not pose a problem.    

    >>> write_pps_delays_conf(etc_name, temp_name, 'contents of the file')
    True
    >>> os.path.exists(etc_name)
    True
    >>> os.path.exists(temp_name)
    False
    >>> open(etc_name).read()
    'contents of the file\n'
    >>> os.remove(etc_name)

    Of course we cannot write to directories to which we have no access:

    >>> unwritable = os.path.join('testdata', 'unwritable-dir')
    >>> if os.path.exists(unwritable):
    ...     os.chmod(unwritable, 0755)
    ...     for file in os.listdir(unwritable):
    ...         os.remove(os.path.join(unwritable, file))
    ...     os.rmdir(unwritable)
    >>> os.mkdir(unwritable)
    >>> os.chmod(unwritable, 0555)
    
    >>> etc_name = 'testdata/unwritable-dir/PPSdelays-output-test.conf'
    >>> temp_name = 'testdata/PPSdelays-output-test-temp.conf'
    >>> write_pps_delays_conf(etc_name, temp_name, 'contents of the file')
    Traceback (most recent call last):
    ...
    OSError: [Errno 13] Permission denied
    >>> os.path.exists(temp_name)
    True
    >>> os.path.exists(etc_name)
    False
    >>> os.remove(temp_name)

    And
    
    >>> etc_name = 'testdata/PPSdelays-output-test.conf'
    >>> temp_name = 'testdata/unwritable-dir/PPSdelays-output-test-temp.conf'
    >>> write_pps_delays_conf(etc_name, temp_name, 'contents of the file')
    Traceback (most recent call last):
    ...
    IOError: [Errno 13] Permission denied: 'testdata/unwritable-dir/PPSdelays-output-test-temp.conf'
    >>> os.path.exists(temp_name)
    False
    >>> os.path.exists(etc_name)
    False
    '''            
    logging.info('Writing %r', temp_name)
    open(temp_name, 'w').write(pps_delays+'\n')
    if open(temp_name, 'r').read() == pps_delays+'\n':
        if os.path.exists(etc_name):
            logging.info('Removing %s', etc_name)
            os.remove(etc_name)
        logging.info('Renaming %s to %s',
            temp_name, etc_name)
        os.rename(temp_name, etc_name)
        logging.info('Done writing %s', etc_name)
        return True
    else:
        raise IOError('Failed writing %s' % temp_name)




def pps_tune_main(argv):
    r'''
    Main routine for pps tuning.

    **Parameters**

    argv : list of strings
        The command line arguments passed to the script. Basically the
        contents of ``sys.argv``.

    **Returns**

    An integer signifying success if 0, and an error otherwise.

    '''
    start_date          = time.time()
    previous_pps_delays = None
    exit_status         = 0
    initial_swlevel     = None
    try:
        options        = parse_command_line(argv)
        station        = station_name()
        log_file_name  = initialize_logging(station, options.log_dir,
                                            options.log_level)
        print('Writing log to %s' % log_file_name)
        logging.info('Beginning PPS tuning with %s version %s',
                     argv[0], version_string())
        logging.info('Command: %r', ' '.join(argv))        
        conf_etc_name  = os.path.join(options.output_dir, 'PPSdelays.conf')
        conf_temp_name = os.path.join(options.output_dir,
                '%s-%4d%02d%02d-%02d%02d%02d-PPSdelays.conf' %
                ((station,) + gmtime_tuple(start_date)))
        
        if options.write_conf_file:
            logging.info('Writing temporary output to %s', conf_temp_name)
        else:
            logging.info('%s will not be overwritten', conf_etc_name)    

        previous_pps_delays = read_pps_delays(conf_etc_name)
        initial_swlevel     = get_swlevel()
        logging.info('Initial swlevel is %d', initial_swlevel)
        if abs(initial_swlevel) < 2:
            logging.error('Initial swlevel below 2; aborting now.')
            return -1
        initial_swlevel = abs(initial_swlevel)

        install_sig_term_handler(start_date, initial_swlevel,
                                 lofar_log_dir = options.log_dir)
        log_cabinet_climate(station)
        
        backup_name = prepare_for_tuning(conf_etc_name = conf_etc_name,
                                         start_date    = start_date,
                                         clock_mhz     = options.clock_mhz,
                                         lofar_log_dir = options.log_dir)
            
        # Actual measurements
        if options.measure_delays:
            if options.edge == 'both':
                diff_errors_rising  = measure_all_delays(station,
                    clock_mhz       = options.clock_mhz,
                    edge            = 'rising',
                    repeat          = options.repeat)

                diff_errors_falling = measure_all_delays(station,
                    clock_mhz       = options.clock_mhz,
                    edge            = 'falling',
                    repeat          = options.repeat,
                    first_delay_step = 29,
                    one_past_last_delay_step = 33)

                diff_errors = diff_errors_rising + diff_errors_falling[1:]

            else:
                diff_errors  = measure_all_delays(station,
                    clock_mhz       = options.clock_mhz,
                    edge            = options.edge,
                    repeat          = options.repeat)
                diff_errors += [[0]*len(diff_errors[0])]*4
            diff_errors += diff_errors



            # Report
            logging.info('*** Failure report %s***\n%s',
                station, sync_failure_report(diff_errors))

            # Determine new optimum
            new_delays = find_optimal_delays(diff_errors)
            if previous_pps_delays:
                logging.info('Old delays:\n%s', previous_pps_delays)
                old_delays = parse_pps_delays(previous_pps_delays)
            else:
                old_delays = None

            pps_delays = pps_delays_conf(station, start_date, new_delays)
            logging.info('New delays:\n%s', pps_delays)
            
            if old_delays:
                logging.info('Difference new - old:\n%r',
                             [p_new - p_old
                              for p_new, p_old
                              in zip(new_delays, old_delays)])

            if options.write_conf_file:
                # Throws IOError or OSError if writing failed
                write_pps_delays_conf(conf_etc_name, conf_temp_name, pps_delays)
            else:
                logging.info('New delays will NOT be written to %s', conf_etc_name)
        else:
            logging.info('      *** Skipping measurements! ***')
            
        log_cabinet_climate(station)
            
        # We do not get here in case of exceptions. It is safe to
        # remove the backup now.
        if os.path.exists(backup_name):
            logging.info('Removing backup %s', backup_name)
            os.remove(backup_name)
            
    except SystemExit:
        logging.error('Caught SystemExit: Aborting NOW')
        remove_sig_term_handlers()
        raise
    except BrokenRSPBoardsError:
        logging.error('Caught %s', str(sys.exc_info()[0]))
        logging.error(str(sys.exc_info()[1]))
        remove_sig_term_handlers()
        swlevel(1)
        logging.error('Aborting NOW')
        sys.exit(-1)
    except RSPDriverDownError:
        logging.error('Caught %s', str(sys.exc_info()[0]))
        logging.error(str(sys.exc_info()[1]))
        remove_sig_term_handlers()
        swlevel(1)
        logging.error('Aborting NOW')
        sys.exit(-1)
    except:
        logging.error('Caught %s', str(sys.exc_info()[0]))
        logging.error(str(sys.exc_info()[1]))
        logging.error('TRACEBACK:\n%s', traceback.format_exc())
        exit_status = -1
        
    if initial_swlevel != None:
        if initial_swlevel >= 2:
            restart_rsp_driver(lofar_log_dir = options.log_dir)
            clock_mhz = wait_for_clocks_to_lock()
            logging.info('Clocks locked to %d MHz', clock_mhz)
            for swlevel_step in range(2, initial_swlevel+1):
                swlevel(swlevel_step)
            logging.info('Starting TBBs...')
            start_tbb_output = check_output('/home/lofarsys/startTBB.sh',
                                            timeout_s = 60.0)
            logging.debug('startTBB.sh output:\n%s', start_tbb_output)
        else:
            swlevel(initial_swlevel)

    clock_mhz = wait_for_clocks_to_lock()
    logging.info('Clocks locked to %d MHz', clock_mhz)
    end_date   = time.time()
    remove_sig_term_handlers()
    logging.info('Execution time %8.3f seconds', end_date - start_date)
    return exit_status
    




    
########################
#       M A I N        #
########################
try:
    if __name__ == '__main__':
        try:
            sys.exit(pps_tune_main(sys.argv))
        except BrokenRSPBoardsError:
            logging.error('Caught %s', str(sys.exc_info()[0]))
            logging.error(str(sys.exc_info()[1]))
            swlevel(1)
            logging.error('Aborting NOW')
            sys.exit(-1)
        except RSPDriverDownError:
            logging.error('Caught %s', str(sys.exc_info()[0]))
            logging.error(str(sys.exc_info()[1]))
            swlevel(1)
            logging.error('Aborting NOW')
            sys.exit(-1)
except SystemExit:
    logging.info('Exit status: %s', str(sys.exc_info()[1]))
except:
    logging.error('Caught %s', str(sys.exc_info()[0]))
    logging.error(str(sys.exc_info()[1]))
    logging.error('TRACEBACK:\n%s', traceback.format_exc())
    logging.error('Aborting NOW')

    
