#!/usr/bin/python
"""
Tool to run GSM pipeline for a given parset.
Multiple parsets can be listed.
Usage: pipeline_runner_test.py [-h] [-D DATABASE] [-M] [-p] [-q]
                               [filename [filename ...]]

***GSM package tool.
***Created by A. Mints (2012).
    Pass several parsets to pipeline.

positional arguments:
  filename              list of parset file names

optional arguments:
  -h, --help            show this help message and exit
  -D DATABASE, --database DATABASE
                        database name to load data into
  -M, --monetdb         database name to load data into
  -p, --profile         add SQL timing output to log
  -q, --quiet           switch console logging off
"""

import time
import argparse
import logging
from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
from src.gsmlogger import switch_console, set_all_levels
from src.errors import GSMException

def run_pipeline(database, filenames, use_monet=True, profile=False, quiet=False):
    def run_parset(pipeline, parname):
        try:
            print parname,
            start = time.time()
            parset = GSMParset(parname)
            set_all_levels(logging.DEBUG)
            pipeline.run_parset(parset)
            print 'Time spent: %s' % (time.time() - start)
            return True
        except GSMException, e:
            print 'ERROR occured: %s' % e
            return False

    pipeline = GSMPipeline(database=database,
                           use_monet=use_monet,
                           profile=profile)

    if quiet:
        switch_console(False)

    for parname in filenames:
        ok = run_parset(pipeline, parname)
        if not ok:
            pipeline.reopen_connection()

parser = argparse.ArgumentParser(description="""
***GSM package tool.
***Created by A. Mints (2012).
    Pass several parsets to pipeline.""",
formatter_class=argparse.RawDescriptionHelpFormatter)

parser.add_argument('-D', '--database', type=str, default='test',
                    help='database name to load data into')
parser.add_argument('-M', '--monetdb', action="store_true", default=False,
                    help='database name to load data into')
parser.add_argument('-p', '--profile', action="store_true", default=False,
                    help='add SQL timing output to log')
parser.add_argument('-q', '--quiet', action="store_true", default=False,
                    help='switch console logging off')
parser.add_argument('filename', type=str, nargs='*',
                    help='list of parset file names')
args = parser.parse_args()

try:
    run_pipeline(database=args.database,
                 use_monet=args.monetdb,
                 profile=args.profile,
                 quiet=args.quiet,
                 filenames=args.filename)
except Error as e:
    print 'Unexpected error: %s' % e
