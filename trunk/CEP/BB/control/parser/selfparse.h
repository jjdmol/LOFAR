//  selfparse.h: common declarations to the configuration parser modules
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

#ifndef SELFPARSE_H_INCLUDED
#define SELFPARSE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

   int selfparseerror(char * s);
   int selfparseparse();

#define YY_DECL   int selfparselex(YYSTYPE* selfparselvalp)

   void saveSubScript(char * bn, char * command, char * options , char * block);
   void saveScript(char * command, char * block);
   int  selfparseGetChars(char * buf, int max_size);
       /* <todo> is this report still needed, or just a relic from development ? </todo> */
   int report(char * s);
  void newSiblings(void);
   char * calculateBrancheNumber();


#ifdef __cplusplus
}
#endif

//extern char * scriptname; // needs to be defined elsewhere
extern char * branch;

#ifdef __cplusplus // don't include this in the real-c code
#include <istream>
extern std::istream *selfparseStream;
#endif

       //#define lexerwrap yywrap

#endif // SELFPARSE_H_INCLUDED
