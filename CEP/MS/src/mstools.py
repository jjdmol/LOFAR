import os
import re

""" Find files on nodes in a cluster matching the given pattern """
def findFiles (msPattern, lsOption=''):
    hostline    = re.compile ('^-+ +[^ ]+ +-+$')
    hostline1   = re.compile ('^-+ +')
    hostline2   = re.compile (' +-+$')
    nomatch     = ['ls: No match.',
                   'ls: cannot access .*: No such file or directory',
                   'ssh: connect to host .*: No route to host',
                   'Warning: No xauth data; .*',
                   '/usr/bin/xauth:  error in locking authority file.*']
    nomatchline = re.compile ('^(%s)$' % '|'.join(nomatch))
    pipe = os.popen ('cexec "ls ' + lsOption + ' ' + msPattern + '"')
    files = []
    hosts = []
    host = ''
    for line in pipe:
        line = line.strip()
        if len(line) > 0:
            if hostline.match(line):
                # get the host from a line like --- lce003 ---
                host = hostline2.sub ('', hostline1.sub('', line))
            elif len(host) > 0  and  not nomatchline.match(line):
                # get the file name from lines after a host
                files.append (line)
                hosts.append (host)
    pipe.close()
    return (hosts,files)


""" Find directories on nodes in a cluster matching the given pattern """
def findDirs (pattern):
    return findFiles (pattern, '-d')


""" Check if all files have the same SAP """
def checkSAP_SB (fileNames, bandsPerBeam):
    sapUsed = -1
    sapre = re.compile ('_SAP')
    for name in fileNames:
        parts = sapre.split (name)
        if len(parts) != 2:
            print "File name %s does not contain a single string _SAP" % name
            return False
        assert (len(parts) == 2)
        sap = int(parts[1][0:3])
        sb  = int(parts[1][6:9])
        if sapUsed < 0:
            sapUsed = sap
            sbmin   = sb
            sbmax   = sb
        else:
            if sap != sapUsed:
                print "Error: multiple SAP numbers found in file names"
                return -1
            if sb < sbmin:
                sbmin = sb
            if sb > sbmax:
                sbmax = sb
    if sbmax - sbmin + 1 > bandsPerBeam:
        print 'Error: SB number range in file names exceeds bands per beam', bandsPerBeam
        return -1
    if sbmax - sbmin + 1 < bandsPerBeam:
        print 'Warning: SB number range in file names < bands per beam', bandsPerBeam
    return sapUsed


""" Copy files in a cluster to achieve that similar files are on the same node
"""
def movemss (srcPattern, dstPattern, userName, bandsPerBeam=80, dryrun=False):
    # First find where all parts are
    (srcHosts, srcFiles) = findDirs(srcPattern)
    (dstHosts, dstFiles) = findDirs(dstPattern)
    if len(dstFiles) == 0:
        print 'Error: no files found matching', dstPattern
        return False
    if len(srcFiles) < len(dstFiles):
        print 'Error: fewer SRC files', srcPattern, 'found than DST files', dstPattern
        return False
    srcSAP = checkSAP_SB(srcFiles, bandsPerBeam)
    dstSAP = checkSAP_SB(dstFiles, bandsPerBeam)
    if dstSAP < 0  or  srcSAP < 0:
        return False
    # Determine if first a directory might need to be created when moving files.
    createDir = (os.path.dirname(srcFiles[0]) != os.path.dirname(dstFiles[0]))
    # An MS name looks like Lnnnnn_SAPnnn_SBnnn*
    # SAP gives the beam number (sub array pointing).
    # The SB numbers always increase, thus beam 0000 has, say, SB 000-079,
    # beam 001 has SB 080-159 and beam 002 has SB 160-239.
    sapre = re.compile ('_SAP')
    sbre  = re.compile ('_SB')
    # Create the template name of the src file to find.
    parts = sbre.split(srcFiles[0])
    assert (len(parts) == 2)
    srcTemplate = parts[0] + '_SB%03d' + parts[1][3:]
    # Now move the SRC files to the DST nodes if that is necessary.
    # Note that a SRC can have been moved before, thus can appear in
    # multiple places.
    # First turn the SRC list into dicts (for faster lookup).
    # One dict per name-node, one dict only per name.
    srcNodeMap = {}
    srcMap = {}
    nInPlace = 0;
    for i in range(len(srcFiles)):
        srcNodeMap[srcHosts[i] + '-' + srcFiles[i]] = i
        srcMap[srcFiles[i]] = i
    for i in range(len(dstFiles)):
        # Generate the name of the SRC to use from the DST name.
        name = os.path.basename(dstFiles[i])
        parts = sapre.split(name)
        dstSB = int(parts[1][6:9])
        srcSB = dstSB - (dstSAP-srcSAP)*bandsPerBeam 
        # See if the SRC is already on the right node.
        srcName = srcTemplate % srcSB
        if srcNodeMap.has_key(dstHosts[i] + '-' + srcName):
            nInPlace += 1
        else:
            # Has DST to be moved from another node?
            if not srcMap.has_key(srcName):
                print 'Src', srcName, 'not found for DST', dstFiles[i]
            else:
                inx = srcMap[srcName]
                print 'Move', srcName, 'from', srcHosts[inx], 'to', dstHosts[i]
                srcDir = os.path.dirname(srcName)
                cmd = ''
                if createDir:
                    cmd = 'ssh -x' + userName + '@' + dstHosts[i] + \
                        ' "mkdir -p ' + srcDir  + '" && '
                cmd += 'ssh -x' + userName + '@' + srcHosts[inx] + \
                    ' "scp -r ' + srcName + ' ' + \
                    userName + '@' + dstHosts[i] + ':' + srcDir + \
                    ' && rm -rf ' + srcName + '"'
#                      '" &'
                print cmd
                if not dryrun:
                    os.system (cmd)
    print nInPlace, "source files are already on the correct destination mode"
