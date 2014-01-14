//# cmpfloat.cc: compare floating point values between two binary files
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

//#include <lofar_config.h>

#include <cstdlib>
#include <complex>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <limits>
#include <boost/lexical_cast.hpp>

#include "fpequals.h"

namespace {

using namespace std;
using LOFAR::Cobalt::fpEquals;

struct Args {
  string filename1;
  string filename2;
  enum type {FLOAT, DOUBLE, CFLOAT, CDOUBLE} type;
  size_t skip;
  size_t nvals;
  double epsilon;
  bool verbose;
};

void pr_usage(const char* progname) {
  cerr << "Usage: " << progname << " [--type=float|double|cfloat|cdouble]"
                                   " [--skip=bytes] [--size=nvals]"
                                   " [--epsilon=fpval] [--verbose] file1 file2" << endl;
  cerr << "  --type     interpret input data as array of specified type" << endl;
  cerr << "             cfloat means complex float. Default: double" << endl;
  cerr << "  --skip     number of bytes to skip before comparison starts. Default: 0" << endl;
  cerr << "  --size     must compare number of values of type. Default: until EOF" << endl;
  cerr << "  --epsilon  maximum absolute difference tolerance. Default: std::numeric_limits<T>::epsilon()" << endl;
  cerr << "  --verbose  print some info to stdout, regardless of exit status" << endl;
}

bool parseArgs(int argc, char *argv[], Args &args) {
  // defaults
  args.type = args.DOUBLE;
  args.skip = 0;
  args.nvals = std::numeric_limits<size_t>::max();
  args.epsilon = std::numeric_limits<double>::epsilon();
  args.verbose = false;

  const string typePrefix("--type=");
  const string skipPrefix("--skip=");
  const string sizePrefix("--size=");
  const string epsilonPrefix("--epsilon=");
  const string verbosePrefix("--verbose");

  bool ok = true;
  bool epsSet = false;
  unsigned nfiles = 0;

  for (int i = 1; i < argc; i++) {
    string opt(argv[i]);
    string val;

    if (opt.compare(0, typePrefix.size(), typePrefix) == 0) {
      val = opt.erase(0, typePrefix.size());

      if (val == "float") {
        args.type = args.FLOAT;
        if (!epsSet)
          args.epsilon = std::numeric_limits<float>::epsilon();
      } else if (val == "double") {
        args.type = args.DOUBLE;
        if (!epsSet)
          args.epsilon = std::numeric_limits<double>::epsilon();
      } else if (val == "cfloat") {
        args.type = args.CFLOAT;
        if (!epsSet)
          args.epsilon = std::numeric_limits<float>::epsilon();
      } else if (val == "cdouble") {
        args.type = args.CDOUBLE;
        if (!epsSet)
          args.epsilon = std::numeric_limits<double>::epsilon();
      } else {
        cerr << "Error: invalid value in --type argument: " << val << endl;
        ok = false;
      }
    } else if (opt.compare(0, skipPrefix.size(), skipPrefix) == 0) {
      val = opt.erase(0, skipPrefix.size());
      try {
        args.skip = boost::lexical_cast<size_t>(val);
        if ((ssize_t)args.skip < 0)
          throw boost::bad_lexical_cast();
      } catch (boost::bad_lexical_cast& exc) {
        cerr << "Error: invalid value in --skip argument: " << val << endl;
        ok = false;
      }
    } else if (opt.compare(0, sizePrefix.size(), sizePrefix) == 0) {
      val = opt.erase(0, sizePrefix.size());
      try {
        args.nvals = boost::lexical_cast<size_t>(val);
        if ((ssize_t)args.nvals < 0)
          throw boost::bad_lexical_cast();
      } catch (boost::bad_lexical_cast& exc) {
        cerr << "Error: invalid value in --size argument: " << val << endl;
        ok = false;
      }
    } else if (opt.compare(0, epsilonPrefix.size(), epsilonPrefix) == 0) {
      val = opt.erase(0, epsilonPrefix.size());
      try {
        args.epsilon = boost::lexical_cast<double>(val);
        args.epsilon = std::abs(args.epsilon);
        epsSet = true;
      } catch (boost::bad_lexical_cast& exc) {
        cerr << "Error: invalid value in --epsilon argument: " << val << endl;
        ok = false;
      }
    } else if (opt == verbosePrefix) {
      args.verbose = true;
    } else { // filename
      if (nfiles == 0) {
        args.filename1 = opt;
      } else if (nfiles == 1) {
        args.filename2 = opt;
      }
      nfiles += 1;
    }
  }

  if (nfiles != 2) {
    cerr << "Error: need 2 file arguments, got " << nfiles << endl;
    ok = false;
  }

  return ok;
}

template <typename T>
bool compareValues(T v1, T v2, double epsilon, size_t pos,
                   T& maxFactor, T& minFactor) {
  if (!fpEquals(v1, v2, (T)epsilon)) {
    cerr << "Error: value diff beyond epsilon at compared value " << pos << ": "
         << v1 << " " << v2 << endl;

    T factor = v2 / v1; // inf is fine, NaN if eps was set to 0 or odd data
    if (maxFactor == T(1.0)) {
      // first unequal val, so 1.0 must be as initialized (not a factor)
      maxFactor = minFactor = factor;
    } else if (factor > maxFactor) {
      maxFactor = factor;
    } else if (factor < minFactor) {
      minFactor = factor;
    }

    return false;
  }

  return true;
}

// Note the plural form of the complex identifiers: both factors are in the cval
template <typename T>
bool compareValues(complex<T> v1, complex<T> v2, double epsilon, size_t pos,
                   complex<T>& maxFactors, complex<T>& minFactors) {
  if (!fpEquals(v1, v2, (T)epsilon)) {
    cerr << "Error: value diff beyond epsilon at compared value " << pos << ": "
         << v1 << " " << v2 << endl;

    T realFactor = v2.real() / v1.real(); // idem as above
    T imagFactor = v2.imag() / v1.imag(); // idem
    if (maxFactors == T(1.0)) {
      // first unequal val, so 1.0 must be as initialized (not a factor)
      maxFactors.real() = minFactors.real() = realFactor;
      maxFactors.imag() = minFactors.imag() = imagFactor;
    } else {
      if (realFactor > maxFactors.real()) {
        maxFactors.real(realFactor);
      } else if (realFactor < minFactors.real()) {
        minFactors.real(realFactor);
      }
      if (imagFactor > maxFactors.imag()) {
        maxFactors.imag(imagFactor);
      } else if (realFactor < minFactors.imag()) {
        minFactors.imag(imagFactor);
      }
    }

    return false;
  }

  return true;
}

template <typename T>
void printCommonFactorMessage(const T& maxFactor, const T& minFactor) {
  // If maxFactor and minFactor are near, then the diff is probably scale only.
  const T facEps = (T)1e-1; // very loose as values can be of any magnitude (but too loose if no scale, yet min and max < 1e-1. The program then still exits non-zero, but possibly with the wrong message.)
  bool eq = fpEquals(maxFactor, minFactor, facEps);
  T avgFac;
  if (eq) {
    avgFac = (T)0.5 * (maxFactor + minFactor);
    cerr << "All errors of vals for this pair of files are within "
         << facEps << " to a factor " << avgFac << " (inverse="
         << (T)1.0 / avgFac << ')' << endl;
  } else
    cerr << "No clear common factor among all errors: maxFactor=" <<
            maxFactor << "; minFactor=" << minFactor << endl;
}

// Note the plural form of the complex identifiers: both factors are in the cval
template <typename T>
void printCommonFactorMessage(const complex<T>& maxFactors,
                              const complex<T>& minFactors) {
  // If maxFactor and minFactor are near, then the diff is probably scale only.
  // For complex types, also see if real and imag are *-1 of each other (conj).
  const T facEps = (T)1e-1; // very loose as values can be of any magnitude
  bool realEq = fpEquals(maxFactors.real(), minFactors.real(), facEps);
  bool imagEq = fpEquals(maxFactors.imag(), minFactors.imag(), facEps);
  T avgRealFac, avgImagFac;
  if (realEq) {
    avgRealFac = (T)0.5 * (maxFactors.real() + minFactors.real());
    cerr << "All errors of real vals for this pair of files are within "
         << facEps << " to a factor " << avgRealFac << " (inverse="
         << (T)1.0 / avgRealFac << ')' << endl;
  }
  if (imagEq) {
    avgImagFac = (T)0.5 * (maxFactors.imag() + minFactors.imag());
    cerr << "All errors of imag vals for this pair of files are within "
         << facEps << " to a factor " << avgImagFac << " (inverse="
         << (T)1.0 / avgImagFac << ')' << endl;
  }
  if (realEq && imagEq && fpEquals(avgRealFac, -avgImagFac, facEps))
      cerr << "Common real and imag factors appear to (also) differ roughly by "
           << "a factor -1.0 (likely conjugation error)" << endl;
  if (!(realEq && imagEq))
    cerr << "No clear common factor among all errors: maxFactors="
         << maxFactors << "; minFactors=" << minFactors << endl;
}

template <typename T>
bool compareStreams(ifstream& ifs1, ifstream& ifs2, size_t skipped,
             size_t nvals, double epsilon, bool verbose) {
  bool ok = true;

  cerr.precision(17); // print full double precision on errors
  T maxFactor = T(1.0); // cmp needs init; 1.0 is not a valid unequality factor
  T minFactor = T(1.0); // symmetry / good practice, but not used

  size_t i;
  for (i = 0; i < nvals; i++) {
    T v1, v2;
    ifs1.read(reinterpret_cast<char *>(&v1), sizeof(T));
    ifs2.read(reinterpret_cast<char *>(&v2), sizeof(T));

    // Simultaneous EOF is ok iff nvals wasn't set as prog arg (default is max).
    bool eof1 = ifs1.eof();
    bool eof2 = ifs2.eof();
    if (eof1 && eof2 && nvals == std::numeric_limits<size_t>::max()) {
      break;
    } else if (eof1 || eof2) {
      cerr << "Error: Unexpected EOF in (at least) one stream after comparing "
           << i << " values" << endl;
      ok = false;
      break;
    }

    size_t nread1 = ifs1.gcount();
    size_t nread2 = ifs2.gcount();
    if (nread1 != nread2 || nread1 < sizeof(T)) {
      cerr << "Failed to read enough data from both streams for another "
           << "comparison after comparing " << i << " values" << endl;
      ok = false;
      break;
    }

    ok &= compareValues(v1, v2, epsilon, i, maxFactor, minFactor);
  }

  if (verbose)
    cout << "Compared " << i << " values (after skipping " << skipped
         << " bytes)" << endl;

  if (!ok && i > 0)
    printCommonFactorMessage(maxFactor, minFactor);

  return ok;
}

} // anon namespace

