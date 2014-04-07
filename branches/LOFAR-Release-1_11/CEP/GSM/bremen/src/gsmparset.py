#!/usr/bin/python
from os import path
from math import cos

from src.ini_load import load_parameters
from src.errors import ParsetContentError, SourceException, GSMException
from src.bbsfilesource import GSMBBSFileSource
from src.pysefilesource import PySEFileSource
from src.sqllist import get_sql, get_svn_version
from src.gsmlogger import get_gsm_logger
from src.queries import sql_insert_run


class GSMParset(object):
    """
    Parset file reader and processor.
    """
    def __init__(self, filename):
        """
        Read parset from a given file.
        """
        self.filename = filename
        self.run_id = None
        self.log = get_gsm_logger('parsets', 'import.log')
        if not path.isfile(filename):
            self.log.error('Parset file does not exist: %s' % filename)
            raise GSMException('Parset file does not exist: %s' % filename)
        self.path = path.dirname(path.realpath(filename))
        self.data = load_parameters(filename)
        self.parset_id = self.data.get('image_id')
        self.image_id = None  # Not yet known.
        self.source_count = None
        self.recalculate_pointing = False
        self.log.info('Parset opened: %s' % filename)

    def process(self, conn):
        """
        Read data from all sources-lists and store it into the database.
        """
        sources = self.data.get('source_lists')
        loaded_sources = 0
        if not sources:
            raise ParsetContentError('Source list (source_lists) missing')
        elif isinstance(sources, str):
            sources = sources.strip(' []').replace(' ', '').split(',')
        elif isinstance(sources, list):
            sources = ','.join(sources).strip(' []'
                                         ).replace(' ', '').split(',')
        if not self.parset_id:
            raise ParsetContentError('"image_id" missing')
        conn.start()
        conn.execute(sql_insert_run())
        self.run_id = conn.exec_return("""
                select max(runid) from runs where status=0;""",
                                            single_column=True)
        for source in sources:
            if self.data.get('bbs_format'):
                if self.data.get('bbs_format') == 'PySE':
                    bbsfile = PySEFileSource(self.parset_id, self.run_id,
                                               "%s/%s" % (self.path, source),
                                               self.data.get('bbs_format'))
                else:
                    bbsfile = GSMBBSFileSource(self.parset_id, self.run_id,
                                               "%s/%s" % (self.path, source),
                                               self.data.get('bbs_format'))
            else:
                bbsfile = GSMBBSFileSource(self.parset_id, self.run_id,
                                        "%s/%s" % (self.path, source))
            bbsfile.read_and_store_data(conn)
            loaded_sources = loaded_sources + bbsfile.sources
        self.image_id = self.save_image_info(conn)
        conn.commit()
        self.log.info('%s sources loaded from parset %s' % (
                      loaded_sources, self.filename))

        self.source_count = loaded_sources
        return loaded_sources

    def get_image_size(self, min_decl, max_decl, min_ra, max_ra,
                       avg_decl, avg_ra):
        """
        >>> t = GSMParset('tests/image1.parset')
        >>> t.get_image_size(1.0, 3.0, 1.0, 3.0, 2.0, 2.0)
        (1.0, 2.0, 2.0)
        >>> t.get_image_size(-4.0, 4.0, 1.0, 359.0, 0.0, 359.8)
        (4.0, 0.0, 0.0)
        """
        if max_ra - min_ra > 250.0:
            # Field across zero-ra. Has to be shifted.
            # E.g. min = 0.1 max = 359.7 avg = 359.9
            # transfers to:
            # min = -0.3 max = 0.1 avg = -0.1
            min_ra, max_ra = max_ra - 360.0, min_ra
            avg_ra = 0.5 * (max_ra + min_ra)
        min_ra = min_ra * cos(avg_decl)
        max_ra = max_ra * cos(avg_decl)
        return max([avg_decl - min_decl, max_decl - avg_decl,
                    avg_ra * cos(avg_decl) - min_ra,
                    max_ra - avg_ra * cos(avg_decl)]), \
                    avg_decl, avg_ra

    def get(self, key):
        """
        SQL-friendly get.
        """
        if key in self.data:
            return self.data.get(key)
        else:
            return 'null'

    def save_image_info(self, conn):
        """
        Write image info into images table.
        """
        if not self.data.get('frequency').isdigit():
            raise SourceException('Frequency should be digital, %s found'
                                    % self.data.get('frequency'))
        band = conn.exec_return(get_sql('get frequency',
                                        self.data.get('frequency')))
        if not band or band == -1:
            raise SourceException(
                'No matching frequency band found for frequency %s' %
                            self.data.get('frequency'))

        if not 'pointing_ra' in self.data or \
           not 'pointing_decl' in self.data or \
           not 'beam_size' in self.data:
            data = conn.exec_return(
            """select min(ldecl), max(ldecl),
                      min(lra), max(lra),
                      avg(ldecl), avg(lra)
                 from detections
                 where run_id = %s;""" % self.run_id, single_column=False)
            size, avg_decl, avg_ra = self.get_image_size(*data)
            self.recalculate_pointing = True
        else:
            size = self.data.get('beam_size')
            avg_decl = self.data.get('pointing_decl')
            avg_ra = self.data.get('pointing_ra')

        conn.execute(get_sql('insert image', self.parset_id, band,
                             avg_ra, avg_decl, size,
                             get_svn_version(), self.run_id,
                             self.get('bmaj'),
                             self.get('bmin'),
                             self.get('bpa')))
        image_id = conn.exec_return(get_sql('get last image_id'))
        self.log.info('Image %s created' % image_id)
        return image_id
