#!/usr/bin/python
from os import path
from configobj import ConfigObj
from src.errors import ParsetContentError, SourceException
from src.bbsfilesource import GSMBBSFileSource
from src.queries import get_insert_image
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
        self.path = path.dirname(path.realpath(filename))
        self.data = ConfigObj(filename, raise_errors=True, file_error=True)
        self.parset_id = self.data.get('image_id')
        self.image_id = None  # Not yet known.
        self.source_count = None
        self.log = get_gsm_logger('parsets', 'import.log')
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
            sources = [sources]
        if not self.parset_id:
            raise ParsetContentError('"image_id" missing')
        self.image_id = self.save_image_info(conn)
        for source in sources:
            bbs = source.strip(' []')
            if self.data.get('bbs_format'):
                bbsfile = GSMBBSFileSource(self.parset_id,
                                           "%s/%s" % (self.path, bbs),
                                           self.data.get('bbs_format'))
            else:
                bbsfile = GSMBBSFileSource(self.parset_id,
                                           "%s/%s" % (self.path, bbs))
            if not bbsfile.read_and_store_data(conn):
                raise SourceException
            loaded_sources = loaded_sources + bbsfile.sources
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
        sql_insert = get_insert_image(self.parset_id,
                                      self.data.get('frequency'))
        result = conn.execute_set(sql_insert)[0][0]
        if not result or result == -1:
            raise SourceException(
                        'No matching frequency band found for frequency %s' %
                            self.data.get('frequency'))
        self.log.info(result)
        return result


