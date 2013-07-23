%define _use_internal_dependency_generator 0
%define glibc_version %(rpm -q glibc | cut -d . -f 1-2 )
%define glibc21 %([ "%glibc_version" = glibc-2.1 ] && echo 1 || echo 0)
%define glibc22 %([ "%glibc_version" = glibc-2.2 ] && echo 1 || echo 0)

%define tarversion 4.2.4p5

Summary: Synchronizes system time using the Network Time Protocol (NTP).
Name: ntp
Version: 4.2.4p5.lofar
Release: 5
License: distributable
Group: System Environment/Daemons
Source0: http://www.eecis.udel.edu/~ntp/ntp_spool/ntp4/ntp-%{tarversion}.tar
Source1: ntp.conf
Source2: ntp.oncore.0
Source3: ntpd.init
Source4: ntpd.sysconfig
Source5: step-tickers
#Source2: ntp.keys

# new find-requires
#Source7: filter-requires-ntp.sh
#define __find_requires %{SOURCE7}

Patch1: patch-ntp-4.2.4p5

URL: http://www.ntp.org
PreReq: /sbin/chkconfig
Prereq: /usr/sbin/groupadd /usr/sbin/useradd
PreReq: /bin/awk sed grep
Requires: libcap
BuildRequires: libcap-devel autoconf automake openssl-devel
BuildRequires: readline-devel
Obsoletes: xntp3 ntpstat
BuildRoot: %{_tmppath}/%{name}-root

%description
The Network Time Protocol (NTP) is used to synchronize a computer's
time with another reference time source. The ntp package contains
utilities and daemons that will synchronize your computer's time to
Coordinated Universal Time (UTC) via the NTP protocol and NTP servers.
The ntp package includes ntpdate (a program for retrieving the date
and time from remote machines via a network) and ntpd (a daemon which
continuously adjusts system time).

Install the ntp package if you need tools for keeping your system's
time synchronized via the NTP protocol.

%prep 
%setup -q -n ntp-%{tarversion}
# -a 5 -a 6

%patch1 -p1 -b .preppskit

%build

perl -pi -e 's|INSTALL_STRIP_PROGRAM="\\\$\{SHELL\} \\\$\(install_sh\) -c -s|INSTALL_STRIP_PROGRAM="\${SHELL} \$(install_sh) -c|g' configure
# XXX work around for anal ntp configure
%define	_target_platform	%{nil}
export CFLAGS="$RPM_OPT_FLAGS -g -DDEBUG -Wall" 
if echo 'int main () { return 0; }' | gcc -pie -fPIE -O2 -xc - -o pietest 2>/dev/null; then
	./pietest && export CFLAGS="$CFLAGS -pie -fPIE"
	rm -f pietest
fi
%configure --sysconfdir=%{_sysconfdir}/ntp --bindir=%{_sbindir} 
# removed flags: --enable-all-clocks --enable-parse-clocks
# added flags:   --enable-ONCORE
unset CFLAGS
%undefine	_target_platform

# XXX workaround glibc-2.1.90 lossage for now.
# XXX still broken with glibc-2.1.94-2 and glibc-2.1.95-1
%if ! %{glibc21} && ! %{glibc22}
( echo '#undef HAVE_CLOCK_SETTIME';
echo '#undef HAVE_TIMER_CREATE';
echo '#undef HAVE_TIMER_SETTIME';
)>>config.h
%endif

make Makefile
for dir in *; do 
	[ -d $dir ] && make -C $dir Makefile || :
done

# Remove -lreadline and -lrt from ntpd/Makefile
# I don't see them used...
perl -pi -e "s|LIBS = -lrt -lreadline|LIBS = |" ntpd/Makefile 

perl -pi -e "s|-lelf||" */Makefile
perl -pi -e "s|-Wcast-qual||" */Makefile
perl -pi -e "s|-Wconversion||" */Makefile
find . -name Makefile -print0 | xargs -0 perl -pi -e "s|-Wall|-Wall -Wextra -Wno-unused|g"
#
# NTP DOES NOT BUILD WITH THESE FLAGS ON (ALTHOUGH IT SHOULD).
# REMOVE THEM FROM THE GLOBAL(!!!) /usr/lib/rpm/redhat/macros file
perl -pi -e "s|-D_FORTIFY_SOURCE=2||" /usr/lib/rpm/redhat/macros
perl -pi -e "s|-fstack-protector||" /usr/lib/rpm/redhat/macros
perl -pi -e "s|--param=ssp-buffer-size=4||" /usr/lib/rpm/redhat/macros

make
#pushd ntpstat-0.2
#make
#popd


%install
rm -rf $RPM_BUILD_ROOT

%makeinstall sysconfdir=%{_sysconfdir}/ntp bindir=${RPM_BUILD_ROOT}%{_sbindir}

#pushd ntpstat-0.2
#mkdir -p ${RPM_BUILD_ROOT}/%{_mandir}/man1
#mkdir -p ${RPM_BUILD_ROOT}/%{_bindir}
mkdir -p ${RPM_BUILD_ROOT}/dev

#install -m 755 ntpstat ${RPM_BUILD_ROOT}/%{_bindir}/
#install -m 644 ntpstat.1 ${RPM_BUILD_ROOT}/%{_mandir}/man1/
#popd
#pushd man
#for i in *.1; do 
#	install -m 644 $i ${RPM_BUILD_ROOT}/%{_mandir}/man1/
#done

