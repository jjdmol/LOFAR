[+ AutoGen5 template tse.prot +]
[+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE protocol_id +][+ IF (exist? "id") +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
[+ DEFINE event_parm +]
	[+ IF (*== (get "type") "[]") +][+ (get "name") +]NOE : t_B4,
	[+ (get "name") +] : t_[+ (substring (get "type") 0 (string-index (get "type") #\[)) +]Array [ [+ (get "name") +]NOE ][+ ELIF (== (get "type") "string") +][+ (get "name") +]NOC : t_StringLen, 
	[+ (get "name") +] : t_String[[+ (get "name") +]NOC][+ ELSE +][+ (get "name") +] : [+ CASE (get "type") +][+ == unsigned int +]t_uInt[+ == long +]t_Long[+ * +][+ (get "type") +][+ ESAC +][+ ENDIF +][+ ENDDEF +]
[+ (out-pop) +]
//
//  [+ (base-name) +].[+ (suffix) +]: [+ description +]
//
//  Copyright (C) 2005
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
[+ (lgpl "This program" "ASTRON" "//  ") +]
//
//  $Id$
//

//    This file contains three sections:
//    [type]      : type definitions
//    [functions] : Messages sent to the Device under Test (DUT)
//    [events]    : Messages received from the DuT.
//
//    The format of each section is explained in brief in the
//    beginning of each section.
//========================================================================

[type]

//    A type has a number of parameters: the first parameter
//    defines the size in bytes. If there is a second and third
//    parameter, these define the lower and upper limit.
//
//    If there is a fourth parameter, it can be a string
//    with the following meanings:
//    TIME          : Parameters of this type reflect a certain
//                    time(span)
//    ENUM          : Parameters of this type have a value with
//                    an enumerated meaning.
//    BITFIELD      : Parameters of this type are bitmap para-
//                    meters.
//    ASCII         : Field contains non-terminated ASCII data.
//    ASCII_0       : Field contains zero-terminated ASCII data.
//                    Field is fixed-sized, but bytes following
//                    the 0-byte are undefined and not checked.
//    ASCII_n       : Field contains <LF> /n-terminated ASCII data.
//                    Field is fixed-sized, but bytes following
//                    the /n-byte are not checked.
//    UNICODE       : Field contains none-terminated UNICODE
//                    data.
//    UNICODE_0     : Field contains zero-terminated UNICODE
//                    data.
//    UNICODE_n     : Field contains <LF> /n-terminated UNICODE
//                    data.
//
//    The meaning of the remaining parameters depends of this
//    string:
//    TIME          : Has one additional parameter, which is
//                    a float. This float indicates the time in
//                    seconds intended of one unit.
//    ENUM          : A list of additional parameters exists.
//                    This list is intended to be self-explaining.
//    BITFIELD      : A list of additional parameters exists.
//                    If the bitfield is denoted as
//                    (0xXXXX,0xXXXX), the first bitfield is
//                    a mask, the second bitfield defines the
//                    bits.


//    First a small list of general purpose types is defined.
//    Default type is interpreted as "Big endian"

t_B1                  = {   1 }
t_B2                  = {   2 }
t_B3                  = {   3 }
t_B4                  = {   4 }
t_B6                  = {   6 }
t_B8                  = {   8 }
t_B16                 = {  16 }

//    The Lofar types:


t_uInt                =  {   4 }
t_Long                =  {   4 }
t_uLong               =  {   4 }
t_StringLen						=	 {   2 }
t_String              = -{ 65535-, ASCII } // (-) indicates little endian, 100- indicates 100 characters or less 
t_doubleArray         = -{ 65535-, ARRAY, 8 } // (-) indicates little endian, 100- indicates 100 characters or less 
t_uint8Array          = -{ 65535-, ARRAY, 1 } // (-) indicates little endian, 100- indicates 100 characters or less 
t_int8Array          	= -{ 65535-, ARRAY, 1 } // (-) indicates little endian, 100- indicates 100 characters or less 
t_int16Array          = -{ 65535-, ARRAY, 2 } // (-) indicates little endian, 100- indicates 100 characters or less	
t_uint16Array         = -{ 65535-, ARRAY, 2 } // (-) indicates little endian, 100- indicates 100 characters or less
t_int32Array          = -{ 65535-, ARRAY, 4 } // (-) indicates little endian, 100- indicates 100 characters or less
t_uint32Array         = -{ 65535-, ARRAY, 4 } // (-) indicates little endian, 100- indicates 100 characters or less
t_int64Array          = -{ 65535-, ARRAY, 8 } // (-) indicates little endian, 100- indicates 100 characters or less
t_uint64Array         = -{ 65535-, ARRAY, 8 } // (-) indicates little endian, 100- indicates 100 characters or less
t_intArray            = -{ 65535-, ARRAY, 4 } // (-) indicates little endian, 100- indicates 100 characters or less

int8 									=  {   1 }
uint8									=  {   1 }
int16									=  {   2 }
uint16								=  {   2 }
int32									=  {   4 }
uint32								=  {   4 }
int64									=  {   8 }
uint64								=  {   8 }
int                   =  {   4 }
time_t                =  {   4 }
timeval               =  {   8 }
double              	=  {   8 }

// Here are the types from the Lofar template that need conversion!!
[+ (get "prelude") +]


[functions]

[+ FOR event "" +]
F_[+ signal_name +] =
{
  [+ (get "SigNr") +],
  msgLength : t_B4[+ FOR param "" +],[+ event_parm +][+ ENDFOR +]
}
[+ ENDFOR +]

[events]
[+ FOR event "" +]
E_[+ signal_name +] =
{
  [+ (get "SigNr") +],
  msgLength : t_B4[+ FOR param "" +],[+ event_parm +][+ ENDFOR +]
}
[+ ENDFOR +]
