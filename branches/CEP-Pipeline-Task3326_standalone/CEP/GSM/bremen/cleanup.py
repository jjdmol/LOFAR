#!/usr/bin/python
"""
***GSM package tool.
***Created by A. Mints (2012).
    Clear all data from the database.
"""

import argparse
from src.gsmconnectionmanager import GSMConnectionManager
from tests.testlib import cleanup_db

parser = argparse.ArgumentParser(description="""
***GSM package tool.
***Created by A. Mints (2012).
    Clear all data from the database.""",
formatter_class=argparse.RawDescriptionHelpFormatter)

parser.add_argument('-D', '--database', type=str, default='test',
                    help='database name to load data into')
parser.add_argument('-M', '--monetdb', action="store_true", default=False,
                    help='Use MonetDB instead of PostgreSQL')
args = parser.parse_args()

cm = GSMConnectionManager(use_monet=args.monetdb, database=args.database)
cleanup_db(cm.get_connection())