{ cd $RPM_BUILD_ROOT

  mkdir -p .%{_sysconfdir}/ntp
  mkdir -p .%{_initrddir}
  install -m644 ${RPM_SOURCE_DIR}/ntp.conf .%{_sysconfdir}/ntp.conf
  install -m644 ${RPM_SOURCE_DIR}/ntp.oncore.0 .%{_sysconfdir}/ntp.oncore.0
  mkdir -p .%{_var}/lib/ntp
  echo '0.0' >.%{_var}/lib/ntp/drift
  install -m644 ${RPM_SOURCE_DIR}/step-tickers .%{_sysconfdir}/ntp/step-tickers
  install -m755 ${RPM_SOURCE_DIR}/ntpd.init .%{_initrddir}/ntpd

  mkdir -p .%{_sysconfdir}/sysconfig
  install -m644 ${RPM_SOURCE_DIR}/ntpd.sysconfig .%{_sysconfdir}/sysconfig/ntpd
}


%clean
rm -rf $RPM_BUILD_ROOT

%pre
/usr/sbin/groupadd -g 38 ntp  2> /dev/null || :
/usr/sbin/useradd -u 38 -g 38 -s /sbin/nologin -M -r -d %{_sysconfdir}/ntp ntp 2>/dev/null || :

%post
/sbin/chkconfig --add ntpd
/sbin/chkconfig --levels 345 ntpd on

if [ "$1" -ge "1" ]; then
  grep %{_sysconfdir}/ntp/drift %{_sysconfdir}/ntp.conf > /dev/null 2>&1
  olddrift=$?
  if [ $olddrift -eq 0 ]; then
    service ntpd status > /dev/null 2>&1
    wasrunning=$?
    # let ntp save the actual drift
    [ $wasrunning -eq 0 ] && service ntpd stop > /dev/null 2>&1
    # copy the driftfile to the new location
    [ -f %{_sysconfdir}/ntp/drift ] \
      && cp %{_sysconfdir}/ntp/drift %{_var}/lib/ntp/drift
    # change the path in the config file
    sed -e 's#%{_sysconfdir}/ntp/drift#%{_var}/lib/ntp/drift#g' \
      %{_sysconfdir}/ntp.conf > %{_sysconfdir}/ntp.conf.rpmupdate \
      && mv %{_sysconfdir}/ntp.conf.rpmupdate %{_sysconfdir}/ntp.conf 
    # remove the temp file
    rm -f %{_sysconfdir}/ntp.conf.rpmupdate
    # start ntp if it was running previously
    [ $wasrunning -eq 0 ] && service ntpd start > /dev/null 2>&1 || :
  fi
fi

%preun
if [ $1 = 0 ]; then
    service ntpd stop > /dev/null 2>&1
    /sbin/chkconfig --del ntpd
fi

rm -f /dev/oncore.serial.0
rm -f /dev/oncore.pps.0

%postun
if [ "$1" -ge "1" ]; then
  service ntpd condrestart > /dev/null 2>&1
fi

%files
%defattr(-,root,root)
%doc html/* NEWS TODO 
%{_sbindir}/ntp-wait
%{_sbindir}/ntptrace
%{_sbindir}/ntp-keygen
%{_sbindir}/ntpd
%{_sbindir}/ntpdate
%{_sbindir}/ntpdc
%{_sbindir}/ntpq
%{_sbindir}/ntptime
%{_sbindir}/tickadj
%{_sbindir}/sntp
%config			%{_initrddir}/ntpd
%config(noreplace)	%{_sysconfdir}/sysconfig/ntpd
%config(noreplace)	%{_sysconfdir}/ntp.conf
%config(noreplace)	%{_sysconfdir}/ntp.oncore.0
%dir 	%{_sysconfdir}/ntp
%config(noreplace) %verify(not md5 size mtime) %{_sysconfdir}/ntp/step-tickers
#%config(noreplace) %{_sysconfdir}/ntp/keys
%dir	%attr(-,ntp,ntp)   %{_var}/lib/ntp
%config(noreplace) %attr(644,ntp,ntp) %verify(not md5 size mtime) %{_var}/lib/ntp/drift
%{_mandir}/man1/*
#%{_bindir}/ntpstat


%changelog
* Wed Oct 11 2006 Klaas Jan Wierenga <wierenga@astron.nl>
- enable TRAIM by default (uncomment TRAIM OFF line), TRAIM has been shown
  to work correctly in the field and it is required to have accurate PPS timing

* Fri Jul 21 2006 Klaas Jan Wierenga <wierenga@astron.nl>
- disabled TRAIM by default (needed for lab setup, should be enable in production)
- better ntp.conf settings, used suggestions from Tac32 manual
- now conforms to specification in section 3.1.1. of LOFAR-ASTRON-RPT-051
  except for TRAIM which is now disabled by default

* Fri Feb 10 2006 Klaas Jan Wierenga <wierenga@astron.nl>
- Fixed problem with step-tickers
- increased package Release to 3

* Thu Feb 2 2006 Klaas Jan Wierenga <wierenga@astron.nl>

- Added ntp[0|1].NL.net to /etc/ntp/step-tickers
- Moved creation of ONCORE symlink /dev/oncore.* to ntpd.init

* Wed Feb 1 2006 Klaas Jan Wierenga <wierenga@astron.nl>
- First attempt at creating a NTP rpm for LOFAR.

