ifndef RELEASE
  $(error Please set $$RELEASE to the name of the release to deploy: export RELEASE=`date +%Y%m%d`)
endif

ifneq "${USER}" "lofarsys"
  $(error Please run as lofarsys)
endif

ifneq "${HOST}" "bgfen0"
  $(error Please run on bgfen0)
endif

# The developer under which a release is branched off
DEVELOPER ?= mol

# The bug number under which the release branch is filed
BUGNR ?= 1362

# Paths in subversion repository
SVN_TRUNK = https://svn.astron.nl/LOFAR/trunk
SVN_BRANCH =  https://svn.astron.nl/LOFAR/branches/OLAP-${RELEASE}-1_0-release-branch

# Directory structure
HOME = /globalhome/lofarsystem
SOURCE_DIR = ${HOME}/source/lofar-${RELEASE}
BUILD_DIR = ${HOME}/source/lofar-${RELEASE}
LOCUS_DIR = /opt/storage/lofar-${RELEASE}
INSTALL_DIR ?= ${HOME}/production/lofar-${RELEASE}

# Host names
LHN_HOST = lhn001.cep2.lofar

# Compilation flags
ION_FLAGS =
CN_FLAGS =
STORAGE_FLAGS = -DUSE_MPI=OFF -DUSE_LOG4CPLUS=ON -DUSE_DAL=ON

# Determine if the requested release is already active
CURRENT_INSTALL := $(shell echo readlink -f ${HOME}/production/lofar)
MY_INSTALL := ${INSTALL_DIR}

ifeq "${CURRENT_INSTALL}" "${MY_INSTALL}"
  ACTIVATION :=
else
  ACTIVATION := activate
endif


# Descriptive names for targets
CHECKOUT      = ${SOURCE_DIR}/CMakeLists.txt
CONFIG_ION    = ${BUILD_DIR}/bgpion_opt/CMakeCache.txt
CONFIG_CN     = ${BUILD_DIR}/bgpcn_opt/CMakeCache.txt
CONFIG_LHN    = ${BUILD_DIR}/lhn/gnu_opt/CMakeCache.txt
CONFIG_BGFEN  = ${BUILD_DIR}/bgfen/gnu_opt/CMakeCache.txt
INSTALL_ION   = ${INSTALL_DIR}/bgpion/bin/ION_Processing
INSTALL_CN    = ${INSTALL_DIR}/bgpcn/bin/CN_Processing
INSTALL_LHN   = ${INSTALL_DIR}/lhn/bin/Storage_main
INSTALL_BGFEN = ${INSTALL_DIR}/bgfen/bin/ApplController

all:		install ${ACTIVATION}
install:	install_ion install_cn install_bgfen install_lhn
activate:	activate_bgfen activate_locus
checkout:	${CHECKOUT}
config_ion:	${CONFIG_ION}
config_cn:	${CONFIG_CN}
config_bgfen:	${CONFIG_BGFEN}
config_lhn:	${CONFIG_LHN}
install_ion:	${INSTALL_ION}
install_cn:	${INSTALL_CN}
install_bgfen:	${INSTALL_BGFEN}
install_lhn:	${INSTALL_LHN}

.DEFAULT:	all
.PHONY:		all install activate
.PHONY:		checkout config_ion config_cn config_bgfen install_ion install_cn install_bgfen

.PHONY:		restart
restart:
	swlevel 0 && swlevel 6

.PHONY:		release_branch
release_branch:
	svn info ${SVN_BRANCH} >/dev/null 2>&1 || svn cp -m "bug 1362: New release: ${RELEASE}" --username ${DEVELOPER} ${SVN_TRUNK} ${SVN_BRANCH}

${CHECKOUT}:		release_branch
	mkdir -p ${SOURCE_DIR}
	svn co -N ${SVN_BRANCH} ${SOURCE_DIR}/LOFAR
	svn up ${SOURCE_DIR}/LOFAR/CMake

${CONFIG_ION}:		checkout
	mkdir -p ${BUILD_DIR}/bgpion_opt
	cd ${BUILD_DIR}/bgpion_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES=IONProc ${ION_FLAGS} -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/bgp_ion

${INSTALL_ION}:		config_ion
	cd ${BUILD_DIR}/bgpion_opt && make -j16 install

${CONFIG_CN}:		checkout
	mkdir -p ${BUILD_DIR}/bgpcn_opt
	cd ${BUILD_DIR}/bgpcn_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES=CNProc ${CN_FLAGS} -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/bgp_cn

${INSTALL_CN}:		config_cn
	cd ${BUILD_DIR}/bgpcn_opt && make -j16 install

${CONFIG_BGFEN}:	checkout
	mkdir -p ${BUILD_DIR}/bgfen/gnu_opt
	cd ${BUILD_DIR}/bgfen/gnu_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES="Appl_Controller;APLCommon;Run" -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/bgfen/gnu_opt

${INSTALL_BGFEN}:	config_bgfen
	cd ${BUILD_DIR}/bgfen/gnu_opt && make -j16 install

${CONFIG_LHN}:		checkout
	mkdir -p ${BUILD_DIR}/lhn/gnu_opt
	ssh ${LHN_HOST} 'cd ${BUILD_DIR}/lhn/gnu_opt && cmake ${SOURCE_DIR}/LOFAR -DBUILD_PACKAGES=Storage ${STORAGE_FLAGS} -DBUILD_TESTING=OFF -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}/lhn && sed -i s#/data/sys##g CMakeCache.txt'

${INSTALL_LHN}:		config_lhn
	ssh ${LHN_HOST} 'cd ${BUILD_DIR}/lhn/gnu_opt && make -j16 install'
	ssh ${LHN_HOST} cexec 'mkdir -p ${LOCUS_DIR} && cp -R ${INSTALL_DIR}/lhn ${LOCUS_DIR} && sudo /sbin/setcap cap_sys_admin,cap_sys_nice=ep ${LOCUS_DIR}/bin/Storage_main'

.PHONY:		activate_bgfen activate_locus
activate_bgfen:	install_ion install_cn install_bgfen
	cd ${HOME}/production && test -L lofar && mv -fT lofar lofar-prev
	cd ${HOME}/production && ln -s ${INSTALL_DIR} lofar
	cd ${HOME}/production && cp lofar-prev/bgfen/etc/*.debug lofar/bgfen/etc
	ln -s ${HOME}/log /opt/lofar/log

activate_locus:		install_lhn
	ssh ${LHN_HOST} cexec '[ `readlink -f /opt/storage/current` = ${LOCUS_DIR} ] || (mv -fT /opt/storage/current /opt/storage/prev && ln -s ${LOCUS_DIR} /opt/storage/current)'

