%define name            lingualeo
%define version         %{package_version}
%define release         %{package_release}

Name:           %{name}
Summary:        LinguaLeo, Eng-Rus translator
Version:        %{version}
Release:        %{release}
URL:            https://github.com/m0r1k/LinguaLeo

Group:          Applications/Multimedia
BuildRoot:      %{_tmppath}/lingualeo
License:        Copyright Roman. E. Chechnev

%description
LinguaLeo is a simple, Qt based, Eng-Rus online translator for Linux.

%build
mkdir -p %buildroot
make -C %{package_path}

%install
cd %{package_path}
make install DESTDIR=%buildroot
./bin/ipnoise-devel-rpms-find_files --buildroot "%buildroot" --out .files.list

%files -f %{package_path}/.files.list

%clean
rm -rf $RPM_BUILD_ROOT

%pre

%post

%preun

%changelog
* Thu Aug 22 2019 - morik@
- New version 1.0.2

* Mon Mar 25 2019 - morik@
- New version 0.9.1

