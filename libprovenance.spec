Summary: CamFlow userspace library
Name: libprovenance
Version: 0.3.8
Release: 1
Group: audit/camflow
License: GPLv2
Source: %{expand:%%(pwd)}
BuildRoot: %{_topdir}/BUILD/%{name}-%{version}-%{release}

%description
%{summary}

%prep
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/lib/
mkdir -p $RPM_BUILD_ROOT/usr/include/
cd $RPM_BUILD_ROOT
cp -f %{SOURCEURL0}/src/libprovenance.so ./usr/lib/libprovenance.so
cp -f %{SOURCEURL0}/include/provenance.h ./usr/include/provenance.h
cp -f %{SOURCEURL0}/include/provenancefilter.h ./usr/include/provenancefilter.h
cp -f %{SOURCEURL0}/include/provenanceutils.h ./usr/include/provenanceutils.h
cp -f %{SOURCEURL0}/include/provenancePovJSON.h ./usr/include/provenancePovJSON.h

%clean
rm -r -f "$RPM_BUILD_ROOT"

%files
%defattr(644,root,root)
/usr/lib/libprovenance.so
/usr/include/provenance.h
/usr/include/provenancefilter.h
/usr/include/provenanceutils.h
/usr/include/provenancePovJSON.h

%post -p /sbin/ldconfig
