#!/usr/bin/env python

# LTACP Python module for transferring data from a remote node to a remote SRM via localhost
#
# Remote data can be individual files or directories. Directories will be tar-ed.
#
# Between the remote and local host, md5 checksums are used to ensure integrity of the file,
# adler32  is used between localhost and the SRM.

import logging
from optparse import OptionParser
from subprocess import Popen, PIPE
import socket
import os, sys, getpass
import time
import random
import math
import atexit
from datetime import datetime, timedelta

# TODO: reuse method from LCS.PyCommon.utils
def humanreadablesize(num, suffix='B', base=1000):
    """ converts the given size (number) to a human readable string in powers of 'base'"""
    try:
        for unit in ['', 'K', 'M', 'G', 'T', 'P', 'E', 'Z']:
            if abs(num) < float(base):
                return "%3.1f%s%s" % (num, unit, suffix)
            num /= float(base)
        return "%.2f%s%s" % (num, 'Y', suffix)
    except TypeError:
        return str(num)

logger = logging.getLogger('Slave')

_ingest_init_script = '/globalhome/ingest/service/bin/init.sh'

class LtacpException(Exception):
     def __init__(self, value):
         self.value = value
     def __str__(self):
         return str(self.value)

def getLocalIPAddress():
    host = socket.gethostname()
    ipaddress = socket.gethostbyname(host)
    return ipaddress

