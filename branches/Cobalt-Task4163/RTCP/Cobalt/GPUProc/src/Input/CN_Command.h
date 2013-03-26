//# CN_Command.h
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

#ifndef LOFAR_GPUPROC_CN_COMMAND_H
#define LOFAR_GPUPROC_CN_COMMAND_H

#include <string>

#include <Stream/Stream.h>

namespace LOFAR
{
  namespace RTCP
  {

    class CN_Command
    {
    public:
      enum Command {
        PREPROCESS = 0x406e7404,
        PROCESS,
        POSTPROCESS,
        STOP,
      };

      CN_Command();
      CN_Command(enum Command, unsigned param = 0);

      enum Command &value();
      unsigned &param();

      void         read(Stream *);
      void         write(Stream *) const;

      std::string  name() const;

    private:
      struct MarshalledData
      {
        enum Command value;
        unsigned param;
      } itsMarshalledData;
    };


    inline CN_Command::CN_Command()
    {
    }

    inline CN_Command::CN_Command(enum Command value, unsigned param)
    {
      itsMarshalledData.value = value;
      itsMarshalledData.param = param;
    }

    inline enum CN_Command::Command &CN_Command::value()
    {
      return itsMarshalledData.value;
    }

    inline unsigned &CN_Command::param()
    {
      return itsMarshalledData.param;
    }

    inline void CN_Command::read(Stream *str)
    {
      str->read(&itsMarshalledData, sizeof itsMarshalledData);
    }

    inline void CN_Command::write(Stream *str) const
    {
      str->write(&itsMarshalledData, sizeof itsMarshalledData);
    }

    inline std::string CN_Command::name() const
    {
      switch(itsMarshalledData.value) {
      case PREPROCESS:  return "PREPROCESS";
      case PROCESS:     return "PROCESS";
      case POSTPROCESS: return "POSTPROCESS";
      case STOP:        return "STOP";
      }

      return "BAD COMMAND";
    }


  } // namespace RTCP
} // namespace LOFAR

#endif

