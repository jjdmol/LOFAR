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
import random
import atexit

logger = logging.getLogger('Slave')

if __name__ == '__main__':
    log_handler = logging.StreamHandler()
    formatter = logging.Formatter('%(asctime)-15s %(levelname)s %(message)s')
    formatter.converter = time.gmtime
    log_handler.setFormatter(formatter)
    logger.addHandler(log_handler)
    logger.setLevel(logging.INFO)


class LtacpException(Exception):
     def __init__(self, value):
         self.value = value
     def __str__(self):
         return str(self.value)

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


class LtaCp:
    def __init__(self,
                 src_host,
                 src_path_data,
                 dst_surl,
                 src_user=None):
        self.src_host = src_host
        self.src_path_data = src_path_data
        self.dst_surl = dst_surl
        self.src_user = src_user if src_user else getpass.getuser()
        self.logId = os.path.basename(self.src_path_data)
        self.started_procs = {}

        # make sure that all subprocesses and fifo's are cleaned up when the program exits
        atexit.register(self.cleanup)

    # transfer file/directory from given src to SRM location with given turl
    def transfer(self):

        # for cleanup
        self.started_procs = {}

        try:
            dst_turl = convert_surl_to_turl(self.dst_surl)
            logger.info('ltacp %s: initiating transfer of %s:%s to %s' % (self.logId, self.src_host, self.src_path_data, self.dst_surl))

            #---
            # Server part
            #---

            # we'll randomize ports
            # to minimize initial collision, randomize based on path and time
            random.seed(hash(self.src_path_data) ^ hash(time.time()))

            # pick initial random port for data receiver
            port_data = str(random.randint(49152, 65535))
            while True:
                # start listen for data stream
                cmd_data_in = ['nc', '-l', '-q','0', port_data]
                logger.info('ltacp %s: listening for data. executing: %s' % (self.logId, ' '.join(cmd_data_in)))
                p_data_in = Popen(cmd_data_in, stdout=PIPE, stderr=PIPE)

                time.sleep(0.5)
                if p_data_in.poll():
                    # nc returned prematurely, pick another port to listen to
                    o, e = p_data_in.communicate()
                    logger.info('ltacp %s: nc returned prematurely: %s' % (self.logId, e.strip()))
                    port_data = str(random.randint(49152, 65535))
                else:
                    self.started_procs[p_data_in] = cmd_data_in
                    break


            # pick initial random port for md5 receiver
            port_md5 = str(random.randint(49152, 65535))
            while True:
                # start listen for checksums
                cmd_md5_receive = ['nc','-l', '-q','0', port_md5]
                logger.info('ltacp %s: listening for md5 checksums. executing: %s' % (self.logId, ' '.join(cmd_md5_receive)))
                p_md5_receive = Popen(cmd_md5_receive, stdout=PIPE, stderr=PIPE)

                time.sleep(0.5)
                if p_md5_receive.poll():
                    # nc returned prematurely, pick another port to listen to
                    o, e = p_md5_receive.communicate()
                    logger.info('ltacp %s: nc returned prematurely: %s' % (self.logId, e.strip()))
                    port_md5 = str(random.randint(49152, 65535))
                else:
                    self.started_procs[p_md5_receive] = cmd_md5_receive
                    break

            # create fifo paths
            self.local_data_fifo = '/tmp/ltacp_datapipe_'+self.src_host+'_'+port_data
            self.remote_data_fifo = '/tmp/ltacp_md5_receivepipe_'+port_md5

            # create local fifo to stream data to globus-url-copy
            logger.info('ltacp %s: creating data fifo for globus-url-copy: %s' % (self.logId, self.local_data_fifo))
            if os.path.exists(self.local_data_fifo):
                os.remove(self.local_data_fifo)
            os.mkfifo(self.local_data_fifo)

            # create local fifo to stream data to adler32
            self.local_adler32_fifo = self.local_data_fifo+'_adler32'
            logger.info('ltacp %s: creating data fifo for adler32: %s' % (self.logId, self.local_adler32_fifo))
            if os.path.exists(self.local_adler32_fifo):
                os.remove(self.local_adler32_fifo)
            os.mkfifo(self.local_adler32_fifo)

            # start tee incoming data stream to fifo (pipe stream further for checksum)
            cmd_tee_data = ['tee', self.local_data_fifo]
            logger.info('ltacp %s: splitting datastream. executing on stdout of data listener: %s' % (self.logId, ' '.join(cmd_tee_data),))
            p_tee_data = Popen(cmd_tee_data, stdin=p_data_in.stdout, stdout=PIPE)
            self.started_procs[p_tee_data] = cmd_tee_data

            # start tee incoming data stream to fifo (pipe stream further for checksum)
            cmd_tee_checksums = ['tee', self.local_adler32_fifo]
            logger.info('ltacp %s: splitting datastream again. executing on stdout of 1st data tee: %s' % (self.logId, ' '.join(cmd_tee_checksums),))
            p_tee_checksums = Popen(cmd_tee_checksums, stdin=p_tee_data.stdout, stdout=PIPE)
            self.started_procs[p_tee_checksums] = cmd_tee_checksums

            # start computing md5 checksum of incoming data stream
            cmd_md5_local = ['md5sum']
            logger.info('ltacp %s: computing local md5 checksum. executing on stdout of 2nd data tee: %s' % (self.logId, ' '.join(cmd_md5_local)))
            p_md5_local = Popen(cmd_md5_local, stdin=p_tee_checksums.stdout, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_md5_local] = cmd_md5_local

            # start computing adler checksum of incoming data stream
            cmd_a32_local = ['./md5adler/a32', self.local_adler32_fifo]
            logger.info('ltacp %s: computing local adler32 checksum. executing: %s' % (self.logId, ' '.join(cmd_a32_local)))
            p_a32_local = Popen(cmd_a32_local, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_a32_local] = cmd_a32_local

            # start copy fifo stream to SRM
            cmd_data_out = ['globus-url-copy', self.local_data_fifo, dst_turl]
            logger.info('ltacp %s: copying data stream into globus-url-copy. executing: %s' % (self.logId, ' '.join(cmd_data_out)))
            p_data_out = Popen(cmd_data_out, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_data_out] = cmd_data_out

            # Check if receiver side is set up correctly
            # and all processes are still waiting for input from client
            finished_procs = dict((p, cl) for (p, cl) in self.started_procs.items() if p.poll() is not None)

            if len(finished_procs):
                msg = ''
                for p, cl in finished_procs.items():
                    msg += "  process pid:%d exited prematurely with exit code %d. cmdline: %s\n" % (p.pid, ret, cl)
                raise LtacpException("ltacp: %s %s local process(es) exited prematurely\n%s" % (self.logId, msg))

            #---
            # Client part
            #---

            # start remote copy on src host:
            # 1) create fifo
            # 2) send tar stream of data/dir + tee to fifo for 3)
            # 3) simultaneously to 2), calculate checksum of fifo stream
            # 4) break fifo
            cmd_remote_mkfifo = ['ssh '+self.src_user+'@'+self.src_host+
                                ' \'mkfifo '+self.remote_data_fifo+
                                '\'']
            logger.info('ltacp %s: remote creating fifo. executing: %s' % (self.logId, ' '.join(cmd_remote_mkfifo)))
            p_remote_mkfifo = Popen(cmd_remote_mkfifo, shell=True, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_mkfifo] = cmd_remote_mkfifo

            # block until fifo is created
            output_remote_mkfifo = p_remote_mkfifo.communicate()
            if p_remote_mkfifo.returncode != 0:
                raise LtacpException('Remote fifo creation failed: '+output_remote_mkfifo[1])

            # start sending remote data, tee to fifo
            src_path_parent, src_path_child = os.path.split(self.src_path_data)
            cmd_remote_data = ['ssh '+self.src_user+'@'+self.src_host+
                            ' \'cd '+src_path_parent+
                            ' ; tar c -O '+src_path_child+' | tee '+self.remote_data_fifo+' | nc --send-only '+getfqdn()+' '+port_data+
                            '\'']
            logger.info('ltacp %s: remote starting transfer. executing: %s' % (self.logId, ' '.join(cmd_remote_data)))
            p_remote_data = Popen(cmd_remote_data, shell=True, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_data] = cmd_remote_data

            # start computation of checksum on remote fifo stream
            cmd_remote_checksum = ['ssh '+self.src_user+'@'+self.src_host+
                                ' \'cat '+self.remote_data_fifo+' | md5sum | nc --send-only '+getfqdn()+' '+port_md5+
                                '\'']
            logger.info('ltacp %s: remote starting computation of md5 checksum. executing: %s' % (self.logId, ' '.join(cmd_remote_checksum)))
            p_remote_checksum = Popen(cmd_remote_checksum, shell=True, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_checksum] = cmd_remote_checksum


            # waiting for output, comparing checksums, etc.
            logger.info('ltacp %s: waiting for remote data transfer to finish...' % self.logId)
            output_remote_data = p_remote_data.communicate()
            if p_remote_data.returncode != 0:
                raise LtacpException('ltacp %s: Error in remote data transfer: %s' % (self.logId, output_remote_data[1]))
            logger.debug('ltacp %s: remote data transfer finished...' % self.logId)

            logger.info('ltacp %s: waiting for remote md5 checksum computation to finish...' % self.logId)
            output_remote_checksum = p_remote_checksum.communicate()
            if p_remote_checksum.returncode != 0:
                raise LtacpException('ltacp %s: Error in remote md5 checksum computation: %s' % (self.logId, output_remote_checksum[1]))
            logger.debug('ltacp %s: remote md5 checksum computation finished.' % self.logId)

            logger.debug('ltacp %s: waiting to receive remote md5 checksum...' % self.logId)
            output_md5_receive = p_md5_receive.communicate()
            if p_md5_receive.returncode != 0:
                raise LtacpException('ltacp %s: Error while receiving remote md5 checksum: %s' % (self.logId, output_md5_receive[1]))
            logger.debug('ltacp %s: received md5 checksum.' % self.logId)

            logger.debug('ltacp %s: waiting for local computation of md5 checksum...' % self.logId)
            output_md5_local = p_md5_local.communicate()
            if p_md5_local.returncode != 0:
                raise LtacpException('ltacp %s: Error while receiving remote md5 checksum: %s' % (self.logId, output_md5_local[1]))
            logger.debug('ltacp %s: computed local md5 checksum.' % self.logId)

            # compare remote and local md5 checksums
            md5_checksum_remote = output_md5_receive[0].split()[0]
            md5_checksum_local = output_md5_local[0].split()[0]
            if(md5_checksum_remote != md5_checksum_local):
                raise LtacpException('md5 checksum reported by client (%s) does not match local checksum of incoming data stream (%s)' % (self.logId, md5_checksum_remote, md5_checksum_local))
            logger.info('ltacp %s: remote and local md5 checksums are equal: %s' % (self.logId, md5_checksum_local,))

            logger.debug('ltacp %s: waiting for transfer via globus-url-copy to LTA to finish...' % self.logId)
            output_data_out = p_data_out.communicate()
            if p_data_out.returncode != 0:
                raise LtacpException('ltacp %s: transfer via globus-url-copy to LTA failed: %s' % (self.logId,output_data_out[1]))
            logger.info('ltacp %s: data transfer via globus-url-copy to LTA complete.' % self.logId)

            logger.debug('ltacp %s: waiting for local adler32 checksum to complete...' % self.logId)
            output_a32_local = p_a32_local.communicate()
            if p_a32_local.returncode != 0:
                raise LtacpException('local adler32 checksum computation failed: '+str(output_a32_local))
            logger.debug('ltacp %s: finished computation of local adler32 checksum' % self.logId)
            a32_checksum_local = output_a32_local[0].split()[1]

            logger.debug('ltacp %s: fetching adler32 checksum from LTA...' % self.logId)
            srm_a32_checksum = get_srm_a32_checksum(self.dst_surl)

            if not srm_a32_checksum:
                raise LtacpException('ltacp %s: Could not get srm adler32 checksum for: %s'  % (self.logId, self.dst_surl))

            if(srm_a32_checksum != a32_checksum_local):
                raise LtacpException('ltacp %s: adler32 checksum reported by srm (%s) does not match original data checksum (%s)' % (self.logId,
                                                                                                                                     srm_a32_checksum,
                                                                                                                                     a32_checksum_local))

            logger.info('ltacp %s: adler32 checksums are equal: %s' % (self.logId, a32_checksum_local))
            logger.info('ltacp %s: transfer to LTA completed successfully.' % (self.logId))

        except Exception as e:
            # Something went wrong
            logger.error('ltacp %s: Error in transfer: %s' % (self.logId, str(e)))
            # re-raise the exception to the caller
            raise
        finally:
            # cleanup
            self.cleanup()

        logger.info('ltacp %s: successfully completed transfer of %s:%s to %s' % (self.logId, self.src_host, self.src_path_data, self.dst_surl))
        return (md5_checksum_local, a32_checksum_local)

    def cleanup(self):
        logger.debug('ltacp %s: cleaning up' % (self.logId))

        if self.remote_data_fifo:
            '''remove a file (or fifo) on a remote host. Test if file exists before deleting.'''
            cmd_remote_rm = ['ssh %s@%s \'if [ -e "%s" ] ; then rm %s ; fi ;\'' % (self.src_user, self.src_host, self.remote_data_fifo, self.remote_data_fifo)]
            logger.info('ltacp %s: remote removing file if existing. executing: %s' % (self.logId, ' '.join(cmd_remote_rm)))
            p_remote_rm = Popen(cmd_remote_rm, shell=True, stdout=PIPE, stderr=PIPE)
            p_remote_rm.communicate()
            if p_remote_rm.returncode != 0:
                logger.error("Could not remove remote file %s@%s:%s\n%s" % (self.src_user, self.src_host, self.remote_data_fifo, p_remote_rm.stderr))
            self.remote_data_fifo = None

        # remove local data fifo
        if os.path.exists(self.local_data_fifo):
            logger.info('ltacp %s: removing local data fifo for globus-url-copy: %s' % (self.logId, self.local_data_fifo))
            os.remove(self.local_data_fifo)

        # remove local data fifo
        if os.path.exists(self.local_adler32_fifo):
            logger.info('ltacp %s: removing local data fifo for adler32: %s' % (self.logId, self.local_adler32_fifo))
            os.remove(self.local_adler32_fifo)

        # cancel any started running process, as they should all be finished by now
        self.stopRunningProcesses()

        logger.debug('ltacp %s: finished cleaning up' % (self.logId))

    def stopRunningProcesses(self):
        running_procs = dict((p, cl) for (p, cl) in self.started_procs.items() if p.poll() == None)

        if len(running_procs):
            logger.warning('ltacp %s: terminating %d running subprocesses...' % (self.logId, len(running_procs)))
            for p,cl in running_procs.items():
                logger.warning('ltacp %s: terminated running process pid=%d cmdline=%s' % (self.logId, p.pid, cl))
                p.terminate()
            logger.info('ltacp %s: terminated %d running subprocesses...' % (self.logId, len(running_procs)))


