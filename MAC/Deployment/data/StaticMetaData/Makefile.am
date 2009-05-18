sbin_SCRIPTS = createFiles

dist_sbin_SCRIPTS = $(sbin_SCRIPTS)

staticmetadir = $(sysconfdir)/StaticMetaData

# These end up in $PREFIX/etc
sysconf_DATA = $(srcdir)/*.conf

# These end up in $PREFIX/etc/StaticMetaData
staticmeta_DATA = $(srcdir)/*.tmpl
staticmeta_DATA += $(srcdir)/*.test
staticmeta_DATA += $(srcdir)/*.dat
staticmeta_DATA += $(srcdir)/AntennaArrays/*.conf
staticmeta_DATA += $(srcdir)/CableDelays/*.conf

#                  RemoteStation.conf.tmpl \
#                  RSPDriver.conf.tmpl \
#                  TBBDriver.conf.tmpl \
#                  RemoteStation.conf.test \
#                  RSPDriver.conf.test \
#                  TBBDriver.conf.test \
#                  StationInfo.dat \
#                  TBBConnections.dat \
#                  RSPConnections.dat \
#                  MAC+IP.dat \
#		  AntennaArrays/cs001-antennaarrays.conf \
#		  AntennaArrays/cs010-antennaarrays.conf \
#		  AntennaArrays/cs016-antennaarrays.conf \
#		  AntennaArrays/cs302-antennaarrays.conf \
#		  AntennaArrays/de001-antennaarrays.conf \
#	  	  AntennaArrays/de602-antennaarrays.conf \
#	  	  AntennaArrays/de603-antennaarrays.conf \
#		  CableDelays/cs001-cabledelays.conf \
#		  CableDelays/cs010-cabledelays.conf \
#		  CableDelays/cs016-cabledelays.conf \
#		  CableDelays/cs302-cabledelays.conf

include $(top_srcdir)/Makefile.common
