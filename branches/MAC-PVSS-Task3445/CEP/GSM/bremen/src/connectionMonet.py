#!/usr/bin/python
"""
Database connection with logging.
Overrides MonetDB connection object.
"""
import monetdb.sql as db
from src.unifiedConnection import UnifiedConnection


class MonetConnection(UnifiedConnection):
    """
    Connection with logging.
    Overrides MonetDB connection object.
    """
    def __init__(self, **params):
        super(MonetConnection, self).__init__()
        self.conn = db.connections.Connection(**params)
        self.in_transaction = False

