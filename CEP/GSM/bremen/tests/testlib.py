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
    cursor.close()

def write_parset(filename, source, freq):
    fil = open(filename, 'w')
    fil.write("""##############################
# Lofar GSM input parset.    #
##############################
source_lists = %s
image_id = %s
image_date = ???
frequency = %s # in Hz
pointing_ra = 0.0 # in degrees
pointing_decl = 0.0 # in degrees
beam_size = 4.00 # in degrees
stokes = I""" % (source, filename, freq))
    fil.close()
