%define	name	 dtl
%define ver      3.6.2
%define rel      monkeyiq9006d
%define srcdir   /usr/local/src

Summary: STL like access to ODBC databases
Name: %{name}
Version: %{ver}
Release: %{rel}
Copyright: OSI Approved
Group: System Environment/Libraries
Source: http://prdownloads.sourceforge.net/dtemplatelib/%{name}-%{ver}.tar.gz
#Patch:  dtl-case-sensitive-tables.patch
#Patch1: dtl-pc.patch
BuildRoot: %{_tmppath}/%{name}-root
Packager: Ben Martin <monkeyiq@users.sourceforge.net>
URL: http://freshmeat.net/redir/dtl/1710/url_homepage/index.htm
Requires: unixODBC >= 2.2.3
Requires: STLport >= 4.5.3

%description 
The goal of the Database Template Library is to make ODBC
recordsets look just like an STL container. As a user, you can move
through containers using standard STL iterators, and if you insert(),
erase(), or replace() records in the containers, changes can be
automatically committed to the database for you. The library's
compliance with the STL iterator and container standards means you can
plug the abstractions into a wide variety of STL algorithms for data
storage, searching, and manipulation. In addition, the C++ reflection
mechanism used by the library to bind to database tables allows
generic indexing and lookup properties to be added to the containers
with no special code required from the enduser. Because the code takes
full advantage of the template mechanism, the library adds minimal
overhead compared with using raw ODBC calls to access a database.

%package doc
Summary: Developer documentation for DTL
Group: System/Libraries
Requires: %{name} >= %{ver}
%description doc
Developer documentation for DTL

%package examples
Summary: Example files for developing code using DTL
Group: System/Libraries
Requires: %{name} >= %{ver}
%description examples
Example files for developing code using DTL

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n dtl_36
#%patch
#%patch1

echo Working dir `pwd`

patch -p0 <<'ENDOFHERE'
--- config/linux-i686-gcc.inc	2004-02-01 16:46:25.000000000 +1000
+++ config/linux-i686-gcc.inc.me	2004-03-23 18:00:48.000000000 +1000
@@ -11,7 +11,7 @@
 # - force all queries to be uppper case
 # Also gcc 2.96 with MySQL does not seem to support bulk fetch operations with std::string
 # correctly so disable these for now
-CFLAGS += -DDTL_UC -DDTL_SINGLE_FETCH 
+CFLAGS += -DDTL_SINGLE_FETCH 
 
 # For some of our internal projects we need to manually define UNIX
 # Also, we need to tell unixODBC that gcc supports the "long long" data type
ENDOFHERE

patch -p0 <<'ENDOFHERE'
--- dtl.pc	1970-01-01 10:00:00.000000000 +1000
+++ dtl.pc.me	2004-03-23 18:09:31.000000000 +1000
@@ -0,0 +1,13 @@
+prefix=/usr
+exec_prefix=/usr
+libdir=${exec_prefix}/lib
+includedir=${prefix}/include
+
+Name: dtl
+Description: STL like ODBC wrapper
+Version: 3.6.2
+Requires: 
+Libs: -L${libdir} ${libdir}/libDTL36.a -lodbc
+Cflags: -I${includedir} -I${includedir}/dtl36
+
+
ENDOFHERE

echo "Building DTL for Linux unixODBC usage..."

cd lib      && sh build.sh  && cd -;
# cd example  && sh build.sh  && cd -;


rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_includedir}/dtl36
mkdir -p $RPM_BUILD_ROOT/%{_libdir}
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
cp -av lib/*h                $RPM_BUILD_ROOT/%{_includedir}/dtl36/
cp -av lib/debug/libDTL.a    $RPM_BUILD_ROOT/%{_libdir}/libDTL36.a
mkdir -p                     $RPM_BUILD_ROOT/%{_libdir}/pkgconfig/
cp -av dtl.pc                $RPM_BUILD_ROOT/%{_libdir}/pkgconfig/dtl.pc
mkdir -p                     $RPM_BUILD_ROOT/%{_docdir}/%{name}-%{ver}/
cp -av docs/*                $RPM_BUILD_ROOT/%{_docdir}/%{name}-%{ver}/
chmod 644                    $RPM_BUILD_ROOT/%{_docdir}/%{name}-%{ver}/*
mkdir -p                     $RPM_BUILD_ROOT/%{srcdir}/%{name}-%{ver}/example
mkdir -p                     $RPM_BUILD_ROOT/%{srcdir}/%{name}-%{ver}/example_db
cp -av example/*             $RPM_BUILD_ROOT/%{srcdir}/%{name}-%{ver}/example
cp -av example_db/*          $RPM_BUILD_ROOT/%{srcdir}/%{name}-%{ver}/example_db

# cp -av example/debug/example $RPM_BUILD_ROOT/%{_bindir}/example_dtl

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,0755)
# %doc AUTHORS README COPYING ChangeLog INSTALL
# %attr(0555, root, root) %{_bindir}/*
%{_includedir}/dtl36/*
%{_libdir}/libDTL36.a
%{_libdir}/pkgconfig/*

%files doc
%defattr(-,root,root,0644)
%{_docdir}/%{name}-%{ver}/*

%files examples
%defattr(-,root,root,0644)
%{srcdir}/%{name}-%{ver}/example/*
%{srcdir}/%{name}-%{ver}/example_db/*

%changelog
* Tue Mar 23 2004 Ben Martin
- Created 
