//  SelfcalEngineStub.h: one line description
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

#if !defined(BB_SELFCALENGINESTUB_H)
#define BB_SELFCALENGINESTUB_H

//# Includes
//# include <otherpackage/file.h>

//# Forward Declarations
//class forward;


// Description of class.

class SelfcalEngineStub
{
  
 public:
  SelfcalEngineStub();
  ~SelfcalEngineStub();

  /**
     The init method is used to reset the SelfcalEngine.
     The local parameter set to be used by the Engine is passed by the 
     parameters argument 
   **/
  void init(int   len,
	    float parameters[]);

  /**
     The Solve() method is used to let the SelfcalEngine perform work. 
     The work definition is passed by the inparams argument.
     The result of the work is passed by the outparams argument. 
     The outparams array should be writable (and is allocated by the calling 
     party)
   **/
  float * Solve(bool    *workdef,   // binary pattern of the parameters to 
	                             // be solved for
                 float  *outparams); // the result of the Solve; 
                                     // All parameter values are returned

  /**
     write current parameter values (itsParamValues) to stdout.
   **/
  void dump();

 private:
  int    itsLen;
  float *itsParamValues;
  
};


#endif

