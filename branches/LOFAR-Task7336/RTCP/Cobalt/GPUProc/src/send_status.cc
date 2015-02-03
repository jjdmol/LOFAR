//# send_status.cc: Send lofar.task.feedback.status information
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <lofar_config.h>

#include <MessageBus/MsgBus.h>
#include <MessageBus/Protocols/TaskFeedbackStatus.h>

#include <boost/format.hpp>

#include <unistd.h>

using namespace LOFAR;
using namespace std;
using boost::format;

static void usage(const char *argv0)
{
  cerr << "Usage: " << argv0 << " success" << endl;
  cerr << endl;
  cerr << "  -h: print this message" << endl;
}

int main(int argc, char **argv)
{
  /*
   * Parse command-line options
   */

  int opt;
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {

    case 'h':
      usage(argv[0]);
      return EXIT_SUCCESS;

    default: /* '?' */
      usage(argv[0]);
      return EXIT_FAILURE;
    }
  }

  // we expect a parset filename as an additional parameter
  if (optind >= argc) {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  int success = atoi(argv[optind]);

  // send status feedback
  ToBus bus("lofar.task.feedback.status");

  Protocols::TaskFeedbackStatus msg(
    "Cobalt/GPUProc/sendStatus",
    "",
    "Status feedback",
    success);

  bus.send(msg);

  return 0;
}

