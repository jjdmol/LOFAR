#!/usr/bin/python

import subprocess
from subprocess import Popen, PIPE

import sys
from time import sleep
import termios
import tty
import fcntl
import os
import datetime
import logging

logFp = None
import glob
import logging
import logging.handlers


workingSettings = [1280, 5, 3261, 35387, 13, 13, ['\x03', '\x1c', '\x7f', '\x15', '\x04', '\x00', '\x01', '\x00', '\x11', '\x13', '\x1a', '\x00', '\x12', '\x0f', '\x17', '\x16', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00']]

logFp = None
rub_logger = None
msgBuffer = []

cmdList = ['ST?', 'FC?', 'DS?', 'MR?', 'TT?', 'PI?', 'SF?']

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
    cmd1 = "./tci/tci.exe "
    cmd1 += cmd
    result = "None"
    cf = Popen(cmd1, shell = True, stdout = PIPE, stderr = PIPE)
        
    (res,resErr) = cf.communicate()
    result = res
    return result


def main():
    global ttyFp, logFp, rub_logger
    args = sys.argv[1:]
    logFilePath = '/var/log/ntpstats/'
    logFileBase = 'rubidium_log_'
    logExt = '.txt'
    hostName = os.getenv('HOSTNAME')
    if hostName == None:
        hostName = ''
    logName = logFileBase + hostName + logExt
    logPath  = os.path.join(logFilePath, logName)

    LOG_FILENAME = '/tmp/logging_rotatingfile_example.out'

    # Set up a specific logger with our desired output level
    rub_logger = logging.getLogger('Rubidium Logger')
    rub_logger.setLevel(logging.INFO)

    # Add the log message handler to the logger
    handler = logging.handlers.TimedRotatingFileHandler(
        logPath, when='midnight', backupCount=100)

    rub_logger.addHandler(handler)


    
    try:
        logFp = open(logPath,'a')
        foo = 1
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


    print "running"
    while True:
        logStr = ''
        for cmd in cmdList:
            line = callCommand(cmd)
            logStr += line.strip() + "; "
        printAndLog(logStr)
        sleep(0.5)

    if logFp != None:
        logFp.close()

def printAndLog(text):
    global  msgBuffer
    logTime = datetime.datetime.utcnow()
    line =  logTime.isoformat() + ' ' + text
    sLine = line.strip()
#    print sLine
    sLine += '\n'
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


