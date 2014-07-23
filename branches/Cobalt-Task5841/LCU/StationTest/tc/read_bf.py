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

"""Testcase for reading the BF coefficients

  Note: No specific arguments, use general arguments -v, --brd, --fpga and --bm
"""

################################################################################
# Constants

nof_reflets_ap  = rsp.c_nof_reflets_ap    # = 4
nof_beamlets    = rsp.c_nof_beamlets      # = 248, maximum capable by RSP gateware
nof_beamlets_ap = rsp.c_nof_beamlets_ap   # = 252, including reflets

# - BF output size
c_bf_reflets_size = rsp.c_pol_phs * nof_reflets_ap
c_bf_size         = rsp.c_pol_phs * nof_beamlets_ap


################################################################################
# - Verify options
rspId   = tc.rspId
blpId   = tc.blpId
ppId    = tc.ppId
bmBanks = tc.bmBanks

tc.setResult('PASSED')   # self checking test, so start assuming it will run PASSED

tc.appendLog(11,'')
tc.appendLog(11,'>>> Read the BF coefficients for RSP-%s, BLP-%s, coeff-%s, BM banks-%s' % (rspId, blpId, ppId, bmBanks))
tc.appendLog(11,'')
  
################################################################################
# - Testcase initializations

# Typically the rspctl updates the BF every pps, so overwriting it does not work.

# Read the BF coefficient from the APs
for ri in rspId:
  for bi in blpId:
    for pp in ppId:
      for bk in bmBanks:
        bf_coef = rsp.read_bf(tc, msg, c_bf_size, [pp], [bi], [ri], [bk])
        bf_rlet_coef = bf_coef[:c_bf_reflets_size]
        bf_blet_coef = bf_coef[c_bf_reflets_size:]
        tc.appendLog(11,'>>> RSP-%s, BLP-%s BF reflets coefficients-%s, BM bank-%d (length %d).' % (ri, bi, pp, bk, len(bf_rlet_coef)))
        tc.appendLog(21,'%s' % bf_rlet_coef)
        tc.appendLog(11,'>>> RSP-%s, BLP-%s, BF beamlets coefficients-%s, BM bank-%d (length %d).' % (ri, bi, pp, bk, len(bf_blet_coef)))
        tc.appendLog(21,'%s' % bf_blet_coef)
