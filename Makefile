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
#
PACKAGES = \
         Common \
         DMI \
         BaseSim \
         LOFARSim \
         CEP/CPA/PSCF \
         CEP/Demo/Platform/RingSim \
         CEP/Tools/PerfTest/P2Perf
#
# Find all variants to be built for this host (without possible domain).
# There can be multiple lines (one for each compiler).
# Prepend each variant with the compiler type and append .variant.
#
HOST = $(shell uname -n | sed -e "s%\..*%%")
VARLINES = $(shell if [ -f ../builds.$(HOST) ]; then egrep "^make\..*\.variants:" ../builds.$(HOST) | sed -e "s%^make\.%%" -e "s%.variants:%/%" -e "s/ \+//g"; else echo "gnu/opt"; fi)
VARIANTS = $(shell for NM in $(VARLINES); do cmp=`echo $$NM | sed -e "s%/.*%%"`; vars=`echo $$NM | sed -e "s%.*/%%" -e "s%,% %g"`; for VAR in $$vars; do echo $${cmp}_$$VAR.variant; done; done)
#
# Keep the make directory (which is LOFAR).
#
LOFARDIR = $(shell pwd)

#
# Standard make options used for every invokation of make
#
MAKE_OPTIONS = -j2

#
# all: Target to compile the specified variants of all packages
#
all: build

#
# daily,weekly: Target to drive daily and weekly builds
#
daily: start_daily bootstrap $(VARIANTS) check stop_daily

weekly: start_weekly bootstrap $(VARIANTS) check stop_weekly

#
# build: Target to compile in a bootstrapped and configured tree
#
build: start_build $(VARIANTS:.variant=.variant_build) stop_build

#
# rebuild: Target to compile from scratch in a bootstrapped and configured tree
#
rebuild: start_rebuild $(VARIANTS:.variant=.variant_rebuild) stop_rebuild

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
#               VARIANTS variable
#
check: $(VARIANTS:.variant=.variant_check)

#
# Bootstrap and configure all packages. Bootstrapping is only needed once,
# configuration is needed for each variant
#
bootstrap: bootstrap_rule $(VARIANTS:.variant=.variant_configure)

bootstrap_rule:
	@for d in $(PACKAGES); do \
		( echo \
		&& echo ":::::: BOOTSTRAPPING $$d" \
		&& echo \
		&& ( ( cd $$d && ( ./bootstrap; ./bootstrap )) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: DONE BOOTSTRAPPING $$d" \
		&& echo ) ; \
	done

#
# Rule to configure variants
#	- Configure package for variant
#
%.variant_configure:
	@date; \
	variant=`basename $@ .variant_configure`; \
	for pkg in $(PACKAGES); do \
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
	done; \
	date

#
# Install crontab to run daily and weekly builds;
# crontab is read from autoconf_share/crontab.builds
#
crontab:
	@crontab -l > $$HOME/crontab_$$$$; \
	if (diff $$HOME/crontab_$$$$ autoconf_share/crontab.builds > /dev/null); then \
	  $(RM) $$HOME/crontab_$$$$; \
	else \
	  echo "New crontab will be created; existing saved in $$HOME/crontab_$$$$"; \
	fi
	crontab autoconf_share/crontab.builds;
