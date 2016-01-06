#!/usr/bin/env python

# LTACP Python module for transferring data from a remote node to a remote SRM via localhost
#
# Remote data can be individual files or directories. Directories will be tar-ed.
#
# Between the remote and local host, md5 checksums are used to ensure integrity of the file,
# adler32  is used between localhost and the SRM.

import logging
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

def getLocalIPAddress():
    host = socket.gethostname()
    ipaddress = socket.gethostbyname(host)
    return ipaddress

# converts given srm url of an LTA site into a transport url as needed by gridftp. (Sring replacement based on arcane knowledge.)
def convert_surl_to_turl(surl):
    sara_nodes = ['fly%d' % i for i in range(1, 10)] # + \
                 #['wasp%d' % i for i in range(1, 10)] + \
                 #['by27-%d' % i for i in range(1, 10)] + \
                 #['bw27-%d' % i for i in range(1, 10)] + \
                 #['by32-%d' % i for i in range(1, 10)] + \
                 #['bw32-%d' % i for i in range(1, 10)]
    sara_turl = 'gsiftp://%s.grid.sara.nl:2811' % sara_nodes[random.randint(0, len(sara_nodes)-1)]
    turl = surl.replace("srm://srm.grid.sara.nl:8443",sara_turl, 1)
    turl = turl.replace("srm://srm.grid.sara.nl",sara_turl,1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de:8443", "gsiftp://dcachepool%d.fz-juelich.de:2811" % (random.randint(9, 16),), 1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de", "gsiftp://dcachepool%d.fz-juelich.de:2811" % (random.randint(9, 16),), 1)
    turl = turl.replace("srm://srm.target.rug.nl:8444","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    turl = turl.replace("srm://srm.target.rug.nl","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    return turl

def createNetCatCmd(user, host):
    '''helper method to determine the proper call syntax for netcat on host'''

    # nc has no version option or other ways to check it's version
    # so, just try the variants and pick the first one that does not fail
    nc_variants = ['nc --send-only', 'nc -q 0']

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

            #---
            # Server part
            #---

            # we'll randomize ports
            # to minimize initial collision, randomize based on path and time
            random.seed(hash(self.src_path_data) ^ hash(time.time()))

            p_data_in, port_data = self._ncListen('data')
            p_md5_receive, port_md5 = self._ncListen('md5 checksums')

            # create fifo paths
            self.local_fifo_basename = '/tmp/ltacp_datapipe_%s_%s' % (self.src_host, self.logId)

            def createLocalFifo(fifo_postfix):
                fifo_path = '%s_%s' % (self.local_fifo_basename, fifo_postfix)
                logger.info('ltacp %s: creating data fifo: %s' % (self.logId, fifo_path))
                if os.path.exists(fifo_path):
                    os.remove(fifo_path)
                os.mkfifo(fifo_path)
                if not os.path.exists(fifo_path):
                    raise LtacpException("ltacp %s: Could not create fifo: %s" % (self.logId, fifo_path))
                self.fifos.append(fifo_path)
                return fifo_path

            self.local_data_fifo = createLocalFifo('globus_url_copy')
            self.local_byte_count_fifo = createLocalFifo('local_byte_count')
            self.local_adler32_fifo = createLocalFifo('local_adler32')

            # tee incoming data stream to fifo (and pipe stream in tee_proc.stdout)
            def teeDataStreams(pipe_in, fifo_out):
                cmd_tee = ['tee', fifo_out]
                logger.info('ltacp %s: splitting datastream. executing: %s' % (self.logId, ' '.join(cmd_tee),))
                tee_proc = Popen(cmd_tee, stdin=pipe_in, stdout=PIPE, stderr=PIPE)
                self.started_procs[tee_proc] = cmd_tee
                return tee_proc

            p_tee_data = teeDataStreams(p_data_in.stdout, self.local_data_fifo)
            p_tee_byte_count = teeDataStreams(p_tee_data.stdout, self.local_byte_count_fifo)
            p_tee_checksums = teeDataStreams(p_tee_byte_count.stdout, self.local_adler32_fifo)

            # start counting number of bytes in incoming data stream
            cmd_byte_count = ['wc', '-c', self.local_byte_count_fifo]
            logger.info('ltacp %s: computing byte count. executing: %s' % (self.logId, ' '.join(cmd_byte_count)))
            p_byte_count = Popen(cmd_byte_count, stdout=PIPE, stderr=PIPE, env=dict(os.environ, LC_ALL="C"))
            self.started_procs[p_byte_count] = cmd_byte_count

            # start computing md5 checksum of incoming data stream
            cmd_md5_local = ['md5sum']
            logger.info('ltacp %s: computing local md5 checksum. executing on data pipe: %s' % (self.logId, ' '.join(cmd_md5_local)))
            p_md5_local = Popen(cmd_md5_local, stdin=p_tee_checksums.stdout, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_md5_local] = cmd_md5_local

            # start computing adler checksum of incoming data stream
            cmd_a32_local = ['./md5adler/a32', self.local_adler32_fifo]
            #cmd_a32_local = ['md5sum', self.local_adler32_fifo]
            logger.info('ltacp %s: computing local adler32 checksum. executing: %s' % (self.logId, ' '.join(cmd_a32_local)))
            p_a32_local = Popen(cmd_a32_local, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_a32_local] = cmd_a32_local

            # start copy fifo stream to SRM
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

            self.remote_data_fifo = '/tmp/ltacp_md5_pipe_%s_%s' % (self.logId, port_md5)
            cmd_remote_mkfifo = self.ssh_cmd + ['mkfifo %s' % (self.remote_data_fifo,)]
            logger.info('ltacp %s: remote creating fifo. executing: %s' % (self.logId, ' '.join(cmd_remote_mkfifo)))
            p_remote_mkfifo = Popen(cmd_remote_mkfifo, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_mkfifo] = cmd_remote_mkfifo

            # block until fifo is created
            output_remote_mkfifo = p_remote_mkfifo.communicate()
            if p_remote_mkfifo.returncode != 0:
                raise LtacpException('ltacp %s: remote fifo creation failed: \nstdout: %s\nstderr: %s' % (self.logId, output_remote_mkfifo[0],output_remote_mkfifo[1]))

            # get input datasize
            cmd_remote_du = self.ssh_cmd + ['du -b --max-depth=0 %s' % (self.src_path_data,)]
            logger.info('ltacp %s: remote getting datasize. executing: %s' % (self.logId, ' '.join(cmd_remote_du)))
            p_remote_du = Popen(cmd_remote_du, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_du] = cmd_remote_du

            # block until du is finished
            output_remote_du = p_remote_du.communicate()
            if p_remote_du.returncode != 0:
                raise LtacpException('ltacp %s: remote fifo creation failed: \nstdout: %s\nstderr: %s' % (self.logId,
                                                                                                          output_remote_du[0],
                                                                                                          output_remote_du[1]))
            # compute various parameters for progress logging
            input_datasize = int(output_remote_du[0].split()[0])
            logger.info('ltacp %s: input datasize: %d bytes, %s' % (self.logId, input_datasize, humanreadablesize(input_datasize)))
            estimated_tar_size = 512*(input_datasize / 512) + 3*512 #512byte header, 2*512byte ending, 512byte modulo data
            tar_record_size = 10240 # 20 * 512 byte blocks

            # start sending remote data, tee to fifo
            src_path_parent, src_path_child = os.path.split(self.src_path_data)
            cmd_remote_data = self.ssh_cmd + ['cd %s; tar c --checkpoint=100 --checkpoint-action="ttyout=checkpoint %%u\\n" -O %s | tee %s | %s %s %s' % (src_path_parent,
                   src_path_child,
                   self.remote_data_fifo,
                   self.remoteNetCatCmd,
                   self.localIPAddress,
                   port_data)]
            logger.info('ltacp %s: remote starting transfer. executing: %s' % (self.logId, ' '.join(cmd_remote_data)))
            p_remote_data = Popen(cmd_remote_data, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_data] = cmd_remote_data

            # start computation of checksum on remote fifo stream
            cmd_remote_checksum = self.ssh_cmd + ['md5sum %s | %s %s %s' % (self.remote_data_fifo, self.remoteNetCatCmd, self.localIPAddress, port_md5)]
            logger.info('ltacp %s: remote starting computation of md5 checksum. executing: %s' % (self.logId, ' '.join(cmd_remote_checksum)))
            p_remote_checksum = Popen(cmd_remote_checksum, stdout=PIPE, stderr=PIPE)
            self.started_procs[p_remote_checksum] = cmd_remote_checksum

            # timedelta.total_seconds is only available for python >= 2.7
            def timedelta_total_seconds(td):
                return (td.microseconds + (td.seconds + td.days * 24 * 3600) * 10**6) / float(10**6)

            # waiting for output, comparing checksums, etc.
            logger.info('ltacp %s: waiting for remote data transfer to finish...' % self.logId)
            transfer_start_time = datetime.utcnow()
            prev_progress_time = datetime.utcnow()
            prev_bytes_transfered = 0
            while p_remote_data.poll() == None:
                try:
                    nextline = p_remote_data.stdout.readline()
                    if len(nextline) > 0:
                        record_nr = int(nextline.split()[-1].strip())
                        total_bytes_transfered = record_nr * tar_record_size
                        percentage_done = 100.0*total_bytes_transfered/input_datasize
                        current_progress_time = datetime.utcnow()
                        elapsed_secs_since_start = timedelta_total_seconds(current_progress_time - transfer_start_time)
                        elapsed_secs_since_prev = timedelta_total_seconds(current_progress_time - prev_progress_time)
                        if percentage_done > 0 and elapsed_secs_since_start > 0 and elapsed_secs_since_prev > 0:
                            avg_speed = total_bytes_transfered / elapsed_secs_since_start
                            current_bytes_transfered = total_bytes_transfered - prev_bytes_transfered
                            current_speed = current_bytes_transfered / elapsed_secs_since_prev
                            if elapsed_secs_since_prev > 60 or current_bytes_transfered > 0.05*input_datasize:
                                prev_progress_time = current_progress_time
                                prev_bytes_transfered = total_bytes_transfered
                                percentage_to_go = 100.0 - percentage_done
                                time_to_go = elapsed_secs_since_start * percentage_to_go / percentage_done
                                logger.info('ltacp %s: transfered %d bytes, %s, %.1f%% at avgSpeed=%s curSpeed=%s to_go=%s' % (self.logId,
                                                                                                        total_bytes_transfered,
                                                                                                        humanreadablesize(total_bytes_transfered),
                                                                                                        percentage_done,
                                                                                                        humanreadablesize(avg_speed, 'Bps'),
                                                                                                        humanreadablesize(current_speed, 'Bps'),
                                                                                                        timedelta(seconds=int(round(time_to_go)))))
                except KeyboardInterrupt:
                    self.cleanup()

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
            try:
                md5_checksum_remote = output_md5_receive[0].split(' ')[0]
                md5_checksum_local = output_md5_local[0].split(' ')[0]
                if(md5_checksum_remote != md5_checksum_local):
                    raise LtacpException('md5 checksum reported by client (%s) does not match local checksum of incoming data stream (%s)' % (self.logId, md5_checksum_remote, md5_checksum_local))
                logger.info('ltacp %s: remote and local md5 checksums are equal: %s' % (self.logId, md5_checksum_local,))
            except Exception as e:
                logger.error('ltacp %s: error while parsing md5 checksum outputs: local=%s received=%s' % (self.logId, output_md5_local[0], output_md5_receive[0]))
                raise

            logger.debug('ltacp %s: waiting for local byte count on datastream...' % self.logId)
            output_byte_count = p_byte_count.communicate()
            if p_byte_count.returncode != 0:
                raise LtacpException('ltacp %s: Error while receiving remote md5 checksum: %s' % (self.logId, output_byte_count[1]))
            byte_count = int(output_byte_count[0].split()[0].strip())
            logger.info('ltacp %s: byte count of datastream is %d %s' % (self.logId, byte_count, humanreadablesize(byte_count)))

            logger.info('ltacp %s: waiting for transfer via globus-url-copy to LTA to finish...' % self.logId)
            output_data_out = p_data_out.communicate()
            if p_data_out.returncode != 0:
                raise LtacpException('ltacp %s: transfer via globus-url-copy to LTA failed: %s' % (self.logId, output_data_out[1]))
            logger.info('ltacp %s: data transfer via globus-url-copy to LTA complete.' % self.logId)

            logger.debug('ltacp %s: waiting for local adler32 checksum to complete...' % self.logId)
            output_a32_local = p_a32_local.communicate()
            if p_a32_local.returncode != 0:
                raise LtacpException('ltacp %s: local adler32 checksum computation failed: %s' (self.logId, str(output_a32_local)))
            logger.debug('ltacp %s: finished computation of local adler32 checksum' % self.logId)
            a32_checksum_local = output_a32_local[0].split()[1]

            logger.info('ltacp %s: fetching adler32 checksum from LTA...' % self.logId)
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
        return (md5_checksum_local, a32_checksum_local, byte_count)

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


    def cleanup(self):
        logger.debug('ltacp %s: cleaning up' % (self.logId))

        if hasattr(self, 'remote_data_fifo') and self.remote_data_fifo:
            '''remove a file (or fifo) on a remote host. Test if file exists before deleting.'''
            cmd_remote_rm = self.ssh_cmd + ['if [ -e "%s" ] ; then rm %s ; fi ;' % (self.remote_data_fifo, self.remote_data_fifo)]
            logger.info('ltacp %s: removing remote fifo. executing: %s' % (self.logId, ' '.join(cmd_remote_rm)))
            p_remote_rm = Popen(cmd_remote_rm, stdout=PIPE, stderr=PIPE)
            p_remote_rm.communicate()
            if p_remote_rm.returncode != 0:
                logger.error("Could not remove remote fifo %s@%s:%s\n%s" % (self.src_user, self.src_host, self.remote_data_fifo, p_remote_rm.stderr))
            self.remote_data_fifo = None

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

    if len(sys.argv) < 4:
        print 'example: ./ltacp.py 10.196.232.11 /home/users/ingest/1M.txt srm://lofar-srm.fz-juelich.de:8443/pnfs/fz-juelich.de/data/lofar/ops/test/eor/1M.txt'
        sys.exit()

    # transfer test:
    cp = LtaCp(sys.argv[1], sys.argv[2], sys.argv[3], 'ingest')
    cp.transfer()

