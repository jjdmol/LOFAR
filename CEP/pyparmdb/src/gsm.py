#!/usr/bin/env python

# Select a subset from the GSM and write into a makesourcdb input file.
# ra and dec both in degrees
# integrated flux [in Jy] above which the sources will be selected
# You should know that:
#  cat_id = 3 => NVSS
#  cat_id = 4 => VLSS
#  cat_id = 5 => WENSS
def doSelect (outfile, dirSelect, fluxi_mins, cat_ids):

    import os
    import monetdb.sql as db
    from monetdb.sql import Error as Error

    db_type = "MonetDB"
    db_host = "ldb001"
    db_port = 50000
    db_dbase = "gsm"
    db_user = "gsm"
    db_passwd = "msss"
    #db_lang = "sql"

    # If a single catalog, use =
    if len(cat_ids) == 1:
        catSelect = 'c1.cat_id = %d AND c1.i_int_avg >= %f' % (cat_ids[0], fluxi_mins[0])
    else:
        # Multiple catalogs.
        # If a single minimum flux, use it for all catalogs.
        # Note that the IN clause needs (), not [].
        if len(fluxi_mins) == 1:
            catSelect = 'c1.cat_id IN (%s) AND c1.i_int_avg >= %f' % (str(cat_ids)[1:-1], fluxi_mins[0])
        else:
            # There is a minimum flux per catalog.
            catSelect = ''
            for i in range(len(cat_ids)):
                if i > 0:
                    catSelect += ' OR ' 
                catSelect += '(c1.cat_id = %d AND c1.i_int_avg >= %f)' % (cat_ids[i], fluxi_mins[i])
    # Form the entire where clause.
    where = 'WHERE (%s) AND (%s)' % (catSelect, dirSelect)

    conn = db.connect(hostname=db_host \
                          ,port=db_port \
                          ,database=db_dbase \
                          ,username=db_user \
                          ,password=db_passwd \
                          )

    # Remove output file if already existing.
    if os.path.isfile(outfile):
        os.remove(outfile)

    try:
        cursor = conn.cursor()
        # This query concatenates the requested columns per row in a single string in the correct BBS format.
        cursor.execute(#"COPY " + \
                  "SELECT t.line " + \
                  "  FROM (SELECT CAST(CONCAT(t0.catsrcname, CONCAT(',',  " + \
                  "                    CONCAT(t0.src_type, CONCAT(',',  " + \
                  "                    CONCAT(ra2bbshms(t0.ra), CONCAT(',',  " + \
                  "                    CONCAT(decl2bbsdms(t0.decl), CONCAT(',',  " + \
                  "                    CONCAT(t0.i, CONCAT(',',  " + \
                  "                    CONCAT(t0.q, CONCAT(',',  " + \
                  "                    CONCAT(t0.u, CONCAT(',',  " + \
                  "                    CONCAT(t0.v, CONCAT(',',  " + \
                  "                    CONCAT(t0.MajorAxis, CONCAT(',',  " + \
                  "                    CONCAT(t0.MinorAxis, CONCAT(',',  " + \
                  "                    CONCAT(t0.Orientation, CONCAT(',',  " + \
                  "                    CONCAT(t0.ReferenceFrequency, CONCAT(',',  " + \
                  "                           CASE WHEN t0.SpectralIndexDegree = 0" + \
                  "                                THEN '[]'" + \
                  "                                ELSE CASE WHEN t0.SpectralIndexDegree = 1" + \
                  "                                          THEN CONCAT('[', CONCAT(t0.SpectralIndex_0, ']'))" + \
                  "                                          ELSE CONCAT('[', CONCAT(t0.SpectralIndex_0, CONCAT(',', CONCAT(t0.SPECTRALINDEX_1, ']'))))" + \
                  "                                     END" + \
                  "                           END" + \
                  ")))))))))))))))))))))))" + \
                  "                          ) AS VARCHAR(300)) AS line " + \
                  "          FROM (SELECT CAST(TRIM(c1.catsrcname) AS VARCHAR(23)) AS catsrcname " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('POINT' AS VARCHAR(23)) " + \
                  "                            ELSE CAST('GAUSSIAN' AS VARCHAR(23)) " + \
                  "                       END AS src_type " + \
                  "                      ,CAST(c1.ra AS VARCHAR(23))  AS ra " + \
                  "                      ,CAST(c1.decl AS VARCHAR(23)) AS decl " + \
                  "                      ,CAST(c1.i_int_avg AS VARCHAR(23)) AS i " + \
                  "                      ,CAST(0 AS VARCHAR(23)) AS q " + \
                  "                      ,CAST(0 AS VARCHAR(23)) AS u " + \
                  "                      ,CAST(0 AS VARCHAR(23)) AS v " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('' AS VARCHAR(23)) " + \
                  "                            ELSE CASE WHEN c1.major IS NULL " + \
                  "                                      THEN CAST('' AS VARCHAR(23)) " + \
                  "                                      ELSE CAST(c1.major AS varchar(23)) " + \
                  "                                 END " + \
                  "                       END AS MajorAxis " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('' AS VARCHAR(23)) " + \
                  "                            ELSE CASE WHEN c1.minor IS NULL " + \
                  "                                      THEN CAST('' AS VARCHAR(23)) " + \
                  "                                      ELSE CAST(c1.minor AS varchar(23)) " + \
                  "                                 END " + \
                  "                       END AS MinorAxis " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('' AS VARCHAR(23)) " + \
                  "                            ELSE CAST(c1.pa AS varchar(23)) " + \
                  "                       END AS Orientation " + \
                  "                      ,CAST(c1.freq_eff AS VARCHAR(23)) AS ReferenceFrequency " + \
                  "                      ,CASE WHEN si.spindx_degree IS NULL " + \
                  "                            THEN 0 " + \
                  "                            ELSE si.spindx_degree " + \
                  "                       END AS SpectralIndexDegree " + \
                  "                      ,CASE WHEN si.c0 IS NULL " + \
                  "                            THEN 0 " + \
                  "                            ELSE si.c0 " + \
                  "                       END AS SpectralIndex_0 " + \
                  "                      ,CASE WHEN si.c1 IS NULL  " + \
                  "                            THEN 0 " + \
                  "                            ELSE si.c1 " + \
                  "                       END AS SpectralIndex_1 " + \
                  "                  FROM catalogedsources c1 " + \
                  "                       LEFT OUTER JOIN spectralindices si ON c1.catsrcid = si.catsrc_id " + \
                      where + 
                  "               ) t0 " + \
                  "       ) t " , ())
        y = cursor.fetchall()
        cursor.close()
        #print "y:", y

    except db.Error, e:
        import logging
        logging.warn("gsmSelect to file %s failed " % (outfile))
        logging.warn("Failed on query for reason: %s " % (e))
        logging.debug("Failed select query: %s" % (e))
        return -1

    file = open(outfile,'w')
    file.write ("format = Name, Type, Ra, Dec, I, Q, U, V, MajorAxis, MinorAxis, Orientation, ReferenceFrequency='60e6', SpectralIndex='[]'\n")
    for i in range(len(y)):
        file.write(y[i][0] + '\n')
    file.close()
    return len(y)


