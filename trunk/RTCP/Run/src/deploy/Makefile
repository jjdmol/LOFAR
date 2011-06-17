#
# This script should be used to create and deploy a new release branch for OLAP. The script
# can also be used to rebuild en redeploy an existing release.
#
# A typical evocation:
#   lofarsys@bgfen0:~> RELEASE=`date +%Y%m%d` make
#
# All variables below which are set using ?= are meant to be
# optionally overruled.
#
# Various useful commands. RELEASE must be set to a date (see above) or to "current".
# which denotes the current installation.
#
#   make install                - compiles and installs the sources
#   make activate               - activate the install
#   make all                    - make install + make activate
#
#   make restart                - restart OLAP (swlevel 0 && swlevel 6)
#
#   make prev                   - switch back to the previous release
#   make sync                   - sync all locus nodes to the current release
#

ifeq "${RELEASE}" "current"
  RELEASE := $(shell basename `readlink -f ${HOME}/production/lofar` 2>/dev/null | cut -d- -f2)
endif

ifeq "${RELEASE}" ""
  $(error $$RELEASE was not set, or could not determine current install. Use export RELEASE=`date +%Y%m%d` for a new release or export RELEASE=current to work with the current release)
endif

ifneq "${USER}" "lofarsys"
  # the software needs to be installed as user lofarsys
  $(error Please run as lofarsys)
endif

ifneq "${HOST}" "bgfen0"
  ifneq "${HOST}" "bgfen1"
    # the software needs to be installed on bgfen0 and/or bgfen1
    $(error Please run on bgfen0 or bgfen1)
  endif
endif

# The developer under which a release is branched off
DEVELOPER ?= mol

# The bug number under which the release branch is filed
BUGNR ?= 1362

# Compilation flags
ION_FLAGS ?=
CN_FLAGS ?=
STORAGE_FLAGS ?= -DUSE_MPI=OFF -DUSE_LOG4CPLUS=ON -DUSE_DAL=ON

# Extra powers for Storage_main
STORAGE_CAPABILITIES = cap_sys_admin,cap_sys_nice,cap_ipc_lock

# Paths in subversion repository
SVN_TRUNK = https://svn.astron.nl/LOFAR/trunk
SVN_BRANCH =  https://svn.astron.nl/LOFAR/branches/OLAP-${RELEASE}-1_0-release-branch

# Directory structure
SOURCE_DIR = ${HOME}/source/lofar-${RELEASE}
BUILD_DIR = ${HOME}/build/lofar-${RELEASE}
LOCUS_DIR = /opt/storage/lofar-${RELEASE}
INSTALL_DIR ?= ${HOME}/production/lofar-${RELEASE}

# Host names
LHN_HOST = lhn001.cep2.lofar

# Determine if the requested release is already active
CURRENT_INSTALL := $(shell readlink -f ${HOME}/production/lofar)
PREV_INSTALL    := $(shell readlink -f ${HOME}/production/lofar-prev)
MY_INSTALL      := ${INSTALL_DIR}

ifeq "${CURRENT_INSTALL}" "${MY_INSTALL}"
  ACTIVATION := sync
else
  ACTIVATION := activate
endif

# Descriptive names for targets
CHECKOUT      = ${SOURCE_DIR}/CMakeLists.txt
INSTALL_ION   = ${INSTALL_DIR}/bgpion/bin/ION_Processing
INSTALL_CN    = ${INSTALL_DIR}/bgpcn/bin/CN_Processing
INSTALL_LHN   = ${INSTALL_DIR}/lhn/bin/Storage_main
INSTALL_BGFEN = ${INSTALL_DIR}/bgfen/bin/ApplController

.DEFAULT:	all
.PHONY:		all install activate prev sync
all:		install ${ACTIVATION}
install:	install_ion install_cn install_bgfen install_lhn
activate:	activate_bgfen activate_locus
prev:		prev_bgfen prev_locus
sync:		activate_locus

.PHONY:		checkout configure install_ion install_cn install_bgfen install_lhn

checkout:	${CHECKOUT}
install_ion:	${INSTALL_ION}
install_cn:	${INSTALL_CN}
install_bgfen:	${INSTALL_BGFEN}
install_lhn:	${INSTALL_LHN}

.PHONY:		restart
restart:
	swlevel 0 && swlevel 6

