INSTHDRS =

NOINSTHDRS =

TCCHDRS =

nobase_pkginclude_HEADERS = $(INSTHDRS) $(TCCHDRS)

noinst_HEADERS = $(NOINSTHDRS)

DOCHDRS = $(INSTHDRS) $(NOINSTHDRS)

#lib_LTLIBRARIES         = libtools.la

#libtools_la_SOURCES =

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

# Install lofarinit without execute permissions.
# They have to be sourced; fill in the correct path. 
install-data-local:
	for FIL in lofarinit.sh lofarinit.csh; do \
	  echo "FIL=$$FIL $(srcdir) $(prefix)"; \
	  sed -e "s%\. #filled in by install%$(prefix)%" $(srcdir)/$$FIL > $$FIL.tmp; \
	  $(INSTALL_DATA) $$FIL.tmp $(prefix)/$$FIL; \
	done

scriptdir = $(bindir)
dist_script_SCRIPTS = 

include $(lofar_sharedir)/Makefile.common