# execute command and return (stdout, stderr, returncode) tuple
def execute(cmd):
    logger.info('executing: %s' % ' '.join(cmd))
    p_cmd = Popen(cmd, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p_cmd.communicate()
    return (stdout, stderr, p_cmd.returncode)


# remove file from srm
def srmrm(surl):
    return execute(['srmrm', surl])[2]


# remove (empty) directory from srm
def srmrmdir(surl):
    return execute(['srmrmdir', surl])[2]


# remove file from srm
def srmll(surl):
    return execute(['srmls', '-l', surl])

# get checksum from srm via srmls
def get_srm_a32_checksum(surl):
    output, errors, code = srmll(surl)

    if code != 0:
        return False

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
        code = execute(['srmls', parent])[2]
        if code == 0:
            logger.info('ltacp %s: srmls returned successfully, so this path apparently exists: %s' % parent)
            break;
        else:
            parent, child = os.path.split(parent)
            missing.append(child)

    # recreate missing dirs
    while len(missing) > 0:
        parent = parent + '/' + missing.pop()
        code = execute(['srmmkdir',"-retry_num=0",parent])[2]
        if code != 0:
            logger.info('ltacp %s: failed to create missing directory: %s' % parent)
            return code

    logger.info('ltacp %s: successfully created parent directory: %s' % parent)
    return 0


# limited standalone mode for testing:
# usage: ltacp.py <remote-host> <remote-path> <surl>
if __name__ == '__main__':

    if len(sys.argv) < 4:
        print 'example: ./ltacp.py 10.196.232.11 /home/users/ingest/1M.txt srm://lofar-srm.fz-juelich.de:8443/pnfs/fz-juelich.de/data/lofar/ops/test/eor/1M.txt'
        sys.exit()

# transfer test:
    cp = LtaCp(sys.argv[1], sys.argv[2], sys.argv[3], 'ingest')
    cp.transfer()

# srmls/srmrm test:
    #print get_srm_a32_checksum(sys.argv[3])
#    print srmrm(sys.argv[3])
#    print get_srm_a32_checksum(sys.argv[3])

# srmmkdir test:
#   print create_missing_directories(sys.argv[3])

