//# MeqJonesMMap.cc: Get part of a mapped MS
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <lofar_config.h>
#include <BBS3/MNS/MeqJonesMMap.h>

namespace LOFAR {

  MeqJonesMMap::MeqJonesMMap (const MMapMSInfo& info, int offsetBL)
    : itsInfo     (&info),
      itsOffsetBL (offsetBL)
  {}

  MeqJonesMMap::~MeqJonesMMap()
  {}

  MeqJonesResult MeqJonesMMap::getJResult (const MeqRequest& request)
  {
    // Get data shape.
    int nrchan = request.nx();
    int nrtime = request.ny();
    int nrcorr = itsInfo->nrCorr();
    int tsize  = itsInfo->timeSize();
    // Channels might be in reversed order.
    int idch = itsInfo->reverseChan() ? nrcorr * (nrchan - 1) : 0;
    int inc  = itsInfo->reverseChan() ? -nrcorr : nrcorr;
    // Allocate the result matrices.
    MeqJonesResult result(request.nspid());
    MeqMatrix& m11 = result.result11().getValueRW();
    MeqMatrix& m12 = result.result12().getValueRW();
    MeqMatrix& m21 = result.result21().getValueRW();
    MeqMatrix& m22 = result.result22().getValueRW();
    m11.setDCMat (nrchan, nrtime);
    m12.setDCMat (nrchan, nrtime);
    m21.setDCMat (nrchan, nrtime);
    m22.setDCMat (nrchan, nrtime);
    // Determine which matrices to use.
    // Zero the ones that are not used.
    double* rl[4];
    double* im[4];
    m11.dcomplexStorage (rl[0], im[0]);
    if (nrcorr == 4) {
      m12.dcomplexStorage (rl[1], im[1]);
      m21.dcomplexStorage (rl[2], im[2]);
      m22.dcomplexStorage (rl[3], im[3]);
    } else {
      m12 = MeqMatrix (makedcomplex(0,0), nrchan, nrtime);
      m21 = MeqMatrix (makedcomplex(0,0), nrchan, nrtime);
      if (nrcorr == 2) {
	m22.dcomplexStorage (rl[1], im[1]);
      } else {
	m22 = MeqMatrix (makedcomplex(0,0), nrchan, nrtime);
      }
    }
    // Move all data from MS to matrices.
    const fcomplex* mdata = itsInfo->inData() +
                            itsInfo->timeOffset() + itsOffsetBL;
    for (int t=0; t<nrtime; ++t) {
      const fcomplex* data = mdata + t*tsize;
      for (int corr=0; corr<nrcorr; ++corr, ++data) {
	double* rlc = rl[corr] + t*nrchan;
	double* imc = im[corr] + t*nrchan;
	int dch = idch;
	for (int ch=0; ch<nrchan; ++ch, dch+=inc) {
	  rlc[ch] = real(data[dch]);
	  imc[ch] = imag(data[dch]);
	}
      }
    }
    return result;
  }

  void MeqJonesMMap::putJResult (const MeqJonesResult& result)
  {
    // Get the result matrices.
    const MeqMatrix& m11 = result.getResult11().getValue();
    const MeqMatrix& m12 = result.getResult12().getValue();
    const MeqMatrix& m21 = result.getResult21().getValue();
    const MeqMatrix& m22 = result.getResult22().getValue();
    // Get data shape.
    int nrchan = m11.nx();
    int nrtime = m11.ny();
    int nrcorr = itsInfo->nrCorr();
    long tsize  = itsInfo->timeSize();
    // Channels might be in reversed order.
    int idch = itsInfo->reverseChan() ? nrcorr * (nrchan - 1) : 0;
    int inc  = itsInfo->reverseChan() ? -nrcorr : nrcorr;
    // Determine which matrices to use.
    // Fill the pointers to their real and imaginary part.
    const double* rl[4];
    const double* im[4];
    m11.dcomplexStorage (rl[0], im[0]);
    if (nrcorr == 4) {
      m12.dcomplexStorage (rl[1], im[1]);
      m21.dcomplexStorage (rl[2], im[2]);
      m22.dcomplexStorage (rl[3], im[3]);
    } else {
      if (nrcorr == 2) {
	m22.dcomplexStorage (rl[1], im[1]);
      }
    }
    // Move all data from matrices to MS.
    fcomplex* mdata = itsInfo->outData() +
                      itsInfo->timeOffset() + itsOffsetBL;
    for (int t=0; t<nrtime; ++t) {
      fcomplex* data = mdata + t*tsize;
      for (int corr=0; corr<nrcorr; ++corr, ++data) {
	const double* rlc = rl[corr] + t*nrchan;
	const double* imc = im[corr] + t*nrchan;
	int dch = idch;
	for (int ch=0; ch<nrchan; ++ch, dch+=inc) {
	  data[dch] = makefcomplex(rlc[ch], imc[ch]);
	}
      }
    }
  }
}
