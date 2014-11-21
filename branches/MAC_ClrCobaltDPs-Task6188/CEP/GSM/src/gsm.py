#!/usr/bin/env python

# Interpret the arguments and do the selection.
def gsmMain (name, argv):

    import sys
 
    import math

    import monetdb
    import monetdb.sql as db
    import lofar.gsm.gsmutils as gsm 
    #import gsmutils as gsm 

    if len(argv) < 4  or  (argv[0] == '-p'  and  len(argv) < 6):
        print ''
        print 'Insufficient arguments given; run as:'
        print ''
        print '   %s [-p patchname] outfile RA DEC radius [vlssFluxCutoff [assocTheta]]' % name
        print 'to select using a cone'
        print ''
        print '   -p patchname    if given, all sources belong to this single patch'
        print '   outfile         path-name of the output file'
        print '                   It will be overwritten if already existing'
        print '   RA              cone center Right Ascension (J2000, degrees)'
        print '   DEC             cone center Declination     (J2000, degrees)'
        print '   radius          cone radius                 (degrees)'
        print '   vlssFluxCutoff  minimum flux (Jy) of VLSS sources to use'
        print '                   default = 4'
        print '   assocTheta      uncertainty in matching     (degrees)'
        print '                   default = 0.00278  (10 arcsec)'
        print ''
        return False

    # Get the arguments.
    patch   = ''
    st = 0
    if argv[0] == '-p':
        patch = argv[1]
        st = 2
    outfile = argv[st]
    try:
      ra      = float(argv[st+1])
      dec     = float(argv[st+2])
    except ValueError:
      # Try to parse ra-dec as in the output of msoverview, e.g. 08:13:36.0000 +48.13.03.0000
      ralst = argv[st+1].split(':')
      ra = math.copysign(abs(float(ralst[0]))+float(ralst[1])/60.+float(ralst[2])/3600.,float(ralst[0]))
      declst = argv[st+2].split('.')
      dec = math.copysign(abs(float(declst[0]))+float(declst[1])/60.+float('.'.join(declst[2:]))/3600.,float(declst[0]))
    radius  = float(argv[st+3])
    cutoff  = 4.
    theta   = 0.00278
    if len(argv) > st+4:
        cutoff = float(argv[st+4])
    if len(argv) > st+5:
        theta = float(argv[st+5])

    db_host = "ldb002.offline.lofar"
    #db_host = "napels"
    db_dbase = "gsm"
    db_user = "gsm"
    db_passwd = "msss"
    #db_passwd = "gsm"
    db_port = 51000
    db_autocommit = True

    try:
        conn = db.connect(hostname=db_host, database=db_dbase, username=db_user,
                          password=db_passwd, port=db_port, autocommit=db_autocommit)
        gsm.expected_fluxes_in_fov (conn, ra, dec, radius, theta, outfile,
                                    patchname=patch,
                                    storespectraplots=False,
                                    deruiter_radius=3.717,
                                    vlss_flux_cutoff=cutoff)
    except db.Error, e:
        raise


# This is the main entry.
if __name__ == "__main__":
    import sys
    try:
        gsmMain (sys.argv[0], sys.argv[1:])
    except Exception as e:
        print "Failed for reason: %s" % (e,)
    #    raise
    #sys.exit(1)
