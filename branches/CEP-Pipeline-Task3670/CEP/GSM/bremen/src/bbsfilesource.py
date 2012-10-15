#!/usr/bin/python
"""
BBS-format file source object for GSM.
Author: Alexey Mints (2012).
"""
import os.path
import healpy as hp
from copy import copy
from math import radians
from src.errors import SourceException
from src.gsmlogger import get_gsm_logger


class GSMBBSFileSource(object):
    """
    Reads source list from BBS file.
    """
    #Number of lines to be read in each iteration;
    BLOCK_SIZE = 100

    FIELD_NAMES = {
      'ra': 'ra',
      'e_ra': 'ra_err',
      'dec': 'decl',
      'e_dec': 'decl_err',
      'total_flux': 'int_flux',
      'e_total_flux': 'int_flux_err',
      'peak_flux': 'peak_flux',
      'e_peak_flux': 'peak_flux_err',
    }

    HEADER_INDEX = {
      'ra': 0,
      'dec': 1,
      'e_ra': 2,
      'e_dec': 3,
      'peak_flux': 4,
      'e_peak_flux': 5,
      'total_flux': 6,
      'e_total_flux': 7,
      'dc_min': 8,
      'e_min': 9,
      'dc_maj': 10,
      'e_maj': 11,
      'dc_pa': 12,
      'e_pa': 13,
    }

    DEFAULTS = map(str, [0, 0, 0.1, 0.1,  # ra/decl
                         0, 0.001, 0, 0.001,  # Flux
                         0.0, 0.001, 0.0, 0.001, 0.0, 0.001,  # Gaussian
                         3.0,
                         ])

    def __init__(self, parset_id, run_id, filename, fileformat="default"):
        """
        :param parset_id: Unique LOFAR image identificator.
        :param filename: Name of the file on disk.
        :param fileformat: 'default' or 'test'.
        Test file format allows setting default values for columns.
        """
        self.filename = filename
        self.parset_id = parset_id
        self.run_id = run_id
        self.fileformat = fileformat
        self.defaults = copy(self.DEFAULTS)
        self.sources = 0
        self.log = get_gsm_logger('bbsfiles', 'import.log')
        self.log.info('BBS file source created for name: %s' % filename)
        self.order = []

    def process_header(self, header):
        """
        Get reordering array for the file.
        Relates columns in the file with the ones in the table Detections.
        Fills out an array *self.order*, that contains tuples of type
        ("column index in file", "column index in table"),
        or updates default values if needed.
        """
        self.order = []
        for ind, item in enumerate(header):
            if type(item) == str and item in self.HEADER_INDEX:
                self.order.append((ind, self.HEADER_INDEX[item]))
            elif type(item) == tuple and item[0] in self.HEADER_INDEX:
                self.defaults[self.HEADER_INDEX[item[0]]] = item[1]

    def process_line(self, line):
        """
        Turn a line into an array using header information.
        """
        answer = copy(self.defaults)
        len_line = len(line)
        for from_index, to_index in self.order:
            if from_index < len_line:
                answer[to_index] = line[from_index]
        return answer

    def get_header_test(self, datafile):
        """
        Get header for a 'test' data-format.
        No comments are supported. First line is a list
        of column-names or column default values, like:
        ra ra_err=0.01 decl decl_err=0.01
        In the example above two columns (ra and decl) are taken from
        the data, and for ra_err and decl_err a default value is taken.
        """
        try:
            header = datafile.readline().split('=',
                                    1)[1].strip(' ').lower().split(',')
            for ind, head_parts in enumerate(header):
                head_part = head_parts.split('=')
                if len(head_part) != 1:  # Default value is given
                    header[ind] = (head_part[0],
                                   head_part[1].strip("'").strip())
        except IndexError:
            raise SourceException('Wrong header in the first line' \
                                  ' of file %s' % self.filename)
        return header

    def get_header_default(self, datafile):
        """
        Get header for a 'default' data-format.
        Comments should start with #.
        List of columns should be in the header:
        # RA DECL...
        """
        line = datafile.readline()
        while not (line.startswith('# Gaus_id') or
                   line.startswith("# RA")):
            line = datafile.readline()
            if not line:
                raise SourceException('No header in file %s' %
                                      self.filename)
        return line[2:].strip().lower().split(' ')

    def read_and_store_data(self, conn):
        """
        Read all from the BBS file.
        """
        header = None
        if not os.path.isfile(self.filename):
            raise SourceException('no file %s' % self.filename)
        datafile = open(self.filename, 'r')
        if self.fileformat == 'test':
            header = self.get_header_test(datafile)
        elif self.fileformat == 'default':
            header = self.get_header_default(datafile)
        if not header:
            raise SourceException('No header in file %s' % self.filename)
        self.process_header(header)

        sql_data = []
        # Switch off autocommit (if it is switched on) for better performance.
        sql_insert = 'insert into detections (run_id, image_name, '\
                     'lra, ldecl, lra_err, ldecl_err,'\
                     'lf_peak, lf_peak_err, lf_int, lf_int_err, ' \
                     'g_minor, g_minor_err, g_major, g_major_err,' \
                     'g_pa, g_pa_err, ldet_sigma, healpix_zone) values'
        while True:
            data_lines = datafile.readlines(self.BLOCK_SIZE)
            if not data_lines:
                break
            for data_line in data_lines:
                if data_line.strip() == '' or data_line.startswith('#'):
                    #skip comments and empty lines
                    continue
                self.sources = self.sources + 1
                dhash = self.process_line(data_line.split())
                pix = hp.ang2pix(16, radians(90. - float(dhash[1])),
                                 radians(float(dhash[0])), nest=True)
                sql_data.append("(%s, '%s', %s, %s )" %
                                (self.run_id, self.parset_id,
                                 ','.join(dhash), pix))
            sql = "%s %s;" % (sql_insert, ',\n'.join(sql_data))
            conn.execute(sql)
            self.log.info('%s sources loaded from %s' % (self.sources,
                                                         self.filename))
            sql_data = []
        #Restore autocommit.
        return True
