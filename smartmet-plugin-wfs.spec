%bcond_without observation
%define DIRNAME wfs
%define SPECNAME smartmet-plugin-%{DIRNAME}
Summary: SmartMet WFS plugin
Name: %{SPECNAME}
Version: 25.8.22
Release: 1%{?dist}.fmi
License: MIT
Group: SmartMet/Plugins
URL: https://github.com/fmidev/smartmet-plugin-wfs
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

# https://fedoraproject.org/wiki/Changes/Broken_RPATH_will_fail_rpmbuild
%global __brp_check_rpaths %{nil}

%if 0%{?rhel} && 0%{rhel} < 9
%define smartmet_boost boost169
%else
%define smartmet_boost boost
%endif

%if 0%{?rhel} && 0%{rhel} <= 9
%define smartmet_fmt_min 11.0.1
%define smartmet_fmt_max 12.0.0
%define smartmet_fmt fmt-libs >= %{smartmet_fmt_min}, fmt-libs < %{smartmet_fmt_max}
%define smartmet_fmt_devel fmt-devel >= %{smartmet_fmt_min}, fmt-devel < %{smartmet_fmt_max}
%else
%define smartmet_fmt fmt
%define smartmet_fmt_devel fmt-devel
%endif

BuildRequires: rpm-build
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: %{smartmet_boost}-devel
BuildRequires: ctpp2-devel
BuildRequires: %{smartmet_fmt_devel}
BuildRequires: gdal310-devel
BuildRequires: jsoncpp-devel
BuildRequires: libcurl-devel
BuildRequires: xerces-c-devel
BuildRequires: xqilla-devel
BuildRequires: openssl-devel
BuildRequires: bzip2-devel
BuildRequires: zlib-devel
BuildRequires: smartmet-library-timeseries-devel >= 25.2.18
BuildRequires: smartmet-library-spine-devel >= 25.3.24
BuildRequires: smartmet-library-gis-devel >= 25.2.18
BuildRequires: smartmet-library-trax-devel >= 25.2.18
BuildRequires: smartmet-library-locus-devel >= 25.2.18
BuildRequires: smartmet-library-macgyver-devel >= 25.2.18
BuildRequires: smartmet-engine-contour-devel >= 25.2.18
BuildRequires: smartmet-engine-geonames-devel >= 25.2.18
BuildRequires: smartmet-engine-gis-devel >= 25.2.18
BuildRequires: smartmet-engine-grid-devel >= 25.8.25
BuildRequires: smartmet-engine-querydata-devel >= 25.2.18
BuildRequires: smartmet-library-grid-content-devel >= 25.8.25
BuildRequires: smartmet-library-grid-files-devel >= 25.8.25
%if %{with observation}
BuildRequires: smartmet-engine-observation-devel >= 25.8.22
%endif
Requires: ctpp2
Requires: %{smartmet_fmt}
Requires: libcurl
Requires: jsoncpp
Requires: zlib
Requires: smartmet-library-locus >= 25.2.18
Requires: smartmet-library-macgyver >= 25.2.18
Requires: smartmet-library-spine >= 25.3.24
Requires: smartmet-library-timeseries >= 25.2.18
Requires: smartmet-library-gis >= 25.2.18
Requires: smartmet-library-trax >= 25.2.18
Requires: smartmet-engine-contour >= 25.2.18
Requires: smartmet-engine-geonames >= 25.2.18
Requires: smartmet-engine-gis >= 25.2.18
Requires: smartmet-engine-grid >= 25.8.25
Requires: smartmet-library-grid-content >= 25.8.25
Requires: smartmet-library-grid-files >= 25.8.25
%if %{with observation}
Requires: smartmet-engine-observation >= 25.8.22
%endif
Requires: smartmet-engine-querydata >= 25.2.18
Requires: smartmet-server >= 25.2.18
Requires: xerces-c
Requires: xqilla
Requires: zlib
Requires: %{smartmet_boost}-chrono
Requires: %{smartmet_boost}-filesystem
Requires: %{smartmet_boost}-iostreams
Requires: %{smartmet_boost}-serialization
Requires: %{smartmet_boost}-system
Requires: %{smartmet_boost}-thread

%if 0%{?rhel} && 0%{rhel} == 8
Requires: libpqxx >= 1:7.7.0, libpqxx < 1:7.8.0
BuildRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
#TestRequires: libpqxx-devel >= 1:7.7.0, libpqxx-devel < 1:7.8.0
%else
%if 0%{?rhel} && 0%{rhel} == 9
Requires: libpqxx >= 1:7.9.0, libpqxx < 1:7.10.0
BuildRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
#TestRequires: libpqxx-devel >= 1:7.9.0, libpqxx-devel < 1:7.10.0
%else
%if 0%{?rhel} && 0%{rhel} >= 10
Requires: libpqxx >= 1:7.10.0, libpqxx < 1:7.11.0
BuildRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
#TestRequires: libpqxx-devel >= 1:7.10.0, libpqxx-devel < 1:7.11.0
%else
Requires: libpqxx
BuildRequires: libpqxx-devel
#TestRequires: libpqxx-devel
%endif
%endif
%endif

Provides: %{SPECNAME}
Obsoletes: smartmet-brainstorm-wfs < 16.11.1
Obsoletes: smartmet-brainstorm-wfs-debuginfo < 16.11.1

#TestRequires: ctpp2
#TestRequires: smartmet-test-db >= 25.2.18
#TestRequires: smartmet-test-data >= 24.8.12
#TestRequires: smartmet-utils-devel >= 25.2.18
#TestRequires: smartmet-library-macgyver >= 25.2.18
#TestRequires: smartmet-library-gis >= 25.2.18
#TestRequires: smartmet-library-newbase >= 25.3.20
#TestRequires: smartmet-library-spine-plugin-test >= 25.3.24
#TestRequires: smartmet-engine-geonames >= 25.2.18
#TestRequires: smartmet-engine-gis >= 25.2.18
#TestRequires: smartmet-engine-querydata >= 25.2.18
%if %{with observation}
#TestRequires: smartmet-engine-observation >= 25.8.22
%endif
#TestRequires: smartmet-engine-grid >= 25.8.25
#TestRequires: redis
#TestRequires: smartmet-engine-grid-test
# Required by top level Makefile
#TestRequires: jsoncpp-devel
#TestRequires: libwebp13

%description
SmartMet WFS plugin

%package -n %{SPECNAME}-devel
Summary: SmartMet WFS plugin development files
Requires: smartmet-library-spine-devel >= 25.3.24
Requires: smartmet-library-gis-devel >= 25.2.18
Requires: smartmet-library-locus-devel >= 25.2.18
Requires: smartmet-library-macgyver-devel >= 25.2.18
Requires: %{SPECNAME} = %{version}-%{release}
%description -n %{SPECNAME}-devel
SmartMet WFS plugin development files (for building testsuite without rebuilding plugin)

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n plugins/%{SPECNAME}

