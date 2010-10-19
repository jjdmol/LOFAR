SUBDIRS = \
	src \
	test

include $(lofar_sharedir)/Makefile.common

INSTALLDIR=/opt/lofar # $(HOME)/cvs/HEAD/LOFAR/installed/gnu_debug

DISTCHECK_CONFIGURE_FLAGS=\
	--with-common=$(INSTALLDIR)\
	--with-suite=$(INSTALLDIR)\
	--with-gcftm=$(INSTALLDIR)\
	--with-gcfcommon=$(INSTALLDIR) \
	--with-rsp=$(INSTALLDIR)

DISTCLEANFILES=\
	pkgext\
	pkgextcppflags\
	pkgextcxxflags\
	pkgextobjs\
	lofar_config.old-h\
	lofar_config.h-pkg\
	lofar_config.h\
	.doxygenrc

EXTRA_DIST = \
	Makefile.common \
	BeamServer.spec \
	autoconf_share/compiletool

if MAKE_RPMS
rpm: dist @RPM_TARGET@
.PHONY: rpm

$(RPM_TARGET): $(DISTFILES)
	${MAKE} dist
	-mkdir -p $(RPM_DIR)/SRPMS
	-mkdir -p `dirname $(RPM_TARGET)`
	$(RPM_PROG) $(RPM_ARGS) $(RPM_TARBALL)
	@echo Congratulations, $(RPM_TARGET) "(and friends)" should now exist.

else
endif
