//#  ION_main.cc:
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: ION_main.cc 15296 2010-03-24 10:19:41Z romein $

#include <lofar_config.h>

#include <CommandServer.h>
#include <Common/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/SystemCallException.h>
#include <ION_main.h>
#include <Job.h>
#include <JobQueue.h>
#include <Stream/SocketStream.h>
#include <StreamMultiplexer.h>

#include <string>

//#if defined HAVE_MPI
//#include <mpi.h>
//#endif


namespace LOFAR {
namespace RTCP {


static bool quit = false;


static void handleCommand(const std::string &command)
{
  LOG_DEBUG_STR("command \"" << command << "\" received");

  if (command.compare(0, 7, "cancel ") == 0) {
    if (myPsetNumber == 0) {
      if (command.compare(0, 10, "cancel all") == 0)
	jobQueue.cancelAll();
      else
	jobQueue.cancel(boost::lexical_cast<unsigned>(command.substr(7)));
    }
  } else if (command == "list_jobs") {
    if (myPsetNumber == 0)
      jobQueue.listJobs();
  } else if (command.compare(0, 7, "parset ") == 0) {
    try {
      jobQueue.insert(new Job(command.substr(7).c_str()));
    } catch (APSException &) { // if file could not be found
    }
  } else if (command == "quit") {
    quit = true;
  } else if (myPsetNumber == 0) {
    LOG_ERROR_STR("command \"" << command << "\" not understood");
  }
}


static void commandMaster()
{
  std::vector<MultiplexedStream *> ionStreams(nrPsets);

  for (unsigned ion = 1; ion < nrPsets; ion ++)
    ionStreams[ion] = new MultiplexedStream(*allIONstreamMultiplexers[ion], 0);

  SocketStream sk("0.0.0.0", 4000, SocketStream::TCP, SocketStream::Server);

  while (!quit) {
    std::string command;

    try {
      command = sk.readLine();
      LOG_DEBUG_STR("read command: " << command);
    } catch (Stream::EndOfStreamException &) {
      sk.reaccept();
      continue;
    }

    unsigned size = command.size() + 1;

    //MPI_Bcast(&size, sizeof size, MPI_INT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(const_cast<char *>(command.c_str()), size, MPI_CHAR, 0, MPI_COMM_WORLD);
    for (unsigned ion = 1; ion < nrPsets; ion ++) {
      ionStreams[ion]->write(&size, sizeof size);
      ionStreams[ion]->write(command.c_str(), size);
    }

    try {
      handleCommand(command);
    } catch (Exception &ex) {
      LOG_ERROR_STR("handleCommand caught Exception: " << ex);
    } catch (std::exception &ex) {
      LOG_ERROR_STR("handleCommand caught std::exception: " << ex.what());
    } catch (...) {
      LOG_ERROR("handleCommand caught non-std::exception: ");
    }
  }

  for (unsigned ion = 1; ion < nrPsets; ion ++)
    delete ionStreams[ion];
}


static void commandSlave()
{
  MultiplexedStream streamFromMaster(*allIONstreamMultiplexers[0], 0);

  while (!quit) {
    unsigned size;

    //MPI_Bcast(&size, sizeof size, MPI_INT, 0, MPI_COMM_WORLD);
    streamFromMaster.read(&size, sizeof size);

    char *command = new char[size];
    //MPI_Bcast(command, size, MPI_CHAR, 0, MPI_COMM_WORLD);
    streamFromMaster.read(command, size);
    handleCommand(command);
    delete [] command;
  }
}


void commandServer()
{
  if (myPsetNumber == 0)
    commandMaster();
  else
    commandSlave();
}


} // namespace RTCP
} // namespace LOFAR
