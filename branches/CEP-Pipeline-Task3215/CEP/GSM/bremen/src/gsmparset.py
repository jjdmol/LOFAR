#!/usr/bin/python
from os import path
try:
    # Try loading LOFAR parset support, fallback to ConfigObj.
    from lofar.parameterset import parameterset
    LOFAR_PARAMETERSET = True
except ImportError:
    from configobj import ConfigObj
    LOFAR_PARAMETERSET = False

from src.errors import ParsetContentError, SourceException, GSMException
from src.bbsfilesource import GSMBBSFileSource
from src.sqllist import get_sql, get_svn_version
from src.gsmlogger import get_gsm_logger


class GSMParset(object):
    """
    Parset file reader and processor.
    """
    def __init__(self, filename):
        """
        Read parset from a given file.
        """
        self.filename = filename
        self.log = get_gsm_logger('parsets', 'import.log')
        if not path.isfile(filename):
            self.log.error('Parset file does not exist: %s' % filename)
            raise GSMException('Parset file does not exist: %s' % filename)
        self.path = path.dirname(path.realpath(filename))
        if LOFAR_PARAMETERSET:
            self.data = parameterset(filename).dict()
        else:
            self.data = ConfigObj(filename, raise_errors=True, file_error=True)
        self.parset_id = self.data.get('image_id')
        self.image_id = None  # Not yet known.
        self.source_count = None
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
            sources = ','.join(sources).strip(' []').replace(' ', '').split(',')
        if not self.parset_id:
            raise ParsetContentError('"image_id" missing')
        conn.start()
        self.image_id = self.save_image_info(conn)
        for source in sources:
            if self.data.get('bbs_format'):
                bbsfile = GSMBBSFileSource(self.parset_id,
                                           "%s/%s" % (self.path, source),
                                           self.data.get('bbs_format'))
            else:
                bbsfile = GSMBBSFileSource(self.parset_id,
                                           "%s/%s" % (self.path, source))
            bbsfile.read_and_store_data(conn)
            loaded_sources = loaded_sources + bbsfile.sources
        conn.commit()
        self.log.info('%s sources loaded from parset %s' % (loaded_sources,
                                                            self.filename))

        self.source_count = loaded_sources
        return loaded_sources

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
        conn.execute(get_sql('insert image', self.parset_id, band,
                             get_svn_version()))
        image_id = conn.exec_return(get_sql('get last image_id'))
        self.log.info('Image %s created' % image_id)
        return image_id


