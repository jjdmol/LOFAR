INSTHDRS =

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

lib_LTLIBRARIES         = libcommon.la

libcommon_la_SOURCES =

bin_PROGRAMS =

glishdir = $(libexecdir)/glish
dist_glish_SCRIPTS = 

pythondir = $(bindir)
dist_python_SCRIPTS = makePackage.py \
		      makeClass.py \
                      makeTest.py

templatedir = $(datadir)/templates
dist_template_DATA  = templates/header.cc_template \
		      templates/header.h_template \
		      templates/main.cc_template \
		      templates/package_bootstrap_template \
		      templates/package_configure.in_template \
		      templates/package_lofarconf.in_template \
		      templates/package_makefile.am_template \
		      templates/src_makefile.am_template \
	              templates/templated_header.cc_template \
		      templates/templated_header.h_template \
		      templates/templated_header.tcc_template \
		      templates/test.cc_template \
		      templates/test.in_template \
		      templates/test.out_template \
		      templates/test.sh_template \
		      templates/test_makefile.am_template


scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(lofar_sharedir)/Makefile.common
