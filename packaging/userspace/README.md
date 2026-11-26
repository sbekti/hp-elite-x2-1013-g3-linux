# Fedora 45 camera RPM experiment

This recipe stages libcamera 0.8.0 with the IPU3 1080p patch and a matching
PipeWire 1.6.9 camera plug-in. Build in a disposable Fedora 45 environment.
The plug-in requires the matching PipeWire 1.6.9 libraries. Fedora's 1.6.11
update requires a plug-in rebuilt for that version.

Pass the original Fedora `libcamera-0.7.2-6.fc45.src.rpm` and
`pipewire-1.6.9-1.fc45.src.rpm` to `prepare.sh` from this directory. The
script prints the staging directory. For each package, use its subdirectory
as the RPM `_topdir` and the spec under `SPECS` as the build target.

Build libcamera first. Install its `libcamera`, `libcamera-ipa`, and
`libcamera-devel` RPMs in the plug-in build environment before building
the plug-in. For a runtime test, use the libcamera, IPA, tools, and
PipeWire plug-in RPMs as a single set.

See [validation results](../../VALIDATION.md) for the modes and capture path
tested on the HP. The higher video mode is available to apps that request it.
