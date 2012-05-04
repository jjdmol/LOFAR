#!/usr/bin/python
import cPickle as pickle
from src.gsmlogger import get_gsm_logger
from src.connectionMonet import MonetLoggedConnection
from src.connectionPostgres import PgConnection


#LoggedConnection = MonetLoggedConnection
LoggedConnection = PgConnection


class GSMConnectionManager(object):
    """
    General connection manager.
    Have options to save/load connection parameters.
    """
    DEFAULTS = {
      'hostname': 'localhost',
      'port': 50000,
      'database': 'gsm',
      'username': 'monetdb',
      'password': 'monetdb',
      'autocommit': True
    }

    def __init__(self, **params):
        """
        """
        self.params = self.DEFAULTS.copy()
        self.params.update(params)
        self.log = get_gsm_logger('sql', 'sql.log')

    def get_connection(self, **ext_params):
        """
        Get a connection with a given params.
        @param params: Connection string parameters for MonetDB
        @return RETURN: Connection object
        """
        connect_params = self.params
        connect_params.update(ext_params)
        self.log.info(connect_params)
        return LoggedConnection(**connect_params)

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
