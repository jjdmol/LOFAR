#!/usr/bin/env python

import socket
import os
import sys
import time
from optparse import OptionParser

host = 'localhost'
port = 32031
replyformat = 0

class switch(object):
    def __init__(self, value):
        self.value = value
        self.fall = False

    def __iter__(self):
        """Return the match method once, then stop"""
        yield self.match
        raise StopIteration
    
    def match(self, *args):
        """Indicate whether or not to enter a case suite"""
        if self.fall or not args:
            return True
        elif self.value in args: # changed for v1.5, see below
            self.fall = True
            return True
        else:
            return False

################################################################
# mmcs_cmd(szSocket, szCmd)
#
# mmcs_cmd -- send an mmcs command and gather and check the response.
# inputs:
#     param0 -- remote tcp port to send command to.
#     param1 -- command string
# outputs:
#     return values in a list.
#

def mmcs_cmd (socket, szCmd):
    lines = list()
    socket.send(szCmd)           # execute the command.
    if replyformat == 0:
        results = socket.recv(1024) # read the result...
	lines.append(results.rstrip("\n"))          # get rid of lf at end.
    else:
        file = socket.makefile()
	for line in file:
	    if line.find('\0') >= 0:
		break
	    lines.append(line.rstrip("\n"))          # get rid of lf at end.
    return lines

def set_replyformat(socket, rformat): 
    results = mmcs_cmd(socket, 'replyformat ' + str(rformat) + '\n')
    if results[0] != 'OK':
        print 'set replyformat:' + str(rformat) + '   ...failed'
    global replyformat
    replyformat = rformat

def list_jobs(socket):
    set_replyformat(socket, 1)
    return mmcs_cmd(socket, 'list_jobs\n')

def list_blocks(socket):
    set_replyformat(socket, 1)
    return mmcs_cmd(socket, 'list_blocks\n')

def jobId(socket):
    results = list_jobs(socket)
    for line in results:
        if line.find(options.blockid) >= 0:
	    return line.split()[0]

def free_block(socket):
    set_replyformat(socket, 0)
    results = mmcs_cmd(socket, 'free ' + options.blockid + '\n');
    if results[0] != 'OK':
        print 'free \'%s\' ' % options.blockid + '   ...failed'
    
def killjob(socket):
    set_replyformat(socket, 0)
    jobid = jobId(socket)
    results = mmcs_cmd(socket, 'killjob ' + options.blockid + ' ' + jobid + '\n');
    if results[0] != 'OK':
        print 'killjob ' + options.blockid + jobid + '   ...failed'

def partition_exist(socket):
    results = list_blocks(socket)
    for line in results:
	if line.find(options.blockid) >= 0 and line.find(options.user) >= 0:
	    return True
    return False	    

def block_status(socket):
    set_replyformat(socket, 1)
    results = list_blocks(socket)
    for line in results:
        if line.find(options.blockid) >= 0:
	    return line.split()[1]
    
def job_status(socket):
    set_replyformat(socket, 1)
    results = list_jobs(socket)
    for line in results:
        if line.find(options.blockid) >= 0:
	    return line.split()[1]

def show_block_status(socket):
    set_replyformat(socket, 1)
    results = list_jobs(socket)
    for line in results:
        if line.find(options.blockid) >= 0:
	    print line


#
# Start of mainline
#
if __name__ == '__main__':
    
    parser = OptionParser()

    parser.add_option('--user'    , dest='user'    , default=os.environ.get('USER', 'default'), type='string', help='username [%default]')
    parser.add_option('--blockid' , dest='blockid' , default='R000_128_0Z'                    , type='string', help='name of the blockid [%default]')
    parser.add_option('--status'  , dest='status'  , default='false'                         , type='string', help='Show status of the blockid ')
    
    # parse the options
    (options, args) = parser.parse_args()
    
    remote = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    remote.connect((host, port))

    results = mmcs_cmd(remote, 'set_username ' + options.user + '\n');
    if results[0] != 'OK':
        print 'set_username ' + options.user + '   ...failed'

    if options.status == 'true':
        if partition_exist(remote):
            print 'UP'
	else:
	    print 'DOWN'
	sys.exit(0)     
	
    partitionExist = partition_exist(remote)
    bl_stat = ''
    job_stat = ''
    while (partitionExist):
        status_block = block_status(remote)
	if status_block != bl_stat:
	    print 'Block %s' % options.blockid + ' status: ' + str(status_block)
	    bl_stat = status_block
        for case in switch(status_block):
            if case('A'):
		free_block(remote)
                break
            if case('B'):
		free_block(remote)
                break
            if case('D'):
                break
            if case('T'):
                break
            if case('I'):
 		status_job = job_status(remote)
		if status_job != job_stat:
		    print 'Job status: ' + str(status_job)
		    job_stat = status_job
		for case in switch(status_job):
		    if case('S'):
			killjob(remote)
			break
		    if case('R'):
			killjob(remote)
			break
		    if case('D'):
			break
                break
            if case(): # default, could also just omit condition or 'if True'
                print "something else!"
                # No need to break here, it'll stop anyway

	time.sleep(2)
	partitionExist = partition_exist(remote)

    remote.close()

    sys.exit(0)
