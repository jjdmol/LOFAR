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
# mpich_prof		--with-mpich --enable-mpi-profiler
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
# Configuration and make options for each possible variant
#
debug.variant.conf		=
opt.variant.conf		= --with-optimize
mpich.variant.conf		= --with-mpich
corba.variant.conf		= --with-vbroker
mpich_corba.variant.conf	= --with-mpich --with-vbroker
insure.variant.conf		= --with-insure
insure.variant.make		= QA=insure
scampi.variant.conf		= --with-scampi
mpich_prof.variant.conf		= --with-mpich --enable-mpi-profiler

#
# List of variants that will be built and checked
#
VARIANTS = \
	debug.variant \
	opt.variant \
	mpich.variant \
	corba.variant \
	mpich_corba.variant \
	scampi.variant \
	mpich_prof.variant

#
# List of packages for which the above variants will be built and checked
#
PACKAGES = \
	BaseSim \
	LOFARSim \
	DMI \
	CEP/Demo/Platform/RingSim \
	CEP/Tools/PerfTest/P2Perf

#
# Standard make options used for every invokation of make
#
MAKE_OPTIONS = -j2

#
# all target: build all specified variants and check these variants
#
all: start bootstrap $(VARIANTS) check stop

start:
	@echo && echo ":::::: BUILD START" && echo

stop:
	@echo && echo ":::::: BUILD COMPLETE" && echo

#
# check target: check all variants that have been specified in the
#               VARIANTS variable
#
check: $(VARIANTS:.variant=.variant_check)

#
# Bootstrap all packages, only needed once, not for every variant
#
bootstrap:
	@for d in $(PACKAGES); do \
		( echo \
		&& echo ":::::: BOOTSTRAPPING $$d" \
		&& echo \
		&& ( ( cd $$d && ./bootstrap && ./bootstrap ) \
			|| echo ":::::: ERROR" ) \
		&& echo \
		&& echo ":::::: DONE BOOTSTRAPPING $$d" \
		&& echo ) ; \
	done

#
# Rule to build variant
#	- Remove previous variant build directory
#	- Recreate variant build directory
#	- Bootstrap package (twice!)
#	- Configure package for variant
#	- Compile variant
#
%.variant:
	@date; \
	variant=`basename $@ .variant`; \
	variant_conf_options=$($@.conf); \
	variant_make_options=$($@.make); \
	for d in $(PACKAGES); do \
		( echo \
		&& echo ":::::: BUILDING VARIANT $$variant FOR PACKAGE $$d" \
		&& echo ":::::: variant_conf_options = $$variant_conf_options" \
		&& echo ":::::: variant_make_options = $$variant_make_options" \
		&& echo \
		&& date \
		&& $(RM) -rf $$d/build/$$variant \
		&& mkdir -p $$d/build/$$variant \
		&& cd $$d/build/$$variant \
		&& ../../configure $$variant_conf_options \
		&& (make -k $(MAKE_OPTIONS) $$variant_make_options \
			|| echo ":::::: ERROR" )) \
		&& echo \
		&& echo ":::::: FINISHED BUILDING VARIANT $$variant FOR PACKAGE $$d" \
		&& echo ; \
	done; \
	date

#
# Rule to check variant
#	- Run make check for each variant
#
%.variant_check:
	@date; \
	variant=`basename $@ .variant_check`; \
	variant_conf_options=$($@.conf); \
	variant_make_options=$($@.make); \
	for d in $(PACKAGES); do \
		( echo \
		&& echo ":::::: CHECKING VARIANT $$variant FOR PACKAGE $$d" \
		&& echo ":::::: variant_conf_options = $$variant_conf_options" \
		&& echo ":::::: variant_make_options = $$variant_make_options" \
		&& echo \
		&& date \
		&& cd $$d/build/$$variant \
		&& (make -k $(MAKE_OPTIONS) $$variant_make_options check \
			|| echo ":::::: ERROR" )) \
		&& echo \
		&& echo ":::::: FINISHED CHECKING VARIANT $$variant FOR PACKAGE $$d" \
		&& echo ; \
	done; \
	date

#
# Install crontab to run daily and weekly builds;
# crontab is read from autoconf_share/crontab.builds
#
crontab:
	crontab autoconf_share/crontab.builds	
