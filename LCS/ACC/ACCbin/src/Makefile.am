
bin_PROGRAMS		= ApplController ACDaemon ACcli ACuserMenu

dist_bin_SCRIPTS	= startAP.sh startBGL.sh startMPI.sh \
			  stopAP.sh stopBGL.sh stopMPI.sh

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
ApplController_DEPENDENCIES = $(LOFAR_DEPEND) 
ApplController_LDADD	= 

### ACDaemon ###
ACDaemon_SOURCES	= ACRequestPool.cc \
			  ACDaemon.cc \
			  ACDaemonMain.cc
ACDaemon_DEPENDENCIES	= $(LOFAR_DEPEND)
ACDaemon_LDADD       	=


### ACcli ###
ACcli_SOURCES		= ACcli.cc
ACcli_DEPENDENCIES	= $(LOFAR_DEPEND)
ACcli_LDADD		=

### ACuserMenu ###
ACuserMenu_SOURCES	= ACuserMenu.cc myACClientFunctions.h
ACuserMenu_DEPENDENCIES	= $(LOFAR_DEPEND) 
ACuserMenu_LDADD	= 

include $(top_srcdir)/Makefile.common
