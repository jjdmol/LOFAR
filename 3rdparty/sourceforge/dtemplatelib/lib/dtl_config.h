/* Copyright © 2001
Michael Gradman and Corwin Joy 

Permission to use, copy, modify, distribute and sell this software and 
its documentation for any purpose is hereby granted without fee, provided 
that the above copyright notice appears in all copies and that both that 
copyright notice and this permission notice appear in supporting documentation. 
Corwin Joy and Michael Gradman make no representations about the suitability 
of this software for any purpose. 
It is provided "as is" without express or implied warranty.
*/ 
// includes configuration options
// Initial: 4/27/2001 - MG


#ifndef DTL_DTL_CONFIG_H
#define DTL_DTL_CONFIG_H

// need to force __SGI_STL_PORT to be #define'd to get proper preprocessor
// behavior below if STLPort is being used

#include "std_warn_off.h"
#include <vector>
#include "std_warn_on.h"

// if defined DTL will convert all SQL strings to lower case before sending to 
// driver 
//#define DTL_LC 

// if defined DTL will convert all SQL strings to upper case before sending to
// driver 
//#define DTL_UC 

// if defined, will output ROWID (autokey) info for variant_row
// #define DTL_OUTPUT_ROWIDS

#define USES_STD_NAMESPACE

// defines standard namespace properly
#if defined(USES_STD_NAMESPACE)
#  ifdef __SGI_STL_PORT
#    ifdef __STLPORT_NAMESPACE
#      define STD_ __STLPORT_NAMESPACE
#    else
#      define STD_ std
#    endif
#  else
#    define STD_ std
#  endif
#else
#  define STD_
#endif

// macros for our library's namespace ... currently set to activate
// on namespace dtl
#define BEGIN_DTL_NAMESPACE namespace dtl {
#define END_DTL_NAMESPACE }

// define the default logging handler
#ifndef DEFAULT_IO_HANDLER
#  define DEFAULT_IO_HANDLER AlwaysThrowsHandler
#endif

#ifdef DTL_SINGLE_FETCH
#define DEFAULT_FETCH_MODE SINGLE_FETCH
#endif

// define the default fetch mode for IndexedDBViews
#ifndef DEFAULT_FETCH_MODE
#  define DEFAULT_FETCH_MODE BULK_FETCH
#endif

// define the default IdxContainer

// for those configurations that provide std::hash_multiset,
// use that as the default IdxContainer
// otherwise use dtl::vec_multiset
#ifndef DEFAULT_IDXCONTAINER
#  if defined(__SGI_STL_PORT) 
#    define DTL_USE_HASH_CONTAINER
#    ifdef DTL_USE_NO_HASH_CONTAINER
#      undef DTL_USE_NO_HASH_CONTAINER
#    endif
#    define DEFAULT_IDXCONTAINER STD_::hash_multiset
#  else
#    define DTL_USE_NO_HASH_CONTAINER
#    ifdef DTL_USE_HASH_CONTAINER
#      undef DTL_USE_HASH_CONTAINER
#    endif
#    define DEFAULT_IDXCONTAINER dtl::vec_multiset
#  endif
#endif

// Attempt to make defining Indexed Views simple!!
//#ifdef __SUNPRO_CC
#ifdef __unix
#  ifdef DTL_USE_NO_HASH_CONTAINER
#    define DEFAULT_INDEX_VIEW(view_type) \
IndexedDBView<view_type, dtl::vec_multiset<view_type::value_type *, \
                                      lt_functor<view_type::value_type> > >
#    define DEFAULT_DYNA_VIEW(view_type) \
DynamicIndexedDBView<view_type, \
                     dtl::vec_multiset<view_type::value_type *,\
                                       lt_functor<view_type::value_type> > >
#  else
#    define DEFAULT_INDEX_VIEW(view_type) \
IndexedDBView<view_type, \
              STD_::hash_multiset<view_type::value_type *, \
                                  hash_functor<view_type::value_type>, \
                                  eq_functor<view_type::value_type> > >
#    define DEFAULT_DYNA_VIEW(view_type) \
DynamicIndexedDBView<view_type, \
                     STD_::hash_multiset<view_type::value_type *,\
                                         hash_functor<view_type::value_type>, \
                                         eq_functor<view_type::value_type> > >
