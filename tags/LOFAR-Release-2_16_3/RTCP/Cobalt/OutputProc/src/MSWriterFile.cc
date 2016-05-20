//# MSWriterFile.cc: a raw file writer
//# Copyright (C) 2009-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "MSWriterFile.h"
#include <Common/Thread/Mutex.h>
#include <Common/SystemUtil.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <boost/algorithm/string.hpp>

namespace LOFAR
{
  namespace Cobalt
  {
    static Mutex makeDirMutex;

    static void makeDir(const string &dirname, const string &logPrefix)
    {
      ScopedLock scopedLock(makeDirMutex);
      struct stat s;

      if (stat(dirname.c_str(), &s) == 0) {
        // path already exists
        if ((s.st_mode & S_IFMT) != S_IFDIR) {
          LOG_WARN_STR(logPrefix << "Not a directory: " << dirname);
        }
      } else if (errno == ENOENT) {
        // create directory
        LOG_DEBUG_STR(logPrefix << "Creating directory " << dirname);

        if (mkdir(dirname.c_str(), 0777) != 0 && errno != EEXIST) {
          THROW_SYSCALL(string("mkdir ") + dirname);
        }
      } else {
        // something else went wrong
        THROW_SYSCALL(string("stat ") + dirname);
      }
    }

    /* create a directory as well as all its parent directories */
    static void recursiveMakeDir(const string &dirname, const string &logPrefix)
    {
      using namespace boost;

      string curdir;
      vector<string> splitName;

      boost::split(splitName, dirname, boost::is_any_of("/"));

      for (unsigned i = 0; i < splitName.size(); i++) {
        curdir += splitName[i] + '/';
        makeDir(curdir, logPrefix);
      }
    }


    MSWriterFile::MSWriterFile (const std::string &msName)
      :
      itsFile((recursiveMakeDir(dirname(msName), ""), msName), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
    {
    }


    MSWriterFile::~MSWriterFile()
    {
    }


    void MSWriterFile::write(StreamableData *data)
    {
      data->write(&itsFile, true, 512);
    }


    size_t MSWriterFile::getDataSize()
    {
      return itsFile.size();
    }


  } // namespace Cobalt
} // namespace LOFAR

