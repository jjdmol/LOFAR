#!/usr/bin/python
"""
Various tools for testing.
"""
from os import path
from src.connectionMonet import MonetConnection

FREQUENCY = {
    1: 30000000,
    2: 34000000,
    3: 38000000,
    4: 42000000,
    5: 120000000,
    6: 130000000,
    7: 140000000,
    8: 150000000,
    9: 160000000,
    10: 170000000,
    11: 325000000,
    12: 352000000,
    13: 640000000,
    14: 850000000,
    15: 1400000000,
    16: 2300000000,
    17: 4800000000,
    18: 8500000000
}

def get_frequency(band):
    """
    Convert GSM band to frequency.
    """
    return str(FREQUENCY[band])


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
        if isinstance(conn, MonetConnection):
            conn._execute_with_cursor("delete from %s;" % tbl, cursor)
        else:
            conn._execute_with_cursor("truncate table %s;" % tbl, cursor)

    for seq in ['seq_datasets',
                'seq_images',
                'seq_runningcatalog']:
        conn._execute_with_cursor('alter sequence %s restart with 1;' % seq, cursor)
    conn._execute_with_cursor('alter sequence seq_extractedsources restart with 1001;', cursor)
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
