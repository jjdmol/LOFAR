#!/usr/bin/python
"""
Various tools for testing.
"""
from os import path
from src.connectionMonet import MonetLoggedConnection

def load_from_csv_file(conn, filename, table):
    """
    Load data from CSV file into a table with the given name.
    """
    cursor = conn.cursor()
    cursor.execute("copy into %s from '%s' USING DELIMITERS ',';" %
                   (table, path.abspath(filename)))
    cursor.close()


def cleanup_db(conn):
    """
    Cleanup some tables in the database.
    """
    conn.start()
    cursor = conn.cursor()
    for tbl in ['assocxtrsources',
                'runningcatalog_fluxes',
                'runningcatalog',
                'temp_associations',
                'images',
                'extractedsources',
                'detections']:
        if isinstance(conn, MonetLoggedConnection):
            cursor.execute("delete from %s;" % tbl)
        else:
            cursor.execute("truncate table %s;" % tbl)

    for seq in ['seq_datasets',
                'seq_extractedsources',
                'seq_images',
                'seq_runningcatalog']:
        cursor.execute('alter sequence %s restart with 1;' % seq)
    conn.commit()
    cursor.close()

def write_parset(filename, source, freq, ra=0.0, decl=0.0):
    fil = open(filename, 'w')
    fil.write("""##############################
# Lofar GSM input parset.    #
##############################
source_lists = %s
image_id = %s
image_date = ???
frequency = %s # in Hz
pointing_ra = %s # in degrees
pointing_decl = %s # in degrees
beam_size = 1.00 # in degrees
stokes = I""" % (source, filename, freq, ra, decl))
    fil.close()