def gsmSelectCone (outfile, ra, dec, radius, fluxi_mins, cat_ids):
    if ra < 0  or  ra > 360:
        print 'RA', ra, 'is invalid: <0 or >360 degrees'
        return -1
    if dec < -90  or  dec > 90:
        print 'DEC', dec, 'is invalid: <-90 or >90 degrees'
        return -1
    if radius < 0  or  radius > 90:
        print 'radius', radius, 'is invalid: <0 or >90 degrees'
        return -1
    # Determine DEC boundaries (for hopefully faster query).
    # Convert degrees to radians.
    import math
    mindec = dec - radius
    maxdec = dec + radius
    d2r = math.pi / 180.
    ra  *= d2r
    dec *= d2r
    radius *= d2r
    # Source (srcRA,srcDEC) is inside the cone if:
    #  sin(dec)*sin(srcDEC) + cos(dec)*cos(srcDEC)*cos(ra-srcRA) >= cos(radius)
    select = '(c1.decl BETWEEN %17.12f AND %17.12f) AND (%17.14f*sin(c1.decl*%17.14f) + %17.14f*cos(c1.decl*%17.14f)*cos(%17.14f-c1.ra*%17.14f) > %17.14f)' % (mindec, maxdec, math.sin(dec), d2r, math.cos(dec), d2r, ra, d2r, math.cos(radius))
    return doSelect (outfile, select, fluxi_mins, cat_ids)

