%global buildforkernels akmod
%global debug_package %{nil}
%global prjname hp-elite-x2-camera
%global AkmodsBuildRequires gcc, make, patch, elfutils-libelf-devel, kmodtool

Name:           %{prjname}-kmod
Version:        0.1.0
Release:        1%{?dist}
Summary:        Front-camera workaround for HP Elite x2 1013 G3
License:        GPL-2.0-only
URL:            https://github.com/sbekti/hp-elite-x2-1013-g3-camera
Source0:        hp-elite-x2-1013-g3-camera-%{version}.tar.gz
BuildRequires:  %{AkmodsBuildRequires}
Provides:       %{prjname}-kmod-common = %{version}
ExclusiveArch:  x86_64

%{expand:%(kmodtool --target %{_target_cpu} --repo rpmfusion --kmodname %{prjname} %{?buildforkernels:--%{buildforkernels}} %{?kernels:--for-kernels "%{?kernels}"} 2>/dev/null)}

%description
Allows the existing OV5675 front camera to operate without waiting for
the unsupported rear and infrared camera power controller. Includes
module-loading settings; the matching akmod supplies the replacement driver.

%prep
%{?kmodtool_check}
%setup -q -n hp-elite-x2-1013-g3-camera-%{version}
patch -p1 < patches/0001-hp-front-camera.patch
for kernel_version in %{?kernel_versions}; do
    cp -a src _kmod_build_${kernel_version%%___*}
done

%build
for kernel_version in %{?kernel_versions}; do
    make %{?_smp_mflags} -C ${kernel_version##*___} M=${PWD}/_kmod_build_${kernel_version%%___*} modules
done

%install
install -Dm0644 packaging/hp-elite-x2-camera.conf %{buildroot}%{_modprobedir}/hp-elite-x2-camera.conf
install -Dm0644 packaging/hp-elite-x2-camera-depmod.conf %{buildroot}%{_prefix}/lib/depmod.d/hp-elite-x2-camera.conf
for kernel_version in %{?kernel_versions}; do
    install -Dm0644 _kmod_build_${kernel_version%%___*}/ipu3-cio2.ko %{buildroot}%{kmodinstdir_prefix}/${kernel_version%%___*}/%{kmodinstdir_postfix}/ipu3-cio2.ko
done
%{?akmod_install}

%files
%license LICENSE
%doc README.md VALIDATION.md SOURCE.json
%{_modprobedir}/hp-elite-x2-camera.conf
%{_prefix}/lib/depmod.d/hp-elite-x2-camera.conf

%changelog
* Tue Nov 24 2026 Samudra Bekti <samudra.bekti@gmail.com> - 0.1.0-1
- Package the opt-in OV5675 workaround for automatic kernel rebuilds.