#  endif
#else
#  define DEFAULT_INDEX_VIEW(view_type) IndexedDBView< view_type >
#  define DEFAULT_DYNA_VIEW(view_type)  DynamicIndexedDBView< view_type >
#endif


// the standard requires that the "typename" keyword be used in the necessary
// places to denote type names in default template parameters, etc., that 
// depend on template parameters already defined
// but MS Visual C++ 6.0, the use of the "typename" keyword in this manner is 
// forbidden
#if (defined  (_MSC_VER) || defined (__SUNPRO_CC) || (defined (__BORLANDC__) && __BORLANDC__ <= 0x551))
#define TYPENAME_IN_TEMPLATE_PARAMS
#else // assume all other compilers conform to the standard in this instance
#define TYPENAME_IN_TEMPLATE_PARAMS typename
#endif

#if defined (__SUNPRO_CC)
#  define NOT_FRIEND_ON_HP friend
#  define DTL_SOLARIS_DISTANCE
#  define _GCC_CAST_BUG
#  define DTL_NO_UNICODE
#  define USING_STD_NAMESPACE using namespace std; 
// Ensure that standard library functions are in scope

#  define DTL_TEMPLATE_FUNC(func, type) func<type> 
// Sun compiler requires template parameter on certain function calls

// #  define DTL_NO_BULK_OPERATIONS
// Merant on Sun platform does not seem to properly support bulk SQL operations
#elif defined (__hpux)
#  define NOT_FRIEND_ON_HP
#  define _GCC_CAST_BUG
#  define DTL_NO_UNICODE
#  define USING_STD_NAMESPACE  
#  define DTL_TEMPLATE_FUNC(func, type) func
#else
#  define NOT_FRIEND_ON_HP friend
#  define USING_STD_NAMESPACE  
#  define DTL_TEMPLATE_FUNC(func, type) func
// on MSVC, specifying such template param forbidden
#endif

#ifdef  WIN32
#  include <tchar.h>
#else
#  include "unix_tchar.h"
#endif

#ifdef __BORLANDC__
typedef _TCHAR    TCHAR;
// fake wctob and inverse for Borland - this should work for WIN32
inline int wctob(wchar_t c) {return c/256;};
inline wchar_t btowc(int c) {return c*256;};
// Must use define here to be in line with sqltypes.h ODBC headers do not appear to declare this for Borland
#define	ODBCINT64 __int64 
#endif

#include "std_warn_off.h"

#include <string>
#include <iostream>
#include <sstream>
#include <iterator>

#ifdef __BORLANDC__
// Strange / yucky workaground for Borland C++ 5.5 / Rogue Wave libraries
// Below, we define our blob datatype as basic_string<BTYE>
// Unfortunately, in Borland 5.5, this gives us an error at link
// time saying that the implementation for basic_string<unsigned char> is not
// found!
// I think Borland may do tricky linkage / export stuff for their string
// templates so that the implementation is only created once for <char> and 
// <wchar_t> and linked as part of their runtime library?
// Anyway, there seems to be a Rogue Wave library define we can use to force it
// to instantiate templates so that basic_string<BTYE> will be found.

#  if !defined(__SGI_STL_PORT)
#    include <string.cc>
#  endif
#endif


#include "std_warn_on.h"

#if defined (__GNUC__)
// libstd version 3 with gcc 2.96 provides 
// no support for wide string streams
// For now, disable any UNICODE operations
// until gcc 3.0
#  define DTL_NO_UNICODE
#endif

#ifdef WIN32
	// define in global scope for Windows to avoid clashes with
	// Windows header files
	typedef unsigned char BYTE; 
#else
	// OK for other platforms to define in dtl namespace
BEGIN_DTL_NAMESPACE
	typedef unsigned char BYTE;
END_DTL_NAMESPACE
#endif

BEGIN_DTL_NAMESPACE
typedef STD_::basic_string<BYTE> blob;
END_DTL_NAMESPACE

#ifdef _UNICODE
BEGIN_DTL_NAMESPACE 
   typedef STD_::wstring tstring;
   typedef STD_::wostream tostream;
   static  STD_::wostream &tcout = STD_::wcout; 
   typedef STD_::wostringstream tostringstream;
   typedef STD_::wistringstream tistringstream;
END_DTL_NAMESPACE
#else
BEGIN_DTL_NAMESPACE 
   typedef STD_::string tstring;
   typedef STD_::ostream tostream;
#  ifndef  __SUNPRO_CC
	 static STD_::ostream &tcout = STD_::cout; 
