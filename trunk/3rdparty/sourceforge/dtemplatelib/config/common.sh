#!/bin/sh
# usage: build.sh [targets]
# this file should be located under config

BUILD=debug # default

MSVC_PROFILE=""
RUN_ARGS=""
ARGS="$@"
set -- `getopt hb:p:r: $*`

export ARGS BUILD PLATFORM TARGET MSVC_PROFILE RUN_ARGS 
set_platform() {
    if test -z "$PLATFORM"
    then
	platform=`uname`
	case "$platform" in
	    CYGWIN* ) PLATFORM=cygwin-i686-gcc ;;
	    HP-UX ) PLATFORM=hpux-hppa-aCC ;;
	    Linux ) PLATFORM=linux-i686-gcc ;;
	    SunOS ) PLATFORM=solaris-sparc-CC ;;
	    * ) echo ZNCompile Error: unknown platform $platform >&2 ; exit 1 ;;
	esac
    fi

    case "$BUILD" in
	boundschecker* )
	    case "$PLATFORM" in
		*msvc ) ;;
		cygwin* ) echo ZNCompile Warning: switching to MSVC build for BoundsChecker ; PLATFORM=cygwin-i686-msvc ;;
		* ) echo ZNCompile Error: no boundschecker build for $PLATFORM >&2 ; exit 2 ;;
	    esac ;;
	profile* )
	    case "$PLATFORM" in
		*msvc ) MSVC_PROFILE=true; BUILD=debug ;; #For a profile just do a debug build as the base
	    esac ;;
    esac

    #if platform is MSVC transform target of the form libXYZZY.a to XYZZY.lib
    case "$PLATFORM" in
		*msvc ) 
			case "$TARGET" in
			  lib*.a ) TARGET=`echo $TARGET | sed 's/lib//' | sed 's/\.a/.lib/'`; echo "Changed target to '$TARGET'";;
			esac;;
    esac
}

help() {
    echo "usage: $0 [-h] [-b BUILD] [-p PLATFORM] [-r RUN_ARGUMENTS] [make-target(s)]"
    echo "The build-target is '$TARGET'."
    echo "The default BUILD is '$BUILD'."
    set_platform
    echo "The default PLATFORM is '$PLATFORM'."
    echo "The default make-target is 'all'."
    echo "The RUN_ARGUMENTS are only used if you are performing an MSVC profile run,"
    echo "i.e. '-b profile' and '-p cygwin-i686-msvc'.  In this case, the program will be built as 'debug'"
    echo "and run with the RUN_ARGUMENTS you provide."
    echo "The config arguments are to build with 'unicode' and 'stlport' and are only set up for MSVC."
    echo "See config\cygwin-i686-msvc.inc for details on these configurations."
    exit
}

while test $# -gt 0
do
    case $1 in
	-h ) help ;;
	-b ) BUILD=$2 ; shift 2 ;;
	-p ) PLATFORM=$2 ; shift 2 ;;
	-r ) RUN_ARGS="$RUN_ARGS $2" ; shift 2 ;;
	-- ) shift ; break ;;
	* ) break ;;
    esac
done

# If run without arguments, default to make targets "all"
test $# -eq 0 && set - all

set_platform

case "$PLATFORM" in
		# Make sure /usr/bin is at the beginning of the path for MSVC
		*msvc ) echo "MAKE: make $*"; env PATH="/usr/bin:$PATH" make -f ../config/common.mak "$*" ;;

		#otherwise just run a regular make
		*) make -f ../config/common.mak "$*" ;;
esac

if test -n "$MSVC_PROFILE"; then
       cd $BUILD
	 prep /om /ft /exc nafxcwd.lib $TARGET 
	 echo profile $TARGET $RUN_ARGS
       profile $TARGET $RUN_ARGS
       prep /m $TARGET 
       plist /st $TARGET 
fi

