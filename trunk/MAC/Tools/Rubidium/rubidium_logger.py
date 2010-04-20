#!/usr/bin/python

import subprocess
from subprocess import Popen, PIPE

import sys
from time import sleep
import termios
import tty
import fcntl
import resource
import os
import datetime
import logging

logFp = None
import glob
import logging
import logging.handlers
import time

try:
    import codecs
except ImportError:
    codecs = None


class MyTimedRotatingFileHandler(logging.handlers.TimedRotatingFileHandler):
  def __init__(self,dir_log):
   self.dir_log = dir_log
   filename =  self.dir_log + "." + time.strftime("%Y%m%d") #dir_log here MUST be with os.sep on the end
   logging.handlers.TimedRotatingFileHandler.__init__(self,filename, when='midnight', interval=1, backupCount=0, encoding=None)
#   os.symlink(self.baseFilename, self.dir_log)
   cmd = 'ln -fs ' + filename + ' ' + self.dir_log
   cf = Popen(cmd, shell = True, stdout = PIPE, stderr = PIPE)
   (res,resErr) = cf.communicate()
   
  def doRollover(self):
   """
   TimedRotatingFileHandler remix - rotates logs on daily basis, and filename of current logfile is time.strftime("%m%d%Y")+".txt" always
   """ 
   self.stream.close()
   # get the time that this sequence started at and make it a TimeTuple
   t = self.rolloverAt - self.interval
   timeTuple = time.localtime(t)
   self.baseFilename = self.dir_log + "." + time.strftime("%Y%m%d")
 
   
   if self.encoding:
     self.stream = codecs.open(self.baseFilename, 'a', self.encoding)
   else:
     self.stream = open(self.baseFilename, 'a')

#   os.symlink(self.baseFilename, self.dir_log)
   cmd = 'ln -fs ' + self.baseFilename + ' ' + self.dir_log
   cf = Popen(cmd, shell = True, stdout = PIPE, stderr = PIPE)
   (res,resErr) = cf.communicate()
   self.rolloverAt = self.rolloverAt + self.interval
   



