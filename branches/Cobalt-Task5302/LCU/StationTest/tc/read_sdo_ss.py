################################################################################
#
# Copyright (C) 2013
# ASTRON (Netherlands Institute for Radio Astronomy) <http://www.astron.nl/>
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
################################################################################

"""Testcase for reading the SDO SS map

  Note: No specific arguments, use general arguments -v, --brd, --fpga and --sm
"""

################################################################################
# Constants

# - SS output size
c_ss_size = rsp.c_sdo_ss_map_size
c_ss_gap  = rsp.c_slice_size - rsp.c_cpx * c_ss_size


################################################################################
# - Verify options
rspId   = tc.rspId
blpId   = tc.blpId
banks   = tc.sdoBanks

tc.setResult('PASSED')   # self checking test, so start assuming it will run PASSED

tc.appendLog(11,'')
tc.appendLog(11,'>>> Read the SDO SS map for RSP-%s, BLP-%s, banks-%s' % (rspId, blpId, banks))
tc.appendLog(11,'')
  
################################################################################
# - Testcase initializations

# Read the SDO SS mapping from the APs
for ri in rspId:
  for bi in blpId:
    for bk in banks:
      ss_map = rsp.read_sdo_ss(tc, msg, c_ss_size, [bi], [ri], [bk])
      tc.appendLog(11,'>>> RSP-%s, BLP-%s, SDO bank-%d SS map (length %d)' % (ri, bi, bk, len(ss_map)))
      tc.appendLog(21,'%s' % ss_map)
