//  main.cc: the commandline interface to the selfcal configuration parser
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
/**
 *
 *
 *
 */
#include <iostream>
#include <fstream>
#include "selfparse.h"


#define DEBUG(x) if(debug_flag_on) std::cout << x << std::endl;
static bool debug_flag_on(true);

char * filename = "fd1";
int main(int ac, char ** av )
{
   int rc = 0;
   
   ++av, --ac;  // skip over program name
   if ( ac > 0 )
   {
      do
      {
         filename = av[0];
         DEBUG("filename: " << filename << ",...");
         
         selfparseStream = new std::ifstream(filename);
         DEBUG("opened.");
         rc = selfparseparse();
         delete selfparseStream;
         ++av;
      }while((--ac > 0) && (rc == 0));
   }
   else
   {
      selfparseStream = &std::cin;
      
      rc = selfparseparse();
   }
   return rc;
}

char *  progname = "selfcalparse";
#ifdef __cplusplus
extern "C"
{
#endif
   int selfparseerror(char * s)
   {
      extern int selfparselineno;
      
      fprintf(stderr,"%s:%s:%u:%s\n",progname,filename,selfparselineno,s);
      return 0;
   }

   int report(char * s)
   {
          // if (debug_or_verbose)
      return fprintf(stdout,s);
   }

   void saveSubScript(char * bn, char * command, char * options , char * block)
   {
      FILE * of;
      char * ext = ".selfcal";
      char * ofname = (char*)(malloc(strlen(bn) + strlen(ext) + 1));
      strcpy(ofname,bn);
      strcat(ofname,ext);
      of = fopen(ofname,"w");
      if(options)
      {
         fprintf(of,"%s %s %s %s", bn, command, options, block);
      }
      else
      {
         fprintf(of,"%s %s %s", bn, command, block);
      }
      fclose(of);
      free(ofname);
   }

   void saveScript( char * command, char * block)
   {
      FILE * of;
      char * ext = ".out";
      char * ofname = (char*)(malloc(strlen(filename) + strlen(ext) + 1));
      strcpy(ofname,filename);
      strcat(ofname,ext);
      of = fopen(ofname,"w");
      fprintf(of,"%s %s", command, block);
      fclose(of);
      free(ofname);
   }

#ifdef __cplusplus
}
#endif
