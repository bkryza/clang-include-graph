%define _unpackaged_files_terminate_build 0

Name:     clang-include-graph
Version:  %{?git_version}
Release:  1%{?dist}
Summary:  Simple tool for analyzing C++ project include graph.
License:  ASL 2.0
URL:      https://github.com/bkryza/clang-include-graph
Source0:  clang-include-graph-%{version}.tar.gz

BuildRequires: cmake
BuildRequires: git
BuildRequires: clang-devel
BuildRequires: clang-tools-extra
BuildRequires: ccache
BuildRequires: boost-devel
BuildRequires: gdb

Requires: clang
Requires: boost
Requires: boost-graph
Requires: boost-program_options
Requires: boost-json
Requires: boost-log
Requires: boost-container
Requires: boost-filesystem
Requires: boost-regex
Requires: boost-chrono
Requires: boost-thread
Requires: boost-atomic

Requires(post): info
Requires(preun): info

%description
This package provides the clang-include-graph binary.


%prep
%setup -q -n clang-include-graph-%{version}

%build
%cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
       -DCMAKE_CXX_FLAGS="-Wno-nonnull -Wno-stringop-overflow -Wno-dangling-reference -Wno-array-bounds" \
       -DCMAKE_NO_SYSTEM_FROM_IMPORTED=ON \
       -DCMAKE_INSTALL_PREFIX=%{_exec_prefix} \
       -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
       -DGIT_VERSION=%{version} \
       -DBUILD_TESTS=OFF

%cmake_build

%install
%cmake_install
rm -f %{buildroot}/%{_infodir}/dir

%post
/sbin/install-info %{_infodir}/%{name}.info %{_infodir}/dir || :

%preun
if [ $1 = 0 ] ; then
/sbin/install-info --delete %{_infodir}/%{name}.info %{_infodir}/dir || :
fi

%files
%{_bindir}/clang-include-graph
%doc CHANGELOG.md README.md AUTHORS.md LICENSE.md
%license LICENSE.md

%changelog
* Wed May 14 2025 Bartek Kryza <bkryza@gmail.com>
- Initial version of the package for Fedora
