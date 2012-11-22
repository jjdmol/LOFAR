#!/usr/bin/python
import cPickle as pickle
from src.gsmlogger import get_gsm_logger, USE_CONSOLE
from src.connectionMonet import MonetConnection
from src.connectionPostgres import PgConnection


class GSMConnectionManager(object):
    """
    General connection manager.
    Have options to save/load connection parameters.
    """
    DEFAULTS = {
        'hostname': 'localhost',
        'port': 52000,
        'database': 'GSM',
        'username': 'monetdb',
        'password': 'monetdb',
        'autocommit': True
    }

    def __init__(self, use_monet=True, use_console=USE_CONSOLE, **params):
        """
        """
        self.params = self.DEFAULTS.copy()
        self.params.update(params)
        self.log = get_gsm_logger('sql', 'sql.log', use_console)
        self.use_monet = use_monet

    def get_connection(self, **ext_params):
        """
        Get a connection with a given params.
        @param params: Connection string parameters for MonetDB
        @return RETURN: Connection object
        """
        connect_params = self.params
        connect_params.update(ext_params)
        self.log.info(connect_params)
        if self.use_monet:
            return MonetConnection(**connect_params)
        else:
            return PgConnection(**connect_params)

    def save_params(self, filename):
        """
        Save connection params into a pickle-file.

        @param filename: name of the file to write to.
        """
        output = open(filename, 'wb')
        pickle.dump(self.params, output)
        output.close()

    def load_params(self, filename):
        """
        Read connection params from file.
        """
        pkl_file = open(filename, 'rb')
        self.params = pickle.load(pkl_file)
        pkl_file.close()
