//# mpidft.h:
//#
//# Copyright (C) 2004
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


#ifndef DFTSERVER_MPIDFT_H
#define DFTSERVER_MPIDFT_H

// Define functions for a distributed DFT calculation.

  double *makeDoubleArray1D (int len);
  double **makeDoubleArray2D (int height, int width);
  void destroyDoubleArray2D (double **arr);

  inline double mulre (double re1, double im1, double re2, double im2)
  {
    return re1*re2 - im1*im2;
  }
  inline double mulim (double re1, double im1, double re2, double im2)
  {
    return re2*im1 + re1*im2;
  }
  inline double mulcre (double re1, double im1, double re2, double im2)
  {
    return re1*re2 + im1*im2;
  }
  inline double mulcim (double re1, double im1, double re2, double im2)
  {
    return re2*im1 - re1*im2;
  }

// The out-ifdef-ed is about 10% slower than the other one.
  inline void mulreim (double re1, double im1, double re2, double im2,
		       double& re, double& im)
  {
#if 0
    re = mulre(re1,im1,re2,im2);
    im = mulim(re1,im1,re2,im2);
#else
    double x = re1*(re2+im2);
    double y = im2*(re1+im1);
    re = x-y;
    im = y + im1*(re2-im2);
#endif
  }

  inline void mulcreim (double re1, double im1, double re2, double im2,
			double& re, double& im)
  {
#if 0
    re = mulcre(re1,im1,re2,im2);
    im = mulcim(re1,im1,re2,im2);
#else
    double x = re1*(re2-im2);
    double y = im2*(re1+im1);
    re = x+y;
    im = im1*(re2+im2) - y;
#endif
  }



  void Master (int nstat, const int* ant,
	       int nt, int nf, int nint, int ninf,
	       double f0, double df,
	       double l, double m, double n, const double *uvw,
	       int nbaseline, const int* ant1, const int* ant2,
	       double* result);

  void Slave(int nt, int nf, int nint, int ninf, double f0, double df);
  
  void doDFTslave (void);
  
  void doDFTmaster (int nstat, const int* ant,
		    int nt, int nf, int nint, int ninf,
		    double f0, double df,
		    double l, double m, double n, const double *uvw,
		    int nbaseline, const int* ant1, const int* ant2,
		    double* result);


#endif