# converts given srm url of an LTA site into a transport url as needed by gridftp. (Sring replacement based on arcane knowledge.)
def convert_surl_to_turl(surl):
    #list of sara doors is based on recent actual transfers using srmcp, which translates the surl to a 'random' turl
    sara_nodes = ['fly%d' % i for i in range(1, 11)]  + \
                 ['by27-%d' % i for i in range(1, 10)] + \
                 ['bw27-%d' % i for i in range(1, 10)] + \
                 ['by32-%d' % i for i in range(1, 10)] + \
                 ['bw32-%d' % i for i in range(4, 10)] + \
                 ['s35-0%d' % i for i in range(1, 5)] + \
                 ['v40-%d' % i for i in range(8, 11)] + \
                 ['rabbit%d' % i for i in range(1, 4)]
    sara_turl = 'gsiftp://%s.grid.sara.nl:2811' % sara_nodes[random.randint(0, len(sara_nodes)-1)]
    turl = surl.replace("srm://srm.grid.sara.nl:8443",sara_turl, 1)
    turl = turl.replace("srm://srm.grid.sara.nl",sara_turl,1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de:8443", "gsiftp://dcachepool%d.fz-juelich.de:2811" % (random.randint(9, 16),), 1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de", "gsiftp://dcachepool%d.fz-juelich.de:2811" % (random.randint(9, 16),), 1)
    turl = turl.replace("srm://srm.target.rug.nl:8444","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    turl = turl.replace("srm://srm.target.rug.nl","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    turl = turl.replace("srm://lta-head.lofar.psnc.pl:8443", "gsiftp://door0%d.lofar.psnc.pl:2811" % (random.randint(1, 2),), 1)
    return turl

def createNetCatCmd(user, host):
    '''helper method to determine the proper call syntax for netcat on host'''

    # nc has no version option or other ways to check it's version
    # so, just try the variants and pick the first one that does not fail
    nc_variants = ['nc --send-only', 'nc -q 0', 'nc']

    for nc_variant in nc_variants:
        cmd = ['ssh', '-n', '-x', '%s@%s' % (user, host), nc_variant]
        p = Popen(cmd, stdout=PIPE, stderr=PIPE)
        out, err = p.communicate()
        if 'invalid option' not in err:
            return nc_variant

    raise LtacpException('could not determine remote netcat version')


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
        self.fifos = []
        self.ssh_cmd = ['ssh', '-tt', '-n', '-x', '-q', '%s@%s' % (self.src_user, self.src_host)]

        self.localIPAddress = getLocalIPAddress()
        self.localNetCatCmd = createNetCatCmd(getpass.getuser(), self.localIPAddress)
        self.remoteNetCatCmd = createNetCatCmd(self.src_user, self.src_host)

        # make sure that all subprocesses and fifo's are cleaned up when the program exits
        atexit.register(self.cleanup)

    # transfer file/directory from given src to SRM location with given turl
    def transfer(self):

        # for cleanup
        self.started_procs = {}
        self.fifos = []

        try:
            dst_turl = convert_surl_to_turl(self.dst_surl)
            logger.info('ltacp %s: initiating transfer of %s:%s to %s' % (self.logId, self.src_host, self.src_path_data, self.dst_surl))

            # get input datasize
            cmd_remote_du = self.ssh_cmd + ['du -b --max-depth=0 %s' % (self.src_path_data,)]
            logger.info('ltacp %s: remote getting datasize. executing: %s' % (self.logId, ' '.join(cmd_remote_du)))
            p_remote_du = Popen(cmd_remote_du, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_du] = cmd_remote_du

            # block until du is finished
            output_remote_du = p_remote_du.communicate()
            del self.started_procs[p_remote_du]
            if p_remote_du.returncode != 0:
                raise LtacpException('ltacp %s: remote du failed: \nstdout: %s\nstderr: %s' % (self.logId,
                                                                                               output_remote_du[0],
                                                                                               output_remote_du[1]))
            # compute various parameters for progress logging
            input_datasize = int(output_remote_du[0].split()[0])
            logger.info('ltacp %s: input datasize: %d bytes, %s' % (self.logId, input_datasize, humanreadablesize(input_datasize)))
            estimated_tar_size = 512*(input_datasize / 512) + 3*512 #512byte header, 2*512byte ending, 512byte modulo data
            logger.info('ltacp %s: estimated_tar_size: %d bytes, %s' % (self.logId, estimated_tar_size, humanreadablesize(estimated_tar_size)))


            #---
            # Server part
            #---

            # we'll randomize ports
            # to minimize initial collision, randomize based on path and time
            random.seed(hash(self.src_path_data) ^ hash(time.time()))

            p_data_in, port_data = self._ncListen('data')


            self.local_data_fifo = '/tmp/ltacp_datapipe_%s_%s' % (self.src_host, self.logId)

            logger.info('ltacp %s: creating data fifo: %s' % (self.logId, self.local_data_fifo))
            if os.path.exists(self.local_data_fifo):
                os.remove(self.local_data_fifo)
            os.mkfifo(self.local_data_fifo)
            if not os.path.exists(self.local_data_fifo):
                raise LtacpException("ltacp %s: Could not create fifo: %s" % (self.logId, self.local_data_fifo))

            # transfer incomming data stream via md5a32bc to compute md5, adler32 and byte_count
            # data is written to fifo, which is then later fed into globus-url-copy
            # on stdout we can monitor progress
            # set progress message step 0f 0.5% of estimated_tar_size
            currdir = os.path.dirname(os.path.realpath(__file__))
            cmd_md5a32bc = [currdir + '/md5adler/md5a32bc', '-p', str(estimated_tar_size/200), self.local_data_fifo]
            logger.info('ltacp %s: processing data stream for md5, adler32 and byte_count. executing: %s' % (self.logId, ' '.join(cmd_md5a32bc),))
            p_md5a32bc = Popen(cmd_md5a32bc, stdin=p_data_in.stdout, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_md5a32bc] = cmd_md5a32bc

            # start copy fifo stream to globus-url-copy
            guc_options = ['-cd', #create remote directories if missing
                           '-p 4', #number of parallel ftp connections
                           '-bs 131072', #buffer size
                           '-b', # binary
                           '-nodcau', # turn off data channel authentication for ftp transfers
                           ]
            cmd_data_out = ['/bin/bash', '-c', 'source %s; globus-url-copy %s file://%s %s' % (_ingest_init_script, ' '.join(guc_options), self.local_data_fifo, dst_turl)]
            logger.info('ltacp %s: copying data stream into globus-url-copy. executing: %s' % (self.logId, ' '.join(cmd_data_out)))
            p_data_out = Popen(cmd_data_out, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_data_out] = cmd_data_out

            # Check if receiver side is set up correctly
            # and all processes are still waiting for input from client
            finished_procs = dict((p, cl) for (p, cl) in self.started_procs.items() if p.poll() is not None)

            if len(finished_procs):
                msg = ''
                for p, cl in finished_procs.items():
                    o, e = p.communicate()
                    msg += "  process pid:%d exited prematurely with exit code %d. cmdline: %s\nstdout: %s\nstderr: %s\n" % (p.pid,
                                                                                                                             p.returncode,
                                                                                                                             cl,
                                                                                                                             o,
                                                                                                                             e)
                raise LtacpException("ltacp %s: %d local process(es) exited prematurely\n%s" % (self.logId, len(finished_procs), msg))

            #---
            # Client part
            #---

            # start remote copy on src host:
            # 1) create fifo
            # 2) send tar stream of data/dir + tee to fifo for 3)
            # 3) simultaneously to 2), calculate checksum of fifo stream
            # 4) break fifo

            self.remote_data_fifo = '/tmp/ltacp_md5_pipe_%s' % (self.logId, )
            #make sure there is no old remote fifo
            self._removeRemoteFifo()
            cmd_remote_mkfifo = self.ssh_cmd + ['mkfifo %s' % (self.remote_data_fifo,)]
            logger.info('ltacp %s: remote creating fifo. executing: %s' % (self.logId, ' '.join(cmd_remote_mkfifo)))
            p_remote_mkfifo = Popen(cmd_remote_mkfifo, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_mkfifo] = cmd_remote_mkfifo

            # block until fifo is created
            output_remote_mkfifo = p_remote_mkfifo.communicate()
            del self.started_procs[p_remote_mkfifo]
            if p_remote_mkfifo.returncode != 0:
                raise LtacpException('ltacp %s: remote fifo creation failed: \nstdout: %s\nstderr: %s' % (self.logId, output_remote_mkfifo[0],output_remote_mkfifo[1]))


            # get input filetype
            cmd_remote_filetype = self.ssh_cmd + ['ls -l %s' % (self.src_path_data,)]
            logger.info('ltacp %s: remote getting file info. executing: %s' % (self.logId, ' '.join(cmd_remote_filetype)))
            p_remote_filetype = Popen(cmd_remote_filetype, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_filetype] = cmd_remote_filetype

            # block until ls is finished
            output_remote_filetype = p_remote_filetype.communicate()
            del self.started_procs[p_remote_filetype]
            if p_remote_filetype.returncode != 0:
                raise LtacpException('ltacp %s: remote file listing failed: \nstdout: %s\nstderr: %s' % (self.logId,
                                                                                                         output_remote_filetype[0],
                                                                                                         output_remote_filetype[1]))

            # determine if input is file
            input_is_file = (output_remote_filetype[0][0] == '-')
            logger.info('ltacp %s: remote path is a %s' % (self.logId, 'file' if input_is_file else 'directory'))

            with open(os.devnull, 'r') as devnull:
                # start sending remote data, tee to fifo
                if input_is_file:
                    cmd_remote_data = self.ssh_cmd + ['cat %s | tee %s | %s %s %s' % (self.src_path_data,
                        self.remote_data_fifo,
                        self.remoteNetCatCmd,
                        self.localIPAddress,
                        port_data)]
                else:
                    src_path_parent, src_path_child = os.path.split(self.src_path_data)
                    cmd_remote_data = self.ssh_cmd + ['cd %s && tar c -O %s | tee %s | %s %s %s' % (src_path_parent,
                        src_path_child,
                        self.remote_data_fifo,
                        self.remoteNetCatCmd,
                        self.localIPAddress,
                        port_data)]
                logger.info('ltacp %s: remote starting transfer. executing: %s' % (self.logId, ' '.join(cmd_remote_data)))
                p_remote_data = Popen(cmd_remote_data, stdin=devnull, stdout=PIPE, stderr=PIPE)
                self.started_procs[p_remote_data] = cmd_remote_data

                # start computation of checksum on remote fifo stream
                cmd_remote_checksum = self.ssh_cmd + ['md5sum %s' % (self.remote_data_fifo,)]
                logger.info('ltacp %s: remote starting computation of md5 checksum. executing: %s' % (self.logId, ' '.join(cmd_remote_checksum)))
                p_remote_checksum = Popen(cmd_remote_checksum, stdin=devnull, stdout=PIPE, stderr=PIPE)
                self.started_procs[p_remote_checksum] = cmd_remote_checksum

                # timedelta.total_seconds is only available for python >= 2.7
                def timedelta_total_seconds(td):
                    return (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / float(10**6)

                # waiting for output, comparing checksums, etc.
                logger.info('ltacp %s: transfering... waiting for progress...' % self.logId)
                transfer_start_time = datetime.utcnow()
                prev_progress_time = datetime.utcnow()
                prev_bytes_transfered = 0

                # wait and poll for progress while all processes are runnning
                while len([p for p in self.started_procs.keys() if p.poll() is not None]) == 0:
                    try:
                        current_progress_time = datetime.utcnow()
                        elapsed_secs_since_prev = timedelta_total_seconds(current_progress_time - prev_progress_time)

                        if elapsed_secs_since_prev > 120:
                            raise LtacpException('ltacp %s: transfer stalled.' % (self.logId))

                        # read and process md5a32bc stdout lines to create progress messages
                        nextline = p_md5a32bc.stdout.readline().strip()

                        if len(nextline) > 0:
                            total_bytes_transfered = int(nextline.split()[0].strip())
                            percentage_done = (100.0*float(total_bytes_transfered))/float(estimated_tar_size)
                            elapsed_secs_since_start = timedelta_total_seconds(current_progress_time - transfer_start_time)
                            if percentage_done > 0 and elapsed_secs_since_start > 0 and elapsed_secs_since_prev > 0:
                                avg_speed = total_bytes_transfered / elapsed_secs_since_start
                                current_bytes_transfered = total_bytes_transfered - prev_bytes_transfered
                                current_speed = current_bytes_transfered / elapsed_secs_since_prev
                                if elapsed_secs_since_prev > 60 or current_bytes_transfered > 0.05*estimated_tar_size:
                                    prev_progress_time = current_progress_time
                                    prev_bytes_transfered = total_bytes_transfered
                                    percentage_to_go = 100.0 - percentage_done
                                    time_to_go = elapsed_secs_since_start * percentage_to_go / percentage_done
                                    logger.info('ltacp %s: transfered %s %.1f%% at avgSpeed=%s (%s) curSpeed=%s (%s) to_go=%s to %s' % (self.logId,
                                                                                                            humanreadablesize(total_bytes_transfered),
                                                                                                            percentage_done,
                                                                                                            humanreadablesize(avg_speed, 'Bps'),
                                                                                                            humanreadablesize(avg_speed*8, 'bps'),
                                                                                                            humanreadablesize(current_speed, 'Bps'),
                                                                                                            humanreadablesize(current_speed*8, 'bps'),
                                                                                                            timedelta(seconds=int(round(time_to_go))),
                                                                                                            dst_turl))
                        time.sleep(0.05)
                    except KeyboardInterrupt:
                        self.cleanup()
                    except Exception as e:
                        logger.error('ltacp %s: %s' % (self.logId, str(e)))

                logger.info('ltacp %s: waiting for transfer via globus-url-copy to LTA to finish...' % self.logId)
                output_data_out = p_data_out.communicate()
                if p_data_out.returncode != 0:
                    raise LtacpException('ltacp %s: transfer via globus-url-copy to LTA failed: %s' % (self.logId, output_data_out[1]))
                logger.info('ltacp %s: data transfer via globus-url-copy to LTA complete.' % self.logId)

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

                logger.info('ltacp %s: waiting for local computation of md5 adler32 and byte_count...' % self.logId)
                output_md5a32bc_local = p_md5a32bc.communicate()
                if p_md5a32bc.returncode != 0:
                    raise LtacpException('ltacp %s: Error while computing md5 adler32 and byte_count: %s' % (self.logId, output_md5a32bc_local[1]))
                logger.debug('ltacp %s: computed local md5 adler32 and byte_count.' % self.logId)

                # process remote md5 checksums
                try:
                    md5_checksum_remote = output_remote_checksum[0].split(' ')[0]
                except Exception as e:
                    logger.error('ltacp %s: error while parsing remote md5: %s\n%s' % (self.logId, output_remote_checksum[0], output_remote_checksum[1]))
                    raise

                # process local md5 adler32 and byte_count
                try:
                    items = output_md5a32bc_local[1].splitlines()[-1].split(' ')
                    md5_checksum_local = items[0].strip()
                    a32_checksum_local = items[1].strip().zfill(8)
                    byte_count = int(items[2].strip())
                except Exception as e:
                    logger.error('ltacp %s: error while parsing md5 adler32 and byte_count outputs: %s' % (self.logId, output_md5a32bc_local[0]))
                    raise

                logger.info('ltacp %s: byte count of datastream is %d %s' % (self.logId, byte_count, humanreadablesize(byte_count)))

                # compare local and remote md5
                if(md5_checksum_remote != md5_checksum_local):
                    raise LtacpException('md5 checksum reported by client (%s) does not match local checksum of incoming data stream (%s)' % (self.logId, md5_checksum_remote, md5_checksum_local))
                logger.info('ltacp %s: remote and local md5 checksums are equal: %s' % (self.logId, md5_checksum_local,))

            logger.info('ltacp %s: fetching adler32 checksum from LTA...' % self.logId)
            srm_ok, srm_file_size, srm_a32_checksum = get_srm_size_and_a32_checksum(self.dst_surl)

            if not srm_ok:
                raise LtacpException('ltacp %s: Could not get srm adler32 checksum for: %s'  % (self.logId, self.dst_surl))

            if srm_a32_checksum != a32_checksum_local:
                raise LtacpException('ltacp %s: adler32 checksum reported by srm (%s) does not match original data checksum (%s)' % (self.logId,
                                                                                                                                     srm_a32_checksum,
                                                                                                                                     a32_checksum_local))

            logger.info('ltacp %s: adler32 checksums are equal: %s' % (self.logId, a32_checksum_local,))

            if int(srm_file_size) != int(byte_count):
                raise LtacpException('ltacp %s: file size reported by srm (%s) does not match datastream byte count (%s)' % (self.logId,
                                                                                                                             srm_file_size,
                                                                                                                             byte_count))

            logger.info('ltacp %s: srm file size and datastream byte count are equal: %s bytes (%s)' % (self.logId,
                                                                                                        srm_file_size,
                                                                                                        humanreadablesize(srm_file_size)))
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
        return (md5_checksum_local, a32_checksum_local, str(byte_count))

    def _ncListen(self, log_name):
        # pick initial random port for data receiver
        port = str(random.randint(49152, 65535))
        while True:
            # start listen for data stream
            cmd_listen = self.localNetCatCmd.split(' ') + ['-l', port]

            logger.info('ltacp %s: listening for %s. executing: %s' % (self.logId, log_name, ' '.join(cmd_listen)))
            p_listen = Popen(cmd_listen, stdout=PIPE, stderr=PIPE)

            time.sleep(0.5)
            if p_listen.poll() is not None:
                # nc returned prematurely, pick another port to listen to
                o, e = p_listen.communicate()
                logger.info('ltacp %s: nc returned prematurely: %s' % (self.logId, e.strip()))
                port = str(random.randint(49152, 65535))
            else:
                self.started_procs[p_listen] = cmd_listen
                return (p_listen, port)


    def _removeRemoteFifo(self):
        if hasattr(self, 'remote_data_fifo') and self.remote_data_fifo:
            '''remove a file (or fifo) on a remote host. Test if file exists before deleting.'''
            cmd_remote_ls = self.ssh_cmd + ['ls %s' % (self.remote_data_fifo,)]
            p_remote_ls = Popen(cmd_remote_ls, stdout=PIPE, stderr=PIPE)
            p_remote_ls.communicate()

            if p_remote_ls.returncode == 0:
                cmd_remote_rm = self.ssh_cmd + ['rm %s' % (self.remote_data_fifo,)]
                logger.info('ltacp %s: removing remote fifo. executing: %s' % (self.logId, ' '.join(cmd_remote_rm)))
                p_remote_rm = Popen(cmd_remote_rm, stdout=PIPE, stderr=PIPE)
                p_remote_rm.communicate()
                if p_remote_rm.returncode != 0:
                    logger.error("Could not remove remote fifo %s@%s:%s\n%s" % (self.src_user, self.src_host, self.remote_data_fifo, p_remote_rm.stderr))
                self.remote_data_fifo = None

    def cleanup(self):
        logger.debug('ltacp %s: cleaning up' % (self.logId))

        self._removeRemoteFifo()

        # remove local fifos
        for fifo in self.fifos:
            if os.path.exists(fifo):
                logger.info('ltacp %s: removing local fifo: %s' % (self.logId, fifo))
                os.remove(fifo)
        self.fifos = []

        # cancel any started running process, as they should all be finished by now
        running_procs = dict((p, cl) for (p, cl) in self.started_procs.items() if p.poll() == None)

        if len(running_procs):
            logger.warning('ltacp %s: terminating %d running subprocesses...' % (self.logId, len(running_procs)))
            for p,cl in running_procs.items():
                if isinstance(cl, list):
                    cl = ' '.join(cl)
                logger.warning('ltacp %s: terminated running process pid=%d cmdline: %s' % (self.logId, p.pid, cl))
                p.terminate()
            logger.info('ltacp %s: terminated %d running subprocesses...' % (self.logId, len(running_procs)))
        self.started_procs = {}

        logger.debug('ltacp %s: finished cleaning up' % (self.logId))



# execute command and return (stdout, stderr, returncode) tuple
def execute(cmd):
    logger.info('executing: %s' % ' '.join(cmd))
    p_cmd = Popen(cmd, stdout=PIPE, stderr=PIPE)
    stdout, stderr = p_cmd.communicate()
    return (stdout, stderr, p_cmd.returncode)


# remove file from srm
def srmrm(surl):
    return execute(['/bin/bash', '-c', 'source %s; srmrm %s' % (_ingest_init_script, surl)])

# remove (empty) directory from srm
def srmrmdir(surl):
    return execute(['/bin/bash', '-c', 'source %s; srmrmdir %s' % (_ingest_init_script, surl)])

# create directory in srm
def srmmkdir(surl):
    return execute(['/bin/bash', '-c', 'source %s; srmmkdir -retry_num=0 %s' % (_ingest_init_script, surl)])

# detailed listing
def srmls(surl):
    return execute(['/bin/bash', '-c', 'source %s; srmls %s' % (_ingest_init_script, surl)])

# detailed listing
def srmll(surl):
    return execute(['/bin/bash', '-c', 'source %s; srmls -l %s' % (_ingest_init_script, surl)])

# get file size and checksum from srm via srmll
def get_srm_size_and_a32_checksum(surl):
    try:
        output, errors, code = srmll(surl)

        if code != 0:
            return (False, None, None)

        pathLine = output.strip()
        pathLineItems = [x.strip() for x in pathLine.split()]

        if len(pathLineItems) < 2:
            #path line shorter than expected
            return (False, None, None)

        file_size = int(pathLineItems[0])

        if not 'Checksum type:' in output:
            return False

        if 'Checksum type:' in output:
            cstype = output.split('Checksum type:')[1].split()[0].strip()
            if cstype.lower() != 'adler32':
                return (False, None, None)

        if 'Checksum value:' in output:
            a32_value = output.split('Checksum value:')[1].lstrip().split()[0]
            return (True, file_size, a32_value)

    except:
        pass

    return (False, None, None)

#recursively checks for presence of parent directory and created the missing part of a tree
def create_missing_directories(surl):

    parent, child = os.path.split(surl)
    missing = []

    # determine missing dirs
    while parent:
        logger.info('checking path: %s' % parent)
        o, e, code = srmls(parent)
        if code == 0:
            logger.info('srmls returned successfully, so this path apparently exists: %s' % parent)
            break;
        else:
            parent, child = os.path.split(parent)
            missing.append(child)

    # recreate missing dirs
    while len(missing) > 0:
        parent = parent + '/' + missing.pop()
        code = srmmkdir(parent)[2]
        if code != 0:
            logger.info('failed to create missing directory: %s' % parent)
            return code

    logger.info('successfully created parent directory: %s' % parent)
    return 0


# limited standalone mode for testing:
# usage: ltacp.py <remote-host> <remote-path> <surl>
if __name__ == '__main__':
    logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

    # Check the invocation arguments
    parser = OptionParser("%prog [options] <source_host> <source_path> <lta-detination-srm-url>",
                          description='copy a file/directory from <source_host>:<source_path> to the LTA <lta-detination-srm-url>')
    parser.add_option("-u", "--user", dest="user", type="string", default=getpass.getuser(), help="username for to login on <host>, default: %s" % getpass.getuser())
    (options, args) = parser.parse_args()

    if len(args) != 3:
        parser.print_help()
        sys.exit(1)

    cp = LtaCp(args[0], args[1], args[2], options.user)
    cp.transfer()

