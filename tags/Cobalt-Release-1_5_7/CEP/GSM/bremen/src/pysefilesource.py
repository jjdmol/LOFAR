import os.path
import healpy as hp
from math import radians
import scipy as sp
from src.errors import SourceException
from src.bbsfilesource import GSMBBSFileSource


class PySEFileSource(GSMBBSFileSource):
    """
    Reads source list from PySE output file.
    """
    BLOCK_SIZE = 1000

    def read_and_store_data(self, conn):
        """ Function doc """
        if not os.path.isfile(self.filename):
            raise SourceException('no file %s' % self.filename)
        header = open(self.filename, 'r').readline()
        data = sp.loadtxt(self.filename, delimiter=',', skiprows=1)
        for fro, to in (('ra', 'lra'), ('dec', 'ldecl'),
                        ('smaj', 'g_major'), ('smin', 'g_minor'), ('pa', 'g_pa'),
                        ('int_flux', 'lf_int'), ('pk_flux', 'lf_peak') ):
            header = header.replace(fro, to)
        sql_data = []
        # Switch off autocommit (if it is switched on) for better performance.
        sql_insert = 'insert into detections (run_id, image_name, '\
                     '%s, ldet_sigma, healpix_zone) values' % header
        self.sources = 0
        for source in data:
            self.sources = self.sources + 1
            pix = hp.ang2pix(32, radians(90. - source[2]),
                             radians(source[0]), nest=True)
            # Convert axes from arcsec to degrees:
            source[4:8] = source[4:8] / 3600.0
            sql_data.append("(%s, '%s', %s, 3, %s )" %
                            (self.run_id, self.parset_id,
                             ','.join(map(str, source)), pix))
            if self.sources % self.BLOCK_SIZE == 0:
                sql = "%s %s;" % (sql_insert, ',\n'.join(sql_data))
                conn.execute(sql)
                self.log.info('%s sources loaded from %s' % (self.sources,
                                                             self.filename))
                sql_data = []
        if len(sql_data) > 0:
            sql = "%s %s;" % (sql_insert, ',\n'.join(sql_data))
            conn.execute(sql)
            self.log.info('%s sources loaded from %s' % (self.sources,
                                                         self.filename))
        return True
