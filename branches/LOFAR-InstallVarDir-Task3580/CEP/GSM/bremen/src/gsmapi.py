#!/usr/bin/python
import sys
import argparse
from texttable import Texttable
from src.gsmconnectionmanager import GSMConnectionManager
from src.sqllist import get_sql
from src.queries import get_field as get_field_sql


STYLE_PLAIN = 0
STYLE_TABLE = 1


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
        return {'header': ['Name', 'Sources', 'MinFlux', 'NewSources'],
                'data': self.connection.get_cursor(
                                         get_sql("APIimage", image_id)
                                         ).fetchall()}

    def get_field(self, ra, decl, radius, band, min_flux=None):
        """
        Get sky-model.
        """
        return {'header': ['ra', 'decl', 'f_peak'],
                'data': self.connection.get_cursor(
                               get_field_sql(ra, decl, radius, band,
                                             min_flux=min_flux)
                                         ).fetchall()}

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run GSM API-call')
    parser.add_argument('-f', '--filename')
    parser.add_argument('-s', '--style', default=STYLE_PLAIN)
    parser.add_argument('-S', '--separator', default=' ')
    parser.add_argument('command', type=str)
    parser.add_argument('--image_id', type=int)
    parser.add_argument('--ra', default=0, type=float)
    parser.add_argument('--decl', default=0, type=float)
    parser.add_argument('--radius', default=5.0, type=float)
    parser.add_argument('-B', '--band', default=8, type=int)
    parser.add_argument('--stokes', default='I', type=str)
    parser.add_argument('--f_peak', default=None, type=float)

    args = parser.parse_args()
    connect = GSMConnectionManager(database='test').get_connection()
    api = GSMAPI(connect)
    if args.command == 'image':
        dataset = api.get_image_properties(args.image_id)
    elif args.command == 'field':
        dataset = api.get_field(args.ra, args.decl, args.radius,
                                args.band, args.f_peak)
    else:
        raise ValueError

    api.output(dataset, int(args.style), args.filename, args.separator)


