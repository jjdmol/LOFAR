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
         LCS \
         DMI \
         BaseSim \
	 CEP/CEPFrame \
         CEP/Tools/PerfTest/P2Perf \
         CEP/CPA/OCTOPUSSY \
         CEP/CPA/OCTOGlish \
         CEP/CPA/VisCube \
         CEP/CPA/GSM \
         CEP/CPA/PSS3/MNS \
         CEP/CPA/PSS3/CAL \
	 CEP/CPA/PSS4 \
         CEP/CPA/AppAgent 
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
DOCPACKAGES = $(addprefix ./,$(PACKAGES))

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
# daily build without installation
#
daily:
	make build_daily WITH_INSTALL=0

#
# daily build with installation
#
daily_install:
	make build_daily WITH_INSTALL=1

#
# Target reached via daily or daily_install
# For all packages do:
#       - Bootstrap. Bootstrapping is only needed once,
#       - Configure. Configuration is needed for each variant.
#	- Compile for each variant
#       - Run make check for each variant
#       - if WITH_INSTALL == 1 install for each variant
#
build_weekly: 
	@date;\
	echo && echo ":::::: DAILY BUILD START" && echo; \
	for pkg in $(PACKAGES); do \
	  if test -d $$pkg ; then \
	    ( echo \
	      && echo ":::::: BOOTSTRAPPING $$pkg" \
	      && echo \
	      && ( ( cd $$pkg && ( ./bootstrap; )) \
		  || echo ":::::: ERROR" ) \
	      && echo \
	      && echo ":::::: DONE BOOTSTRAPPING $$pkg" \
	      && echo ); \
	      for var in $(VARIANTS); do \
	        echo ":::::: VARIANT $$var"; \
		case $$var in gnu3*) inst_var=gcc3;; \
	                      gnu*)  inst_var=gcc2;; \
	                      icc*)  inst_var=icc;; \
	        esac; \
	        (echo \
		&& echo ":::::: CONFIGURING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& ((cd $$pkg/build/$$var \
		&& $(LOFARDIR)/autoconf_share/lofarconf --prefix=/data/LOFAR/installed/$$inst_var ) \
	   	  || echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED CONFIGURING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo ; ); \
		(echo \
		&& echo ":::::: BUILDING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( cd $$pkg/build/$$var \
		&& make -k $(MAKE_OPTIONS) `cat makeoptions` ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED BUILDING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo ;); \
		( echo \
		&& echo ":::::: CHECKING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& cd $$pkg/build/$$var \
		&& (make -k $(MAKE_OPTIONS) `cat makeoptions` check \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED CHECKING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo ;); \
		if test 1 -eq $$WITH_INSTALL; then \
		  ( echo \
		  && echo ":::::: INSTALLING VARIANT $$var FOR PACKAGE $$pkg" \
		  && echo \
		  && date \
		  && (( cd $$pkg/build/$$var \
		  && make install ) \
			  || echo ":::::: ERROR" ) \
		  && echo \
		  && echo ":::::: FINISHED INSTALLING VARIANT $$var FOR PACKAGE $$pkg" \
		  && echo ; ) \
		fi \
	      done; \
	  else \
	    echo ":::::: ERROR $$pkg does not exist"; \
	  fi \
	done; \
	echo && echo ":::::: DAILY BUILD COMPLETE" && echo; \
	date;


#
# weekly build without installation
#
weekly:
	make build_weekly WITH_INSTALL=0

#
# weekly build with installation
#
weekly_install:
	make build_weekly WITH_INSTALL=1

#
# Target reached via weekly or weekly_install
# For all packages do:
#       - Bootstrap. Bootstrapping is only needed once,
#	- Remove previous variant build directory
#	- Recreate variant build directory
#	- Configure package for variant
#	- Compile variant
#       - Run make check for each variant
#       - if WITH_INSTALL == 1 install for each variant
#
build_weekly: 
	@date;\
	echo && echo ":::::: WEEKLY BUILD START" && echo; \
	for pkg in $(PACKAGES); do \
	  if test -d $$pkg ; then \
	    ( echo \
	      && echo ":::::: BOOTSTRAPPING $$pkg" \
	      && echo \
	      && ( ( cd $$pkg && ( ./bootstrap; )) \
		  || echo ":::::: ERROR" ) \
	      && echo \
	      && echo ":::::: DONE BOOTSTRAPPING $$pkg" \
	      && echo ); \
	      for var in $(VARIANTS); do \
	        echo ":::::: VARIANT $$var"; \
		case $$var in gnu3*) inst_var=gcc3;; \
	                      gnu*)  inst_var=gcc2;; \
	                      icc*)  inst_var=icc;; \
	        esac; \
	        (echo \
		&& echo ":::::: CONFIGURING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( $(RM) -rf $$pkg/build/$$var \
		&& mkdir -p $$pkg/build/$$var \
		&& cd $$pkg/build/$$var \
		&& $(LOFARDIR)/autoconf_share/lofarconf --prefix=/data/LOFAR/installed/$$inst_var ) \
	   	  || echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED CONFIGURING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo ; ); \
		(echo \
		&& echo ":::::: BUILDING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( cd $$pkg/build/$$var \
		&& make -k $(MAKE_OPTIONS) `cat makeoptions` ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED BUILDING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo ;); \
		( echo \
		&& echo ":::::: CHECKING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& cd $$pkg/build/$$var \
		&& (make -k $(MAKE_OPTIONS) `cat makeoptions` check \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED CHECKING VARIANT $$var FOR PACKAGE $$pkg" \
		&& echo ;); \
		if test 1 -eq $$WITH_INSTALL; then \
		  ( echo \
		  && echo ":::::: INSTALLING VARIANT $$var FOR PACKAGE $$pkg" \
		  && echo \
		  && date \
		  && (( cd $$pkg/build/$$var \
		  && make install ) \
			  || echo ":::::: ERROR" ) \
		  && echo \
		  && echo ":::::: FINISHED INSTALLING VARIANT $$var FOR PACKAGE $$pkg" \
		  && echo ; ) \
		fi \
	      done; \
	  else \
	    echo ":::::: ERROR $$pkg does not exist"; \
	  fi \
	done; \
	echo && echo ":::::: WEEKLY BUILD COMPLETE" && echo; \
	date;

#
# build: Target to compile in a bootstrapped and configured tree
#
build: start_build $(VARIANTNAMES:.variant=.variant_build) stop_build

#
# install: Target to install a buildtree
#
install: start_install $(VARIANTNAMES:.variant=.variant_install) stop_install

#
# rebuild: Target to compile from scratch in a bootstrapped and configured tree
#
rebuild: start_rebuild $(VARIANTNAMES:.variant=.variant_rebuild) stop_rebuild

start_build:
	@echo && echo ":::::: BUILD START" && echo

start_install:
	@echo && echo ":::::: INSTALL START" && echo

start_rebuild:
	@echo && echo ":::::: REBUILD START" && echo

start_daily:
	@echo && echo ":::::: DAILY BUILD START" && echo

start_weekly_build:
	@echo && echo ":::::: WEEKLY BUILD START" && echo

start_weekly_install:
	@echo && echo ":::::: WEEKLY INSTALL START" && echo

stop_build:
	@echo && echo ":::::: BUILD COMPLETE" && echo

stop_install:
	@echo && echo ":::::: INSTALL COMPLETE" && echo

stop_rebuild:
	@echo && echo ":::::: REBUILD COMPLETE" && echo

stop_daily:
	@echo && echo ":::::: DAILY BUILD COMPLETE" && echo

stop_weekly_build:
	@echo && echo ":::::: WEEKLY BUILD COMPLETE" && echo

stop_weekly_install:
	@echo && echo ":::::: WEEKLY INSTALL COMPLETE" && echo

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
		&& ( ( cd $$pkg && ( ./bootstrap; )) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: DONE BOOTSTRAPPING $$pkg" \
		&& echo ) ; \
	    else \
	        echo ":::::: ERROR $$pkg does not exist"; \
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
	variant=`basename $@ .variant_configure`; \
	case $$variant in gnu3*) inst_var=gcc3;; \
	                  gnu*)  inst_var=gcc2;; \
	                  icc*)  inst_var=icc;; \
	esac; \
	(echo \
	&& echo ":::::: CONFIGURING VARIANT $$variant FOR PACKAGE $$pkg" \
	&& echo \
	&& date \
	&& (( $(RM) -rf $$pkg/build/$$variant \
	&& mkdir -p $$pkg/build/$$variant \
	&& cd $$pkg/build/$$variant \
	&& $(LOFARDIR)/autoconf_share/lofarconf --prefix=/data/LOFAR/installed/$$inst_var ) \
	   || echo ":::::: ERROR" ) \
	&& echo \
	&& echo ":::::: FINISHED CONFIGURING VARIANT $$variant FOR PACKAGE $$pkg" \
	&& echo ; )

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
	    else \
	        echo ":::::: ERROR $$pkg does not exist"; \
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
	    else \
	        echo ":::::: ERROR $$pkg does not exist"; \
	    fi\
	done; \
	date

