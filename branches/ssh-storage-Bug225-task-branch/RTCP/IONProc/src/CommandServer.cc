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
#include <Common/LofarLogger.h>
#include <Common/Exceptions.h>
#include <ION_main.h>
#include <Job.h>
#include <Stream/SocketStream.h>
#include <Stream/SystemCallException.h>
#include <StreamMultiplexer.h>

#include <string>

//#if defined HAVE_MPI
//#include <mpi.h>
//#endif


namespace LOFAR {
namespace RTCP {

extern unsigned				myPsetNumber, nrPsets;
extern std::vector<StreamMultiplexer *> allIONstreamMultiplexers;


static bool quit = false;


static void handleCommand(const std::string &command)
{
  LOG_DEBUG_STR("command \"" << command << "\" received");

  if (command == "quit") {
    quit = true;
  } else if (command.compare(0, 7, "parset ") == 0) {
    try {
      new Job(command.substr(7).c_str());
    } catch (APSException &) { // if file could not be found
    }
  } else if (myPsetNumber == 0) {
    LOG_ERROR_STR("command \"" << command << "\" not understood");
  }
}


static void commandMaster()
{
  std::vector<MultiplexedStream *> ionStreams(nrPsets);

  for (unsigned ion = 1; ion < nrPsets; ion ++)
    ionStreams[ion] = new MultiplexedStream(*allIONstreamMultiplexers[ion], 0);

  while (!quit) {
    try {
      SocketStream sk("0.0.0.0", 400, SocketStream::TCP, SocketStream::Server);

      while (!quit) {
	std::string command;

	for (char ch; sk.read(&ch, 1), ch != '\n';) // TODO: do not do a syscall per char
	  command.push_back(ch);

	LOG_DEBUG_STR("read command: " << command);
	unsigned size = command.size() + 1;

	//MPI_Bcast(&size, sizeof size, MPI_INT, 0, MPI_COMM_WORLD);
	//MPI_Bcast(const_cast<char *>(command.c_str()), size, MPI_CHAR, 0, MPI_COMM_WORLD);
	for (unsigned ion = 1; ion < nrPsets; ion ++) {
	  ionStreams[ion]->write(&size, sizeof size);
	  ionStreams[ion]->write(command.c_str(), size);
	}

	handleCommand(command);
      }
    } catch (Stream::EndOfStreamException &) {
    } catch (SystemCallException &ex) {
      if (ex.error == EADDRINUSE) {
	LOG_WARN("address in use");
	sleep(1);
      } else {
	throw;
      }
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