int main(int argc, char *argv[]) {
  Args args;
  if (!parseArgs(argc, argv, args)) {
    cerr << endl;
    pr_usage(argv[0]);
    return 2;
  }

  // open files
  ifstream ifs1(args.filename1.c_str(), std::ios::binary);
  if (!ifs1) {
    cerr << "Failed to open file " << args.filename1 << endl;
    return 2;
  }
  ifstream ifs2(args.filename2.c_str(), std::ios::binary);
  if (!ifs2) {
    cerr << "Failed to open file " << args.filename2 << endl;
    return 2;
  }

  // skip bytes (e.g. file header)
  // Don't check, as it turns out that this does not fail if skip > file size.
  ifs1.seekg(args.skip);
  ifs2.seekg(args.skip);

  // compare
  if (args.verbose)
    cout << "Comparing using an epsilon of " << args.epsilon << endl;
  bool cmpOk;
  if (args.type == args.FLOAT)
    cmpOk = compareStreams<float>(ifs1, ifs2, args.skip, args.nvals,
                                  args.epsilon, args.verbose);
  else if (args.type == args.DOUBLE)
    cmpOk = compareStreams<double>(ifs1, ifs2, args.skip, args.nvals,
                                   args.epsilon, args.verbose);
  else if (args.type == args.CFLOAT)
    cmpOk = compareStreams<complex<float> >(ifs1, ifs2, args.skip, args.nvals,
                                            args.epsilon, args.verbose);
  else if (args.type == args.CDOUBLE)
    cmpOk = compareStreams<complex<double> >(ifs1, ifs2, args.skip, args.nvals,
                                             args.epsilon, args.verbose);
  else {
      cerr << "Internal error: unknown data type" << endl;
      return 2;
  }

  return cmpOk ? 0 : 1;
}