%build -q -n plugins/%{SPECNAME}
make %{_smp_mflags} \
     %{?!with_observation:CFLAGS=-DWITHOUT_OBSERVATION}

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(0775,root,root,0775)
%{_datadir}/smartmet/plugins/wfs.so
%defattr(0664,root,root,0775)
%{_datadir}/smartmet/wfs/*.c2t
%{_datadir}/smartmet/wfs/XMLGrammarPool.dump
%{_datadir}/smartmet/wfs/XMLSchemas.cache

%files -n %{SPECNAME}-devel
%{_libdir}/libsmartmet-plugin-wfs.a
%{_includedir}/smartmet/plugin/wfs/*.h
%{_includedir}/smartmet/plugin/wfs/request/*.h

%changelog
* Fri Aug 22 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.8.22-1.fmi
- Repackaged due to smartmet-engine-observation changes

* Tue Apr  8 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.4.8-1.fmi
- Repackaged due to base library ABI changes

* Wed Mar 19 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.3.19-1.fmi
- Repackaged due to base library ABI changes

* Tue Feb 18 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.2.18-1.fmi
- Update to gdal-3.10, geos-3.13 and proj-9.5

* Fri Jan 10 2025 Andris Pavēnis <andris.pavenis@fmi.fi> 25.1.10-1.fmi
- Admin/info request update

* Thu Jan  9 2025 Mika Heiskanen <mika.heiskanen@fmi.fi> - 25.1.9-1.fmi
- Repackaged due to GRID library changes

* Thu Nov 14 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.11.14-1.fmi
- Fix links in output of /admin?what=what=wfs:constructors

* Fri Nov  8 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.11.8-2.fmi
- Register WFS related admin queries instead of usingh local content handler (/wfs/admin?...)

* Fri Nov  8 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.11.8-1.fmi
- Repackage due to smartmet-library-spine ABI changes

* Wed Oct 23 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.10.23-1.fmi
- Repackaged due to ABI changes

* Wed Oct 16 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.10.16-1.fmi
- Repackaged due to grid library ABI changes

* Tue Oct  8 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.10.8-1.fmi
- StoredForecastQueryHandler: fail in case of empty producer and remove duplicate check

* Sat Sep 28 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.9.28-1.fmi
- Repackaged due to PostgreSQLConnection ABI change

* Tue Sep  3 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.9.3-1.fmi
- Repackage due smartmlibrary-grid-data and smartmet-engine-querydata changes

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-2.fmi
- Fix requires (RHEL9, libpxx)

* Wed Aug  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.8.7-1.fmi
- Update to gdal-3.8, geos-3.12, proj-94 and fmt-11

* Mon Aug  5 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.8.5-1.fmi
- Fixed bug in flash data formatting

* Mon Jul 22 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.22-1.fmi
- Rebuild dues to smartmet-library-spine changes

* Wed Jul 17 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.17-1.fmi
- Do not link with libboost_filesystem

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-2.fmi
- Fix muistikorruption store query konfiguraation virheiden tapauksessa

* Fri Jul 12 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.7.12-1.fmi
- Replace many boost library types with C++ standard library ones

* Fri Jun  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.6.7-1.fmi
- Some optimizations (avoid std::ostringstream or reuse it)

* Mon Jun  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.6.3-1.fmi
- Repackaged due to ABI changes

* Tue May 28 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.28-1.fmi
- Do not use LocalTimePool

* Thu May 16 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.16-1.fmi
- Clean up boost date-time uses

* Tue May  7 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.5.7-1.fmi
- Use Date library (https://github.com/HowardHinnant/date) instead of boost date_time

* Fri May  3 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.5.3-1.fmi
- Repackaged due to GRID library changes

* Sat Apr  6 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.4.6-1.fmi
- Fixed to use new Spine::Station variables

* Fri Feb 23 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.23-1.fmi
- Full repackaging

* Tue Feb 20 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.20-1.fmi
- Repackaged due to grid-files ABI changes

* Mon Feb  5 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.2.5-1.fmi
- Repackaged due to grid-files ABI changes

* Tue Jan 30 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> 24.1.30-1.fmi
- Repackaged due to newbase ABI changes

* Mon Jan 29 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.1.29-1.fmi
- Fix support of NFmiLatLonArea and NFmiMercatorArea (attempt 2)

* Thu Jan 11 2024 Andris Pavēnis <andris.pavenis@fmi.fi> 24.1.11-1.fmi
- Fix error quering ecmwf data

* Thu Jan  4 2024 Mika Heiskanen <mika.heiskanen@fmi.fi> - 24.1.4-1.fmi
- Repackaged due to TimeSeriesGeneratorOptions ABI changes

* Fri Dec 22 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.12.22-1.fmi
- Repackaged due to ThreadLock ABI changes

* Tue Dec  5 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.12.5-1.fmi
- Repackaged due to an ABI change in SmartMetPlugin

* Mon Dec  4 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.12.4-1.fmi
- Repackaged since QEngine API changed due to removal of backend synchronization

* Fri Nov 17 2023 Pertti Kinnia <pertti.kinnia@fmi.fi> - 23.11.17-1.fmi
- Repackaged due to API changes in grid-files and grid-content

* Fri Nov 10 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.11.10-1.fmi
- Repackaged due to API changes in grid-content

* Mon Oct 30 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.10.30-1.fmi
- Repackaged due to ABI changes in GRID libraries

* Sat Oct 21 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.10.21-1.fmi
- Changed storedQueryTemplateDir to be an optional setting with default value /usr/share/smartmet/wfs

* Thu Oct 19 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.10.19-1.fmi
- Move templates from /etc to /usr/share

* Thu Oct 12 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.10.12-1.fmi
- Moved some aggregation related functions to timeseries-library

* Fri Sep 29 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.9.29-1.fmi
- Repackaged due to changes in grid libraries

* Mon Sep 11 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.9.11-1.fmi
- Repackaged due to ABI changes in grid-files

* Mon Aug 28 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.8.28-1.fmi
- Repackaged since Contour::Options ABI changed

* Fri Jul 28 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.28-1.fmi
- Repackage due to bulk ABI changes in macgyver/newbase/spine

* Tue Jul 25 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.7.25-1.fmi
- Repackaged since Contour::Options size changed

* Wed Jul 12 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.7.12-2.fmi
- Use postgresql 15, gdal 3.5, geos 3.11 and proj-9.0

* Wed Jul 12 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.7.12-1.fmi
- Repackaged due to ObsEngine API changes

* Tue Jul 11 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.7.11-1.fmi
- Repackaged due to QEngine API changes

* Tue Jun  6 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.6.6-1.fmi
- Repackaged due to GRID ABI changes

* Thu Jun  1 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.6.1-1.fmi
- Changed to use the new ObsEngine API

* Wed May 10 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.5.10-2.fmi
- Use the new obsengine API for getting the latest observation only

* Wed May 10 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.5.10-1.fmi
- /wfs/admin?request=constructor: update response and default format

* Tue May  9 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.5.9-1.fmi
- Improve /wfs/admin?request=constructors output

* Thu Apr 27 2023 Andris Pavēnis <andris.pavenis@fmi.fi> 23.4.27-1.fmi
- Repackage due to macgyver ABI changes (AsyncTask, AsyncTaskGroup)

* Mon Apr 17 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.4.17-1.fmi
- Repackaged due to GRID ABI changes

* Wed Mar 22 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.3.22-1.fmi
- Repackaged due to ObsEngine API changes

* Wed Mar 15 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.3.15-1.fmi
- Repackaged due to ObsEngine API changes

* Thu Mar  9 2023 Mika Heiskanen <mheiskan@rhel8.dev.fmi.fi> - 23.3.9-1.fmi
- Fixed xerces memory leaks

* Mon Feb 13 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.13-1.fmi
- Fixed HostName stack trace output

* Wed Feb  8 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.2.8-1.fmi
- Add host name to stack traces

* Wed Feb 1 2023 Anssi Reponen <anssi.reponen@fmi.fi> - 23.2.1-1.fmi
- Added language support for station names (BRAINSTORM-2514)

* Thu Jan 26 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.1.26-2.fmi
- Repackaged due to timeseries API changes

* Thu Jan 26 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.1.26-1.fmi
- Repackaged due to contour-engine API changes

* Thu Jan 19 2023 Mika Heiskanen <mika.heiskanen@fmi.fi> - 23.1.19-1.fmi
- Repackaged due to ABI changes in grid libraries

* Wed Jan 11 2023 Anssi Reponen <anssi.reponen@fmi.fi> - 23.1.11-1.fmi
- Added support for moving stations icebuoy and copernicus (BRAINSTORM-2409)

* Fri Dec 16 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.12.16-1.fmi
- Repackaged since PostgreSQLConnection ABI changed

* Mon Dec 12 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.12.12-1.fmi
- Repackaged due to ABI changes

* Fri Dec  2 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.12.2-1.fmi
- Update HTTP request method checking and support OPTIONS method

* Fri Nov 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.25-1.fmi
- Added apikey to stack traces

* Tue Nov 15 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.15-1.fmi
- Disable logging of error messages on unknown handlers due to too many errors after HIRLAM was stopped

* Tue Nov  8 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.11.8-1.fmi
- Repackaged due to base library ABI changes

* Thu Oct 20 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.20-1.fmi
- Repackaged due to base library ABI changes

* Mon Oct 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.10-1.fmi
- Repackaged due to base library ABI changes

* Wed Oct  5 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.10.5-1.fmi
- Repackaged since QEngine::Model ABI changed

* Fri Sep  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.9.9-1.fmi
- Repackaged due to ABI changes in timeseries library

* Thu Jul 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.7.28-1.fmi
- Repackaged due to QEngine ABI change

* Wed Jul 27 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.7.27-1.fmi
- Repackaged since macgyver CacheStats ABI changed

* Fri Jul 22 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.7.22-1.fmi
- Plugin initialization error hanldling update

* Wed Jul 20 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.7.20-1.fmi
- Repackage due to macgyver API changes

* Mon Jul 18 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.7.18-1.fmi
- Parameter case sensitivity update, fail loading plugin in case of stored query config errors

* Tue Jun 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.6.21-1.fmi
- Add support for RHEL9, upgrade libpqxx to 7.7.0 (rhel8+) and fmt to 8.1.1

* Tue May 31 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.5.31-1.fmi
- Repackage due to smartmet-engine-querydata and smartmet-engine-observation ABI changes

* Tue May 24 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.24-1.fmi
- Repackaged due to NFmiArea ABI changes

* Fri May 20 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.20-1.fmi
- Repackaged due to ABI changes to newbase LatLon methods

* Wed May  4 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.5.4-1.fmi
- Updated to use the latest Contour-engine API

* Tue May  3 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.5.3-1.fmi
- Update of stored query parameter handling

* Thu Apr 28 2022 Andris Pavenis <andris.pavenis@fmi.fi> 22.4.28-1.fmi
- Repackage due to SmartMet::Spine::Reactor ABI changes

* Wed Apr 13 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.4.13-1.fmi
- Update sounding observation support

* Thu Apr  7 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.4.7-1.fmi
- Improved sounding type support for sounding queries

* Mon Apr  4 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.4.4-1.fmi
- Add support for querying several sounding types

* Mon Mar 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.28-1.fmi
- Repackaged due to ABI changes in grid-content library

* Mon Mar 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.3.21-1.fmi
- Update due to changes in smartmet-library-spine and smartnet-library-timeseries

* Wed Mar 16 2022 Anssi Reponen <anssi.reponen@fmi.fi> - 22.3.16-1.fmi
- Added support for swe->sv language code conversion (BRAINSTORM-2276)

* Thu Mar 10 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.10-1.fmi
- Repackaged due to base library ABI changes

* Tue Mar 8 2022 Anssi Reponen <anssi.reponen@fmi.fi> - 22.3.8-1.fmi
- Started using timeseries-library (BRAINSTORM-2259)

* Mon Mar  7 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.3.7-1.fmi
- Repackaged due to base library API changes

* Mon Feb 28 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.2.28-1.fmi
- Repackaged due to base library/engine ABI changes

* Fri Feb 11 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.2.11-1.fmi
- StoredSoundingQuerieshandler: do not restrict soundingType to 1

* Wed Feb  9 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.2.9-1.fmi
- Repackaged due to ABI changes in grid libraries

* Tue Feb  1 2022 Anssi Reponen <anssi.reponen@fmi.fi> - 22.2.1-1.fmi
- Use DistanceParser for maxdistance URL- and config-parameter (BRAINSTORM-605)

* Mon Jan 31 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.31-1.fmi
- Use observation_data_r1 instead of observation_data_v1

* Tue Jan 25 2022 Mika Heiskanen <mika.heiskanen@fmi.fi> - 22.1.25-1.fmi
- Repackaged due to ABI changes in libraries/engine

* Fri Jan 21 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.1.21-1.fmi
- Repackage due to upgrade of packages from PGDG repo: gdal-3.4, geos-3.10, proj-8.2

* Mon Jan 10 2022 Andris Pavēnis <andris.pavenis@fmi.fi> 22.1.10-1.fmi
- Use Fmi::WorkerPool

* Tue Dec  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.12.7-1.fmi
- Upgrade to PostgreSQL 13 and GDAL-3.3

* Thu Nov 25 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.25-1.fmi
- Repackaged since Spine::location_parameter() API changed

* Mon Nov 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.15-1.fmi
- Repackaged due to ABI changes in base grid libraries

* Thu Nov 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.11.11-1.fmi
- Repackaged since ValueFormatter ABI changed

* Fri Oct 29 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.29-1.fmi
- Repackaged due to ABI changes in base grid libraries

* Fri Oct 29 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - upcoming
- Added test dependency for smartmet-library-newbase-devel

* Tue Oct 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.19-1.fmi
- Repackaged due to ABI changes in base grid libraries

* Mon Oct 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.11-1.fmi
- Simplified grid storage structures

* Mon Oct  4 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.10.4-1.fmi
- Repackaged due to grid-files ABI changes

* Wed Sep 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.15-2.fmi
- Improved error messages on configuration errors

* Wed Sep 15 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.9.15-1.fmi
- Repackaged due to NetCDF related ABI changes in base libraries

* Tue Sep  7 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.9.7-1.fmi
- Repackaged due to dependency changes (libconfig -> libconfig17)

* Mon Aug 30 2021 Anssi Reponen <anssi.reponen@fmi.fi> - 21.8.30-1.fmi
- Cache counters added (BRAINSTORM-1005)

* Tue Aug 24 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - 21.8.24-2.fmi
- To avoid unnecessary manual configuration, add missing layers from layerParamNameMap to layerMap to use layer name as table name for layers not entered in layerMap (BRAINSTORM-2137)

* Tue Aug 24 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - 21.8.24-1.fmi
- Use layerMap's layer name as default table name (BRAINSTORM-2137)
- Ignore dot (scheme) as the last character in table name when encapsulating it for database query

* Mon Aug 23 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - 21.8.23-1.fmi
- Enclose geoserver layer scheme and table name within quotes in database query (BRAINSTORM-2137)

* Sat Aug 21 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.21-1.fmi
- Repackaged due to LocalTimePool ABI changes

* Thu Aug 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.19-1.fmi
- Start using local time pool to avoid unnecessary allocations of local_date_time objects (BRAINSTORM-2122)

* Tue Aug 17 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.17-1.fmi
- Use the new shutdown API

* Mon Aug  2 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.8.2-1.fmi
- Use atomic_shared_ptr instead of atomic_store/load

* Tue Jul 20 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.7.20-1.fmi
- Use Fmi::Database::PostgreSQLConnection for geoserver database access

* Fri Jul  9 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.7.9-1.fmi
- Use libpqxx7 for RHEL8

* Mon Jul  5 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.7.5-1.fmi
- Rebuild after moving DataFilter from obsengine to spine

* Tue Jun 29 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.29-2.fmi
- Repackaged due to Observation::Engine::Settings ABI change

* Tue Jun  8 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.8-1.fmi
- Repackaged due to memory saving ABI changes in base libraries

* Tue Jun  1 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.6.1-1.fmi
- Repackaged due to ABI changes in grid libraries

* Fri May 28 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.5.28-1.fmi
- Use https://xml.fmi.fi for corresponding XML schema locations

* Tue May 25 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.25-1.fmi
- Grid-engine API for image painting changed

* Fri May 21 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.21-1.fmi
- Use FMI hash functions

* Thu May  6 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.5.6-1.fmi
- Repackaged due to ABI changes in NFmiAzimuthalArea

* Tue Apr 27 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.4.27-1.fmi
- Use HTTPS for INSPIRE schema locations

* Thu Apr 22 2021 Andris Pavēnis <andris.pavenis@fmi.fi> 21.4.22-1.fmi
- XML schema use update

* Thu Apr  1 2021 Pertti Kinnia <pertti.kinnia@fmi.fi> - 21.4.1-1.fmi
- Repackaged due to grid-files API changes

* Mon Mar  8 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.8-1.fmi
- Merged grid-branch to master

* Wed Mar  3 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.3-1.fmi
- Grid-engine may now be disabled

* Mon Mar  1 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.3.1-1.fmi
- Added support for numbered metaparameters

* Fri Feb 19 2021 Anssi Reponen <anssi.reponen@fmi.fi> - 21.2.19-1.fmi
- Added support for FMISIDs,WMOs,LPNNs in forecast queries (BRAINSTORM-1848)

* Thu Feb 18 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.18-1.fmi
- Repackaged due to newbase changes

* Tue Feb 16 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.16-1.fmi
- Repackaged due to NFmiArea ABI changes

* Thu Feb 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.2.11-1.fmi
- Merged master and WGS84 branches

* Wed Feb 3 2021 Anssi Reponen <anssi.reponen@fmi.fi> - 21.2.3-1.fmi
- Support for sensors (INSPIRE-874)
- Base library API changes

* Wed Jan 27 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.27-1.fmi
- Repackaged due to base library ABI changes

* Mon Jan 25 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.25-1.fmi
- Repackaged due to DirectoryMonitor API changes

* Tue Jan 19 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.19-1.fmi
- Repackaged due to base library ABI changes

* Thu Jan 14 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.14-1.fmi
- Repackaged smartmet to resolve debuginfo issues

* Mon Jan 11 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.11-1.fmi
- Repackaged due to grid-files API changes

* Fri Jan  8 2021 Andris Pavenis <andris.pavenis@fmi.fi> - 21.1.8-1.fmi
- Compatibility with RHEL8 update

* Mon Jan  4 2021 Mika Heiskanen <mika.heiskanen@fmi.fi> - 21.1.4-1.fmi
- Ported to GDAL 3.2

* Thu Dec  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.12.3-1.fmi
- Repackaged due to library ABI changes

* Mon Nov 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.30-1.fmi
- Repackaged due to gird-content library API changes

* Tue Nov 24 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.11.24-1.fmi
- Repackaged due to library ABI changes

* Wed Oct 28 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.28-1.fmi
- Rebuild due to fmt upgrade

* Thu Oct 22 2020 Pertti Kinnia <pertti.kinnia@fmi.fi> - 20.10.22-1.fmi
- Added call to setSensorNumber
- Fix multiple sensor requests
- Harmonized the use of Fmi::Exception in SupportsMeteoParameterOptions.cpp
- Rebuild due to libconfig upgrade to version 1.7.2

* Tue Oct 20 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.20-1.fmi
- Repackaged with the latest libconfig

* Thu Oct 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.15-1.fmi
- Repackaged due to library ABI changes

* Tue Oct  6 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.10.6-1.fmi
- Build update: use makefile.inc from smartmet-library-macgyver

* Thu Oct  1 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.10.1-1.fmi
- Repackaged due to library ABI changes

* Wed Sep 30 2020 Andris Pavēnis <andris.pavenis@fmi.fi> - 20.9.30-1.fmi
- Support overriding meteo parameter output accurracy for forecast queries

* Wed Sep 23 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.23-1.fmi
- Use Fmi::Exception instead of Spine::Exception

* Fri Sep 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.18-1.fmi
- Fixed parsing of stored obs querystring parameters, maxlocations setting was not obeyed

* Tue Sep 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.15-1.fmi
- Repackaged due to library ABI changes

* Mon Sep 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.14-1.fmi
- Repackaged due to library ABI changes

* Mon Sep  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.7-1.fmi
- Repackaged due to library ABI changes

* Thu Sep  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.3-2.fmi
- Updated IBPlott icedata schema address

* Thu Sep  3 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.9.3-1.fmi
- Improve shutdown support

* Wed Sep  2 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.9.1-1.fmi
- Repackaged since Observation::Settings size changed

* Mon Aug 31 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.31-1.fmi
- Repackaged due to library ABI changes

* Fri Aug 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.21-1.fmi
- Upgrade to fmt 6.2

* Thu Aug 20 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.8.20-1.fmi
- Adapt to smartmet-library-macgyver changes (use Fmi::AsyncTaskGroup)

* Tue Aug 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.18-1.fmi
- Repackaged due to grid library ABI changes

* Fri Aug 14 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.14-1.fmi
- Repackaged due to grid library ABI changes

* Tue Aug 11 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.8.11-1.fmi
- Speed improvements

* Fri Jul 31 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.7.31-1.fmi
- Repackaged due to libpqxx upgrade

* Wed Jul 22 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.7.22-1.fmi
- Stored query configuration support update
- Refaktorointi

* Tue Jul 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.21-1.fmi
- Improved handling of locations with fmisid information available

* Mon Jun 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.15-1.fmi
- Renamed .so to enable simultaneous installation of wfs and gribwfs

* Thu Jun 11 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.11-1.fmi
- Handle flash queries into the future more gracefully

* Wed Jun 10 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.6.10-1.fmi
- Fix returning location parameters in response

* Mon Jun  8 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.6.8-1.fmi
- Upgraded libpqxx dependencies
- Repackaged due to base library changes

* Fri May 15 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.5.15-1.fmi
- Repackaged due to base library changes

* Thu May 14 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.5.14-1.fmi
- Refactor (basic functionality in static library, add devel package)
- Makefile cleanups

* Tue May 12 2020 Anssi Reponen <anssi.reponen@fmi.fi> - 20.5.12-1.fmi
- Observation-engine API changed (BRAINSTORM-1678)

* Fri May  8 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.5.8-1.fmi
- Use CRSRegistry from smartmet-library-spine

* Tue May  5 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.5.5-1.fmi
- Avoid unnecessary stack traces
- Removed unused members from PluginImpl class

* Thu Apr 30 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.30-1.fmi
- Repackaged due to base library API changes

* Tue Apr 28 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.28-1.fmi
- Use GDAL 3.0

* Sat Apr 18 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.18-1.fmi
- Upgraded to Boost 1.69

* Wed Apr 15 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.4.15-1.fmi
- Class SupportsTimeZones now uses Fmi::TimeZines git from Geonames engine
- Support of location and time special parameters like in timeseries plugin
- Update of support of QC parameters (not yet in use and not supported in templates)

* Thu Apr  9 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.4.9-1.fmi
- Access SmartMet engines using C++ class virtual inheritance instead of initializing
  access separately each time

* Fri Apr  3 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.4.3-1.fmi
- Repackaged due to library API changes

* Thu Mar 12 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.3.12-1.fmi
- Rebuilt due to obsengine API changes

* Thu Mar  5 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.3.5-1.fmi
- Use ParameterTools from smartmet-library-spine (part 1)

* Tue Feb 25 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.25-1.fmi
- Repackaged due to base library API changes

* Fri Feb 21 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.2.21-1.fmi
- Support special parameters fom smartmet-engine-observation

* Wed Feb 19 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.2.19-1.fmi
- Use SHA1 instead of SHA for generating feature ID

* Thu Feb 13 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.13-1.fmi
- Register admin requests as private so that the frontend will not know about them

* Tue Feb 11 2020 Andris Pavenis <andris.pavenis@fmi.fi> - 20.2.11-1.fmi
- Use classes MultiLanguageString and MultiLanguageStringArray from smartmet-library-spine instead of local copies

* Sun Feb  9 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.9-1.fmi
- Fixed handling of missing wmo numbers

* Fri Feb  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.7-2.fmi
- Spine::Station API changed due to default construction of POD members

* Fri Feb  7 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.7-1.fmi
- Add support of boolean type for XML format requests

* Thu Feb  6 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.2.6-1.fmi
- WMO numbers have been deprecated, removed them from responses
- Minor fix to request base output handling
- Added support special boost posix_time values
- Renamed admin request "constructorMap" to "constructors"

* Wed Jan 29 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.29-1.fmi
- Repackaged due to base library API changes
- Add admin request constructorMap

* Thu Jan 23 2020 Andri Pavenis <andris.pavenis@fmi.fi> -20.1.23-1.fmi
- New version [Merge from master branch]
- Add support of request fallback encoding when not UTF-8 (20.1.23-1.fmi)
- Update support of outputFormat parameter (20.1.21-1.fmi)
- Update monitoring stored query configuration changes (19.11.29-1.fmi)
- Additionally add smartmet-engines-grid to dependencies
- Add support of request fallback encoding when not UTF-8

* Tue Jan 21 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.21-1.fmi
- Repackaged due to grid-content and grid-engine API changes

* Thu Jan 16 2020 Mika Heiskanen <mika.heiskanen@fmi.fi> - 20.1.16-1.fmi
- Repackaged due to base library API changes

* Thu Dec 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.12-1.fmi
- Upgrade to GDAL 3.0

* Wed Dec 11 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.11-1.fmi
- Fixed handling of "{name}.raw" parameters

* Wed Dec  4 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.12.4-1.fmi
- Repackaged due to base library changes

* Fri Nov 29 2019 Andris Pavenis <andris.pavenis@fmi.fi> - 19.11.29-1.fmi
- Update monitoring stored query configuration changes

* Fri Nov 22 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.22-1.fmi
- Repackaged due to API changes in grid-content library

* Wed Nov 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.20-1.fmi
- Repackaged due to newbase/spine ABI changes

* Thu Nov  7 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.11.7-1.fmi
- Repackaged due to library/engine changes

* Wed Oct 30 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.30-1.fmi
- Full repackaging of GRIB server components

* Mon Oct 21 2019 Anssi Reponen <anssi.reponen@fmi.fi> - 19.10.21-1.fmi
- Added support for PAP_PT1S_AVG parameter in sounding query (INSPIRE-899)
- Test added for sounding data (BRAINSTORM-1694)

* Tue Oct 15 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.15-1.fmi
- Improved handling of missing observations

* Tue Oct  1 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.10.1-1.fmi
- Repackaged due to SmartMet library ABI changes

* Thu Sep 26 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.26-1.fmi
- Fixed thread safety issue in hostname handling

* Fri Sep 20 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.20-1.fmi
- Repackaged everything with -fno-omit-frame-pointer

* Thu Sep 19 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.19-1.fmi
- New release version

* Thu Sep 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.12-1.fmi
- Repackaged due to obsengine ABI changes (virtual destructors added)

* Thu Sep  5 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.5-1.fmi
- Removed unnecessary support for location parameters in contour handlers

* Wed Sep  4 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.9.4-1.fmi
- Optimized stored lightning queries for speed, generating the rows of data via the template is too slow

* Wed Aug 28 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.28-2.fmi
- Repackaged since Spine::Location ABI changed

* Wed Aug 28 2019 Anssi Reponen <anssi.reponen@fmi.fi> - 19.8.28-1.fmi
- Added data source to cache key (source URL-parameter or primaryForecastSource config-parameter)

* Mon Aug 26 2019 Anssi Reponen <anssi.reponen@fmi.fi> - 19.8.26-1.fmi
- Limits of contours supported also as URL-parameter in grid-version (BRAINSTORM-1656)
- Added support for primaryForecastSource, gridengine_disabled configuration parameters

* Mon Aug 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.12-1.fmi
- Fixed handling of missing analysis times

* Fri Aug  9 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.8.9-1.fmi
- Added analysis time to contour queries

* Tue Jul 30 2019 Andris Pavenis <andris.pavenis@fmi.fi> - 19.7.30-1.fmi
- XML schema validation update (supports XML schema download including
  through proxy (BRAINSTORM-1640)
- Admin request support (BRAINSTORM-1645)

* Tue Jun 25 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.6.25-1.fmi
- Improved WMS server and APIKEY settings

* Tue Jun 18 2019 Andris Pavenis <andris.pavenis@fmi.fi> - 19.6.18-1.fmi
- Add WMS server address to plugin configuration

* Fri Jun 14 2019 Andris Pavenis <andris.pavenis@fmi.fi> - 19.6.14-1.fmi
- Do not use std::ostringstream in SmartMet::Plugin::WFS::SupportsTimeZone

* Wed Jun 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.6.12-1.fmi
- Fixed flash queries to use the spatialite cache if possible by not using the makeQuery API

* Fri Jun  7 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.6.7-1.fmi
- Repackaged due to obsengine changes

* Tue May 14 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.5.15-1.fmi
- Repackaged due to engine changes

* Mon May 13 2019 Andris Pavenis <andris.pavenis@fmi.fi> - 19.5.13-1.fmi
- Make config param geoserverConnStr optional

* Mon May  6 2019 Andris Pavenis <andris.pavenis@fmi.fi> - 19.5.6-1.fmi
- Plugin reload support (BRAINSTORM-1030)

* Wed Apr 24 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.4.24-1.fmi
- Repackaged due to obsengine changes

* Thu Feb 21 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.21-2.fmi
- Repackaged since Contour::Options size changed

* Thu Feb 21 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.21-1.fmi
- Add client IP to console stack traces

* Mon Feb 18 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.18-1.fmi
- Fix to scope of variables used in futures
- Schema version fixes
- Cache schemas

* Tue Feb 12 2019 Mika Heiskanen <mika.heiskanen@fmi.fi> - 19.2.12-1.fmi
- Use futures instead of threads to avoid exit on exceptions
- Limit floating point accuracy of printed coordinates
- More logging
- Get locale from configuration files

* Tue Dec  4 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.12.4-1.fmi
- Repackaged since Spine::Table size changed

* Fri Nov 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.23-1.fmi
- Fixed handling of timestep=0

* Mon Nov 12 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.12-1.fmi
- Refactored TemplateFactory into macgyver library

* Fri Nov  9 2018 Anssi Reponen <anssi.reponen@fmi.fi> - 18.11.9-2.fmi
- Support for data_source-field added (BRAINSTORM-1233)

* Fri Nov  9 2018 Anssi Reponen <anssi.reponen@fmi.fi> - 18.11.9-1.fmi
- Limits of contours supported also as a URL-parameter (BRAINSTORM-864)
- Support for data_source-field added (BRAINSTORM-1233)

* Thu Nov  8 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.8-1.fmi
- Fixed thread safety issues on caching template formatters

* Tue Nov  6 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.11.6-1.fmi
- Fixed destructors not to throw
- Fixed std::ostringstream output to use str() methods

* Thu Oct 18 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.18-3.fmi
- Disabled stack traces if user requested parameters are invalid

* Thu Oct 18 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.18-2.fmi
- Disabled stack traces if user reqested time parameters are invalid

* Thu Oct 18 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.18-1.fmi
- Do not print a stack trace if user provided location name is unavailable to avoid unnecessary logging

* Thu Oct 11 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.11-2.fmi
- Use C++11 thread_local instead of boost::thread_specific_ptr for thread safety

* Thu Oct 11 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.11-1.fmi
- Replaced unsafe TemplateFormatterMT with TemplateFactory

* Mon Oct  1 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.10.1-1.fmi
- Do not use boost::lexical_cast

* Sat Sep 29 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.9.29-1.fmi
- Upgrade to latest fmt

* Tue Sep 11 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.9.11-1.fmi
- Using querydata modification time as data result time for contours too

* Mon Sep  3 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.9.3-1.fmi
- Test result changes; querydata modification times changed to origin time

* Tue Aug 28 2018 Pertti Kinnia <pertti.kinnia@fmi.fi> - 18.8.28-1.fmi
- Using querydata modification time as data result time

* Thu Aug 16 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.16-1.fmi
- Switched to use new castTo API in obsengine, and prefer toString instead

* Mon Aug 13 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.13-1.fmi
- Repackaged since Spine::Location size changed

* Thu Aug  9 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.8.9-1.fmi
- Value ABI change forced repackaging

* Thu Jul 26 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.7.26-1.fmi
- Prefer nullptr over NULL

* Mon Jul 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.7.23-1.fmi
- Repackaged since spine ValueFormatter ABI changed

* Thu Jun  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.6.7-1.fmi
- Use the new TimedCache instead of the unsafe Cache with InstantExpiration policy

* Wed May 23 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.23-1.fmi
- Fixed cache expiration mechanism

* Mon May 21 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.21-1.fmi
- Arrange soundings using message_time to get the latest request parameter work correctly

* Mon May  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.7-2.fmi
- Require newer querydata-engine so that data independent latlon parameters can be used

* Mon May  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.7-1.fmi
- Changed latitude and longitude to be data derived parameters

* Wed May  2 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.5.2-1.fmi
- Repackaged since newbase NFmiEnumConverter ABI changed
- Use wml2:qualifier element to view quality codes instead of wml2:quality

* Wed Apr 25 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.25-1.fmi
- Add linear transformation parameters to the radar result

* Thu Apr 12 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.12-1.fmi
- Fixed GetCapabilities for capabilities.template to be for commercial services.

* Sat Apr  7 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.4.7-1.fmi
- Upgrade to boost 1.66

* Tue Mar 20 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.20-1.fmi
- Full recompile of all server plugins

* Mon Mar 19 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.19-3.fmi
- Removed obsolete call to Observation::Engine::setGeonames

* Mon Mar 19 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.19-2.fmi
- Added more ice model parameters to ibplott_ice_array.template 

* Mon Mar 19 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.19-1.fmi
- Recompiled due to obsengine API changes

* Mon Mar 12 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.3.12-1.fmi
- Added IceSpeed, IceDirection, RaftIceThickness and RaftIceConcentration to ibplott template

* Tue Feb 27 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.27-1.fmi
- Improved shutdown code to use thread joining

* Tue Feb 20 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.20-1.fmi
- Added check against empty obsengine results

* Mon Feb 19 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.19-1.fmi
- Fixed shutdown not to segfault

* Fri Feb  9 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.2.9-1.fmi
- Repackaged due to TimeZones API change

* Mon Jan 15 2018 Mika Heiskanen <mika.heiskanen@fmi.fi> - 18.1.15-1.fmi
- Updated pqxx and postgresql (9.5) dependencies

* Fri Dec 15 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.12.15-1.fmi
- Bug fix to coordinate handling

* Thu Nov  9 2017 Santeri Oksman <santeri.oksman@fmi.fi> - 17.11.9-1.fmi
- With multifile data q_engine->get() origintime must not be set/locked; SOL-5897

* Wed Nov  8 2017  <santeri.oksman@fmi.fi> - 17.11.8-1.fmi
- Added templates for querydata observations in multipointcoverage and timevaluepair formats

* Tue Nov  7 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.7-1.fmi
- Added automatic reload of modified stored queries

* Wed Nov  1 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.11.1-1.fmi
- Rebuilt due to GIS-library API change

* Wed Sep 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.27-1.fmi
- Improvements into environmental monitoring facility parameters

* Thu Sep 21 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.9.21-1.fmi
- Rebuilt with the latest contourer

* Mon Aug 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.8.28-1.fmi
- Upgrade to boost 1.65

* Thu Jul 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.7.27-1.fmi
- Fixed maxdistance units to be kilometers when calling geoengine

* Mon Jul 10 2017 Ville Karppinen <ville.karppinen@fmi.fi> - 17.7.10-1.fmi
- IceThickness parameter added into ibplott_ice_array.template.

* Mon May 29 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.5.29-1.fmi
- Replace few suggest methods with nameSearch to avoid server crash.

* Wed May 24 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.5.24-1.fmi
- Check joinability of an option thread in StoredEnvMonitoringFacilityQueryHandler.

* Tue May 23 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.5.23-1.fmi
- Add tests to test data fetching from arbitrary heights of hybrid forecast model.
- Add support for data fetching from an arbitrary height from model topography.

* Fri May 05 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.5.5-1.fmi
- http/https scheme selection based on X-Forwarded-Proto header; STU-5084

* Wed Apr 26 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.26-1.fmi
- Format the code with new clang-format rules and fixed the compilation.
- Changed most configuration path settings to be relative to the configuration file.

* Tue Apr 25 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.25-1.fmi
- Fixed bug on dereferencing empty contour results

* Mon Apr 10 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.10-1.fmi
- Added detection of global data that needs to be wrapped around when contouring

* Sat Apr  8 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.8-1.fmi
- Simplified error reporting

* Mon Apr  3 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.4.3-1.fmi
- Add keywords support to StoredSoundingQueryHandler
- Modifications due to observation engine API changes:
  redundant parameter in values-function removed,
  redundant Engine's Interface base class removed

* Wed Mar 15 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.15-1.fmi
- Recompiled since Spine::Exception changed

* Tue Mar 14 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.14-1.fmi
- Switched to using macgyver StringConversion tools
- Sounding measurement positions are not relative any more.
- Set a replacement value if sounding measurement is missing.
- Implement feature id to EMF and EMN handler.
- Authority domain is now configurable on EMF and EMN stored queries.
- Add station network class configuration capability to EMF and EMN handlers
- Remove bounding box calculation from EMN handler
- Make observing capabilities as optional in EMF handler.
- Add parameter name for observing capability of EMF
- Add nillReason attribute for empty elements of EMF response
- Make Inspire namespace identifier configurable in EMF and EMN stored queries.

* Mon Feb 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.13-3.fmi
- Remove extra slash punctuation character from a namespace of EMN template.

* Mon Feb 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.13-2.fmi
- Use correct INSPIRE code list register references in EMF and EMN template. 

* Mon Feb 13 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.13-1.fmi
- Use INSPIRE code list register category for MeasurementRegime
- Remove extra slash punctuation character from a namespace of EMF template

* Sat Feb 11 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.11-1.fmi
- Repackaged due to newbase API changes

* Thu Feb  2 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.2-1.fmi
- Unified handling of apikeys

* Thu Jan  5 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.5-1.fmi
- Fixed typos in configuration files

* Wed Jan  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.4-1.fmi
- Changed to use renamed SmartMet base libraries

* Wed Nov 30 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.30-1.fmi
- Using test databases in test configuration
- No installation for configuration
- Note: tests not yet working properly against test databases

* Tue Nov 29 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.29-1.fmi
- Added fmi::forecast::ecmwf::surface::coverage::icing.conf
- Quality code support added to sounding SQ.
- MaxSoundings restriction parameter added to sounding SQ.
- Added coordinate conversion possibility to sounding SQ.
- Disabled AERO sounding observation type.
- Added beginPosition and endPosition for an sounding observation.
- Prevent private soundings to go public.
- The plugin tests are now using installed files as much as possible.
- The plugin test environment changed to use SmartMet naming style.
- Added a stored query for sounding observations.
- Added a XML template for sounding observations.
- Added a custom handler which fetch sounding observations from observation engine.
- Added TrajectoryObservation feature type configuration.
- Fixed the plugin rpm build.

* Mon Nov 14 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.14-2.fmi
- Re-enable contour smoothing

* Mon Nov 14 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.14-1.fmi
- Added Copernicus stored queries

* Tue Nov  1 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.11.1-1.fmi
- Pori Rajakari surface temperature buoy added to the list of default fmisids of wave stored queries.
- Namespace changed
- Optimized contour formatting for speed

* Tue Sep 13 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.9.13-1.fmi
- Code modified bacause of Contour-engine API changes: vector of isolines/isobands queried at once instead of only one isoline/isoband

* Tue Sep  6 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.9.6-1.fmi
- New exception handler
- Using a simpler caching method for XML responses.
- Content handler registrations changed so that all requests go through the callRequestHanlder() method.

* Tue Aug 30 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.8.30-1.fmi
- Base class API change
- Use response code 400 instead of 503
- Added fmi::forecast::wamec::grid
- Added fmi::forecast::edited::weather::scandinavia::grid

* Tue Aug 23 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.8.23-1.fmi
- Hotfix: disable smoothing for now, it caused segmentation faults

* Mon Aug 15 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.8.15-1.fmi
- Fixed regression tests and the results.
- Added a new sqd datafile used in winterweather probability test.
- Removed the sorting of latlon boundary points causing invalid values in AreaUtils.
- Handler parameter for smoothing parameters added (smoothing_degree, smoothing_size)
- Optional smoothing of isolines ('smoothing' handler parameter)
- The init(),shutdown() and requestHandler() methods are now protected methods
- The requestHandler() method is called from the callRequestHandler() method

* Thu Jul 21 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.7.21-1.fmi
- Added more default buoy stations to the wave obs stored queries.
- Fixed Tuliset-data size to 850x1345 like it is in the original geotiff

* Thu Jun 30 2016 Roope Tervo <roope.tervo@fmi.fi> - 16.6.30-1.fmi
- Fixed Satellite Sentinel 1 stored query WMS url

* Wed Jun 29 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.29-1.fmi
- Fixed Satellite Sentinel 1 WFS bbox and default time range
- QEngine API changed, fixed calls to QEngine::find accordingly

* Tue Jun 28 2016 Roope Tervo <roope.tervo@fmi.fi> - 16.6.28-1.fmi
- Added Satellite Sentinel 1 WFS stored query

* Tue Jun 14 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.14-1.fmi
- Full recompile

* Thu Jun  2 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.2-1.fmi
- Full recompile

* Wed Jun  1 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.1-1.fmi
- Added graceful shutdown

* Mon May 16 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.5.16-2.fmi
- Added new Testlab stored queries

* Mon May 16 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.5.16-1.fmi
- Use TimeZones instead of TimeZoneFactory

* Wed Apr 20 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.4.20-1.fmi
- Rebuild against new Contour-engine
- More corrections for test results.
- Stored query abstract fixes.
- Added integer list support for the levels parameter of Hirlam pressure grid SQ.
- Some corrections for test results
- Using static stations files on real_open tests.
- Switched to use the latest TimeSeriesGenerator API which handles climatological data too

* Wed Mar 30 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.30-1.fmi
- Reintroduced global myocean

* Wed Mar 23 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.23-1.fmi
- Reverted MyOcean stored queries to match arctic data set

* Tue Mar 22 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.22-1.fmi
- Fixed template configuration for the stored queries of wave data.

* Wed Mar 16 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.16-1.fmi
- Segfault hotfix

* Wed Mar  9 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.9-1.fmi
- New MyOcean stored queries for TestLab

* Mon Mar  7 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.4-1.fmi
- Fixed TopLink products
- Added possibility to select vertical layer of HBM model.
- Added possibility to select HBM response data file format.
- Opendata tests were separated from 'real' tests into 'real_open'.

* Thu Mar  3 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.3.3-1.fmi
- Modified MyOcean queries to behave correctly with global data

* Tue Feb  9 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.2.9-1.fmi
- Rebuilt against the new TimeSeries::Value definition

* Tue Feb  2 2016 Tuomo Lauri <tuomo.lauri@fmi.fi> - 16.2.2-1.fmi
- Pointers to std::shared_ptr replacement of QueryResult container (API change)
- Added Petajavesi radar layers.
- Now using Timeseries None-type
- Added Petajavesi radar layers.
- Descriptions of storedquery parameters were fixed and improved.

* Sat Jan 23 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.23-1.fmi
- Fmi::TimeZoneFactory API changed

* Mon Jan 18 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.18-1.fmi
- newbase API changed, full recompile

* Fri Jan 15 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.15-1.fmi
- Added safeguard against not finding fes:Filter:TMP root

* Mon Jan 11 2016 Santeri Oksman <santeri.oksman@fmi.fi> - 16.1.11-1.fmi
- Added safeguard against empty result in StoredGridQueryHandler

* Tue Dec 15 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.12.15-1.fmi
- Removed the name check for wmos and fmisids

* Mon Dec 14 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.12.14-1.fmi
- Replacing timestep value 0 as 1 in StoredObsQueryHandler

* Fri Dec  4 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.11.4-1.fmi
- Enabled  observation cache

* Mon Nov 30 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.11.30-1.fmi
- STUK inspire id targets added.

* Wed Nov 25 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.11.25-1.fmi
- Updated templates
- Added parameters to pollen stored queries

* Wed Nov 18 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.11.18-1.fmi
- Added wave height and wind coverage stored queries
- SmartMetPlugin now receives a const HTTP Request
- Added HourlyMaximumGust parameter to ECMWF stored point queries

* Mon Nov  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.9-1.fmi
- Using fast case conversion without locale locks when possible

* Tue Nov  3 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.11.3-1.fmi
- Stopped using deprecated Cast.h functions
- Improvements to regression tests

* Wed Oct 28 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.10.28-1.fmi
- Return only surface level values of HBM
- Removed default geoids from oaas::sealevel queries
- Using overwritable keyword in all sealevel::point queries
- Added STUK class info feature restrictions of location searches
- Added DescribeFeatureType requests

* Mon Oct 26 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.10.26-1.fmi
- Added proper debuginfo packaging

* Tue Oct 20 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.10.20-1.fmi
- Fixes oaas stored query
- STUK-related things

* Mon Oct 12 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.10.12-1.fmi
- Added StoredQueryHandlers for isoline and coverage stored queries
- Now limitings foreign observation results to maximum of 200 stations

* Wed Sep 30 2015 Santeri Oksman <santeri.oksman@fmi.fi> - 15.9.30-1.fmi
- WinterWeather probabilities can now be requested with typenames

* Tue Sep 29 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.9.29-1.fmi
- Now limitings foreign observation results to maximum of 200 stations
- WinterWeather probability product, iteration 1
- Various opendata additions

* Tue Sep 15 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.9.15-1.fmi
- Fixed segfault bug in WWContourHandler

* Thu Sep 10 2015 Santeri Oksman <santeri.oksman@fmi.fi> - 15.9.10-2.fmi
- Removed possibility to change producer in winterweather queries

* Thu Sep 10 2015 Roope Tervo <roope.tervo@fmi.fi> - 15.9.10-1.fmi
- Changed typename for wp:WinterWeatherGeneralContours

* Tue Sep  8 2015 Santeri Oksman <santeri.oksman@fmi.fi> - 15.9.8-1.fmi
- Added TOPLINK stored query numbers 2 and 3

* Wed Sep  2 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.9.2-1.fmi
- Added MyOcean stored queries

* Thu Aug 27 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.27-1.fmi
- TimeSeriesGenerator API changed

* Tue Aug 25 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.25-1.fmi
- Recompiled due to obsengine API changes

* Mon Aug 24 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.24-1.fmi
- Recompiled due to Convenience.h API changes

* Thu Aug 20 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.8.20-1.fmi
- Added urban air quality stored queries

* Tue Aug 18 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.18-1.fmi
- Use time formatters from macgyver to avoid global locks from sstreams

* Mon Aug 17 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.17-1.fmi
- Use -fno-omit-frame-pointer to improve perf use

* Fri Aug 14 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.8.14-1.fmi
- Recompiled with the latest version of spine jne which

* Tue Aug  4 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.8.4-1.fmi
- Added TOPLINK products
- WFS implementation improvements

* Mon May 25 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.5.25-1.fmi
- Adjusted parameter types in Forecast Query Handler
- Added pollen stored forecast queries

* Tue May  5 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.5.5-1.fmi
- Fixed capabilities: fixed schema version
- Removed radar Vimpeli hclass layer for not being available

* Thu Apr 30 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.29-1.fmi
- Increased lighting bounding box to cover whole scandinavia
- Regression test result changes.
- Inspire schema version changes to official versions (e.g. 2.0rc3 -> 2.0).
- XMLGrammar pool and XMLSchema cache update.
- Added aliases for allowed output formats

* Tue Apr 14 2015 Santeri Oksman <santeri.oksman@fmi.fi> - 15.4.14-1.fmi
- Rebuild against new obsengine

* Thu Apr  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.9-2.fmi
- Fixed flash parameters to be regular data types

* Thu Apr  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.9-1.fmi
- newbase API changed

* Wed Apr  8 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.8-1.fmi
- Added Kesalahti radar layers and few missing hclass layers into opendata stored queries.
- Using dynamic linking of smartmet libraries

* Mon Mar 23 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.3.23-1.fmi
- Removed some compile time warnings (uninitialized boost::optional parameters)
- Added an inspire id for sea ice forcasts.

* Tue Feb 24 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.24-1.fmi
- Recompiled due to linkage changes in newbase

* Mon Feb 23 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.2.23-1.fmi
- Feature id (gml:id) added in the members of mast stored query result.

* Mon Feb 16 2015 Tuomo Lauri <tuomo.lauri@fmi.fi> - 15.2.16-1.fmi
- NetCDF codespace uri change (previously defined target xml is not available anymore).
- XML schema cache and XML grammar pool file updated.
- GetCapabilities respose template for opendata was greated and put into operation.
- INSPIRE-656 Fixed GetFeature hits only respose xsi namespace problem.
- INSPIRE-658 Fixed GetPropertyValue response document schema location problem.
- INSPIRE-657 Fixed GetFeature response document schema location problem.
- INSPIRE-655 Fixed GetCapabilities response namespace problems.
- SupportsLocationParameters class does not anymore include wmo or fmisid locations into result list unless separately requested.
- Mareograph station 100669 added into the default fmisid list of mareo stored queries.

* Mon Jan 26 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.1.26-1.fmi
- Removed Gis Engine default crs dependency.
- First implementation of Filter Encoding functionality.

* Thu Jan 15 2015 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 15.1.15-2.fmi
- Gis engine return random default crs, so using a static one for iwxxm queries.

* Thu Jan 15 2015 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 15.1.15-1.fmi
- Rebuild with correct spine version.

* Mon Jan 12 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.1.12-1.fmi
- Most of the WFS simple feature stored queries moved from commercial to opendata folder.
- Top level gml schema namespace name change in WFS simple xml template. 
- WFS simple schema location uri change in WFS simple xml template.

* Wed Jan  7 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.1.7-1.fmi
- Recompiled due to obsengine API changes

* Thu Dec 18 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.12.18-1.fmi
- Helmi ice model stored query is now visible.
- New StoredMastQueryHandler handler and a stored query for mast data.
- StoredAviationObservationQueryHandler changed to use DBRegistry of ObsEngine.

* Mon Dec 15 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.12.15-1.fmi
- Fine tuned ECMWF stored query descriptions

* Fri Dec 12 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.12.12-1.fmi
- Added ECMWF stored query to commercial stored queries

* Tue Nov 25 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.11.25-1.fmi
- Srs reference to crs with epoch in flash SQ template.

* Thu Oct 23 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.10.23-1.fmi
- fmi::avi::* storedqueries are not BETA anymore.
- Stored query default parameter additions (WaWa and m_man).
- Parameter / layer mapping into GeoserverHandler added and the mapping is used in radar SQs.
- Stored Query configurations can now be overridden when redefined
- boundedBy coordinates truncation in StoredAviationObservationQueryHandler.
- Improved fmi::avi::observations:: SQ descriptions.

* Mon Sep  8 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.9.8-1.fmi
- Recompiled due to geoengine API changes

* Tue Aug 26 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.8.26-1.fmi
- Axis labels atribute added in boundedBy element of aviation observation template.
- Swap of iwxxm boundedBy coordinates if CRS registry requires it.
- 4 stored query regression tests for livi observations with output samples.
- New License added into GetCapabilities response.
- Avoid to add wfs:member to the child of wfs:member when GetPropertyValue result is formed.
- Added namespace declarations into aviation observation template needed by GetPropertyValue request.

* Wed Jul 9 2014 Roope Tervo <roope.tervo@fmi.fi> - 14.7.9-1.fmi
- Moved simple feature stored queriest to customer side and unhide them to test geoserver wfs store.

* Tue Jul 1 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.7.1-1.fmi
- Road weather SQ starttime lowerLimit changed to 2010-01-01T00:00:00Z.
- Regression test update
- Initial version of StoredAviationObservationQueryHandler.

* Mon Jun 30 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.6.30-1.fmi
- Recompiled due to spine API changes
- Wfs simple fixed to follow the schemas it uses.
- Test environment update enabled Gis engine support.
- Changed the simple stored queries in opendata pool to hidden state. Those are not yet ready to go public.

* Thu Jun 19 2014 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 14.6.19-1.fmi
- Hotfix: Reverting a commit that has broken GetCapabilities response.

* Mon May 26 2014 Ville Karppinen <ville.karppinen@fmi.fi> - 14.5.26-1.fmi
- Added suomi_tuliset_rr_eureffin.conf for commercial stored queries

* Thu May 22 2014 Roope Tervo <roope.tervo@fmi.fi> - 14.5.22-1.fmi
- Added simple feature stored queries and templates

* Wed May 14 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.5.14-1.fmi
- Use shared macgyver and locus libraries

* Thu May  8 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.5.8-1.fmi
- Fixed UV-stored query parameters

* Tue May  6 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.5.7-1.fmi
- Hotfix to timeparser

* Tue May  6 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.5.6-1.fmi
- qengine API changed

* Mon Apr 28 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.28-1.fmi
- Full recompile due to large changes in spine etc APIs

* Tue Apr 1 2014 Roope Tervo <roope.tervo@fmi.fi> - 14.4.1-1.fmi
- Added UVIndex to uv stored query

* Fri Mar 28 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.3.28-2.fmi
- Fixed bug in foreign observations stored query

* Fri Mar 28 2014 Roope TErvo <roope.tervo@fmi.fi> - 14.3.28-1.fmia
- Changed deprecated uv data to the new one in fmi::forecast::ecmwf::uv::point::* in private stored queries

* Thu Mar 20 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.3.20-1.fmi
- Hirlam pressure SQ: Hidden parameter value configured from true to false.
- Quality code support is deactivated until quality code data is ready to go public.
- Quality code support implemented separate ways on multipointcoverage and timevaluepair formats.
- Quality code implementation not using anymore metadata element block, because result validation failed on some parsers.
- Quality code links are not shown in results and meaning of gmd:dataQualityInfo and gmd:dataSetURI is changed.
- Included crs, timestep and timezone parameters into the quality code link of observation response.
- Added parameters option into mareograph SQ configurations.
- Improved quality code link implementation for observation parameters: timevaluepair and multipointcoverage.

* Thu Feb 27 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.2.27-2.fmi
- Fixed foreign obs stored queries

* Thu Feb 27 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.2.27-1.fmi
- Added new stored query for foreign observations

* Tue Feb 4 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.2.4-1.fmi
- Avoid oracle errors thrown by obsengine when a proper stationType is not configured for an alias parameter.
- Fix: Geoengine nameSearch language code conversion from "eng" to "en" and "fin" to "fi".

* Mon Feb 3 2014 Mikko Visa <mikko.visa@fmi.fi> - 14.2.3-2.fmi
- Open data 2014-02-03 release
- 30 year reference SQs: data type of geoid changed from uint to int.
- Removed an empty line from template of timevaluepair result.
- Support for getting location options by using wmoids and fmisids.
- Fixed atmoshphere typo in stored query configurations.
- HELMI forecast model configuration set to hidden mode.
- RadiationDiffuseAccumulation removed from the list of default parameters.
- Stored query abstract configuration fix. (fmi::forecast::hirlam::surface::finland::grid)
- Added a hidden stored query for 30 year normal period data fetched by using Inspire-ID 1000550.
- Changed default parameter VerticalVelocityMMS to VelocityPotential in fmi::forecast::hirlam::pressure::grid configuration.
- Changed unknown missing value back to NaN because lack of support in WaterML schema.
- Keeping meteo parameters as case sensitive in stored query response xml files.
- Hidden true value set into fmi::forecast::hirlam::pressure::grid configuration.
- Mimetype of grib and netcdf file changed to application/octet-stream reported in grid type stored query response xml.
- MissingText of the 30-year normal period stored queries changed from NaN to unknown.
- Implemented quality code support for parameters like qc_t2m.
- Stored query format parameter update of Helmi model as grid format.
- Grid type stored query template codespace update from version 25 to 26.
- UVB_U observation parameter included into the default parameter list of radiation SQs. 
- Missingtext changed from NaN to unknown in commercial stored query configurations.
- Geoid included into stored queries that use wfs_obs_handler_factory constructor.
- Fixed missingText values from NaN to unknown of stored queries.
- Implemented format selection for the units parameter used in metaplugin links.
- Removed RadiationNetTopAtmLW parameter from defaults of hirlam::surface*grid SQs.
- Sun radiation observation stored queries got 3 parameters into default.
- hirlam::surface*grid SQs got some new Radiation accumulated parameters.
- Fixed gml:TimePeriod and gml:TimeInstant xlink:href links of timevaluepair Obs SQs
- Timestep of Mareograph observations changed from 15 min to 60 min.

* Thu Jan 16 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.1.16-1.fmi
- Improved IBPlott-specific stored query

* Mon Jan 13 2014 Santeri Oksman <santeri.oksman@fmi.fi> - 14.1.13-2.fmi
- Rebuild with newer macgyver

* Mon Jan 13 2014 Santeri Oksman <santeri.oksman@fmi.fi> - 14.1.13-1.fmi
- Added format parameter for the hirlam pressure grid stored query.
- Removed version number from the media type of grib.
- Removed hourly min and max parameters from soil stored queries.
- Logging verbosity decreased when plugin is loaded into brainstorm.
- Requested parameter names are now checked before the actual obsengine query.
- Activated stored queries of 30year normal periods.
- Increased BuildRequires version of smartmet-library-macgyver.
- SQ observations: lowerLimit value of starttime parameter changed to 1829-01-01T00:00:00.
- Avoid 'no data found' and 'no data available' exceptions in SQ results.

* Thu Dec 12 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.12.12-2.fmi
- Fixed ice velocity param names in IBPlott stored query

* Thu Dec 12 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.12.12-1.fmi
- LevelType of HELMI stored query changed to match with querydata.
- Added new stored query for IBPlott xml arrays.
- Added new handler for array-like stored queries
- Created HELMI Ice Model stored query configuration.
- Created 4 stored query configurations for the parameters of 30year normal periods.
- Codespace URLs of grid type stored queries defined into result template.
- Stored queries geoid types changed from uint to int.
- Abstracts, returnType and separateGroups fixes for stored queries.
- Fixed file name to match with soil SQ ID (fmi::observations::soil::hourly::timevaluepair).
- Changed place parameter examples of the soil SQ abstracts.
- Created stored query configuration as multipoincoverage format for soil parameters
- Created test/base targets for opendata configuration files and updated the tests.
- Took empty direction values into account which fix a regression test problem.

* Fri Nov 29 2013 Santeri Oksman <santeri.oksman@fmi.fi> - 13.11.29-1.fmi
- Recompiled due obsengine settings change

* Mon Nov 25 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.25-1.fmi
- Created 2 stored query configuration files for soil parameters.
- Optional formats parameter implemented for stored query configuration.
- MimeType fixed to match with the file format of grid type SQ result.

* Thu Nov 14 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.14-1.fmi
- Changed to use the Locus library

* Tue Nov  5 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.5-1.fmi
- Added new stored query for hirlam forecast pressure levels.
- Now complying with the new getMethod - API
- Added two stored queries for soil observations.

* Wed Oct  9 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.10.9-1.fmi
- Now conforming with the new Reactor initialization API

* Mon Sep 23 2013 Tuomo Lauri    <tuomo.lauri@fmi.fi>    - 13.9.23-1.fmi
- Starttime and endtime value relay from request to download URL.
- Multiple stored query folder support.
- Segregation of stored queries to opendata and commercial folders.
- New main configuration file for opendata.

* Fri Sep 6  2013 Tuomo Lauri    <tuomo.lauri@fmi.fi>    - 13.9.6-1.fmi
- Recompiled due Spine changes

* Thu Sep  5 2013 Tuomo Lauri        <tuomo.lauri@fmi.fi>     - 13.9.5-1.fmi
- Added ECMWFS UV data to stored queries
- Configured expiresSeconds kvp for every stored query.

* Wed Aug 28 2013 Tuomo Lauri        <tuomo.lauri@fmi.fi>     - 13.8.28-1.fmi
- New release

* Thu Aug 15 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.8.15-1.fmi
- StoredObsQueryHandlers maxStationCount, maxEpochs and maxHours now depends on storedQueryRestrictions parameter.
- The missingText value of multipointcoverage stored queries fixed from NaN to unknown.

* Wed Aug  7 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.8.7-1.fmi
- SRS included into radar stored query abstract examples.

* Mon Aug  5 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.8.5-1.fmi
- Added *optional* expiresSeconds configuration parameter for the request and for main configuration.

* Wed Jul 31 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.31-1.fmi
- XmlGrammarPool update and test update of a stored query.
- The rest stored queries that use AC-MF 2.0 schema changed to under OMSO 2.0rc3 schema.

* Tue Jul 23 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.23-1.fmi
- Recompiled due to thread safety fixes in newbase & macgyver

* Fri Jul 19 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.19-1.fmi
- Stored query starttime, rounded down and abstract corrections.

* Thu Jul 18 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.18-1.fmi
- Bbox added template URLs of stored queries of grid type.

* Mon Jul 15 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.15-1.fmi
- Added timezone parameter option for stored queries of type of timevaluepair.

* Fri Jul 12 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.12-1.fmi
- A few forgotten stored query *default* beginTime and endTime values rounded down.

* Wed Jul 10 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.10-1.fmi
- Rounded down *default* beginTime and endTime values of stored queries.

* Wed Jul  3 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.3-2.fmi
- Update to boost 1.54

* Wed Jul 3 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.7.3-1.fmi
- XML validation tool fixed to work with http_proxy definition.

* Wed Jun 26 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.6.26-1.fmi
- Added optional wfs.conf parameter with timerelated effect to StoredFlashQueryHandler.

* Wed Jun 19 2013 Andris Pavenis <andris.pavenis@fmi-fi> - 2013.06.19-3.fmi
- XML escaping updates and fixes

* Wed Jun 19 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi> - 13.6.19-2.fmi
- Harmonized naming convention of the forecast::hirlam stored queries.

* Wed Jun 19 2013 Andris Pavenis <andris.pavenis@fmi-fi> - 2013.06.19-1.fmi
- Numerous small fixes and changes

* Tue Jun 18 2013 Roope Tervo <roope.tervo@fmi.fi> - 13.6.18-1.fmi
- Start time lower limit from 2013 to 2010 in 10 minutes observation data

* Mon Jun 17 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.6.17-1.fmi
- Missing geoids support for the wfs_obs_handler_factory stored queries.
- Keyword support for the wfs_obs_handler_factory stored queries.

* Fri Jun  7 2013 Andris Pavenis <andris.pavenis@fmi.fi> - 13.6.7-1.fmi
- Split CRS descriptions into separate files

* Tue Jun 4 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi>
- Fixed DescribeFeatureType typename constraint bug.

* Mon Jun  3 2013 Tuomo Lauri <tuomo.lauri@fmi.fi> - 13.6.3-1.fmi
- Rebult against the new Spine

* Fri May 31 2013 Roope Tervo <roope.tervo@fmi.fi> - 13.5.31-1.fmi
- Try to missing thunder strikes by doing new package
- Fixed hirlam stored query names (ground -> surface) again

* Wed May 29 2013 Roope Tervo <roope.tervo@fmi.fi> - 13.5.29-1.fmi
- Fixes in templates.

* Mon May 27 2013 Jukka A. Pakarinen <jukka.pakarinen@fmi.fi>
- Added stored query configuration files.

* Mon May 27 2013 lauri <tuomo.lauri@fmi.fi>      - 13.5.27-1.fmi
- Rebuild due to changes in Spine TimeseriesGeneratorOptions

* Fri May 24 2013 <andris.pavenis@fmi.fi>         - 13.5.24-1.fmi
- Support of StandardPresentationParameters

* Wed May 22 2013 <andris.pavenis@fmi.fi>         - 13.5.22-1.fmi
- Support selecting Geoserver data by additional database column values
- Support interval start time equal to end time
- Support station count limit inside bounding box for observations

* Thu May 16 2013 <andris.pavenis@fmi.fi>         - 13.5.16-2.fmi
- Fix table use for observations (hotfix)

* Thu May 16 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.16-1.fmi
- Fixed data sets, repaired crash when asking timezone as a parameter

* Wed May 15 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.15-1.fmi
- Ground -> Surface

* Tue May 14 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.14-1.fmi
- Tuning

* Mon May 13 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.13-1.fmi
- Fixed observable properties links

* Sat May 11 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.11-2.fmi
- Removed n_man from dataset 

* Sat May 11 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.11-1.fmi
- Changed to open data database tables.
- Possibility to give projection with the bbox.
- Minor fixes in parameters and templates.

* Wed May  8 2013 oksman <santeri.oksman@fmi.fi> - 13.5.8-1.fmi
- Fixes to flash bounding box handling.

* Tue May  7 2013 oksman <santeri.oksman@fmi.fi> - 13.5.7-1.fmi
- Rebuild to get master and develop to the same stage.

* Thu May  2 2013 tervo    <roope.tervo@fmi.fi>   - 13.5.2-1.fmi
- Initial support for GetPropertyValue, removed deprecated stored queries. Hide stored queries data sets.

* Mon Apr 29 2013 tervo <roope.tervo@fmi.fi> - 13.4.29-3.fmi
- New configrations.

* Wed Apr 24 2013 tervo <roope.tervo@fmi.fi> - 13.4.24-3.fmi
- One need to do git pull to get changes into the rpm

* Wed Apr 24 2013 tervo <roope.tervo@fmi.fi> - 13.4.24-2.fmi
- Timevaluepairs use separate groups for templates. Corrected Hirlam parameters.

* Wed Apr 24 2013 lauri <tuomo.lauri@fmi.fi> - 13.4.24-1.fmi
- Built against the new Spine

* Tue Apr 23 2013 tervo <roope.tervo@fmi.fi> - 13.4.23-1.fmi
- Added maxhours to daily and monthly stored queries. They don't make sense with the default.

* Mon Apr 22 2013 oksman <santeri.oksman@fmi.fi> - 13.4.22-1.fmi
- New beta build.

* Fri Apr 12 2013 lauri <tuomo.lauri@fmi.fi>    - 13.4.12-1.fmi
- Rebuild due to changes in Spine

* Mon Apr 8 2013 oksman <santeri.oksman@fmi.fi> - 13.4.8-2.fmi
- A couple of config files changed.

* Mon Apr 8 2013 oksman <santeri.oksman@fmi.fi> - 13.4.8-1.fmi
- New beta build.

* Tue Mar 26 2013 Tuomo Lauri    <tuomo.lauri@fmi.fi>    - 13.3.26-1.fmi
- Built new version for open data

* Thu Mar 21 2013 Andris Pavēnis <andris-pavenis@fmi.fi> - 13.3.22-1.fmi
- reimplement feature and operation registration (used for GetCapabilities etc.)

* Thu Mar 21 2013 tervo <roope.tervo@fmi.fi> - 13.3.21-1.fmi
- Added fmi-apikey to capabilities operation end point urls.

* Wed Mar 20 2013 tervo <roope.tervo@fmi.fi> - 13.3.20-1.fmi
- Refined stored queries and templates. Enabled Finnish.

* Sat Mar 16 2013 tervo <roope.tervo@fmi.fi> - 16.3.14-2.fmi
- PointTimeSeriesObservation -> GridSeriesObservation in multi point coverages.

* Sat Mar 16 2013 tervo <roope.tervo@fmi.fi> - 16.3.14-1.fmi
- Added daily and monthly stored queries.
- Deprecated realtime stored queries.
- Added start and end time query parameters to aws stored queries.
- Added additional information about queried locations to the templates.
- Added fmi-apikey to EF-schema station information link.

* Thu Mar 14 2013 oksman <santeri.oksman@fmi.fi> - 13.3.14-1.fmi
- New build from develop branch.

* Fri Mar  1 2013 tervo    <roope.tervo@fmi.fi>    - 13.3.1-1.fmi
- Repaired feature of interests
- Checked that gml:ids are valid (in this implementation).
- Added feature of interest for every member since it contains essential information about observation stations.
- Fixed srs dimension attributes.

* Thu Feb  28 2013 tervo    <roope.tervo@fmi.fi>    - 13.2.28-1.fmi
- Tuned templates

* Wed Feb  27 2013 tervo    <roope.tervo@fmi.fi>    - 13.2.27-3.fmi
- Tuned configuration

* Wed Feb  27 2013 tervo    <roope.tervo@fmi.fi>    - 13.2.27-2.fmi
- Updated dependencies

* Wed Feb  27 2013 tervo    <roope.tervo@fmi.fi>    - 13.2.27-1.fmi
- First Beta Release configuration

* Wed Feb  6 2013 lauri    <tuomo.lauri@fmi.fi>    - 13.2.6-1.fmi
- Built against new Spine and Server

* Wed Dec 19 2012 Andris Pavenis <andris.pavenis@fmi.fi> 12.12.19.fmi
- Updated for initial RPM build support

* Wed Nov  7 2012 Andris Pavenis <andris.pavenis@fmi.fi> 12.11.7-1.fmi
- Mostly done KVP observations stored requests

* Mon Oct  1 2012 pavenis <andris.pavenis@fmi.fi> - 12.10.1-1.fmi
- Initial skeleton
