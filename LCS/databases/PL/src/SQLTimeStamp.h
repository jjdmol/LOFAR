//#  SQLTimeStamp.h: Provides a thin wrapper to handle SQL time stamps.
//#
//#  Copyright (C) 2002
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

#ifndef LOFAR_PL_SQLTIMESTAMP_H
#define LOFAR_PL_SQLTIMESTAMP_H

#if !defined(HAVE_ODBC)
#error "ODBC support is required"
#endif

//# Includes
#include <lofar_config.h>
#include <sqltypes.h>
#include <Common/LofarTypes.h>
#include <Common/Debug.h>

namespace LOFAR
{
  namespace PL
  {

    // SQLTimeStamp is a thin wrapper around the \c TIMESTAMP_STRUCT which is
    // defined in the ODBC layer. The main purpose of this wrapper class is to
    // provide easy comparison of time stamps. \c TIMESTAMP_STRUCT is
    // transformed into an internal format which guarantees a strict weak
    // ordering. In other words, SQLTimeStamp is LessThanComparable.
    class SQLTimeStamp
    {
    public:

      // \c TIMESTAMP_STRUCT is the type that is used by ODBC to represent an
      // SQL time stamp.
      typedef TIMESTAMP_STRUCT sql_timestamp_t;

      // Default constructor.
      SQLTimeStamp() : itsTimeStampRep(0) {}

      // Construct an SQLTimeStamp.
      explicit SQLTimeStamp(const sql_timestamp_t& ts) : 
        itsTimeStampRep(toTimeStampRep(ts.year, ts.month, ts.day, ts.hour, 
                                       ts.minute, ts.second, ts.fraction/1000))
      {}

      // Compare two SQL TimeStamps.
      inline friend bool operator<(const SQLTimeStamp& lhs, 
                                   const SQLTimeStamp& rhs);

    private:
      // TimeStampRep is the internal representation of an SQL time
      // stamp. Year, month, day, hour, min, sec, and usec are all separately
      // stored into one 64 bits integer, each unit using the minimally
      // required number of bits, according to the table below.
      // \verbatim
      //        year month day hour min sec usec
      // #bits   18    4    5    5   6   6   20
      // \endverbatim
      typedef int64  TimeStampRep;

      // Offsets (in number of bits) of the date and time units in
      // TimeStampRep.
      struct Offset
      {
        static const uint sec   = 20;
        static const uint min   =  6 + sec;
        static const uint hour  =  6 + min;
        static const uint day   =  5 + hour;
        static const uint month =  5 + day;
        static const uint year  =  4 + month;
      };

      // Convert year, month, day, hour, min, sec, usec into a TimeStampRep.
      //
      // \warning No check is done on validity of input data. Furthermore,
      // \c year should fit into an 18-bits signed integer!
      inline TimeStampRep toTimeStampRep(short year, ushort month, ushort day, 
                                         ushort hour, ushort min, ushort sec,
                                         ulong usec);

      // Internal representation of the SQL TimeStamp.
      TimeStampRep itsTimeStampRep;

    };


    inline bool operator<(const SQLTimeStamp& lhs, const SQLTimeStamp& rhs)
    {
      return lhs.itsTimeStampRep < rhs.itsTimeStampRep;
    }


    inline SQLTimeStamp::TimeStampRep 
    SQLTimeStamp::toTimeStampRep(short year, ushort month, ushort day, 
                                 ushort hour, ushort min, ushort sec, 
                                 ulong usec)
    {
      Assert( month < 13 && day < 32 && hour < 24 && min < 60 && 
              sec < 60 && usec < 1000000);
      return 
        (static_cast<int64>(year)  << Offset::year)  + 
        (static_cast<int64>(month) << Offset::month) +
        (static_cast<int64>(day)   << Offset::day)   + 
        (static_cast<int64>(hour)  << Offset::hour)  + 
        (static_cast<int64>(min)   << Offset::min)   + 
        (static_cast<int64>(sec)   << Offset::sec)   + 
        (static_cast<int64>(usec));
    }

  } // namespace PL

} // namespace LOFAR

#endif
