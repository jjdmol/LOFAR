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
staticmeta_DATA += $(srcdir)/HBADeltas/*.conf

include $(top_srcdir)/Makefile.common
