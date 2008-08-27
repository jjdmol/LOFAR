configfilesdir=$(bindir)
configfiles_DATA= \
	VB1.param
	
%.param: %.param.in
	cp $< $@

EXTRA_DIST = $(configfiles_DATA)

include $(top_srcdir)/Makefile.common

