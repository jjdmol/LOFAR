
noinst_SCRIPTS     = wrapexec.in unwrapexec.in
nodist_bin_SCRIPTS = wrapexec unwrapexec
EXTRA_DIST         = wrapexec.in unwrapexec.in

wrapexec: $(srcdir)/wrapexec.in
	sed -e 's%INSTALL_BINDIR%$(bindir)%' < $(srcdir)/wrapexec.in > wrapexec.in.1
	sed -e 's%INSTALL_SYSCONFDIR%$(sysconfdir)%' < wrapexec.in.1 > wrapexec
	chmod +x wrapexec

unwrapexec: $(srcdir)/unwrapexec.in
	sed -e 's%INSTALL_BINDIR%$(bindir)%' < $(srcdir)/unwrapexec.in > unwrapexec
	chmod +x unwrapexec

distclean-local:
	rm -f wrapexec unwrapexec
