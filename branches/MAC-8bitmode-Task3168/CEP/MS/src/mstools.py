import os
import os.path
import re
import lofar.parameterset

""" Find files on nodes in a cluster matching the given pattern """
def findFiles (msPattern, lsOption='', cluster=''):
    hostline    = re.compile ('^-+ +[^ ]+ +-+$')
    hostline1   = re.compile ('^-+ +')
    hostline2   = re.compile (' +-+$')
    nomatch     = ['ls: No match.',
                   'ls: cannot access .*: No such file or directory',
                   'ssh: connect to host .*: ' +
                       '(No route to host|Connection refused)',
                   'Permission denied \(publickey,keyboard-interactive\).',
                   'Warning: No xauth data; .*',
                   '/usr/bin/xauth:  error in locking authority file.*'] 
    nomatchline = re.compile ('^(%s)$' % '|'.join(nomatch))
    pipe = os.popen ('cexec ' + cluster + ' "ls ' + lsOption + ' ' + msPattern + '"')
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
def findDirs (pattern, cluster=''):
    return findFiles (pattern, '-d', cluster)


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
    nInPlace = 0
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
                    cmd = 'ssh -x ' + userName + '@' + dstHosts[i] + \
                        ' "mkdir -p ' + srcDir  + '" && '
                cmd += 'ssh -x ' + userName + '@' + srcHosts[inx] + \
                    ' "scp -r ' + srcName + ' ' + \
                    userName + '@' + dstHosts[i] + ':' + srcDir + \
                    ' && rm -rf ' + srcName + '"'
#                      '" &'
                print cmd
                if not dryrun:
                    os.system (cmd)
    print nInPlace, "source files are already on the correct destination mode"

def addfileglob (filename, pattern):
    """ If needed, add the glob pattern to the filename

    If the basename of the filename does not contain glob characters
    (*, ?, [], or {}), the glob pattern is added.

    """
    hasglob = False
    if filename[-1] == '/':
        filename = filename[:-1]
    else:
        import os
        bname = os.path.basename(filename)
        hasglob = False
        for c in '*?[{':
            if c in bname:
                hasglob = True
                break
    if hasglob:
        return filename
    return filename + '/' + pattern

