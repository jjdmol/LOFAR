#
# Toplevel makefile for the LOFAR project.
#
# This Makefile will build all applicable variants for all packages
# and it is used to perform daily and weekly builds.
#
# All packages will be configured and built for various combinations
# op configure options. Each combination is a separate variant.
#
# Variant Name		Variant options
#
# Simple variants
# debug			no options
# opt			--with-optimize
# doxygen		--with-doxygen
# mpich			--with-mpich
# mpich-prof		--with-mpich --enable-mpi-profiler
# scampi		--with-scampi
# corba			--with-vbroker
# xml			--with-xerces
# matlab		--with-matlab
# aipspp		--with-aipspp
# insure		--with-insure
# 
# Combined variants
# mpich_corba		--with-corba --with-mpich
#


#
# Packages with a configure.in will be built (excluding the import stuff).
# It kan be overridden on the make line.
# Note that PACKAGE and PACKAGES are synonyms.
#
PACKAGE = \
         Common \
         DMI \
         BaseSim \
         LOFARSim \
         StationSim \
	 LCS/CRD/Coord \
	 LCS/CRD/CoordServer \
         CEP/Tools/PerfTest/P2Perf \
         CEP/CPA/OCTOPUSSY \
         CEP/CPA/UVD \
         CEP/CPA/uvplot \
         CEP/CPA/MNS \
         CEP/CPA/GSM \
         CEP/CPA/CAL
PACKAGES = $(PACKAGE)
#

# Find all variants to be built for this host (without possible domain).
# There can be multiple lines (one for each compiler).
# Prepend each variant with the compiler type and append .variant.
# The VARIANTS variable can be overridden on the make line.
# Note that VARIANT is a synonym for VARIANTS.
#
HOST = $(shell uname -n | sed -e "s%\..*%%")
VARLINES = $(shell if [ -f ../builds.$(HOST) ]; then egrep "^make\..*\.variants:" ../builds.$(HOST) | sed -e "s%^make\.%%" -e "s%.variants:%/%" -e "s/ \+//g"; else echo "gnu/opt"; fi)
VARIANT = $(shell for NM in $(VARLINES); do cmp=`echo $$NM | sed -e "s%/.*%%"`; vars=`echo $$NM | sed -e "s%.*/%%" -e "s%,% %g"`; for VAR in $$vars; do echo $${cmp}_$$VAR; done; done)
VARIANTS = $(VARIANT)
VARIANTNAMES = $(addsuffix .variant, $(VARIANTS))
#
# Keep the make directory (which is LOFAR).
#
LOFARDIR = $(shell pwd)

#
# Standard make options used for every invokation of make
#
MAKE_OPTIONS = -j2

#
# Define defaults for crontab variables.
#
CRONUSER = $$USER
CRONMAILTO = $(CRONUSER)@astron.nl
CRONCVSUSER = $(CRONUSER)
CRONROOT = $$HOME

#
# all: Target to compile the specified variants of all packages
#
all: build

#
# daily,weekly: Target to drive daily and weekly builds
#
daily: start_daily bootstrap configure $(VARIANTNAMES) check stop_daily

weekly: start_weekly bootstrap configure $(VARIANTNAMES) check stop_weekly

#
# build: Target to compile in a bootstrapped and configured tree
#
build: start_build $(VARIANTNAMES:.variant=.variant_build) stop_build

#
# rebuild: Target to compile from scratch in a bootstrapped and configured tree
#
rebuild: start_rebuild $(VARIANTNAMES:.variant=.variant_rebuild) stop_rebuild

start_build:
	@echo && echo ":::::: BUILD START" && echo

start_rebuild:
	@echo && echo ":::::: REBUILD START" && echo

start_daily:
	@echo && echo ":::::: DAILY BUILD START" && echo

start_weekly:
	@echo && echo ":::::: WEEKLY BUILD START" && echo

stop_build:
	@echo && echo ":::::: BUILD COMPLETE" && echo

stop_rebuild:
	@echo && echo ":::::: REBUILD COMPLETE" && echo

stop_daily:
	@echo && echo ":::::: DAILY BUILD COMPLETE" && echo

stop_weekly:
	@echo && echo ":::::: WEEKLY BUILD COMPLETE" && echo

#
# check target: check all variants that have been specified in the
#               VARIANTNAMES variable
#
check: $(VARIANTNAMES:.variant=.variant_check)

#
# Bootstrap and configure all packages. Bootstrapping is only needed once,
# configuration is needed for each variant
#
bootstrap: bootstrap_rule

