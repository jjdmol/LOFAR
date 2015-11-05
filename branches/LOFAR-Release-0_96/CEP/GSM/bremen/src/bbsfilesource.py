#!/usr/bin/python
"""
BBS-format file source object for GSM.
Author: Alexey Mints (2012).
"""
from src.errors import SourcePartMissingException
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
      'total_flux': 'flux',
      'e_total_flux': 'flux_err',
    }

    def __init__(self, file_id, filename, fileformat="default"):
        self.filename = filename
        self.file_id = file_id
        self.fileformat = fileformat
        # Header is a hash of column header.
        # For each column name a (column-number, default) tuple is stored.
        self.header = {}
        self.sources = 0
        self.log = get_gsm_logger('bbsfiles', 'import.log')
        self.log.info('BBS file source created for name: %s' % filename)

    def get_part(self, name, line):
        """
        Get a named part from the line using header.
        """
        if name in self.header:
            aheader = self.header[name]
            if (aheader[0] < len(line) and line[aheader[0]]):
                return line[aheader[0]]
            elif aheader[1]:
                return aheader[1]
            else:
                raise SourcePartMissingException(
                                    "Missing data for column %s in file %s"
                                            % (name, self.filename))
        else:
            raise SourcePartMissingException("No column %s in file %s"
                                            % (name, self.filename))

    def process_line(self, line):
        """
        Turn a line into a hash-dataset using header information.
        """
        answer = {}
        for part in ["ra", "decl", "flux", "flux_err"]:
            answer[part] = self.get_part(part, line)
        return answer

    def read_and_store_data(self, conn):
        """
        Read all from the BBS file.
        """
        line = None
        datafile = open(self.filename, 'r')
        if self.fileformat == 'test':
            header = datafile.readline().split('=',
                                        1)[1].strip(' ').lower().split(',')
            ind = 0
            for head_parts in header:
                head_part = head_parts.split('=')
                if len(head_part) == 1:
                    # No default value
                    self.header[head_parts.strip()] = (ind, None)
                else:
                    self.header[head_part[0].strip()] = (ind,
                                            head_part[1].strip("'").strip())
                ind = ind + 1
        elif self.fileformat == 'default':
            line = datafile.readline()
            while line.startswith('#'):
                comments = line
                line = datafile.readline()
            header = comments[2:].strip().lower().split(' ')
            ind = 0
            for head_part in header:
                self.header[self.FIELD_NAMES[head_part]] = (ind, head_part)
                ind = ind + 1

        sql_data = []
        commit = conn.autocommit
        conn.set_autocommit(False)
        sql_insert = 'insert into detections (image_id, lra, ldecl, lra_err, '\
                     'ldecl_err, lf_peak, lf_peak_err, ldet_sigma) values'

        while True:
            data_lines = datafile.readlines(self.BLOCK_SIZE)
            if not data_lines:
                break
            for data_line in data_lines:
                self.sources = self.sources + 1
                dhash = self.process_line(data_line.split())
                sql_data.append("('%s', %s, %s, 0.1, 0.1, %s, %s, 3.0)" %
                                (self.file_id, dhash['ra'],
                                 dhash['decl'], dhash['flux'],
                                 dhash['flux_err']))
            sql = "%s %s;" % (sql_insert, ',\n'.join(sql_data))
            result = conn.execute(sql)
            self.log.info('%s sources loaded' % str(result).strip())
            conn.commit()
        #Restore autocommit.
        conn.set_autocommit(commit)
        return True
