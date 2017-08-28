Summary: CamFlow userspace library
Name: libprovenance
Version: 0.3.7
Release: 1
Group: audit/camflow
License: GPLv2
Source: %{expand:%%(pwd)}
BuildRoot: %{_topdir}/BUILD/%{name}-%{version}-%{release}

%description
%{summary}

%prep
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local/lib/
mkdir -p $RPM_BUILD_ROOT/usr/local/include
cd $RPM_BUILD_ROOT
cp -f %{SOURCEURL0}/src/libprovenance.a ./usr/local/lib/libprovenance.a
cp -f %{SOURCEURL0}/include/provenance.h ./usr/local/include/provenance.h
cp -f %{SOURCEURL0}/include/provenancefilter.h ./usr/local/include/provenancefilter.h
cp -f %{SOURCEURL0}/include/provenanceutils.h ./usr/local/include/provenanceutils.h
cp -f %{SOURCEURL0}/include/provenancePovJSON.h ./usr/local/include/provenancePovJSON.h

%clean
rm -r -f "$RPM_BUILD_ROOT"

%files
%defattr(644,root,root)
/usr/local/lib/libprovenance.a
/usr/local/include/provenance.h
/usr/local/include/provenancefilter.h
/usr/local/include/provenanceutils.h
/usr/local/include/provenancePovJSON.h
