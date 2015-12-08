#!/usr/bin/env python

# LTACP Python module for transferring data from a remote node to a remote SRM via localhost
#
# Remote data can be individual files or directories. Directories will be tar-ed.
#
# Between the remote and local host, md5 checksums are used to ensure integrity of the file,
# adler32  is used between localhost and the SRM.

import logging
from subprocess import Popen, PIPE
from socket import getfqdn
import os, sys, getpass
import time

log_handler = logging.StreamHandler()
formatter = logging.Formatter('%(asctime)-15s %(levelname)s %(message)s')
formatter.converter = time.gmtime
log_handler.setFormatter(formatter)
logger = logging.getLogger('Slave')
logger.addHandler(log_handler)
logger.setLevel(logging.INFO)

logger = logging.getLogger('Slave')

class LtacpException(Exception):
     def __init__(self, value):
         self.value = value
     def __str__(self):
         return repr(self.value)

def getfqdn():
    return "10.178.1.2"

# converts given srm url of an LTA site into a transport url as needed by gridftp. (Sring replacement based on arcane knowledge.)
def convert_surl_to_turl(surl):
    turl = surl.replace("srm://srm.grid.sara.nl:8443","gsiftp://gridftp.grid.sara.nl",1)
    turl = turl.replace("srm://srm.grid.sara.nl","gsiftp://gridftp.grid.sara.nl",1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de:8443","gsiftp://lofar-gridftp.fz-juelich.de",1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de","gsiftp://lofar-gridftp.fz-juelich.de",1)
    turl = turl.replace("srm://srm.target.rug.nl:8444","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    turl = turl.replace("srm://srm.target.rug.nl","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    return turl

def removeRemoteFile(user, host, filepath):
    '''remove a file (or fifo) on a remote host. Test if file exists before deleting.'''
    cmd_remote_rm = ['ssh %s@%s \'if [ -e "%s" ] ; then rm %s ; fi ;\'' % (user, host, filepath, filepath)]
    logger.debug('ltacp: remote removing file if existing. executing: %s' % ' '.join(cmd_remote_rm))
    p_remote_rm = Popen(cmd_remote_rm, shell=True, stdout=PIPE, stderr=PIPE)
    p_remote_rm.communicate()
    if p_remote_rm.returncode != 0:
        raise LtacpException("Could not remove remote file %s@%s:%s\n%s" % (user, host, filepath, p_remote_rm.stderr))

# transfer file/directory from given src to SRM location with given turl
def transfer(src_host,
             src_path_data,
             dst_surl,
             src_user=getpass.getuser(),
             port_data='40000',
             port_md5='50000',
             local_data_fifo=None,
             remote_data_fifo=None
            ):

    dst_turl = convert_surl_to_turl(dst_surl)
    logger.info('ltacp: initiating transfer of %s:%s to %s' % (src_host, src_path_data, dst_surl))

    # default return code
    code = 0

    # for cleanup
    started_procs = []

    # create default fifo paths if not given
    if not local_data_fifo:
        local_data_fifo = '/tmp/ltacp_datapipe_'+src_host+'_'+port_data
    if not remote_data_fifo:
        remote_data_fifo = '/tmp/ltacp_md5_receivepipe_'+port_md5

    #---
    # Server part
    #---

    # create local fifo to stream data to globus-url-copy
    logger.info('ltacp: creating data fifo for globus-url-copy: %s' % local_data_fifo)
    if os.path.exists(local_data_fifo):
        os.remove(local_data_fifo)
    os.mkfifo(local_data_fifo)

    # create local fifo to stream data to adler32
    local_adler32_fifo = local_data_fifo+'_adler32'
    logger.info('ltacp: creating data fifo for adler32: %s' % local_adler32_fifo)
    if os.path.exists(local_adler32_fifo):
        os.remove(local_adler32_fifo)
    os.mkfifo(local_adler32_fifo)

    # start listen for data stream
    cmd_data_in = ['nc', '-l', '-q','0', port_data]
    logger.info('ltacp: listening for data. executing: %s' % ' '.join(cmd_data_in))
    p_data_in = Popen(cmd_data_in, stdout=PIPE)
    started_procs.append(p_data_in)

    # start listen for checksums
    cmd_md5_receive = ['nc','-l', '-q','0', port_md5]
    logger.info('ltacp: listening for checksums. executing: %s' % ' '.join(cmd_md5_receive))
    p_md5_receive = Popen(cmd_md5_receive, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_md5_receive)

    # start tee incoming data stream to fifo (pipe stream further for checksum)
    cmd_tee_data = ['tee', local_data_fifo]
    logger.info('ltacp: splitting datastream. executing on stdout of data listener: %s' % (' '.join(cmd_tee_data),))
    p_tee_data = Popen(cmd_tee_data, stdin=p_data_in.stdout, stdout=PIPE)
    started_procs.append(p_tee_data)

    # start tee incoming data stream to fifo (pipe stream further for checksum)
    cmd_tee_checksums = ['tee', local_adler32_fifo]
    logger.info('ltacp: splitting datastream again. executing: on stdout of 1st data tee: %s' % (' '.join(cmd_tee_checksums),))
    p_tee_checksums = Popen(cmd_tee_checksums, stdin=p_tee_data.stdout, stdout=PIPE)
    started_procs.append(p_tee_checksums)

    # start computing md5 checksum of incoming data stream
    cmd_md5_local = ['md5sum']
    logger.info('ltacp: computing local md5 checksum. executing on stdout of 2nd data tee: %s' % (' '.join(cmd_md5_local)))
    p_md5_local = Popen(cmd_md5_local, stdin=p_tee_checksums.stdout, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_md5_local)

    # start computing adler checksum of incoming data stream
    cmd_a32_local = ['./md5adler/a32', local_adler32_fifo]
    logger.info('ltacp: computing local adler32 checksum. executing: %s' % ' '.join(cmd_a32_local))
    p_a32_local = Popen(cmd_a32_local, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_a32_local)

    # start copy fifo stream to SRM
    cmd_data_out = ['globus-url-copy', local_data_fifo, dst_turl]
    logger.info('ltacp: copying data stream into globus-url-copy. executing: %s' % ' '.join(cmd_data_out))
    p_data_out = Popen(cmd_data_out, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_data_out)

    # Check if our side is set up correctly
    # and all processes are still waiting for input from client
    for p in started_procs:
        ret = p.poll()
        if ret is not None:
            raise LtacpException("process %d exited prematurely with exit code %d" % (p.pid, ret))

    #---
    # Client part
    #---

    # start remote copy on src host:
    # 1a) remove any obsolete fifo which might be in the way
    # 1b) create fifo
    # 2) send tar stream of data/dir + tee to fifo for 3)
    # 3) simultaneously to 2), calculate checksum of fifo stream
    # 4) break fifo
    removeRemoteFile(src_user, src_host, remote_data_fifo)
    cmd_remote_mkfifo = ['ssh '+src_user+'@'+src_host+
                         ' \'mkfifo '+remote_data_fifo+
                         '\'']
    logger.info('ltacp: remote creating fifo. executing: %s' % (' '.join(cmd_remote_mkfifo)))
    p_remote_mkfifo = Popen(cmd_remote_mkfifo, shell=True, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_remote_mkfifo)


    try:
        # block until fifo is created
        output_remote_mkfifo = p_remote_mkfifo.communicate()
        if p_remote_mkfifo.returncode != 0:
            raise LtacpException('Remote fifo creation failed: '+output_remote_mkfifo[1])

        # start sending remote data, tee to fifo
        src_path_parent, src_path_child = os.path.split(src_path_data)
        cmd_remote_data = ['ssh '+src_user+'@'+src_host+
                           ' \'cd '+src_path_parent+
                           ' ; tar c -O '+src_path_child+' | tee '+remote_data_fifo+' | nc --send-only '+getfqdn()+' '+port_data+
                           '\'']
        logger.info('ltacp: remote starting transfer. executing: %s' % ' '.join(cmd_remote_data))
        p_remote_data = Popen(cmd_remote_data, shell=True, stdout=PIPE, stderr=PIPE)
        started_procs.append(p_remote_data)

        # start computation of checksum on remote fifo stream
        cmd_remote_checksum = ['ssh '+src_user+'@'+src_host+
                               ' \'cat '+remote_data_fifo+' | md5sum | nc --send-only '+getfqdn()+' '+port_md5+
                               '\'']
        logger.info('ltacp: remote starting md5 checksum. executing: %s' % ' '.join(cmd_remote_checksum))
        p_remote_checksum = Popen(cmd_remote_checksum, shell=True, stdout=PIPE, stderr=PIPE)
        started_procs.append(p_remote_checksum)


        # waiting for output, comparing checksums, etc.
        logger.debug('ltacp: waiting for transfer to finish...')
        output_remote_data = p_remote_data.communicate()
        logger.debug('ltacp: remote data transfer finished...')
        output_remote_checksum = p_remote_checksum.communicate()
        logger.debug('ltacp: remote md5 checksum transfer finished.')

        if p_remote_data.returncode == 0 and p_remote_checksum.returncode == 0:
            logger.debug('ltacp: waiting for remote md5 checksum...')
            output_md5_remote = p_md5_receive.communicate()
            logger.debug('ltacp: waiting for local md5 checksum...')
            output_md5_local = p_md5_local.communicate()

            if p_md5_receive.returncode == 0 and p_md5_local.returncode == 0:
                md5_checksum_remote = output_md5_remote[0].split()[0]
                md5_checksum_local = output_md5_local[0].split()[0]

                if(md5_checksum_remote == md5_checksum_local):
                    logger.info('ltacp: remote and local md5 checksums are equal: %s' % (md5_checksum_local,))
                else:
                    raise LtacpException('md5 checksum reported by client (%s) does not match local checksum of incoming data stream (%s)' % (md5_checksum_remote, md5_checksum_local))

                logger.debug('ltacp: waiting for transfer via globus-url-copy to LTA to finish...')
                output_data_out = p_data_out.communicate()
                if p_data_out.returncode == 0:
                    logger.info('ltacp: data transfer to LTA complete.')

                    logger.debug('ltacp: waiting for local adler32 checksum to complete...')
                    output_a32_local = p_a32_local.communicate()
                    a32_checksum_local = output_a32_local[0].split()[1]

                    if p_a32_local.returncode != 0:
                        raise LtacpException('local adler32 checksum computation failed: '+str(output_a32_local))

                    logger.debug('ltacp: fetching adler32 checksum from LTA...')
                    srm_a32_checksum = get_srm_a32_checksum(dst_surl)

                    if not srm_a32_checksum:
                        raise LtacpException('Could not get srm adler32 checksum for: '+dst_surl)

                    if(srm_a32_checksum != a32_checksum_local):
                        raise LtacpException('adler32 checksum reported by srm ('+srm_a32_checksum+') does not match original data checksum ('+a32_checksum_local+')')

                    logger.info('ltacp: adler32 checksums are equal: %s' % a32_checksum_local)
                    logger.info('ltacp: transfer to LTA completed successfully.')
                else:
                    raise LtacpException('Transfer to SRM failed: '+output_data_out[1])
            else:
                raise LtacpException('MD5 checksum comparison failed, remote error: '+output_md5_remote[1]+' --- '+output_md5_local[1])
        else:
            raise LtacpException('SSH (for sending data and checksum) failed: '+output_remote_data[1]+' --- '+output_remote_checksum[1])

    except LtacpException as e:
        # Something went wrong
        logger.error('ltacp: ! Fatal Error: %s' % str(e))
        code = 1

    # ---
    # wrapping up
    # ---

    # remove remote fifo
    removeRemoteFile(src_user, src_host, remote_data_fifo)

    # remove local data fifo
    logger.info('ltacp: removing local data fifo for globus-url-copy: %s' % local_data_fifo)
    os.remove(local_data_fifo)

    logger.info('ltacp: waiting for subprocesses to complete...')
    # cancel any started process
    for p in started_procs:
        if p.poll() == None:
            p.terminate()
            logger.info('ltacp: terminated', p.pid)

    logger.info('ltacp: successfully completed transfer of %s:%s to %s' % (src_host, src_path_data, dst_surl))
    return code


# execute command and optionally return exit code or output streams
def execute(cmd, return_output=False):
    logger.info('ltacp: executing: %s' % ' '.join(cmd))
    p_cmd = Popen(cmd, stdout=PIPE, stderr=PIPE)
    output_cmd = p_cmd.communicate()
    if return_output:
        return output_cmd
    else:
        return p_cmd.returncode


# remove file from srm
def srmrm(surl):
    return execute(['srmrm', surl])


# remove (empty) directory from srm
def srmrmdir(surl):
    return execute(['srmrmdir', surl])


# remove file from srm
def srmll(surl):
    return execute(['srmls', '-l', surl], return_output=True)

# get checksum from srm via srmls
def get_srm_a32_checksum(surl):
    output = srmll(surl)[0]

    if not 'Checksum type:' in output:
        return False

    if 'Checksum type:' in output:
        cstype = output.split('Checksum type:')[1].split()[0].strip()
        if cstype.lower() != 'adler32':
            return False

    if 'Checksum value:' in output:
        return output.split('Checksum value:')[1].lstrip().split()[0]

    return False

#recursively checks for presence of parent directory and created the missing part of a tree
def create_missing_directories(surl):

    parent, child = os.path.split(surl)
    missing = []

    # determine missing dirs
    while parent:
        code = execute(['srmls', parent])
        if code == 0:
            logger.info('ltacp: srmls returned successfully, so this path apparently exists: %s' % parent)
            break;
        else:
            parent, child = os.path.split(parent)
            missing.append(child)

    # recreate missing dirs
    while len(missing) > 0:
        parent = parent + '/' + missing.pop()
        code = execute(['srmmkdir',"-retry_num=0",parent])
        if code != 0:
            logger.info('ltacp: failed to create missing directory: %s' % parent)
            return code

    logger.info('ltacp: successfully created parent directory: %s' % parent)
    return 0


# limited standalone mode for testing:
# usage: ltacp.py <remote-host> <remote-path> <surl>
if __name__ == '__main__':

# transfer test:
    transfer(sys.argv[1], sys.argv[2], sys.argv[3])

# srmls/srmrm test:
    #print get_srm_a32_checksum(sys.argv[3])
#    print srmrm(sys.argv[3])
#    print get_srm_a32_checksum(sys.argv[3])

# srmmkdir test:
#   print create_missing_directories(sys.argv[3])