# Create a release branch, if it does not yet exist.
.PHONY:		release_branch
release_branch:
	svn info ${SVN_BRANCH} &>/dev/null || svn cp -m "bug 1362: New release: ${RELEASE}" --username ${DEVELOPER} ${SVN_TRUNK} ${SVN_BRANCH}

# Checkout the release branch into the source directory, but only LOFAR/* and LOFAR/CMake/*, as cmake will pull in the rest.
${CHECKOUT}:		release_branch
	mkdir -p ${SOURCE_DIR}
	test -d ${SOURCE_DIR}/LOFAR || svn co -q --depth=files ${SVN_BRANCH} ${SOURCE_DIR}/LOFAR
	svn up -q ${SOURCE_DIR}/LOFAR/CMake

# Configure all packages
# cmake needs to pull in additional packages from subversion. A parallel build would pull the same directories to the same destinations in parallel,
# which subversion doesn't allow.
configure:		checkout
	mkdir -p ${BUILD_DIR}/bgpion_opt
	cd ${BUILD_DIR}/bgpion_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES=IONProc ${ION_FLAGS} -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/bgp_ion

	mkdir -p ${BUILD_DIR}/bgpcn_opt
	cd ${BUILD_DIR}/bgpcn_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES=CNProc ${CN_FLAGS} -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/bgp_cn

	mkdir -p ${BUILD_DIR}/bgfen/gnu_opt
	cd ${BUILD_DIR}/bgfen/gnu_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES="Appl_Controller;APLCommon;Run" -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/bgfen/gnu_opt

	mkdir -p ${BUILD_DIR}/lhn/gnu_opt
        # remove /data/sys: on lhn, /opt is a symlink for /data/sys/opt, which cmake resolves for us. However, on locus,
        # we really need it as /opt. 
	ssh ${LHN_HOST} 'cd ${BUILD_DIR}/lhn/gnu_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES="Storage;Deployment" ${STORAGE_FLAGS} -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${LOCUS_DIR} && sed -i s#/data/sys##g CMakeCache.txt'

# Build and install ION_Processing
${INSTALL_ION}:		configure
	cd ${BUILD_DIR}/bgpion_opt && make -j16 install

# Build and install CN_Processing
${INSTALL_CN}:		configure
	cd ${BUILD_DIR}/bgpcn_opt && make -j16 install

# Build and install support software on bgfen
${INSTALL_BGFEN}:	configure
	cd ${BUILD_DIR}/bgfen/gnu_opt && make -j16 install
	cd ${HOME}/production && cp lofar/bgfen/etc/*.debug ${INSTALL_DIR}/bgfen/etc
	ln -s ${HOME}/log ${INSTALL_DIR}/bgfen/log

# Build and install Storage_main
${INSTALL_LHN}:		configure
	ssh ${LHN_HOST} 'cd ${BUILD_DIR}/lhn/gnu_opt && make -j16 install'
	ssh ${LHN_HOST} "cexec 'mkdir -p ${LOCUS_DIR} && scp -rq ${LHN_HOST}:${LOCUS_DIR} . && sudo /sbin/setcap ${STORAGE_CAPABILITIES}=ep ${LOCUS_DIR}/bin/Storage_main'"

# Activate the installation
.PHONY:		activate_bgfen activate_locus
activate_bgfen:		install_ion install_cn install_bgfen
	ln -sfT ${MY_INSTALL} ${HOME}/production/lofar
	ln -sfT ${CURRENT_INSTALL} ${HOME}/production/lofar-prev

activate_locus:		install_lhn
	ssh ${LHN_HOST} cexec ln -sfT /opt/storage/`basename ${CURRENT_INSTALL}` /opt/storage/current
	ssh ${LHN_HOST} cexec ln -sfT /opt/storage/`basename ${PREV_INSTALL}` /opt/storage/prev

# Swap previous and current release
.PHONY:		prev_bgfen prev_locus
prev_bgfen:
	ln -sfT ${PREV_INSTALL} ${HOME}/production/lofar
	ln -sfT ${CURRENT_INSTALL} ${HOME}/production/lofar-prev

prev_locus:
	ssh ${LHN_HOST} cexec ln -sfT /opt/storage/`basename ${PREV_INSTALL}` /opt/storage/current
	ssh ${LHN_HOST} cexec ln -sfT /opt/storage/`basename ${CURRENT_INSTALL}` /opt/storage/prev