#  else
#    define tcout STD_::cout
#  endif
   typedef STD_::ostringstream tostringstream;
   typedef STD_::istringstream tistringstream;
END_DTL_NAMESPACE
#endif 

BEGIN_DTL_NAMESPACE
#if !defined (__GNUC__)

// unfortunately, STLPort doesn't support basic_ostream, only ostream and 
// wostream so use overloads that should still compile properly
#  if defined(__SGI_STL_PORT) && !defined(__SUNPRO_CC)

// should only work for C = char
template<typename T, typename C> 
STD_::ostream_iterator<T, C>  tostream_iterator(STD_::ostream &o,
												const C *pc = 0)
{
	return STD_::ostream_iterator<T, C>(o, pc);
}

// should only work for C = wchar_t
template<typename T, typename C>
STD_::ostream_iterator<T, C>  tostream_iterator(STD_::wostream &o,
												 const C *pc = 0)
{
	return STD_::ostream_iterator<T, C>(o, pc);
}

#  else
template<typename T, typename C>
STD_::ostream_iterator<T, C>  tostream_iterator(STD_::basic_ostream<C> &o, 
											   const C* pc=0)
{
    return STD_::ostream_iterator<T, C>(o, pc);
}
#  endif
#else 
template<typename T, typename C>
STD_::ostream_iterator<T>     tostream_iterator(tostream &o, const C* pc=0)
{
    return STD_::ostream_iterator<T>(o, pc);
}
#endif


#if  ((__GNUC__ >= 3) && (__GNUC_MINOR__ > 0)) 
#  define DTL_USE_STD_ITERATOR_TYPES
#endif


#if defined(__SGI_STL_PORT)  || (defined(__GNUC__) && !defined(DTL_USE_STD_ITERATOR_TYPES))
#  define DTL_USE_HP_ITERATOR_TYPES
#endif

END_DTL_NAMESPACE

#if (__GNUC__ >= 3)

namespace std 
{

	// GCC needs char_traits inplementation for
	// char_traits<unsigned char>


	/// 21.1.4  char_traits specializations

#ifdef WIN32
  template<>
    struct char_traits<BYTE>
    {
      typedef BYTE 		char_type;

#else
	template<>
	struct char_traits<dtl::BYTE>
    {
      typedef dtl::BYTE 		char_type;

#endif
      typedef int 	        int_type;
      typedef streampos 	pos_type;
      typedef streamoff 	off_type;
      typedef mbstate_t 	state_type;

      static void 
      assign(char_type& __c1, const char_type& __c2)
      { __c1 = __c2; }

      static bool 
      eq(const char_type& __c1, const char_type& __c2)
      { return __c1 == __c2; }

      static bool 
      lt(const char_type& __c1, const char_type& __c2)
      { return __c1 < __c2; }

      static int 
      compare(const char_type* __s1, const char_type* __s2, size_t __n)
      { return memcmp(__s1, __s2, __n); }

      static size_t
      length(const char_type* __s)
      { return strlen((const char *)__s); }

      static const char_type* 
      find(const char_type* __s, size_t __n, const char_type& __a)
      { return static_cast<const char_type*>(memchr(__s, __a, __n)); }

      static char_type* 
      move(char_type* __s1, const char_type* __s2, size_t __n)
      { return static_cast<char_type*>(memmove(__s1, __s2, __n)); }

      static char_type* 
      copy(char_type* __s1, const char_type* __s2, size_t __n)
      {  return static_cast<char_type*>(memcpy(__s1, __s2, __n)); }

      static char_type* 
      assign(char_type* __s, size_t __n, char_type __a)
      { return static_cast<char_type*>(memset(__s, __a, __n)); }

      static char_type 
      to_char_type(const int_type& __c)
      { return static_cast<char_type>(__c); }

      // To keep both the byte 0xff and the eof symbol 0xffffffff
      // from ending up as 0xffffffff.
      static int_type 
      to_int_type(const char_type& __c)
      { return static_cast<int_type>(static_cast<unsigned char>(__c)); }

      static bool 
      eq_int_type(const int_type& __c1, const int_type& __c2)
      { return __c1 == __c2; }

      static int_type 
      eof() { return static_cast<int_type>(EOF); }

      static int_type 
      not_eof(const int_type& __c)
      { return (__c == eof()) ? 0 : __c; }
  };
}

#endif

#endif