def gsmSelectBox (outfile, ra_st, ra_end, dec_st, dec_end, fluxi_mins, cat_ids):
    for ra in [ra_st,ra_end]:
        if ra < 0  or  ra > 360:
            print 'RA', ra, 'is invalid: <0 or >360 degrees'
            return -1
    for dec in [dec_st,dec_end]:
        if dec < -90  or  dec > 90:
            print 'DEC', dec, 'is invalid: <-90 or >90 degrees'
            return -1
    if dec_st > dec_end:
        print 'invalid DEC: start', dec_st, '> end', dec_end
        return -1
    # Form the selection for the RA,DEC.
    # If RA crosses 360 degrees, split in two parts.
    if ra_st <= ra_end:
        select = '(c1.ra BETWEEN %17.12f AND %17.12f)' % (ra_st, ra_end)
    else:
        select = '(c1.ra BETWEEN %17.12f AND 360 OR c1.ra BETWEEN 0 AND %17.12f)' % (ra_st, ra_end)
    select += ' AND (c1.decl BETWEEN %17.12f AND %17.12f)' % (dec_st, dec_end)
    return doSelect (outfile, select, fluxi_mins, cat_ids)

# Interpret the arguments and do the selection.
def gsmMain (name, argv):
    useBox = False
    starg = 0
    optarg = 4
    if len(argv) > 0  and  argv[0] == '-b':
        # select using a box
        useBox = True
        starg = 1
        optarg = 6
    if len(argv) < optarg:
        print ''
        print 'Insufficient arguments given; run as:'
        print ''
        print '   %s outfile RA DEC radius [minFluxI [catalogs]]' % name
        print 'to select using a cone'
        print ''
        print '   %s -b outfile stRA endRA stDEC endDEC [minFluxI [catalogs]]' % name
        print 'to select using a box'
        print ''
        print '   outfile    path-name of the output file'
        print '              It will be overwritten if already existing'
        print '   RA         cone center Right Ascension (J2000, degrees)'
        print '   DEC        cone center Declination     (J2000, degrees)'
        print '   radius     cone radius                 (degrees)'
        print '   stRA       box start   Right Ascension (J2000, degrees)'
        print '   endRA      box end     Right Ascension (J2000, degrees)'
        print '      stRA can be > endRA indicating crossing 360 deg'
        print '   minDEC     box start   Declination     (J2000, degrees)'
        print '   maxDEC     box end     Declination     (J2000, degrees)'
        print '   minFluxI   minimum flux (integrated Stokes I flux in Jy)'
        print '              If a single value is given, it is used for all catalogs.'
        print '              If multiple values given (separated by commas), each applies'
        print '              to the corresponding catalog.' 
        print '              Default is 0.5'
        print '   catalogs   names of catalogs to search (case-insensitive)'
        print '              (NVSS, VLSS, and/or WENSS)'
        print '              If multiple, separate by commas'
        print '              Default is WENSS'
        print ''
        return False
    # Get the arguments.
    outfile = argv[starg]
    a1 = float(argv[starg+1])
    a2 = float(argv[starg+2])
    a3 = float(argv[starg+3])
    if useBox:
        a4 = float(argv[starg+4])
    minFlux = [0.5]
    if len(argv) > optarg  and  len(argv[optarg]) > 0:
        minFlux = [float(x) for x in argv[optarg].split(',')]
    #  cat_id = 3 => NVSS
    #  cat_id = 4 => VLSS
    #  cat_id = 5 => WENSS
    cats = ['WENSS']
    if len(argv) > optarg+1  and  len(argv[optarg+1]) > 0:
        cats = argv[optarg+1].split (',')
    cat_ids = []
    for cat in cats:
        cat = cat.upper()
        if cat == 'NVSS':
            cat_ids.append (3)
        elif cat == 'VLSS':
            cat_ids.append (4)
        elif cat == 'WENSS':
            cat_ids.append (5)
        else:
            print cat, 'is an invalid catalog name'
            return False
    if len(minFlux) != 1  and  len(minFlux) != len(cat_ids):
        print 'Nr of minFlux values (%s) must be 1 or match nr of catalogs' % argv[5]
        return False;
    # Do the selection and create the makesourcedb file.
    if useBox:
        nr = gsmSelectBox  (outfile, a1, a2, a3, a4, minFlux, cat_ids)
    else:
        nr = gsmSelectCone (outfile, a1, a2, a3, minFlux, cat_ids)
    if nr < 0:
        return False
    elif nr == 0:
        print 'Warning: no matching sources found in GSM for'
        if useBox:
            print '  stRA=%f endRA=%f stDEC=%f endDEC=%f' % (a1,a2,a3,a4)
        else:
            print '  RA=%f DEC=%f radius=%f' % (a1,a2,a3)
        print '  minFluxI=%s catalogs=%s' % (str(minFlux), cats)
    else:
        print '%d sources written into %s' % (nr, outfile)
    return True


# This is the main entry.
if __name__ == "__main__":
    import sys
    if gsmMain (sys.argv[0], sys.argv[1:]):
        sys.exit(0)
    sys.exit(1)
