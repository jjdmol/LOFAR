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
  -M, --monetdb         use MonetDB instead of PostgreSQL
  -p, --profile         add SQL timing output to log
  -q, --quiet           switch console logging off
"""

import time
import argparse
import logging
import traceback

from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
from src.gsmlogger import switch_console, set_all_levels
from src.errors import GSMException


def run_pipeline(database, filenames, use_monet=True, 
                 profile=False, quiet=False, verbose=False, loglevel='DEBUG'):
    """
    Run a pipeline for a set of parsets.
    @param database: database name;
    @param filenames: array of parset filenames;
    @param use_monet: if True run on MonetDB, otherwise - PostgreSQL;
    @param profile: use profiler;
    @param quiet: suppress warnings and log messages;
    """
    def run_parset(pipeline, parname):
        """
        Run single parset
        @param pipeline: pipeline object to use;
        @param parname: name of the parset to run
        """
        try:
            print parname,
            start = time.time()
            parset = GSMParset(parname)
            pipeline.run_parset(parset)
            print 'Time spent: %s seconds' % (time.time() - start)
            return True
        except GSMException, exc:
            print 'ERROR occured: %s' % exc
            traceback.print_exc()
            return False

    pipeline = GSMPipeline(database=database,
                           use_monet=use_monet,
                           profile=profile)
    if quiet:
        switch_console(False)
    if verbose:
        switch_console(True)

    set_all_levels(loglevel.upper())
    
    for parname in filenames:
        ok = run_parset(pipeline, parname)
        if not ok:
            pipeline.reopen_connection()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="""
    ***GSM package tool.
    ***Created by A. Mints (2012).
        Pass several parsets to pipeline.""",
    formatter_class=argparse.RawDescriptionHelpFormatter)
    
    parser.add_argument('-D', '--database', type=str, default='test',
                        help='database name to load data into')
    parser.add_argument('-M', '--monetdb', action="store_true", default=False,
                        help='use MonetDB instead of PostgreSQL')
    parser.add_argument('-p', '--profile', action="store_true", default=False,
                        help='add SQL timing output to log')
    parser.add_argument('-Q', '--quiet', action="store_true", default=False,
                        help='switch console logging off')
    parser.add_argument('-V', '--verbose', action="store_true", default=False,
                        help='switch console logging on')
    parser.add_argument('-l', '--loglevel', type=str, default='DEBUG',
                        help='set logging level')
    parser.add_argument('filename', type=str, nargs='*',
                        help='list of parset file names')
    args = parser.parse_args()
    
    try:
        run_pipeline(database=args.database,
                     use_monet=args.monetdb,
                     profile=args.profile,
                     quiet=args.quiet,
                     verbose=args.verbose,
                     loglevel=args.loglevel,
                     filenames=args.filename)
    except Exception as exc:
        print 'Unexpected error: %s' % exc
        traceback.print_exc()
    
