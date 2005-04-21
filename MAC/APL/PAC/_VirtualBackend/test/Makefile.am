INSTALL=/usr/bin/install -C

configfilesdir=$(bindir)
configfiles_DATA= \
	VB1.param
	
%.param: %.param.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common

