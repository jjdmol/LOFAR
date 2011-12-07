//# Stopwatch.cc: timer class
//#
//# Copyright (C) 2002
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Stopwatch.h>
    
#include <unistd.h>
#include <cmath>
#include <cstdio>

namespace LOFAR
{

  //##ModelId=3DB9546402FF
  double Stopwatch::scale = 1.0/sysconf(_SC_CLK_TCK);
  //##ModelId=3DB954640301
  struct tms Stopwatch::dummy_tms;
  
  //##ModelId=3DB954640306
  Stopwatch Stopwatch::delta (bool do_reset) 
  {
    Stopwatch prev = *this;
    Stopwatch now(fire_secs);
    now.namewidth = namewidth;
    prev.tick = now.tick - prev.tick;
    prev.tms.tms_utime = now.tms.tms_utime - prev.tms.tms_utime;
    prev.tms.tms_stime = now.tms.tms_stime - prev.tms.tms_stime;
    prev.tms.tms_cutime = now.tms.tms_cutime - prev.tms.tms_cutime;
    prev.tms.tms_cstime = now.tms.tms_cstime - prev.tms.tms_cstime;
    if( do_reset )
      *this = now;
    return prev;
  }

  //##ModelId=3DB954640308
  string Stopwatch::toString (long long nops,int opts,const char * format) const
  {
    string out1,out2;
    char str[256];
    double ut = user(), st = sys(), rt = real();
    if( opts&USER )
    {
      snprintf(str,sizeof str,format,ut); out1 += str;
      if( nops ) {
        snprintf(str,sizeof str,"%12lld",(long long)(nops/(ut+st)+0.5)); out2 += str;
      }
    }
    if( opts&SYSTEM )
    {
      snprintf(str,sizeof str,format,st); out1 += str;
    }
    if( opts&REAL )
    {
      snprintf(str,sizeof str,format,rt); out1 += str;
      if( nops ) {
        snprintf(str,sizeof str,"%12lld",(long long)(nops/rt+0.5)); out2 += str;
      }
    }
    if( nops )
    { snprintf(str,sizeof str,"%12lld",nops); out2 += str; }
    return out1+out2;
  }

  //##ModelId=3DB954640315
  string Stopwatch::dump (const string &name,long long nops,bool do_reset,int opts) 
  {
    char* str = new char[namewidth+32];
    snprintf(str,namewidth+32,"%-*.*s:",namewidth,namewidth,name.c_str());
    string sstr(str);
    delete [] str;
    return sstr + sdelta(nops,do_reset,opts);
  }
    
  //##ModelId=3DB95464031A
  string Stopwatch::header (long long nops)
  {
    static char hdr[2][80];
    snprintf(hdr[0],sizeof hdr[0],"%10s%10s%10s","user","sys","real");
    snprintf(hdr[1],sizeof hdr[1],"%10s%10s%10s%12s%12s%12s","t/user","t/sys","t/real","p/cpusec","p/sec","count");
  
    if( nops )
      return hdr[1];
    else
      return hdr[0];
  }

} // namespace LOFAR
