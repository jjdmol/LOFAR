#
# Rules for autogen code generation
#
AUTOGEN = autogen
SUFFIXES = .ph
%.ph: %.prot
	$(AUTOGEN) -L $(top_srcdir)/autogen $<
	$(chmod +w $<)

BUILT_SOURCES = \
	F_Supervisory_Protocol.ph

DOCHDRS	=	\
GPI_Controller.hxx GPI_Resources.hxx

bin_PROGRAMS		= PropertyInterface
PropertyInterface_SOURCES	= $(DOCHDRS) \
  GPI_Main.cxx \
  GPI_Controller.cxx  \
  GPI_Resources.cxx

PropertyInterface_LDADD	    =

PropertyInterface_DEPENDENCIES =

include $(top_srcdir)/Makefile.common

