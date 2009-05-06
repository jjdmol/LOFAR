sbin_SCRIPTS = createFiles

dist_sbin_SCRIPTS = $(sbin_SCRIPTS)

staticmetadir = $(sysconfdir)/StaticMetaData

# These end up in /etc
sysconf_DATA = DipoleModel.conf \
               HBADelays.conf \
               HBADeltas.conf \
               SourceCatalog.conf \
               Attenuation.conf \
               CableDelays.conf

# These end up in /etc/StaticMetaData
staticmeta_DATA = RemoteStation.conf.tmpl \
                  RSPDriver.conf.tmpl \
                  TBBDriver.conf.tmpl \
                  RemoteStation.conf.test \
                  RSPDriver.conf.test \
                  TBBDriver.conf.test \
                  StationInfo.dat \
                  TBBConnections.dat \
                  RSPConnections.dat \
                  MAC+IP.dat \
		  AntennaArrays/cs001-antennaarrays.conf \
		  AntennaArrays/cs010-antennaarrays.conf \
		  AntennaArrays/cs016-antennaarrays.conf \
		  AntennaArrays/cs302-antennaarrays.conf \
		  AntennaArrays/de001-antennaarrays.conf \
	  	  AntennaArrays/de602-antennaarrays.conf \
	  	  AntennaArrays/de603-antennaarrays.conf

include $(top_srcdir)/Makefile.common
