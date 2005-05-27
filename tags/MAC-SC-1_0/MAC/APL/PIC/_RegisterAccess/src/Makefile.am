bin_PROGRAMS 		= RegisterAccess

RegisterAccess_CPPFLAGS= \
  -Wno-deprecated \
	-fmessage-length=0 \
	-fdiagnostics-show-location=once

RegisterAccess_SOURCES 		= \
	ARAMain.cc \
	ARAAnswer.cc \
	ARAPhysicalModel.cc \
	ARARegisterAccessTask.cc

#in case of make install these files will be copied to the bindir beside the test apps
configfilesdir=$(bindir)
configfiles_DATA= \
	RegisterAccess.conf \
	RegisterAccess.log_prop
	
%.log_prop: %.log_prop.in
	cp $< $@
	
%.conf: %.conf.in
	cp $< $@

include $(lofar_sharedir)/Makefile.common
