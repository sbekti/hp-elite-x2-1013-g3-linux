%global __meson_auto_features disabled

Name:           pipewire-plugin-libcamera
Version:        1.6.9
Release:        1.1.hp%{?dist}
Summary:        PipeWire camera plug-in for libcamera 0.8
License:        MIT
URL:            https://pipewire.org/
Source0:        https://gitlab.freedesktop.org/pipewire/pipewire/-/archive/%{version}/pipewire-%{version}.tar.gz
Patch0:         pipewire-libcamera-0.8.patch

BuildRequires:  gcc-c++
BuildRequires:  libcamera-devel >= 0.8.0
BuildRequires:  libdrm-devel
BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  patch
Requires:       pipewire-libs%{?_isa} = %{version}-1%{?dist}
Requires:       libcamera%{?_isa} >= 0.8.0
Requires:       libdrm

%description
Builds the libcamera SPA plug-in from the Fedora PipeWire release against
the libcamera 0.8 interface.

%prep
%autosetup -p1 -n pipewire-%{version}
# PipeWire 1.6.9 did not include the version header in this translation unit.
sed -i '/property_ids.h/a #include <libcamera/version.h>' \
    spa/plugins/libcamera/libcamera-device.cpp

%build
%meson -Dlibcamera=enabled -Ddbus=disabled -Dflatpak=disabled \
       -Dtests=disabled -Dexamples=disabled -Dgsettings-pulse-schema=disabled -Dsession-managers=[]
%meson_build spa-libcamera

%install
install -Dpm0755 %{_vpath_builddir}/spa/plugins/libcamera/libspa-libcamera.so \
    %{buildroot}%{_libdir}/spa-0.2/libcamera/libspa-libcamera.so

%files
%license LICENSE
%{_libdir}/spa-0.2/libcamera/
