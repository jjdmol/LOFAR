#!/usr/bin/python
"""
Tool to run GSM pipeline for a given parset.
Multiple parsets can be listed.
"""
import time
import argparse
from src.gsmparset import GSMParset
from src.pipeline import GSMPipeline
from src.gsmlogger import switch_console

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

if args.quiet:
    switch_console(False)

pipeline = GSMPipeline(database=args.database,
                       use_monet=args.monetdb,
                       profile=args.profile)

for parname in args.filename:
    print parname
    start = time.time()
    parset = GSMParset(parname)
    pipeline.run_parset(parset)
    print 'Time spent: %s' % (time.time() - start)
