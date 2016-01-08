#!/usr/bin/python
"""
Database connection with logging.
Overrides MonetDB connection object.
"""
import monetdb.sql as db
from src.unifiedConnection import UnifiedConnection
from monetdb.mapi import logger as mapi_logger
import logging


class MonetConnection(UnifiedConnection):
    """
    Connection with logging.
    Overrides MonetDB connection object.
    """
    def __init__(self, **params):
        super(MonetConnection, self).__init__()
        self.conn = db.connections.Connection(**params)
        self.in_transaction = False
        mapi_logger.setLevel(logging.INFO)

