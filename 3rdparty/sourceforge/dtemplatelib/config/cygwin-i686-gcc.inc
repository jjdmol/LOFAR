# Written for GNU -*- makefile -*-.

include ../config/gcc.inc
MAKEFILEDEPS += ../config/gcc.inc

CC += -march=i686 -mno-cygwin 
LDLIBS += -lodbc32 -lodbccp32

# Windows extensions for executables
EXE = .exe
