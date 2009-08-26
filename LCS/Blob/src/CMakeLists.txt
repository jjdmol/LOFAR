#  $Id$
#
#  Copyright (C) 2008-2009
#  ASTRON (Netherlands Foundation for Research in Astronomy)
#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

include(LofarPackageVersion)

# Process input files for Flex and Bison
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(KeyParse KeyParse.yy 
  ${CMAKE_CURRENT_BINARY_DIR}/KeyParse.cc
  COMPILE_FLAGS "-y -pKeyParse")

flex_target(KeyTokenize KeyTokenize.ll 
  ${CMAKE_CURRENT_BINARY_DIR}/KeyTokenize.cc
  COMPILE_FLAGS "-PKeyTokenize")

add_flex_bison_dependency(KeyTokenize KeyParse)

# Create symbolic link from KeyParse.h -> KeyParse.hh, because KeyParser.cc
# #include's KeyParse.h.
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  KeyParse.hh
  KeyParse.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# On Apple, bison creates KeyParse.cc.h, instead of KeyParse.hh. So, when on
# Apple, we need to create another symbolic link KeyParse.hh -> KeyParse.cc.h
if(APPLE)
  execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
    KeyParse.cc.h
    KeyParse.hh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif(APPLE)

# Add current binary directory to include, because that's where flex/bison
# will output the files they generate.
include_directories(${CMAKE_CURRENT_BINARY_DIR})

lofar_add_library(blob
  Package__Version.cc
  BlobAipsIO.cc
  BlobArray.cc
  BlobField2.cc
  BlobFieldSet.cc
  BlobHeader.cc
  BlobIBufChar.cc
  BlobIBufStream.cc
  BlobIStream.cc
  BlobOBufChar.cc
  BlobOBufNull.cc
  BlobOBufStream.cc
  BlobOBufString.cc
  BlobOStream.cc
  BlobStreamable.cc
  BlobString.cc
  BlobStringType.cc
  ${BISON_KeyParse_OUTPUTS}
  ${FLEX_KeyTokenize_OUTPUTS}
  KeyParser.cc
  KeyValue.cc
  KeyValueMap.cc)

lofar_add_bin_program(versionblob versionblob.cc)