#
# Rule to install build
#
%.variant_install:
	@date; \
	variant=`basename $@ .variant_install`; \
	for pkg in $(PACKAGES); do \
	    if test -d $$pkg; then \
		( echo \
		&& echo ":::::: INSTALLING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo \
		&& date \
		&& (( cd $$pkg/build/$$variant \
		&& make install ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: FINISHED INSTALLING VARIANT $$variant FOR PACKAGE $$pkg" \
		&& echo ; ) \
	    else \
	        echo ":::::: ERROR $$pkg does not exist"; \
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
	    else \
	        echo ":::::: ERROR $$pkg does not exist"; \
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
	    else \
	        echo ":::::: ERROR $$pkg does not exist"; \
	    fi\
	done; \
	date


# Rules for building documentation using doxygen or doc++.
# Default output directory is docxxhtml.
DOCDIR := /data/LOFAR/installed/docxxhtml
DOXYGEN := /usr/bin/doxygen

docxx:
	@cp autoconf_share/doxygen.cfg doxygen.cfg ; \
	echo "PROJECT_NAME = LOFAR" >> doxygen.cfg ; \
	echo "INPUT = $(DOCPACKAGES)" >> doxygen.cfg ; \
	echo "RECURSIVE = YES" >> doxygen.cfg ; \
	echo "HTML_OUTPUT = $(DOCDIR)" >> doxygen.cfg ; \
	echo "EXCLUDE = build test" >> doxygen.cfg; \
	$(DOXYGEN) doxygen.cfg ; \


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
DEFAULT_FILES_TO_TAG=Makefile Versions autoconf_share LCS/Common
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
	while test "x$$rem" != "x" -o "x$$tagname" = "x"; do\
		rem='';\
		read -e -p"=> " tagname rem;\
		if test "x$$rem" != "x"; then\
			echo "ERROR: branch tag name should not contain spaces or tabs.";\
		else if test "x$$tagname" = "x"; then\
			echo "ERROR: no tag name specified";\
			rem='error';\
		fi\
		fi\
	done;\
	echo "Release branch tag name set to: '$$tagname'"; \
	echo ;\
	rem='empty';\
	echo "** Release test directory:";\
	while test "x$$rem" != "x" -o "x$$testdir" = "x"; do\
		rem='';\
		read -e -p"=> " testdir rem;\
		if test "x$$rem" != "x"; then\
			echo "ERROR: test dir should be single directory name.";\
			rem='error';\
		else if test ! -d $$testdir; then\
			echo "INFO: creating directory $$testdir";\
			rem='';\
			mkdir $$testdir;\
			if test ! -d $$testdir; then\
				echo "ERROR: failed to create directory $$testdir";\
				rem='error';\
			fi;\
		else if test -e $$testdir/LOFAR; then\
			echo -n "ERROR: There is already a $$testdir/LOFAR file/directory. ";\
			echo -e "Please remove it or specify a different directory.";\
			rem='error';\
		else if test "x$$testdir" = "x"; then\
			echo "ERROR: no directory specified";\
			rem='error';\
		fi\
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
	cvs tag -bF $$tagname $(DEFAULT_FILES_TO_TAG) $$files > tag.log 2>&1 ;\
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