bootstrap_rule:
	@for pkg in $(PACKAGES); do \
	    if test -d $$pkg ; then \
		( echo \
		&& echo ":::::: BOOTSTRAPPING $$pkg" \
		&& echo \
		&& ( ( cd $$pkg && ( ./bootstrap; ./bootstrap )) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: DONE BOOTSTRAPPING $$pkg" \
		&& echo ) ; \
	    fi \
	done

#
# Configure all packages. Configuration is needed for each variant.
#
configure: $(VARIANTNAMES:.variant=.variant_configure)

#
# Rule to configure variants
#	- Configure package for variant
#
%.variant_configure:
	@date; \
	variant=`basename $@ .variant_configure`; \
	for pkg in $(PACKAGES); do \
	    if test -d $$pkg; then \
		( echo \
		&& echo ":::::: CONFIGURING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( $(RM) -rf $$pkg/build/$$variant \
		&& mkdir -p $$pkg/build/$$variant \
		&& cd $$pkg/build/$$variant \
		&& $(LOFARDIR)/autoconf_share/lofarconf ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED CONFIGURING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo ; ) \
	    fi\
	done; \
	date

#
# Rule to build variant for daily or weekly build
#	- Remove previous variant build directory
#	- Recreate variant build directory
#	- Configure package for variant
#	- Compile variant
#
%.variant:
	@date; \
	variant=`basename $@ .variant`; \
	for pkg in $(PACKAGES); do \
	    if test -d $$pkg; then \
		( echo \
		&& echo ":::::: BUILDING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( cd $$pkg/build/$$variant \
		&& make -k $(MAKE_OPTIONS) `cat makeoptions` ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED BUILDING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo ; ) \
	    fi\
	done; \
	date

#
# Rule to build variant 
#	- Compile variant
#
%.variant_build:
	@date; \
	variant=`basename $@ .variant_build`; \
	for pkg in $(PACKAGES); do \
	    if test -d $$pkg; then \
		( echo \
		&& echo ":::::: BUILDING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( cd $$pkg/build/$$variant \
		&& make -k $(MAKE_OPTIONS) `cat makeoptions` ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED BUILDING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo ; ) \
	    fi\
	done; \
	date

#
# Rule to rebuild variant 
#	- Clean variant
#	- Compile variant
#
%.variant_rebuild:
	@date; \
	variant=`basename $@ .variant_rebuild`; \
	for pkg in $(PACKAGES); do \
	    if test -d $$pkg; then \
		( echo \
		&& echo ":::::: REBUILDING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& ((cd $$pkg/build/$$variant \
		&& make -k clean $(MAKE_OPTIONS) `cat makeoptions` \
		&& make -k $(MAKE_OPTIONS) `cat makeoptions` ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED REBUILDING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo ; ) \
	    fi\
	done; \
	date

#
# Rule to check variant
#	- Run make check for each variant
#
%.variant_check:
	@date; \
	variant=`basename $@ .variant_check`; \
	for pkg in $(PACKAGES); do \
	    if test -d $$pkg; then \
		( echo \
		&& echo ":::::: CHECKING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& cd $$pkg/build/$$variant \
		&& (make -k $(MAKE_OPTIONS) `cat makeoptions` check \
			|| echo ":::::: ERROR" )) \
		&& echo \
		&& echo ":::::: FINISHED CHECKING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo ; \
	    fi\
	done; \
	date

#
# Install crontab to run daily and weekly builds;
# crontab is read from autoconf_share/crontab.builds
#
crontab:
	@pidnr=$$$$; \
	if [ -d $(CRONROOT)/weekly -o -d $(CRONROOT)/daily ]; then \
	  echo "Error: LOFAR build directories daily and weekly will be used,"; \
	  echo "       but one or both already exist in $(CRONROOT)"; \
	  echo " Delete them and redo 'make crontab' if you really want to"; \
	  exit 2; \
	fi; \
	crontab -l > $$HOME/crontab-$$pidnr-old; \
	sed -e "s%LOFAR_CRONMAILTO%$(CRONMAILTO)%" -e "s%LOFAR_CRONROOT%$(CRONROOT)%" -e "s%LOFAR_CRONCVSUSER%$(CRONCVSUSER)%" autoconf_share/crontab.builds > $$HOME/crontab-$$pidnr-new; \
	if (diff $$HOME/crontab-$$pidnr-old $$HOME/crontab-$$pidnr-new > /dev/null); then \
	  $(RM) $$HOME/crontab-$$pidnr-old; \
	else \
	  echo "New crontab will be created; root=$(CRONROOT)"; \
	  echo " its output is sent to $(CRONMAILTO)"; \
	  echo " existing crontab is saved in $$HOME/crontab-$$pidnr-old"; \
	fi; \
	crontab $$HOME/crontab-$$pidnr-new; \
	$(RM) $$HOME/crontab-$$pidnr-new;

#
# Show the variables
#
show:
	@echo ""
	@echo "PACKAGES = $(PACKAGES)"
	@echo "VARIANTS = $(VARIANTS)"
	@echo "LOFARDIR = $(LOFARDIR)"
	@echo ""
	@echo "CRONUSER    = $(CRONUSER)"
	@echo "CRONMAILTO  = $(CRONMAILTO)"
	@echo "CRONCVSUSER = $(CRONCVSUSER)"
	@echo "CRONROOT    = $(CRONROOT)"
	@echo ""

#
# Show the possible targets
#
help:
	@echo "Targets in main LOFAR Makefile"
	@echo "------------------------------"
	@echo ""
	@echo "all:       default target; same as build"
	@echo "bootstrap: bootstrap all packages"
	@echo "configure: configure all packages for all variants"
	@echo "build:     build all packages for all variants"
	@echo "check:     build and run all test programs for all variants"
	@echo ""
	@echo "daily:     bootstrap, configure, build, and check daily system"
	@echo "weekly:    bootstrap, configure, build, and check weekly system"
	@echo ""
	@echo "%.variant_configure: configure all packages for given variant"
	@echo "%.variant_build:     build all packages for given variant"
	@echo "%.variant:           same as %.variant_build"
	@echo "%.variant_check:     build and run all test programs for given variant"
	@echo ""
	@echo "crontab:   create a cron job for daily and weekly system"
	@echo "show:      show the important variables"
	@echo ""

#
# Create a new release
#
DEFAULT_FILES_TO_TAG=Makefile Versions autoconf_share Common
release:
	@trap "rm -f /tmp/inputrc.$$PPID; exit" SIGINT;\
	echo "TAB: complete-filename" > /tmp/inputrc.$$PPID;\
	echo "set expand-tilde On" >> /tmp/inputrc.$$PPID;\
	INPUTRC=/tmp/inputrc.$$PPID;\
	echo "About to create a new release branch."; \
	echo ;\
	echo "You will be asked for a TAG name for the release (e.g. PSS_1_0)"; \
	echo "and for a list of files/directories to be tagged, and for a"; \
	echo "directory location where the release should be tested. The tagged";\
	echo "files will be checked out to that location for you to test the release.";\
	echo ;\
	rem='empty';\
	echo "** Release branch tag name:";\
	while test "x$$rem" != "x"; do\
		rem='';\
		read -e -p"=> " tagname rem;\
		if test "x$$rem" != "x"; then\
			echo "ERROR: branch tag name should not contain spaces or tabs.";\
		fi\
	done;\
	echo "Release branch tag name set to: '$$tagname'"; \
	echo ;\
	rem='empty';\
	echo "** Release test directory:";\
	while test "x$$rem" != "x"; do\
		rem='';\
		read -e -p"=> " testdir rem;\
		if test "x$$rem" != "x"; then\
			echo "ERROR: test dir should be single directory name.";\
			rem='error';\
		else if test ! -d $$testdir; then\
			echo "INFO: creating directory $$testdir";\
			mkdir $$testdir;\
			rem='';\
		else if test -e $$testdir/LOFAR; then\
			echo -n "ERROR: There is already a $$testdir/LOFAR file/directory. ";\
			echo -e "Please remove it or specify a different directory.";\
			rem='error';\
		fi\
		fi\
		fi\
	done;\
	echo "Test directory set to: '$$testdir'";\
	echo ;\
	files='';\
	echo -e "** Specify the files and directories to be included in the release.";\
	echo -e "   Each file separated by a space.";\
	echo -e "   These files and directories are always tagged:";\
	echo -e "\t\"$(DEFAULT_FILES_TO_TAG)\"";\
	while test "x$$files" = "x"; do\
		read -e -p"=> " files;\
		if test "x$$files" = "x"; then\
			echo "ERROR: no files or directories specified.";\
		fi;\
		for f in $$files; do\
			if test ! -f $$f -a ! -d $$f ; then\
				echo "ERROR: file or directory '$$f' not found.";\
				files='';\
			fi\
		done;\
	done;\
	echo ;\
	echo -e "\tTag:       $$tagname";\
	echo -e "\tTest dir:  $$testdir";\
	for f in $$files; do\
		if test -f $$f; then\
			echo -e "\tFILE:      $$f";\
		else\
		if test -d $$f; then\
			echo -e "\tDIRECTORY: $$f";\
		else\
			echo "ERROR: file or directory '$$f' not found.";\
		fi\
		fi\
	done;\
	rm -f /tmp/inputrc.$$PPID;\
	echo ;\
	echo -n "TAGGING (output to tag.log) ...";\
	cvs tag -bf $$tagname $(DEFAULT_FILES_TO_TAG) $$files > tag.log 2>&1 ;\
	if test ! 0 -eq $$?; then\
		echo "FAILURE: failed to tag, see tag.log for information.";\
		exit;\
	fi;\
	echo " DONE.";\
	lofardir=$(PWD);\
	echo -n "EXPORT TO $$testdir (output to export.log) ...";\
	cd $$testdir;\
	cvs co -r $$tagname LOFAR > export.log 2>&1 ;\
	echo " DONE.";\
	echo -n "CREATING $$testdir/LOFAR/ChangeLog (output to cvs2cl.log) ...";\
	$$lofardir/autoconf_share/cvs2cl.pl --stdout > LOFAR/ChangeLog 2> cvs2cl.log;\
	echo " DONE.";\
	find $$testdir/LOFAR -name CVS | xargs rm -rf;\

