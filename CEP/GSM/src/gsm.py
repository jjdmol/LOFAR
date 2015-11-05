#!/usr/bin/env python

# Interpret the arguments and do the selection.
def gsmMain (name, argv):

    import sys

    import monetdb
    import monetdb.sql as db
    import lofar.gsm.gsmutils as gsm 
    #import gsmutils as gsm 

    if len(argv) < 4:
        print ''
        print 'Insufficient arguments given; run as:'
        print ''
        print '   %s outfile RA DEC radius [vlssFluxCutoff [assocTheta]]' % name
        print 'to select using a cone'
        print ''
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
    outfile = argv[0]
    ra      = float(argv[1])
    dec     = float(argv[2])
    radius  = float(argv[3])
    cutoff  = 4.
    theta   = 0.00278
    if len(argv) > 4:
        cutoff = float(argv[4])
    if len(argv) > 5:
        theta = float(argv[5])

    db_host = "ldb002"
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
