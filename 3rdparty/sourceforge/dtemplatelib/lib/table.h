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
// quick table macros
// Initial: 2/21/2003 - CJ
// Reviewed: 11/12/2000 - CJ
// Edited: 12/19/2000 - MG - added namespaces

#ifndef DTL_TABLE_H
#define DTL_TABLE_H

#include "DTL.h"


#define DTL_LESS(A, B, FIELD) \
	if ((A.FIELD) < (B.FIELD)) return true; \
	else if ((A.FIELD) > (B.FIELD)) return false;

#define DTL_BIND_FIELD(FIELD) \
	cols[_TEXT( #FIELD )] >> row.FIELD

#define DTL_STRUCT1(STRUCT_NAME,TYPE1,FIELD1) \
	struct STRUCT_NAME { \
	TYPE1 FIELD1; \
	}; \
	\
	dtl::tostream &operator<<(dtl::tostream &o, const STRUCT_NAME &s) \
	{ \
		o <<  _TEXT("("); \
		o << s.FIELD1; \
		o << _TEXT(")"); \
		return o; \
	}; \
	\
	bool operator<(const STRUCT_NAME &lhs, const STRUCT_NAME &rhs) { \
		DTL_LESS (lhs, rhs, FIELD1); \
		return false; \
	} \
	\
    BEGIN_DTL_NAMESPACE \
	template<> class DefaultBCA<STRUCT_NAME> \
	{ \
	public:\
		void operator()(BoundIOs &cols, STRUCT_NAME &row) \
		{ \
			DTL_BIND_FIELD(FIELD1); \
		} \
	};\
	END_DTL_NAMESPACE 


#define DTL_STRUCT2(STRUCT_NAME,TYPE1,FIELD1,TYPE2,FIELD2) \
	struct STRUCT_NAME { \
	TYPE1 FIELD1; \
	TYPE2 FIELD2; \
	}; \
	\
	dtl::tostream &operator<<(dtl::tostream &o, const STRUCT_NAME &s) \
	{ \
		o <<  _TEXT("("); \
		o << s.FIELD1 << _TEXT(", "); \
		o << s.FIELD2; \
		o << _TEXT(")"); \
		return o; \
	}; \
	\
	bool operator<(const STRUCT_NAME &lhs, const STRUCT_NAME &rhs) { \
		DTL_LESS (lhs, rhs, FIELD1); \
		DTL_LESS (lhs, rhs, FIELD2); \
		return false; \
	} \
	\
	BEGIN_DTL_NAMESPACE \
	template<> class DefaultBCA<STRUCT_NAME> \
	{ \
	public:\
		void operator()(BoundIOs &cols, STRUCT_NAME &row) \
		{ \
			DTL_BIND_FIELD(FIELD1); \
			DTL_BIND_FIELD(FIELD2); \
		} \
	};\
	END_DTL_NAMESPACE 




#define DTL_STRUCT3(STRUCT_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3) \
	struct STRUCT_NAME { \
	TYPE1 FIELD1; \
	TYPE2 FIELD2; \
	TYPE3 FIELD3; \
    }; \
	\
	dtl::tostream &operator<<(dtl::tostream &o, const STRUCT_NAME &s) \
	{ \
		o <<  _TEXT("("); \
		o << s.FIELD1 << _TEXT(", "); \
		o << s.FIELD2 << _TEXT(", "); \
		o << s.FIELD3; \
		o << _TEXT(")"); \
		return o; \
	}; \
	\
	bool operator<(const STRUCT_NAME &lhs, const STRUCT_NAME &rhs) { \
		DTL_LESS (lhs, rhs, FIELD1); \
		DTL_LESS (lhs, rhs, FIELD2); \
		DTL_LESS (lhs, rhs, FIELD3); \
		return false; \
	} \
	\
	BEGIN_DTL_NAMESPACE \
	template<> class DefaultBCA<STRUCT_NAME> \
	{ \
	public:\
		void operator()(BoundIOs &cols, STRUCT_NAME &row) \
		{ \
			DTL_BIND_FIELD(FIELD1); \
			DTL_BIND_FIELD(FIELD2); \
			DTL_BIND_FIELD(FIELD3); \
		} \
	};\
	END_DTL_NAMESPACE 




#define DTL_STRUCT4(STRUCT_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4) \
	struct STRUCT_NAME { \
	TYPE1 FIELD1; \
	TYPE2 FIELD2; \
	TYPE3 FIELD3; \
	TYPE4 FIELD4; \
	}; \
	\
	dtl::tostream &operator<<(dtl::tostream &o, const STRUCT_NAME &s) \
	{ \
		o <<  _TEXT("("); \
		o << s.FIELD1 << _TEXT(", "); \
		o << s.FIELD2 << _TEXT(", "); \
		o << s.FIELD3 << _TEXT(", "); \
		o << s.FIELD4; \
		o << _TEXT(")"); \
		return o; \
	}; \
	\
	bool operator<(const STRUCT_NAME &lhs, const STRUCT_NAME &rhs) { \
		DTL_LESS (lhs, rhs, FIELD1); \
		DTL_LESS (lhs, rhs, FIELD2); \
		DTL_LESS (lhs, rhs, FIELD3); \
		DTL_LESS (lhs, rhs, FIELD4); \
		return false; \
	} \
	\
	BEGIN_DTL_NAMESPACE \
	template<> class DefaultBCA<STRUCT_NAME> \
	{ \
	public:\
		void operator()(BoundIOs &cols, STRUCT_NAME &row) \
		{ \
			DTL_BIND_FIELD(FIELD1); \
			DTL_BIND_FIELD(FIELD2); \
			DTL_BIND_FIELD(FIELD3); \
			DTL_BIND_FIELD(FIELD4); \
		} \
	};\
	END_DTL_NAMESPACE 



#define DTL_STRUCT5(STRUCT_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4,TYPE5,FIELD5) \
	struct STRUCT_NAME { \
	TYPE1 FIELD1; \
	TYPE2 FIELD2; \
	TYPE3 FIELD3; \
	TYPE4 FIELD4; \
	TYPE5 FIELD5; \
	}; \
	\
	dtl::tostream &operator<<(dtl::tostream &o, const STRUCT_NAME &s) \
	{ \
		o <<  _TEXT("("); \
		o << s.FIELD1 << _TEXT(", "); \
		o << s.FIELD2 << _TEXT(", "); \
		o << s.FIELD3 << _TEXT(", "); \
		o << s.FIELD4 << _TEXT(", "); \
		o << s.FIELD5; \
		o << _TEXT(")"); \
		return o; \
	}; \
	\
	bool operator<(const STRUCT_NAME &lhs, const STRUCT_NAME &rhs) { \
		DTL_LESS (lhs, rhs, FIELD1); \
		DTL_LESS (lhs, rhs, FIELD2); \
		DTL_LESS (lhs, rhs, FIELD3); \
		DTL_LESS (lhs, rhs, FIELD4); \
		DTL_LESS (lhs, rhs, FIELD5); \
		return false; \
	} \
	\
	BEGIN_DTL_NAMESPACE \
	template<> class DefaultBCA<STRUCT_NAME> \
	{ \
	public:\
		void operator()(BoundIOs &cols, STRUCT_NAME &row) \
		{ \
			DTL_BIND_FIELD(FIELD1); \
			DTL_BIND_FIELD(FIELD2); \
			DTL_BIND_FIELD(FIELD3); \
			DTL_BIND_FIELD(FIELD4); \
			DTL_BIND_FIELD(FIELD5); \
		} \
	};\
	END_DTL_NAMESPACE 




#define DTL_STRUCT6(STRUCT_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4,TYPE5,FIELD5,TYPE6,FIELD6) \
	struct STRUCT_NAME { \
	TYPE1 FIELD1; \
	TYPE2 FIELD2; \
	TYPE3 FIELD3; \
	TYPE4 FIELD4; \
	TYPE5 FIELD5; \
	TYPE6 FIELD6; \
	}; \
	\
	dtl::tostream &operator<<(dtl::tostream &o, const STRUCT_NAME &s) \
	{ \
		o <<  _TEXT("("); \
		o << s.FIELD1 << _TEXT(", "); \
		o << s.FIELD2 << _TEXT(", "); \
		o << s.FIELD3 << _TEXT(", "); \
		o << s.FIELD4 << _TEXT(", "); \
		o << s.FIELD5 << _TEXT(", "); \
		o << s.FIELD6; \
		o << _TEXT(")"); \
		return o; \
	}; \
	\
	bool operator<(const STRUCT_NAME &lhs, const STRUCT_NAME &rhs) { \
		DTL_LESS (lhs, rhs, FIELD1); \
		DTL_LESS (lhs, rhs, FIELD2); \
		DTL_LESS (lhs, rhs, FIELD3); \
		DTL_LESS (lhs, rhs, FIELD4); \
		DTL_LESS (lhs, rhs, FIELD5); \
		DTL_LESS (lhs, rhs, FIELD6); \
		return false; \
	} \
	\
	BEGIN_DTL_NAMESPACE \
	template<> class DefaultBCA<STRUCT_NAME> \
	{ \
	public:\
		void operator()(BoundIOs &cols, STRUCT_NAME &row) \
		{ \
			DTL_BIND_FIELD(FIELD1); \
			DTL_BIND_FIELD(FIELD2); \
			DTL_BIND_FIELD(FIELD3); \
			DTL_BIND_FIELD(FIELD4); \
			DTL_BIND_FIELD(FIELD5); \
			DTL_BIND_FIELD(FIELD6); \
		} \
	};\
	END_DTL_NAMESPACE 



#define DTL_TABLE1(TABLE_NAME,TYPE1,FIELD1) \
  DTL_STRUCT1(TABLE_NAME ## _row, TYPE1,FIELD1); \
  typedef dtl::DBView<TABLE_NAME ## _row> TABLE_NAME ## _view; \
  TABLE_NAME ## _view TABLE_NAME(_TEXT( #TABLE_NAME ))

#define DTL_TABLE2(TABLE_NAME,TYPE1,FIELD1,TYPE2,FIELD2) \
  DTL_STRUCT2(TABLE_NAME ## _row, TYPE1,FIELD1,TYPE2,FIELD2); \
  typedef dtl::DBView<TABLE_NAME ## _row> TABLE_NAME ## _view; \
  TABLE_NAME ## _view TABLE_NAME(_TEXT( #TABLE_NAME ))

#define DTL_TABLE3(TABLE_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3) \
  DTL_STRUCT3(TABLE_NAME ## _row, TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3); \
  typedef dtl::DBView<TABLE_NAME ## _row> TABLE_NAME ## _view; \
  TABLE_NAME ## _view TABLE_NAME(_TEXT( #TABLE_NAME ))

#define DTL_TABLE4(TABLE_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4) \
  DTL_STRUCT4(TABLE_NAME ## _row, TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4); \
  typedef dtl::DBView<TABLE_NAME ## _row> TABLE_NAME ## _view; \
  TABLE_NAME ## _view TABLE_NAME(_TEXT( #TABLE_NAME ))

#define DTL_TABLE5(TABLE_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4,TYPE5,FIELD5) \
  DTL_STRUCT5(TABLE_NAME ## _row, TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4,TYPE5,FIELD5); \
  typedef dtl::DBView<TABLE_NAME ## _row> TABLE_NAME ## _view; \
  TABLE_NAME ## _view TABLE_NAME(_TEXT( #TABLE_NAME ))

#define DTL_TABLE6(TABLE_NAME,TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4,TYPE5,FIELD5,TYPE6,FIELD6) \
  DTL_STRUCT6(TABLE_NAME ## _row, TYPE1,FIELD1,TYPE2,FIELD2,TYPE3,FIELD3,TYPE4,FIELD4,TYPE5,FIELD5,TYPE6,FIELD6); \
  typedef dtl::DBView<TABLE_NAME ## _row> TABLE_NAME ## _view; \
  TABLE_NAME ## _view TABLE_NAME(_TEXT( #TABLE_NAME ))


#endif
