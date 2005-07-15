[+ AutoGen5 template tseprot +]
[+ (out-push-add "/dev/null") +]
[+ DEFINE prefix_cap +][+ IF (exist? "prefix") +][+ (get "prefix") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE prefix_ucase +][+ IF (exist? "prefix") +][+ (string-upcase (get "prefix")) +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE protocol_id +][+ IF (exist? "id") +][+ (get "id") +][+ ENDIF +][+ ENDDEF +]
[+ DEFINE signal_name +][+ prefix_ucase +]_[+ (get "signal") +][+ ENDDEF +]
[+ DEFINE cap_signal +][+ (string-substitute (string-capitalize! (get "signal")) '( "_" )' ( "" )) +][+ ENDDEF +]
[+ DEFINE protocol_name +][+ (string-upcase (base-name)) +][+ ENDDEF +]
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

t_Double              =  {   4 }
t_Int                 =  {   4 }
t_uInt                =  {   4 }
t_Long                =  {   4 }
t_uLong               =  {   4 }
t_String              = -{ 100-, ASCII } // (-) indicates little endian

// Example type remove this when you converted the actual Lofar Enums!
t_WrRegBitField       = { 1, 0x00, 0xFF, BITFIELD,
                             0x01 : "Use CTRL",
                             0x02 : "Unspecified",
                             0x04 : "Unspecified",
                             0x08 : "Unspecified",
                             0x10 : "Unspecified",
                             0x20 : "Unspecified",
                             0x40 : "Unspecified",
                             0x80 : "Unspecified"
                       }
                       
// Here are the types from the Lofar template that need conversion!!
[+ (get "prelude") +]


[functions]

// Note: For each function remove the last ',' before the '}'  !!!!
[+ FOR event "" +]
[+ signal_name +] =
{
  [+ (get "SigNr") +],
  [+ FOR param "" +][+ (get "name") +] : [+ CASE (get "type") +][+ == unsigned int +]t_uInt[+ == int +]t_Int[+ == double +]t_Double[+ == long +]t_Long[+ == string +]t_String[+ * +][+ (get "type") +][+ ESAC +],
  [+ ENDFOR +]
}
[+ ENDFOR +]

[events]
