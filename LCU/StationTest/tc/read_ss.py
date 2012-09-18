################################################################################
#
# Copyright (C) 2012
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

"""Testcase for reading the SS map

  Note: No specific arguments, use general arguments -v, --brd, --fpga and --bm
"""

################################################################################
# Constants

nof_reflets_ap  = rsp.c_nof_reflets_ap    # = 4
nof_beamlets    = rsp.c_nof_beamlets      # = 248, maximum capable by RSP gateware
nof_beamlets_ap = rsp.c_nof_beamlets_ap   # = 252, including reflets

# - SS output size
c_ss_reflets_size = rsp.c_pol * nof_reflets_ap
c_ss_size         = rsp.c_pol * nof_beamlets_ap

c_ss_gap          = rsp.c_slice_size - rsp.c_cpx * rsp.c_pol * nof_beamlets_ap


################################################################################
# - Verify options
rspId   = tc.rspId
blpId   = tc.blpId
bmBanks = tc.bmBanks

repeat = tc.repeat
tc.setResult('PASSED')   # self checking test, so start assuming it will run PASSED

tc.appendLog(11,'')
tc.appendLog(11,'>>> Read the SS map for RSP-%s, BLP-%s, BM banks-' % (rspId, blpId, bmBanks))
tc.appendLog(11,'')
  
################################################################################
# - Testcase initializations

# Apparently rspctl updates the SS every pps, so overwriting it does not work.
# Disabling SS update in RSPDriver.conf may be an option.

# Read the SS mapping from the APs
for ri in rspId:
  for bi in blpId:
    for bk in bmBanks:
      ss_map = rsp.read_ss(tc, msg, c_ss_size, [bi], [ri], [bk])
      ss_rlet_map = ss_map[:c_ss_reflets_size]
      ss_blet_map = ss_map[c_ss_reflets_size:]
      tc.appendLog(11,'>>> RSP-%s, BLP-%s, BM-%d SS reflets map (length %d).' % (ri, bi, bk, len(ss_rlet_map)))
      tc.appendLog(21,'%s' % ss_rlet_map)
      tc.appendLog(11,'>>> RSP-%s, BLP-%s, BM-%d SS beamlets map (length %d).' % (ri, bi, bk, len(ss_blet_map)))
      tc.appendLog(21,'%s' % ss_blet_map)
