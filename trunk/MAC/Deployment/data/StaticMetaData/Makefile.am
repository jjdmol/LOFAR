sbin_SCRIPTS = createFiles

dist_sbin_SCRIPTS = $(sbin_SCRIPTS)

staticmetadir = $(sysconfdir)/StaticMetaData

sysconf_DATA = DipoleModel.conf \
               HBADelays.conf \
               HBADeltas.conf \
               SourceCatalog.conf 

staticmeta_DATA = RemoteStation.conf.tmpl \
                  RSPDriver.conf.tmpl \
                  TBBDriver.conf.tmpl \
                  StationInfo.dat \
                  TBBConnections.dat \
                  RSPConnections.dat \
                  MAC+IP.dat

include $(top_srcdir)/Makefile.common
