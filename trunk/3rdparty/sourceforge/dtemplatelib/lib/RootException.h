/* Copyright © 2000 
Michael Gradman and Corwin Joy 

Permission to use, copy, modify, distribute and sell this software and 
its documentation for any purpose is hereby granted without fee, provided 
that the above copyright notice appears in all copies and that both that 
copyright notice and this permission notice appear in supporting documentation. 
Corwin Joy and Michael Gradman make no representations about the suitability 
of this software for any purpose. 
It is provided "as is" without express or implied warranty.
*/ 
// root exception type for DBObjects
// Initial: 11/14/2000 - MG
// Edited: 12/19/2000 - MG - added namespaces

// derives from standard STL exception class
#ifndef DTL_ROOT_EXCEPTION_H
#define DTL_ROOT_EXCEPTION_H

#include "dtl_config.h"

#include "std_warn_off.h"
#include <iosfwd>
#include <string>
#include <exception>
#include "std_warn_on.h"


BEGIN_DTL_NAMESPACE

class RootException : public STD_::exception
{
public:
  const tstring method;
  const tstring errmsg;
  const tstring exType;

  mutable tstring whatbuf;	// buffer used to store result of what() operation
							// so it doesn't get destroyed on us

#ifdef _UNICODE
  mutable STD_::string whatbuf_narrow;
#endif

  RootException();

  RootException(const tstring &meth, const tstring &err,
	  const tstring &excType = _TEXT("RootException"));

  friend tostream &operator<<(tostream &o, const RootException &ex);

  // override if you have additional members to print
  virtual const char *what() const throw();
  virtual const TCHAR *twhat() const throw();
  virtual ~RootException() throw() {};

};

END_DTL_NAMESPACE

#endif
