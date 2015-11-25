#!/usr/bin/env python

# LTACP Python module for transferring data from a remote node to a remote SRM via localhost
#
# Remote data can be individual files or directories. Directories will be tar-ed.
#
# Between the remote and local host, md5 checksums are used to ensure integrity of the file,
# adler32  is used between localhost and the SRM.


from subprocess import Popen, PIPE
from socket import getfqdn
import os, sys, getpass

class LtacpException(Exception):
     def __init__(self, value):
         self.value = value
     def __str__(self):
         return repr(self.value)

# converts given srm url of an LTA site into a transport url as needed by gridftp. (Sring replacement based on arcane knowledge.)
def convert_surl_to_turl(surl):
    turl = surl.replace("srm://srm.grid.sara.nl:8443","gsiftp://gridftp.grid.sara.nl",1)
    turl = turl.replace("srm://srm.grid.sara.nl","gsiftp://gridftp.grid.sara.nl",1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de:8443","gsiftp://lofar-gridftp.fz-juelich.de",1)
    turl = turl.replace("srm://lofar-srm.fz-juelich.de","gsiftp://lofar-gridftp.fz-juelich.de",1)
    turl = turl.replace("srm://srm.target.rug.nl:8444","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    turl = turl.replace("srm://srm.target.rug.nl","gsiftp://gridftp02.target.rug.nl/target/gpfs2/lofar/home/srm",1)
    return turl


# transfer file/directory from given src to SRM location with given turl
def transfer(src_host,
             src_path_data,
             dst_turl,
             src_user=getpass.getuser(),
             port_data='40000',
             port_md5='50000',
             path_fifo='auto',
             src_path_fifo='auto'
            ):

    # default return code
    code = 0

    # for cleanup
    started_procs = []

    # default fifo paths
    if path_fifo == 'auto':
        path_fifo = '/tmp/ltacp_datapipe_'+src_host+'_'+port_data
    if src_path_fifo == 'auto':
        src_path_fifo = '/tmp/ltacp_md5pipe_'+port_md5

    #---
    # Server part
    #---

    # create local fifo to stream data to globus-url-copy
    print 'creating fifo:', path_fifo
    os.mkfifo(path_fifo)

    # start listen for checksums
    cmd_md5_in = ['nc','-l', '-q','0', port_md5]
    print 'executing:', cmd_md5_in
    p_md5 = Popen(cmd_md5_in, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_md5)

    # start listen for data stream
    cmd_data_in = ['nc', '-l', '-q','0', port_data]
    print 'executing:', cmd_data_in
    p_data_in = Popen(cmd_data_in, stdout=PIPE)
    started_procs.append(p_data_in)

    # start tee incoming data stream to fifo (pipe stream further for checksum)
    cmd_tee = ['tee', path_fifo]
    print 'executing:', cmd_tee
    p_tee = Popen(cmd_tee, stdin=p_data_in.stdout, stdout=PIPE)
    started_procs.append(p_tee)

    # start computating checksum of incoming data stream
    cmd_md5_local = ['md5sum']
    print 'executing:', cmd_md5_local
    p_md5_local = Popen(cmd_md5_local, stdin=p_tee.stdout, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_md5_local)

    # start copy fifo stream to SRM
    cmd_data_out = ['globus-url-copy', path_fifo, dst_turl]
    print 'executing:', cmd_data_out
    p_data_out = Popen(cmd_data_out, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_data_out)


    #---
    # Client part
    #---

    # start remote copy on src host:
    # 1) create fifo
    # 2) send tar stream of data/dir + tee to fifo for 3)
    # 3) simultaneously to 2), calculate checksum of fifo stream
    # 4) break fifo
    cmd_remote_mkfifo = ['ssh '+src_user+'@'+src_host+
                         ' \'mkfifo '+src_path_fifo+
                         '\'']
    print 'executing:', cmd_remote_mkfifo
    p_remote_mkfifo = Popen(cmd_remote_mkfifo, shell=True, stdout=PIPE, stderr=PIPE)
    started_procs.append(p_remote_mkfifo)


    try:
        # block until fifo is created
        output_remote_mkfifo = p_remote_mkfifo.communicate()
        if p_remote_mkfifo.returncode == 0:
            print 'Remote fifo created!'
        else:
            raise LtacpException('Remote fifo creation failed: '+output_remote_mkfifo[1])

        # start sending remote data, tee to fifo
        src_path_parent, src_path_child = os.path.split(src_path_data)
        cmd_remote_data = ['ssh '+src_user+'@'+src_host+
                           ' \'cd '+src_path_parent+
                           ' ; tar c -O '+src_path_child+' | tee '+src_path_fifo+' | nc -q 0 '+getfqdn()+' '+port_data+
                           '\'']
        print 'executing:', cmd_remote_data
        p_remote_data = Popen(cmd_remote_data, shell=True, stdout=PIPE, stderr=PIPE)
        started_procs.append(p_remote_data)

        # start computation of checksum on remote fifo stream
        cmd_remote_checksum = ['ssh '+src_user+'@'+src_host+
                               ' \'cat '+src_path_fifo+' | md5sum | nc -q 0 '+getfqdn()+' '+port_md5+
                               '\'']
        print 'executing:', cmd_remote_checksum
        p_remote_checksum = Popen(cmd_remote_checksum, shell=True, stdout=PIPE, stderr=PIPE)
        started_procs.append(p_remote_checksum)


        # waiting for output, comparing checksums, etc.
        print "Waiting for SSH to finish (data and checksums are send by src host)..."
        output_remote_data = p_remote_data.communicate()
        output_remote_checksum = p_remote_checksum.communicate()
        if p_remote_data.returncode == 0 and p_remote_checksum.returncode == 0:
            print "Remote transfer successful. Checking md5 hash from src host..."
            output_md5 = p_md5.communicate()
            output_md5_local = p_md5_local.communicate()
            if p_md5.returncode == 0 and p_md5_local.returncode == 0:
                md5_hash = output_md5[0].split()[0]
                md5_hash_local = output_md5_local[0].split()[0]
                if(md5_hash != md5_hash_local):
                    raise LtacpException('md5 hash reported by client ('+md5_hash+') does not match hash of incoming data stream ('+md5_hash_local+')')
                print "Waiting for transfer to SRM to finish..."
                output_data_out = p_data_out.communicate()
                if p_data_out.returncode == 0:
                    print "Transfer to SRM successful!"
                    # todo: srmls + adler compare
                else:
                    raise LtacpException('Transfer to SRM failed: '+output_data_out[1])
            else:
                raise LtacpException('MD5 hash comparison failed, remote error: '+output_md5[1]+' --- '+output_md5_local[1])
        else:
            raise LtacpException('SSH (for sending data and checksum) failed: '+output_remote_data[1]+' --- '+output_remote_checksum[1])

    except LtacpException as e:
        # Something went wrong
        print "! Fatal Error: "+e.value+' '+e.message
        code = 1

    # ---
    # wrapping up
    # ---

    # remove remote fifo
    #todo: can this be done safely? What if remote fifo existed?
    print 'breaking remote fifo'
    cmd_remote_rmfifo = ['ssh '+src_user+'@'+src_host+
                         ' \'rm '+src_path_fifo+
                         '\'']
    print 'executing:', cmd_remote_rmfifo
    p_remote_rmfifo = Popen(cmd_remote_rmfifo, shell=True, stdout=PIPE, stderr=PIPE)
    p_remote_rmfifo.communicate()
    if p_remote_rmfifo.returncode != 0:
        print "Could not remove remote fifo!"
    started_procs.append(p_remote_rmfifo)

    # remove local data fifo
    print 'breaking fifo: ', path_fifo
    os.unlink(path_fifo)

    # cancel any started process
    for p in started_procs:
            if p.poll() == None:
                p.terminate()
                print 'terminated', p.pid

    print 'we\'re done!'
    return code


# execute command and optionally return exit code or output streams
def execute(cmd, return_output=False):
    print 'executing:', cmd
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
def srmls_l(surl):
    return execute(['srmls', '-l', surl], return_output=True)

# get checksum from srm via srmls
def get_srm_checksum(surl):
    output = srmls_l(surl)[0]
    if 'Checksum value:' in output:
        return output.split('Checksum value:')[1].lstrip().split()[0]
    else:
        return False

#recursively checks for presence of parent directory and created the missing part of a tree
def create_missing_directories(surl):

    parent, child = os.path.split(surl)
    missing = []

    # determine missing dirs
    while parent:
        code = execute(['srmls', parent])
        if code == 0:
            print "srmls returned successfully, so this path apparently exists:", parent
            break;
        else:
            parent, child = os.path.split(parent)
            missing.append(child)

    # recreate missing dirs
    while len(missing) > 0:
        parent = parent + '/' + missing.pop()
        code = execute(['srmmkdir',"-retry_num=0",parent])
        if code != 0:
            print "failed to create missing directory:",parent
            return code

    print "Successfully created parent directory:", parent
    return 0


# limited standalone mode for testing:
# usage: ltacp.py <remote-host> <remote-path> <surl>
if __name__ == '__main__':
# transfer test:
    turl = convert_surl_to_turl(sys.argv[3])
    print 'transferring ', sys.argv[1]+':'+sys.argv[2] +' to '+turl
    transfer(sys.argv[1], sys.argv[2], turl)

# srmls/srmrm test:
    print get_srm_checksum(sys.argv[3])
#    print srmrm(sys.argv[3])
#    print get_srm_checksum(sys.argv[3])

# srmmkdir test:
#   print create_missing_directories(sys.argv[3])