def expandps (parsetin, parsetout, keymap, nsubbands=0, nodeindex=0):
    """ Expand dataset names in a parset file

    The names of the input and possible output datasets are expanded to create
    a parset expected by the pipeline scripts (as created by the scheduler).
    The input name expansion is done by looking up all dataset names matching
    the name pattern in the parset.
    The output names are expanded by substituting the names found for the first
    input in the output name pattern that can contain cexecms-like placeholders
    like <BN>.

     parsetin
      The name of the input parset file
     parsetout
      The name of the output parset file
     keymap
      A dict mapping the parameter names in the input parset to the output
      parset. The dict can contain the following entries:
      | 'in' maps to a list of pairs. Each pair defines an input dataset type.
        The first value of a pair defines the parameter name in the input
        parset; the second value defines the name in the output parset.
        The parameter in the input parset defines one or more filename glob
        patterns. Usually one pattern will be used, but multiple are needed
        for e.g. the imaging pipeline (a pattern per observation slice).
        Instead of a parameter name, it is also possible to directly give a
        list of glob patterns directly. Thus passing a string means a parameter
        name, while a list means glob patterns.
      | 'out' maps to a list of pairs. Each pair defines the names of the
        parameter in the input and output parset. The parameter value in the
        input parset must define the location of the output. It can contain
        the following cexecms-like placeholders:
        | <DN>  is the directory name of the input dataset
        | <BN>  is the basename of the input dataset
        | <BN.> is the basename till the first dot (thus without the extension)
        | <.BN> is the basename after the first dot (thus the extension)
        | <SEQ> is a 3 digit sequence number (000, 001, ...) useful for the
                imaging pipeline.
        | <OBSID> is the obsid part of <BN.> (till first _)
        | <SAP> is the SAP part of <BN.> (till next _)
        | <SB> is the obsid part of <BN.> (till next _)
        | <TYPE> is the obsid part of <BN.> (after last _)
        Instead of an input parameter name, it is possible to directly give
        the output location. by passing it as a list containing one element.
     nsubbands
      If > 0, the number of subbands in a subband group
      (i.e., the number of subbands to combine in an image).
     nodeindex
      The index of the subband in a subband group whose node will be used
      to do the imaging on (default is 0).


    For example, for a calibration pipeline one can use::

      keymap = {'in':  [('msin',  'Input_Correlated')],
                'out': [('pbout', 'Output_InstrumentModel')]}
      msin = /data/L32517/*.MS
      pbout = /data/scratch/pipeline/L32517/<BN.>.instrument

    where 'keymap' is the function argument and 'msin' and 'pbout' are
    parameters in the input parset.
    Suppose that the 'msin' pattern matches two datasets. Then the
    output parset will contain something like::

      ObsSW.Observation.DataProducts.Input_Correlated.locations =
          ['locus001:/data/L32517', 'locus002:/data/L32517']
      ObsSW.Observation.DataProducts.Input_Correlated.filenames =
          ['L32517_SAP000_SB000_uv.MS', 'L32517_SAP000_SB001_uv.MS']
      ObsSW.Observation.DataProducts.Output_InstrumentModel.locations =
          ['locus001:/data/scratch/pipeline/L32517',
           'locus002:/data/scratch/pipeline/L32517']
      ObsSW.Observation.DataProducts.Output_InstrumentModel.filenames =
          ['L32517_SAP000_SB000_uv.instrument',
           'L32517_SAP000_SB))1_uv.instrument']

    The target pipeline has two input datasets, so it should use two pairs
    in the 'in' keyword.
    For example::
      
      keymap = {'in':  (('msin',  'Input_Correlated'),
                        ('pbin',  'Input_InstrumentModel')),
                'out': (('msout', 'Output_Correlated'))}
      msin = /data/scratch/pipeline/L32517/*_dppp.MS
      pbin = /data/scratch/pipeline/L32517/*.instrument
      msout = /data/scratch/pipeline/L32517/<BN.>_cal.MS

    The imaging pipeline combines multiple slices, so the 'msin' contains
    multiple patterns. A single image contains 10 subbands, so the output
    name contains <SEQ> to name them uniquely.
    Furthermore, it has to be told how many subbands to combine in an image.
    The imaging can be done on the nodes of any of the subbands to combine,
    so it has to be told which one to use (default is the first one).
    and which node should be used to do the imaging on/
    For example::
      
      keymap = {'in':  (('msin',  'Input_Correlated'))}
                'out': (('imgout', 'Output_Image'))}
      nsubbands = 10
      nodeindex = 0
      msin = [/data/scratch/pipeline/L32517/*_dppp_cal.MS,
              /data/scratch/pipeline/L32519/*_dppp_cal.MS]
      imgout = /data/scratch/pipeline/L32520/image_<SEQ>.img

"""
    # Open parset and get all keywords.
    ps = lofar.parameterset.parameterset (parsetin)
    pskeys = ps.keys()
    # See if nsubbands parameter is given; otherwise set to 1.
    havesubbands = True
    if nsubbands <= 0:
        havesubbands = False
        nsubbands = 1
        # Check and initialize.
    if nodeindex < 0  or  nodeindex >= nsubbands:
        raise ValueError, "Argument nsubbands or nodeindex has an invalid value"
    nfiles = -1
    nslice = -1
    locations=[]
    filenames=[]

    # Process input keywords. They must be present.
    inkeys = keymap["in"]
    nrproc = 1
    for (keyin,keyout) in inkeys:
        # If a string, find keyin in the parset.
        # Otherwise it defines the glob patterns.
        if isinstance(keyin, str):
            if keyin not in pskeys:
                raise KeyError, "keyword " + keyin + " not found in parset " + parsetin
            # Get the file name pattern
            patterns = ps.getStringVector(keyin)
            ps.remove (keyin)
        else:
            patterns = keyin
        locs  = []
        names = []
        for patt in patterns:
            # Get all nodes and file names
            (nodes,files) = findDirs(patt)
            ##(nodes,files) = (['locus1','locus2'], ['/data/L1/L1a.MS','/data/L1/L1b.MS'])
            # Turn into a list of pairs (instead of pair of lists) and sort
            filesnodes = [(os.path.basename(files[i]),
                           os.path.dirname(files[i]),
                           nodes[i]) for i in range(len(files))]
            filesnodes.sort()
            # Split into location (node:dir/) and basename.
            for (file,dir,node) in filesnodes:
                locs.append  (node + ':' + dir + '/')
                names.append (file)
        nf = len(names)
        if nfiles < 0:
            # First input keyword
            nfiles = nf
            nslice = len(patterns)
            locations = locs
            filenames = names
        elif nf != nfiles:
            raise ValueError, "Number of files found for " + key + " differs from previous key with input names"
        # Add prefix to output parameter name
        newkey = 'ObsSW.Observation.DataProducts.' + keyout
        ps.add (newkey + '.locations', str(locs))
        ps.add (newkey + '.filenames', str(names))

    # Write nsubbands if needed.
    if havesubbands:
        ps.add ('subbands_per_image', str(nsubbands))
        ps.add ('slices_per_image', str(nslice))

    # Process output keywords if they are present.
    if 'out' in keymap:
        if len(filenames) == 0:
            raise ValueError, "No input datasets have been defined"
        inkeys = keymap["out"]
        nrproc += 1
        for (keyin,keyout) in inkeys:
            if isinstance(keyin, str):
                if keyin not in pskeys:
                    raise KeyError, "keyword " + keyin + " not found in parset " + parsetin
                name = ps.getString(keyin)
                ps.remove (keyin)
            else:
                if len(keyin) != 1:
                    raise KeyError, "Output key " + keyin + " is not a string, thus should be a sequence of length 1"
                name = keyin[0]
            locs  = []
            names = []
            # Create output for all input names replacing tags like <BN>.
            re0 = re.compile ('<DN>')
            re1 = re.compile ('<BN>')
            re2 = re.compile ('<BN\.>')
            re3 = re.compile ('<\.BN>')
            re4 = re.compile ('<SEQ>')
            re5 = re.compile ('<OBSID>')
            re6 = re.compile ('<SAP>')
            re7 = re.compile ('<SB>')
            re8 = re.compile ('<TYPE>')
            for i in range(len(filenames) / (nslice*nsubbands)):
                inx = i*nslice*nsubbands + nodeindex
                locparts = locations[inx].split(':', 1)
                filparts = filenames[inx].split('.', 1)
                if len(filparts) == 1:
                    filparts.append('')
                # Find OBSID, SAP, SB, and TYPE (as in L12345_SAP000_SB000_uv)
                bnparts = filparts[0].split('_', 3)
                if len(bnparts) == 1:
                    bnparts.append('')
                nm = re0.sub(locparts[1], name) # <DN>  = directory name
                nm = re1.sub(filenames[i], nm)  # <BN>  = basename
                nm = re2.sub(filparts[0], nm)   # <BN.> = basename till first .
                nm = re3.sub(filparts[1], nm)   # <.BN> = basename after first .
                nm = re4.sub('%03i'%i, nm)      # <SEQ> = seqnr
                nm = re5.sub(bnparts[0], nm)    # <OBSID> = basename till _
                if len(bnparts) > 1:
                    nm = re6.sub(bnparts[1], nm) # <SAP> = basename till next _
                if len(bnparts) > 2:
                    nm = re7.sub(bnparts[2], nm) # <SB>  = basename till next _
                if len(bnparts) > 3:
                    nm = re8.sub(bnparts[3], nm) # <TYPE> = rest of basename
                names.append (os.path.basename(nm))
                locs.append (locparts[0] + ':' + os.path.dirname(nm) + '/')
            newkey = 'ObsSW.Observation.DataProducts.' + keyout
            ps.add (newkey + '.locations', str(locs))
            ps.add (newkey + '.filenames', str(names))

    # Check if all keymap keywords have been processed.
    if nrproc != len(keymap):
        raise ValueError, "Only keys 'in' and 'out' are possible in the keymap argument"
    # Write the resulting parset.
    ps.writeFile (parsetout)
    print "Created output parset " + parsetout

