#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 LIBCAMERA_FEDORA_SRPM PIPEWIRE_FEDORA_SRPM" >&2
    exit 2
fi

camera_srpm=$(realpath "$1")
pipewire_srpm=$(realpath "$2")
repo=$(cd "$(dirname "$0")/../.." && pwd)
source_version() {
    rpm -qp --qf '%{NAME}-%{VERSION}-%{RELEASE} %{SOURCEPACKAGE}\n' "$1"
}
if [[ $(source_version "$camera_srpm") != 'libcamera-0.7.2-6.fc45 1' ||
      $(source_version "$pipewire_srpm") != 'pipewire-1.6.9-1.fc45 1' ]]; then
    echo "Expected Fedora 45 libcamera 0.7.2-6 and PipeWire 1.6.9-1 source RPMs." >&2
    exit 1
fi

mkdir -p "$repo/.build"
work=$(mktemp -d "$repo/.build/userspace-rpm.XXXXXX")
cam="$work/libcamera"
pw="$work/pipewire"
mkdir -p "$cam"/{SOURCES,SPECS} "$pw"/{SOURCES,SPECS}
rpm2cpio "$camera_srpm" | (cd "$cam/SOURCES" && cpio --quiet -idm)
rpm2cpio "$pipewire_srpm" | (cd "$pw/SOURCES" && cpio --quiet -idm)

camera_commit=df8e348d64b4932a0676256ed24f0da3ee9438bc
git init --bare --quiet "$work/libcamera.git"
git -C "$work/libcamera.git" fetch --quiet --depth=1 \
    https://github.com/libcamera-org/libcamera.git "$camera_commit"
git -C "$work/libcamera.git" archive --prefix=libcamera-v0.8.0/ "$camera_commit" |
    bzip2 -c > "$cam/SOURCES/libcamera-v0.8.0.tar.bz2"

cp "$repo/patches/libcamera/0001-ipu3-advertise-1080p.patch" "$cam/SOURCES/"
sed -e 's/^Version: 0\.7\.2$/Version: 0.8.0/' \
    -e 's/^Release: 6%{?dist}$/Release: 0.1.hp%{?dist}/' \
    -e 's/^Patch02: .*/Patch02: 0001-ipu3-advertise-1080p.patch/' \
    -e 's/\.so\.0\.7$/\.so.0.8/' \
    "$cam/SOURCES/libcamera.spec" > "$cam/SPECS/libcamera.spec"

cp "$repo/packaging/userspace/pipewire-plugin-libcamera.spec" "$pw/SPECS/"
curl -fsSL https://github.com/PipeWire/pipewire/commit/54f2f51f5233026e608f8235f5377b9b473918e3.patch \
    -o "$pw/SOURCES/pipewire-libcamera-0.8.patch"

printf '%s\n' "$work"
