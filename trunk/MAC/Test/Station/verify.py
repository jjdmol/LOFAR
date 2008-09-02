"""Master script to execute a LOFAR station testcase script

  Features:
  - Imports several modules
  - Provides argument parsing using optparse
  - Instantiates a Testcase class for logging
  - Instantiates a MepMessage class for using rspctl --writeblock, --readblock
  - Executes one or more testcase scripts (all use the same arguments)
  
  Notes:
  - The structure resembles the TCL/C testcase suite used for the station
    gateware (VHDL) development.
  - Most TCL testcase will work directly if they are translated to Python. The
    python scripts ougth to use high level rspctl commands wherever possible.
  - The TCL suite only works under Windows, because it uses WinPCap for ethernet
    access. This Python suite only works under Linux, because it uses rspctl.
"""

################################################################################
# Parse command line for testcase and options

import sys
from optparse import OptionParser

# Define empty class to be used as record, to keep local verify variables less
# easily known in the testcase scripts.
class v:
  pass           
  
verify = OptionParser(usage='usage: python %prog [options]', version='%prog 0.1')

# - Common options
verify.add_option('-v', type='int', dest='verbosity',
  help='Verbosity level for the log file', default=11)
verify.add_option('--te', type='string', dest='testname',
  help='File names of one or more testcases')
  # Multiple testcases can be run using comma seperator, each with the same options though
verify.add_option('--rep', type='int', dest='repeat',
  help='Repeat the test', default=1)
verify.add_option('--brd', type='string', dest='brdId',
  help='Board id: rsp0,rsp1 for RSP 0 and 1, tbb0 for TBB 0', default='rsp0')
  # Note multiple values for an option are possible by providing them with comma seperator and no spaces
verify.add_option('--fpga', type='string', dest='fpId',
  help='FPGA id: rsp for BP, blp0 for AP0, tbb for TP, mp0 for MP0', default='rsp')
  # On RSP and BLP is equivalent to an AP, but generaly an AP could implement multiple BLP
verify.add_option('--pol', type='string', dest='polId',
  help='Polarization id: x, y or x,y', default='x,y')
  
# - Testcase specific options
#   Define the testcase specific options here, rather than passing an --args
#   string to the testcase. The advantage is that they all show up with --help.
#   The disadvantage is that for every new options also this verify.py needs to
#   be updated.
verify.add_option('--read', action='store_true', dest='read',
  help='Run the testcase read only')
verify.add_option('--pid', type='string', dest='pid',
  help='Process ID: rsp, eth, mep, diag, bs, rcuh, rsu, ado, rad, all', default='all')
verify.add_option('--mode', type='string', dest='diag_mode',
  help='Diag mode: off, tx, rx, tx_rx, local', default='local')
verify.add_option('--sync', type='int', dest='diag_sync',
  help='Diag sync: 0 for pps, > 0 for alt sync interval in s', default=1)
verify.add_option('--data', type='string', dest='diag_data',
  help='Diag data: lfsr, cntr', default='lfsr')
  
v.opts, v.args = verify.parse_args()

# - Option checks and/or reformatting
if v.opts.testname==None:
  verify.error('Option --te must specify a testcase file name')
else:
  v.testname = v.opts.testname.split(',')

v.strId = v.opts.brdId.split(',')
v.rspId = []
v.tbbId = []
for brd in v.strId:
  if brd[:3] == 'rsp':
    v.rspId.append(brd)
  elif brd[:3] == 'tbb':
    v.tbbId.append(brd)
  else:
    verify.error('Option --brd has invalid board id %s' % brd)

v.strId = v.opts.fpId.split(',')
v.bpId = []  # RSP
v.blpId = []
v.tpId = []  # TBB
v.mpId = []
for fp in v.strId:
  if fp == 'rsp':
    v.bpId.append(fp)
  elif fp[:3] == 'blp':
    v.blpId.append(fp)
  elif fp == 'tbb':
    v.tpId.append(fp)
  elif fp[:2] == 'mp':
    v.mpId.append(fp)
  else:
    verify.error('Option --fp has invalid FPGA id %s' % fp)

v.polId = v.opts.polId.split(',')

# Pass the testcase specific options on directly, to avoid having to edit
# testcase.py for every new option. Rename with prefix arg_ so it is easier
# to search for the specific arguments, e.g. with grep or an editor.

arg_read   = v.opts.read
arg_procid = v.opts.pid
arg_mode   = v.opts.diag_mode
arg_sync   = v.opts.diag_sync
arg_data   = v.opts.diag_data


################################################################################
# Run the testcase

# Import here so no need in Testcase
import time
import mep
import testcase
import rsp

msg = mep.MepMessage()
  
for te in v.testname:
  startTime = time.time()
  # Pass the common options on via the testcase class instance.
  tc = testcase.Testcase(v.opts.verbosity,
                         te,
                         v.opts.repeat,
                         v.rspId, v.bpId, v.blpId,
                         v.tbbId, v.tpId, v.mpId,
                         v.polId)
  tc.appendLog(2,'--------------------------------------------------------------------------------')
  tc.setResult('RUNONLY')
  execfile(tc.testName)
  endTime = time.time()
  dt = int(endTime - startTime)
  tc.appendLog(2,'Duration: %d %02d:%02d:%02d' % (dt/60/60/24, dt/60/60 % 24, dt/60 % 60, dt % 60))
  tc.appendLog(0,tc.getResult())
  tc.closeLog()
