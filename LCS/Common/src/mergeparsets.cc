//# merge_parsets.cc: Merge one or more parset files into one.
//#
//# Copyright (C) 2014
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
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>

using namespace LOFAR;
using namespace std;

struct Arguments 
{
  bool case_insensitive;
  string output_file;
  vector<string> input_files;
};

void usage(const char* progname)
{
  cerr << "Usage: " << progname << " [-i] [-o outfile] infile1 [infile2 ...]"
       << "\nOptions:"
       << "\n  -i            treat parset keys case-insensitive"
       << "\n  -o outfile    write output to file 'outfile'"
       << endl;
}

void print(const Arguments& args)
{
  cout << "args.case_insensitive = " << args.case_insensitive << endl;
  cout << "args.output_file = " << args.output_file << endl;
  cout << "args.input_files = [";
  for (size_t i = 0; i < args.input_files.size(); i++) {
    if (i > 0) cout << ", ";
    cout << args.input_files[i];
  }
  cout << "]" << endl;
}

bool parse_arguments(int argc, char* argv[], Arguments& args)
{
  args.case_insensitive = false;
  int opt;
  bool error = false;
  while ((opt = getopt(argc, argv, "io:")) != -1) {
    switch (opt) {
    case 'i':
      args.case_insensitive = true;
      break;
    case 'o':
      args.output_file = optarg;
      break;
    default:
      error = true;
      break;
    }
  }
  for (int i = optind; i < argc; i++) {
    args.input_files.push_back(argv[i]);
  }
  if (args.input_files.empty()) {
    cerr << argv[0] << ": missing argument" << endl;
    error = true;
  }
  return error;
}

int main(int argc, char* argv[])
{
  INIT_LOGGER("default.log_prop");
  Arguments args;
  if (parse_arguments(argc, argv, args)) {
    usage(argv[0]);
    return 1;
  }
  print(args);

  try {
    ParameterSet ps(args.case_insensitive);
    for (size_t i = 0; i < args.input_files.size(); i++)
      ps.adoptFile(args.input_files[i]);
    if (args.output_file.empty())
      ps.writeStream(cout);
    else 
      ps.writeFile(args.output_file);
  } catch (Exception& ex) {
    cerr << ex.what() << endl;
    return 1;
  }
  return 0;
}

