//# ParmDBLog.h: Class to log results when solving parameters
//#
//# Copyright (C) 2010
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
//# $Id: ParmDBLog.h 16590 2010-10-21 19:47:28Z duscha $

// @file
// @brief Class to log results when solving parameters
// @author Sven Duscha (duscha AT astron nl)


#ifndef LOFAR_PARMDB_PARMDBLOGLEVEL_H
#define LOFAR_PARMDB_PARMDBLOGLEVEL_H

#include <string>


class ParmDBLoglevel
{
public:
   // Setting which logging level is used
   typedef enum {
         NONE, 
         PERSOLUTION, 
         PERITERATION,
         PERSOLUTION_CORRMATRIX, 
         PERITERATION_CORRMATRIX,
         //# Insert new logging values HERE !!
         N_LEVELS
   }LoggingLevel; 
   
   
   // default constructor
   ParmDBLoglevel()
   {
      itsLoggingLevel=NONE;         // sets logging level to NONE
   }
   
   
   ParmDBLoglevel(LoggingLevel level)
   {
      set(level);
   }
   
   
   ParmDBLoglevel(const std::string &level)
   {
      set(level);
   }
   
   
   ~ParmDBLoglevel()
   {
      // desctrutor, does nothing
   }

   
   void set(LoggingLevel level)
   {
      if(NONE<level && level<N_LEVELS)
         itsLoggingLevel=level;  
      else
         itsLoggingLevel=NONE;
   }
   
   
   void set(const std::string &level)
   {
      // TODO: make this nicer
      if(level=="PERSOLUTION")
         itsLoggingLevel=PERSOLUTION;
      else if(level=="PERITERATION")
         itsLoggingLevel=PERITERATION;
      else if(level=="PERSOLUTION_CORRMATRIX")
         itsLoggingLevel=PERSOLUTION_CORRMATRIX;
      else if(level=="PERITERATION_CORRMATRIX")
         itsLoggingLevel=PERITERATION_CORRMATRIX;
      else
         itsLoggingLevel=NONE;
   }
   
   
   LoggingLevel get() const
   {
      return itsLoggingLevel; 
   }
   
   
   std::string asString() const
   {
      // Create an array of strings, and keep that consistent with
      // the definition of the enum(eration) entries
      
      // TODO: make this nicer
      std::string strLevel[N_LEVELS]={
         "NONE", 
         "PERSOLUTION", 
         "PERITERATION",
         "PERSOLUTION_CORRMATRIX",
         "PERITERATION_CORRMATRIX",
      };
   
      return strLevel[itsLoggingLevel];
   }
   
   
   bool is(LoggingLevel level) const
   {
      return(itsLoggingLevel==level);     
   }
         
   
private:
   LoggingLevel itsLoggingLevel;   // the logging level that is set
   
};


#endif
