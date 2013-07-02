//# makefftwisdom.cc: Collect FFTW wisdom for 2D Complex-Complex transforms
//#
//# Copyright (C) 2011
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>
#include <LofarFT/FFTCMatrix.h>
#include <Common/lofar_iostream.h>
#include <casa/OS/Path.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>     //# for strerror

using namespace LOFAR;

void showhelp()
{
  cout << endl;
  cout << "makefftwisdom2d creates FFTW wisdom for the LOFAR imager." << endl;
  cout << "The result is written into a file read by class FFTCMatrix." << endl;
  cout << endl;
  cout << "The wisdom is collected by making plans (with FFTW_PATIENT)" << endl;
  cout << "for FFTs of various odd optimal sizes and various power of 2 sizes." 
       << endl;
  cout << "An optimal FFTW size is a size that can be written as" << endl;
  cout << "    2^a * 3^b * 5^c * 7^d * 11^e * 13^f   with e+f<=1" << endl;
  cout << "The LOFAR imager tries to use such sizes as much as possible when"
       << endl;
  cout << "calculating the convolution functions for the A-projection." << endl;
  cout << endl;
  cout << "Run as:" << endl;
  cout << "    makefftwisdom2d [filename] [maxsize]" << endl;
  cout << endl;
  cout << "        filename   The name of the output file" << endl;
  cout << "                   It defaults to fftwisdom2d.txt" << endl;
  cout << "        maxsize    The maximum optimal size to plan" << endl;
  cout << "                   It defaults to 10000" << endl;
  cout << endl;
}

int main (int argc, char* argv[])
{
  bool argparallel = false;
  int st = 1;
  if (argc > st) {
    casa::String arg(argv[st]);
    if (arg == "-h"  ||  arg == "--help"  ||  arg == "?") {
      showhelp();
      return 1;
    }
    if (arg == "-p"  ||  arg == "--parallel") {
      argparallel = true;
      st++;
    }
  }
  if (!argparallel) {

    // Serial FFTs.
    casa::String name = "fftwisdom2d.txt";
    if (argc > st) {
      name = casa::Path(argv[st]).absoluteName();
    }
    FILE* wisdomFile = fopen (name.c_str(), "w");
    if (!wisdomFile) {
      cerr << "makefftwisdom2d: could not create output file " << name << "; "
           << strerror(errno) << endl;
      return 1;
    }
    int maxsize = 10000;
    if (argc > 2) {
      istringstream iss(argv[2]);
      iss >> maxsize;
    }
    cout << "Creating 2D FFTW wisdom for optimal sizes between 1 and "
         << maxsize << " ..." << endl;
    FFTCMatrix fftmat;
    // Collect the wisdom for creating convolution functions in awimager.
    // They use odd sizes FFTs of optimal length as defined in FFTCMatrix.
    const int* sizes = FFTCMatrix::getOptimalOddFFTSizes();
    int nsizes = FFTCMatrix::nOptimalOddFFTSizes();
    for (int i=0; i<nsizes && sizes[i]<maxsize; ++i) {
      cerr << ' ' << sizes[i];
      fftmat.plan (sizes[i], true, FFTW_PATIENT);
      fftmat.plan (sizes[i], false, FFTW_PATIENT);
    }
    cerr << endl;
    cout << "Creating 2D FFTW wisdom for some multiples of 1024 (till 16384) ..."
         << endl;
    int otherSizes[] = {512, 1024};//, 2048, 4096, 6144, 8192, 10240, 12288, 16384};
    nsizes = sizeof(otherSizes) / sizeof(int);
    for (int i=0; i<2; ++i) {
      cerr << ' ' << otherSizes[i];
      fftmat.plan (otherSizes[i], true, FFTW_PATIENT);
      fftmat.plan (otherSizes[i], false, FFTW_PATIENT);
    }
    cerr << endl;

    cout << "Writing 2D FFTW wisdom into file '" << name << "'" << endl;
    fftwf_export_wisdom_to_file (wisdomFile);
    fclose (wisdomFile);
    return 0;

  } else {

    // Parallel FFTs
    casa::String name = "fftwisdom2d_parallel.txt";
    if (argc > st) {
      name = casa::Path(argv[st]).absoluteName();
    }
    FILE* wisdomFile = fopen (name.c_str(), "w");
    if (!wisdomFile) {
      cerr << "makefftwisdom2d: could not create output file " << name << "; "
           << strerror(errno) << endl;
      return 1;
    }
    int maxsize = 2000;
    // if (argc > 2) {
    //   istringstream iss(argv[2]);
    //   iss >> maxsize;
    // }
    cout << "Creating 2D FFTW wisdom for optimal sizes between 1 and "
         << maxsize << " ..." << endl;
    FFTCMatrix fftmat;
    // Collect the wisdom for creating convolution functions in awimager.
    // They use odd sizes FFTs of optimal length as defined in FFTCMatrix.
    fftwf_init_threads();

    fftwf_plan_with_nthreads (1);
    const int* sizes = FFTCMatrix::getOptimalOddFFTSizes();
    int nsizes = FFTCMatrix::nOptimalOddFFTSizes();
    // for (int i=0; i<nsizes && sizes[i]<maxsize; ++i) {
    //   cerr << ' ' << sizes[i];
    //   fftmat.plan (sizes[i], true, FFTW_PATIENT);
    //   fftmat.plan (sizes[i], false, FFTW_PATIENT);
    // }
    cerr << endl;
    cout << "Creating 2D FFTW wisdom for some multiples of 1024 (till 16384) ..."
         << endl;
  
    int otherSizes[] = { 8192};//, 9216, 10240};//, 11264, 12288, 13312};
    //nsizes = sizeof(otherSizes) / sizeof(int);
    fftwf_plan_with_nthreads (6);
    for (int i=0; i<1; ++i) {
      cerr << ' ' << otherSizes[i];
      fftmat.plan (otherSizes[i], true, FFTW_MEASURE);//FFTW_PATIENT);
      fftmat.plan (otherSizes[i], false, FFTW_MEASURE);//FFTW_PATIENT);
    }

    cout << "Writing 2D FFTW wisdom into file '" << name << "'" << endl;
    fftwf_export_wisdom_to_file (wisdomFile);
    fclose (wisdomFile);
    return 0;
    
  }
}
