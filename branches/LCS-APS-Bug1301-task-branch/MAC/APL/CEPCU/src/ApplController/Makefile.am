
bin_PROGRAMS		= ApplController ACDaemon ACcli ACuserMenu

dist_bin_SCRIPTS	= startAP.sh startBGL.sh startMPI.sh \
			  		  stopAP.sh stopBGL.sh stopMPI.sh

sysconf_DATA		= ACDaemon.log_prop ACDaemon.conf \
					  ApplController.log_prop \
					  ACuserMenu.log_prop

ACDaemonHEADERS		= ACDaemon.h ACRequestPool.h

ApplControllerHEADERS = ACCmdImpl.h \
						ACDaemonComm.h \
						APAdmin.h \
						APAdminPool.h \
						ApplController.h \
						CmdStack.h \
						ItemList.h \
						ProcRule.h \
						ProcRuler.h \
						PR_MPI.h \
						PR_BGL.h \
						PR_Shell.h \
						StateEngine.h

ACuserMenuHEADERS	= myACClientFunctions.h

noinst_HEADERS 		= $(ACDaemonHEADERS) $(ApplControllerHEADERS) $(ACuserMenuHEADERS)

### ApplController ###
ApplController_SOURCES	= ACCmdImpl.cc \
						  ACDaemonComm.cc \
						  APAdmin.cc \
						  APAdminPool.cc \
						  CmdStack.cc \
						  ItemList.cc \
						  ProcRule.cc \
						  ProcRuler.cc \
						  PR_Shell.cc \
						  PR_MPI.cc \
						  PR_BGL.cc \
						  StateEngine.cc \
						  ApplController.cc \
						  ApplControllerMain.cc
ApplController_DEPENDENCIES = $(LOFAR_DEPEND) $(ApplControllerHEADERS)
ApplController_LDADD	= 

### ACDaemon ###
ACDaemon_SOURCES		= ACRequestPool.cc \
						  ACDaemon.cc \
						  ACDaemonMain.cc
ACDaemon_DEPENDENCIES	= $(LOFAR_DEPEND) $(ACDaemonHEADERS)
ACDaemon_LDADD       	=


### ACcli ###
ACcli_SOURCES			= ACcli.cc
ACcli_DEPENDENCIES		= $(LOFAR_DEPEND)
ACcli_LDADD				=

### ACuserMenu ###
ACuserMenu_SOURCES		= ACuserMenu.cc
ACuserMenu_DEPENDENCIES	= $(LOFAR_DEPEND)  $(ACuserMenuHEADERS)
ACuserMenu_LDADD		= 

include $(top_srcdir)/Makefile.common
