#!/usr/bin/env python

# Select a subset from the GSM and write into a makesourcdb input file.
# ra and dec both in degrees
# integrated flux [in Jy] above which the sources will be selected
# You should know that:
#  cat_id = 3 => NVSS
#  cat_id = 4 => VLSS
#  cat_id = 5 => WENSS
def gsmSelect (outfile, ra_st, ra_end, dec_st, dec_end, fluxi_mins, cat_ids):

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

    # If RA crosses 360 degrees, split in two parts.
    if ra_st <= ra_end:
        ra_where = '(c1.ra BETWEEN %f AND %f)' % (ra_st, ra_end)
    else:
        ra_where = '(c1.ra BETWEEN %f AND 360 OR c1.ra BETWEEN 0 AND %f)' % (ra_st, ra_end)
    # If a single catalog, use =
    if len(cat_ids) == 1:
        cat_where = '(c1.cat_d = %d AND c1.i_int_avg >= %f)' % (cat_ids[0], fluxi_mins[0])
    else:
        # Multiple catalogs.
        # If a single minimum flux, use it for all catalogs.
        # Note that the IN clause needs (), not [].
        if len(fluxi_mins) == 1:
            cat_where = '(c1.cat_d IN (%s) AND c1.i_int_avg >= %f)' % (str(cat_ids)[1:-1], fluxi_mins[0])
        else:
            # We have a minimum flux per catalog.
            cat_where = '('
            for i in range(len(cat_ids)):
                if i > 0:
                    cat_where += ' OR ' 
                cat_where += '(c1.cat_d = %d AND c1.i_int_avg >= %f)' % (cat_ids[i], fluxi_mins[i])
            cat_where += ')'
    # Form the entire where clause.
    where = 'WHERE %s AND %s AND c1.decl BETWEEN %f AND %f' % (cat_where, ra_where, dec_st, dec_end)
    print where

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
                  "          FROM (SELECT CAST(TRIM(c1.catsrcname) AS VARCHAR(20)) AS catsrcname " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('POINT' AS VARCHAR(20)) " + \
                  "                            ELSE CAST('GAUSSIAN' AS VARCHAR(20)) " + \
                  "                       END AS src_type " + \
                  "                      ,CAST(c1.ra AS VARCHAR(20))  AS ra " + \
                  "                      ,CAST(c1.decl AS VARCHAR(20)) AS decl " + \
                  "                      ,CAST(c1.i_int_avg AS VARCHAR(20)) AS i " + \
                  "                      ,CAST(0 AS VARCHAR(20)) AS q " + \
                  "                      ,CAST(0 AS VARCHAR(20)) AS u " + \
                  "                      ,CAST(0 AS VARCHAR(20)) AS v " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('' AS VARCHAR(20)) " + \
                  "                            ELSE CASE WHEN c1.major IS NULL " + \
                  "                                      THEN CAST('' AS VARCHAR(20)) " + \
                  "                                      ELSE CAST(c1.major AS varchar(20)) " + \
                  "                                 END " + \
                  "                       END AS MajorAxis " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('' AS VARCHAR(20)) " + \
                  "                            ELSE CASE WHEN c1.minor IS NULL " + \
                  "                                      THEN CAST('' AS VARCHAR(20)) " + \
                  "                                      ELSE CAST(c1.minor AS varchar(20)) " + \
                  "                                 END " + \
                  "                       END AS MinorAxis " + \
                  "                      ,CASE WHEN c1.pa IS NULL " + \
                  "                            THEN CAST('' AS VARCHAR(20)) " + \
                  "                            ELSE CAST(c1.pa AS varchar(20)) " + \
                  "                       END AS Orientation " + \
                  "                      ,CAST(c1.freq_eff AS VARCHAR(20)) AS ReferenceFrequency " + \
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
    file.write ("format = Name, Type, Ra, Dec, I, Q, U, V, MajorAxis, MinorAxis, Orientation, ReferenceFrequency='60e6', SpectralIn\dex='[]'\n")
    for i in range(len(y)):
        file.write(y[i][0] + '\n')
    file.close()
    return len(y)


def gsmMain (name, argv):
    if len(argv) < 5:
        print ''
        print 'Insufficient arguments given; run as:'
        print ''
        print '   %s outfile stRA endRA stDEC endDEC [minFluxI [catalogs]]' % name
        print ''
        print '      outfile    path-name of the output file'
        print '                 It will be overwritten if already existing'
        print '      stRA       start Right Ascension (J2000, degrees)'
        print '      endRA      end   Right Ascension (J2000, degrees)'
        print '         stRA can be > endRA indicating crossing 360 deg'
        print '      minDEC     start Declination (J2000, degrees)'
        print '      maxDEC     end   Declination (J2000, degrees)'
        print '      minFluxI   minimum flux (integrated Stokes I in Jy)'
        print '                 default = 0.5'
        print '                 If a single value is given, it is used for all catalogs.'
        print '                 If multiple values (separated by commas), each applies'
        print '                 to the corresponding catalog.' 
        print '      catalogs   names of catalogs to search (case-insensitive)'
        print '                 (NVSS, VLSS, and/or WENSS)'
        print '                 If multiple, separate by commas'
        print '                 Default is WENSS'
        return False
    outfile = argv[0]
    stRA    = float(argv[1])
    endRA   = float(argv[2])
    stDEC   = float(argv[3])
    endDEC  = float(argv[4])
    for ra in [stRA,endRA]:
        if ra < 0  or  ra > 360:
            print 'RA', ra, 'is invalid: <0 or >360 degrees'
            return False
    for dec in [stDEC,endDEC]:
        if dec < -90  or  dec > 90:
            print 'DEC', dec, 'is invalid: <-90 or >90 degrees'
            return False
    if stDEC > endDEC:
        print 'invalid DEC: start', stDEC, '> end', endDEC
        return False
    minFlux = [0.5]
    if len(argv) > 5:
        minFlux = [float(x) for x in argv[5].split (',')]
    #  cat_id = 3 => NVSS
    #  cat_id = 4 => VLSS
    #  cat_id = 5 => WENSS
    cats = ['WENSS']
    if len(argv) > 6:
        cats = argv[6].split (',')
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
        print 'Nr of minFlux values must be 1 or match nr of catalogs'
        return False;
    # Do the selection and create the makesourcedb file.
    nr = gsmSelect (outfile, stRA, endRA, stDEC, endDEC, minFlux, cat_ids)
    if nr < 0:
        return False
    if nr == 0:
        print 'No matching sources found in GSM catalogs for '
        print '  stRA=%f endRA=%f stDEC=%f endDEC=%f' % (stRA,endRA,stDEC,endDEC)
        print '  minFluxI=%f catalogs=%s' % (minFlux, cats)
    return True


# This is the main entry.
if __name__ == "__main__":
    import sys
    if gsmMain (sys.argv[0], sys.argv[1:]):
        sys.exit(0)
    sys.exit(1)