workingSettings = [1280, 5, 3261, 35387, 13, 13, ['\x03', '\x1c', '\x7f', '\x15', '\x04', '\x00', '\x01', '\x00', '\x11', '\x13', '\x1a', '\x00', '\x12', '\x0f', '\x17', '\x16', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00']]

logFp = None
rub_logger = None
msgBuffer = []

cmdList = ['ST?', 'TT?']
advCmdList = ['AD0?', 'AD1?', 'AD2?', 'AD3?', 'AD4?', 'AD5?', 'AD6?', 'AD7?', 'AD8?', 'AD9?', 'AD10?', 'AD11?', 'AD12?', 'AD13?', 'AD14?', 'AD15?', 'AD16?', 'AD17?', 'AD18?','SD0?', 'SD1?', 'SD2?', 'SD3?', 'SD4?', 'SD5?', 'SD6?', 'SD7?','SP?','SF?','SS?','MO?','MR?','MS?','LO?','GA?','PH?','EP?','FC?','DS?', 'TO?','TS?','PS?','PL?','PT?','PF?','PI?','LM?']

def checkSettings(fp):
    curSet = termios.tcgetattr(fp)
    if curSet != workingSettings:
        cmd = 'stty < /dev/ttyS0'
        cf = Popen(cmd, shell = True, stdout = PIPE, stderr = PIPE)
        (result, resultErr) = cf.communicate()
        print result, resultErr
    else:
        return True

def callCommand(cmd):
    cmd1 = "/opt/lofar/sbin/tci "
    cmd1 += cmd
    result = "None"
    cf = Popen(cmd1, shell = True, stdout = PIPE, stderr = PIPE)
        
    (res,resErr) = cf.communicate()
    result = res
    return result


# How to become a daemon
# ----------------------
# See Advanced Programming in the UNIX Environment by Stevens and Rago, section 13.3, pp. 425/426.
# See also the UNIX Programming FAW (http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC16)
#
# 1. call umask(0) to clear the file mode creation mask. the umask is inherited and may be set to
#    deny certain permissions.
# 2. fork() and let the parent exit. this returns control to the shell. also, the child inherits
#    the parent's process group id, but gets its own process id. by definition, it is therefore
#    not a process group leader. this is a prerequisite for calling setsid(), which is the next
#    step.
# 3. call setsid() to create a new session. the process becomes leader of a new sessions, process
#    group leader of a new process group and has no controlling terminal.
# 4. fork() again to ensure the process is not a session leader. (System V systems may allocate a
#    controlling terminal for session leaders under certain conditions).
# 5. chdir("/") to ensure that the daemon does not tie up any mounted file systems.
# 6. close all open file descriptors we may have inherited from our parent.
# 7. re-open stdin, stdout, and stderr; for instance, point stdout and stderr to a log file, or
#    simply redirect to /dev/null.
#
def daemonize(logfile):
    try:
        no_file = resource.getrlimit(resource.RLIMIT_NOFILE)
    except ValueError, ex:
        print "error: unable to determine NOFILE resource limit; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    # 1. clear umask
    try:
        os.umask(0)
    except OSError, ex:
        print "error: unable to set umask; daemon not started %s" % ex
        sys.exit(EXIT_ERROR)

    # 2. fork and let parent exit
    try:
        pid = os.fork()
    except OSError, ex:
        print "error: unable to fork; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    if pid != 0:
        # parent exits
        # NOTE: Steven and Rago specify a standard exit(0) here, while the UNIX Programming FAQ
        # recommends _exit(0) (see http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC6). We favour
        # the latter approach, because it seems correct to only do user-mode clean-up once.
        os._exit(0)

    #
    #    FIRST CHILD PROCESS
    #

    # 3. create a new session
    try:
        sid = os.setsid()
    except OSError, ex:
        print "error: unable to create a new session; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    # 4. second fork
    try:
        pid = os.fork()
    except OSError, ex:
        print "error: unable to fork; daemon not started (%s)" % ex
        sys.exit(EXIT_ERROR)

    if pid != 0:
        # parent exits
        # NOTE: Steven and Rago specify a standard exit(0) here, while the UNIX Programming FAQ
        # recommends _exit(0) (see http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC6). We favour
        # the latter approach, because it seems correct to only do user-mode clean-up once.
        os._exit(0)

    #
    #    SECOND CHILD PROCESS
    #

    try:
        # 5. change the working directory
        os.chdir("/")

        print "closing stdout; from this moment on, any errors will be sent to the syslog."

        # 6a. flush stdout and stderr
        sys.stdout.flush()
        sys.stderr.flush()

        # 6b. close all open file descriptors (uses soft NO_FILE limit)
        for fd in range(0, max(no_file[0], 1024)):
            try:
                os.close(fd)
            except OSError:
                pass

        # 7. re-open stdin, stdout, and stderr as /dev/null
        # for python >= 2.4 use os.devnull
        sys.stdin = open("/dev/null", "r")
        sys.stdout = open(logfile, "a")
        sys.stderr = open(logfile, "a")
    except (IOError, OSError), ex:
        syslog.syslog(syslog.LOG_ERR, "unable to chdir, close open file descriptors, or open %s; daemon not started (%s)" % logfile, ex)
        sys.exit(EXIT_ERROR)

    #
    # properly daemonized from now on...
    #


def main():
    global ttyFp, logFp, rub_logger
    args = sys.argv[1:]
    logFilePath = '/var/log/ntpstats/'
    logFileBase = 'rubidium_log'
    logExt = '.txt'
    hostName = os.getenv('HOSTNAME')
    if hostName == None:
        hostName = ''
    logName = logFileBase #+ hostName #+ logExt
    logPath  = os.path.join(logFilePath, logName)

    LOG_FILENAME = '/tmp/logging_rotatingfile_example.out'

    # Set up a specific logger with our desired output level
    rub_logger = logging.getLogger('Rubidium Logger')
    rub_logger.setLevel(logging.INFO)

    # Add the log message handler to the logger 
    #handler = logging.handlers.TimedRotatingFileHandler(
    #logPath, when='midnight', backupCount=100)
    handler = MyTimedRotatingFileHandler(
        logPath)

    rub_logger.addHandler(handler)
    
    try:
        logFp = open(logPath,'a')

    except Exception, e:
        print "Trouble while opening a log file, details: " + e.__str__()
        sys.exit()
        
    try:
        # check if ttyS0 can be opened
        ttyFp = open("/dev/ttyS0", "w+")
    except Exception, e:
        print "Trouble while opening the serial port, details: " + e.__str__()
        sys.exit()

    checkSettings(ttyFp)
    if ttyFp != None:
        ttyFp.close()

    # Now make yourself a daemon...!
    daemonize(logPath)

    first = True
    oldCur = -1
    cur = -1
    count = 0
    interval = 60
    cmdIndex = 0
    cmdStep = 4
    while True:
        count += 1
        while cur == oldCur:
            sleep(0.01)
            cur = round(time.time())
            
        oldCur = cur
        logStr = ''
        for cmd in cmdList:
            line = callCommand(cmd)
#            print cmd
            if cmd.find("TT?")!= -1:
#                print "TT cmd found!"
                conVal = int(line.strip())
                if conVal > 500000000 :
#                    print 'high value!'
                    conVal = conVal - 1000000000
                    line = str(conVal)

            logStr += cmd.strip('?') + ' ' + line.strip() + "; "
        if count > interval or cmdIndex != 0:
            if cmdIndex == 0:
                count = 0
            for i in range(cmdStep):
                curCmdIndex = cmdIndex + i
                if curCmdIndex < len(advCmdList):
                    cmd = advCmdList[curCmdIndex]
                    line = callCommand(cmd)

                    logStr += cmd.strip('?') + ' ' + line.strip() + "; "
            cmdIndex += cmdStep
            if cmdIndex > len(advCmdList):
                cmdIndex = 0 
                
                
        printAndLog(logStr)
        if count > (round(float(len(advCmdList))/cmdStep)+1):
            sleep(0.5)
        else:
            sleep(0.2)
            
    if logFp != None:
        logFp.close()

def printAndLog(text):
    global  msgBuffer
    logTime = datetime.datetime.utcnow()
    line =  logTime.isoformat() + '; ' + text
    sLine = line.strip()
#    print sLine
#    sLine += '\n'
    if rub_logger == None:
        msgBuffer.append(sLine)
    else:
        if len(msgBuffer)!= 0:
            for bLine in msgBuffer:
                rub_logger.info(bLine)
#                logFp.write(bLine)

            msgBuffer = []
        rub_logger.info(sLine)
#        logFp.write(sLine)
#        logFp.flush()




main()


"""
[1280, 5, 3261, 35387, 13, 13, ['\x03', '\x1c', '\x7f', '\x15', '\x04', '\x00', '\x01', '\x00', '\x11', '\x13', '\x1a', '\x00', '\x12', '\x0f', '\x17', '\x16', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00']]

[   0, 4, 3261,  2608, 13, 13, ['\x03', '\x1c', '\x7f', '\x15', '\x04', 0, 1, '\x00', '\x11', '\x13', '\x1a', '\x00', '\x12', '\x0f', '\x17', '\x16', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00, '\x00', '\x00', '\x00', '\x00']]

"""


