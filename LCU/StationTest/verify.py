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
verify.add_option('--pid', type='string', dest='pid',
  help='Process ID: rsp, eth, mep, diag, bs, rcuh, rsu, ado, rad, all', default='all')
verify.add_option('--data', type='string', dest='data',
  help='Data values(s) to write or verify read', default='40')
verify.add_option('--count', action='store_true', dest='count',
  help='Use counter data values')
verify.add_option('--rand', action='store_true', dest='rand',
  help='Use random data values')
verify.add_option('--read', action='store_true', dest='read',
  help='Run the testcase read only')
verify.add_option('--pps_edge', type='string', dest='pps_edge',
  help='Capture PPS on rising or falling clock edge: r, f', default='r')
verify.add_option('--pps_delay', type='int', dest='pps_delay',
  help='Increment PPS input delay in steps of about 75 ps, use 0 for reset', default=1)
verify.add_option('--diag_mode', type='string', dest='diag_mode',
  help='Diag mode: off, tx, rx, tx_rx, local', default='tx_rx')
verify.add_option('--diag_sync', type='int', dest='diag_sync',
  help='Diag sync: 0 for pps, > 0 for alt sync interval in s', default=0)
verify.add_option('--diag_data', type='string', dest='diag_data',
  help='Diag data: lfsr, cntr', default='lfsr')
verify.add_option('--rad_lane_mode', type='string', dest='rad_lane_mode',
  help='RAD lane mode: X lane 3:0, B lane 3:0, where 0=local, 1=disable, 2=combine, 3=remote', default='0,0,0,0,0,0,0,0')
verify.add_option('--client_rcu', type='string', dest='client_rcu',
  help='HBA control via this RCU, power via the other RCU: x or y', default='y')
verify.add_option('--client_access', type='string', dest='client_access',
  help='HBA client access: r = read only, w = write only, wr = first write then readback', default='r')
verify.add_option('--client_reg', type='string', dest='client_reg',
  help='HBA client register: request, response, led, vref, version, speed', default='led')
verify.add_option('--server', type='string', dest='server',
  help='HBA server range, first server and last server', default='1,16')
verify.add_option('--server_access', type='string', dest='server_access',
  help='HBA server access: bc = broadcast to all servers, uc = unicast to first server', default='uc')
verify.add_option('--server_function', type='string', dest='server_function',
  help='HBA server function: gb, gw, sb, sw', default='gb')
verify.add_option('--server_reg', type='string', dest='server_reg',
  help='HBA server register: delay_x, delay_y, version, address', default='delay_x')

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

arg_procid              = v.opts.pid
data_str                = v.opts.data.split(',')
arg_data = []
for di in data_str:
  arg_data.append(int(di))
arg_count               = v.opts.count
arg_rand                = v.opts.rand
arg_read                = v.opts.read
arg_pps_edge            = v.opts.pps_edge
arg_pps_delay           = v.opts.pps_delay
arg_diag_mode           = v.opts.diag_mode
arg_diag_sync           = v.opts.diag_sync
arg_diag_data           = v.opts.diag_data
rad_lane_mode_str       = v.opts.rad_lane_mode.split(',')
arg_rad_lane_mode = []
for lm in rad_lane_mode_str:
  arg_rad_lane_mode.append(int(lm))
arg_hba_client_rcu      = v.opts.client_rcu
arg_hba_client_access   = v.opts.client_access
arg_hba_client_reg      = v.opts.client_reg
server_str              = v.opts.server.split(',')
arg_hba_server = []
for si in server_str:
  arg_hba_server.append(int(si))
arg_hba_server_access   = v.opts.server_access
arg_hba_server_function = v.opts.server_function
arg_hba_server_reg      = v.opts.server_reg


################################################################################
# Run the testcase

# Import here so no need in Testcase
import random
import mep
import testcase
import rsp
import smbus

msg = mep.MepMessage()
  
for te in v.testname:
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
  dt = tc.getRunTime()
  tc.appendLog(2,'Duration: %d %02d:%02d:%02d' % (dt/60/60/24, dt/60/60 % 24, dt/60 % 60, dt % 60))
  tc.appendLog(0,tc.getResult())
  tc.closeLog()
