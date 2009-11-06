//#  CN_Command.h:
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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
//#  $Id$

#ifndef LOFAR_INTERFACE_CN_COMMAND_H
#define LOFAR_INTERFACE_CN_COMMAND_H

#include <Stream/Stream.h>


namespace LOFAR {
namespace RTCP {

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
		 CN_Command(enum Command);

    enum Command &value();

    void	 read(Stream *);
    void	 write(Stream *) const;

  private:
    struct MarshalledData
    {
      enum Command value;
    } itsMarshalledData;
};


inline CN_Command::CN_Command()
{
}

inline CN_Command::CN_Command(enum Command value)
{
  itsMarshalledData.value = value;
}

inline enum CN_Command::Command &CN_Command::value()
{
  return itsMarshalledData.value;
}

inline void CN_Command::read(Stream *str)
{
  str->read(&itsMarshalledData, sizeof itsMarshalledData);
}

inline void CN_Command::write(Stream *str) const
{
  str->write(&itsMarshalledData, sizeof itsMarshalledData);
}


} // namespace RTCP
} // namespace LOFAR

#endif 
