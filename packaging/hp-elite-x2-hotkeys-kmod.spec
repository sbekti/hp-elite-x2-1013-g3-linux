%global buildforkernels akmod
%global debug_package %{nil}
%global prjname hp-elite-x2-hotkeys
%global AkmodsBuildRequires gcc, make, elfutils-libelf-devel, kmodtool

Name:           %{prjname}-kmod
Version:        0.1.0
Release:        2%{?dist}
Summary:        Brightness keys for the HP Elite x2 1013 G3
License:        GPL-2.0-only
Source0:        %{prjname}-%{version}.tar.gz
BuildRequires:  %{AkmodsBuildRequires}
Provides:       %{prjname}-kmod-common = %{version}
ExclusiveArch:  x86_64

%{expand:%(kmodtool --target %{_target_cpu} --repo rpmfusion --kmodname %{prjname} %{?buildforkernels:--%{buildforkernels}} %{?kernels:--for-kernels "%{?kernels}"} 2>/dev/null)}

%description
Reports brightness and microphone keys on the HP Elite x2 1013 G3
using the firmware's last-hotkey query.

%prep
%{?kmodtool_check}
%setup -q -n %{prjname}-%{version}
for kernel in %{?kernel_versions}; do
    mkdir "build_${kernel%%___*}"
    cp hotkeys/Makefile hotkeys/hp_elite_x2_hotkeys.c "build_${kernel%%___*}/"
done

%build
for kernel in %{?kernel_versions}; do
    make %{?_smp_mflags} -C "${kernel##*___}" M="$PWD/build_${kernel%%___*}" modules
done

%install
install -Dm0644 packaging/hp-elite-x2-hotkeys.conf %{buildroot}%{_modulesloaddir}/hp-elite-x2-hotkeys.conf
for kernel in %{?kernel_versions}; do
    install -Dm0644 "build_${kernel%%___*}/hp_elite_x2_hotkeys.ko" "%{buildroot}%{kmodinstdir_prefix}/${kernel%%___*}/%{kmodinstdir_postfix}/hp_elite_x2_hotkeys.ko"
done
%{?akmod_install}

%files
%license LICENSE
%{_modulesloaddir}/hp-elite-x2-hotkeys.conf

%changelog
* Wed Nov 25 2026 HP Elite x2 Project - 0.1.0-2
- Package the board-specific brightness and microphone hotkeys.