#
# Create a new tag
#
DEFAULT_FILES_TO_TAG=Makefile Versions autoconf_share LCS/Common
tag:
	@trap "rm -f /tmp/inputrc.$$PPID; exit" SIGINT;\
	echo "TAB: complete-filename" > /tmp/inputrc.$$PPID;\
	echo "set expand-tilde On" >> /tmp/inputrc.$$PPID;\
	INPUTRC=/tmp/inputrc.$$PPID;\
	echo "About to create a new tag."; \
	echo ;\
	echo "You will be asked for a TAG name (e.g. profile_1)"; \
	echo "and for a list of files/directories to be tagged, and for a"; \
	echo "directory location where the tagged files should be tested. The tagged";\
	echo "files will be checked out to that location for you to test the tagged files.";\
	echo ;\
	rem='empty';\
	echo "** Tag name:";\
	while test "x$$rem" != "x" -o "x$$tagname" = "x"; do\
		rem='';\
		read -e -p"=> " tagname rem;\
		if test "x$$rem" != "x"; then\
			echo "ERROR: tag name should not contain spaces or tabs.";\
		else if test "x$$tagname" = "x"; then\
			echo "ERROR: no tag name specified";\
			rem='error';\
		fi\
		fi\
	done;\
	echo "Tag name set to: '$$tagname'"; \
	echo ;\
	rem='empty';\
	echo "** Tagged files test directory:";\
	while test "x$$rem" != "x" -o "x$$testdir" = "x"; do\
		rem='';\
		read -e -p"=> " testdir rem;\
		if test "x$$rem" != "x"; then\
			echo "ERROR: test dir should be single directory name.";\
			rem='error';\
		else if test ! -d $$testdir; then\
			echo "INFO: creating directory $$testdir";\
			rem='';\
			mkdir $$testdir;\
			if test ! -d $$testdir; then\
				echo "ERROR: failed to create directory $$testdir";\
				rem='error';\
			fi;\
		else if test -e $$testdir/LOFAR; then\
			echo -n "ERROR: There is already a $$testdir/LOFAR file/directory. ";\
			echo -e "Please remove it or specify a different directory.";\
			rem='error';\
		else if test "x$$testdir" = "x"; then\
			echo "ERROR: no directory specified";\
			rem='error';\
		fi\
		fi\
		fi\
		fi\
	done;\
	echo "Test directory set to: '$$testdir'";\
	echo ;\
	files='';\
	echo -e "** Specify the files and directories to be tagged.";\
	echo -e "   Each file/dir separated by a space.";\
	echo -e "   These files and directories are always tagged:";\
	echo -e "   Directories are tagged recursively.";\
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
	cvs tag -F $$tagname $(DEFAULT_FILES_TO_TAG) $$files > tag.log 2>&1 ;\
	if test ! 0 -eq $$?; then\
		echo "FAILURE: failed to tag, see tag.log for information.";\
		exit;\
	fi;\
	echo " DONE.";\
	lofardir=$(PWD);\
	echo -n "CHECKOUT TO $$testdir (output to checkout.log) ...";\
	cd $$testdir;\
	cvs co -r $$tagname LOFAR > checkout.log 2>&1 ;\
	echo " DONE.";\
	echo -n "CREATING $$testdir/LOFAR/ChangeLog (output to cvs2cl.log) ...";\
	$$lofardir/autoconf_share/cvs2cl.pl --stdout > LOFAR/ChangeLog 2> cvs2cl.log;\
	echo " DONE.";\
	find $$testdir/LOFAR -name CVS | xargs rm -rf;\

_force_data_sync_:
	@echo "Synchronizing files:"; \
	echo -e "\tfrom '$(USER)@lofar9:/home/lofar/data'"; \
	echo -e "\t  to '$(PWD)'"; \
	rsync --rsh=ssh -rv $(USER)@lofar9:/home/lofar/data $(PWD)

data: _force_data_sync_

