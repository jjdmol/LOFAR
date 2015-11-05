#!/usr/bin/python
import sys
from texttable import Texttable
from src.sqllist import get_sql
from src.queries import get_field as get_field_sql

STYLE_PLAIN = 0
STYLE_TABLE = 1  # Unsupported


class GSMAPI(object):
    """
    API module for GSM database.
    """

    def __init__(self, conn=None):
        """
        """
        self.connection = conn
        self.style = STYLE_PLAIN
        self.separator = ' '

    def output(self, data, style=STYLE_PLAIN, filename=None, separator=' '):
        """
        Output a dataset with given parameters.
        """
        if filename:
            xfile = open(filename, 'w')
            oldstdout = sys.stdout
            sys.stdout = xfile
        if style == STYLE_PLAIN:
            print separator.join(map(str, data['header']))
        elif style == STYLE_TABLE:
            table = Texttable()
            table.set_deco(Texttable.HEADER)
            table.add_row(data['header'])
        else:
            raise NotImplementedError('Unsupported style')

        for aline in data['data']:
            if style == STYLE_PLAIN:
                print separator.join(map(str, aline))
            elif style == STYLE_TABLE:
                table.add_row(aline)
            else:
                raise NotImplementedError('Unsupported style')

        if style == STYLE_TABLE:
            print table.draw()

        if filename:  # restore the old value
            sys.stdout = oldstdout

    def get_image_properties(self, image_id):
        """
        List basic image properties.
        """
        return {'header': ['Name', 'Sources', 'MinFlux'],
                'data': self.connection.get_cursor(
                                         get_sql("APIimage", image_id)
                                         ).fetchall()}

    def get_field(self, ra, decl, radius, band, min_flux=None):
        """
        Get sky-model.
        """
        return {'header': ['ra', 'decl', 'f_peak'],
                'data': self.connection.get_cursor(
                               get_field_sql(ra, decl, radius, band, min_flux)
                                         ).fetchall()}


