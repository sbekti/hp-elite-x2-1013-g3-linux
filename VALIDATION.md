# Validation

2026-11-24 and 2026-11-25, HP Elite x2 1013 G3, Fedora 45 Workstation,
Secure Boot enabled.

The manual prototype passed libcamera, PipeWire and GNOME Snapshot capture on
kernel 7.2.11-300.fc45.x86_64. See README for measurements and limitations.

## Fedora akmod package 0.1.0

- Source and binary RPMs built successfully using Fedora's kmodtool template.
- Installation registered the source RPM with akmods.
- Akmods built and installed signed modules for both
  `7.2.11-300.fc45.x86_64` and `7.3.4-300.fc45.x86_64`.
- Both modules use the already-enrolled local signing key. No private key or
  certificate is shipped in the source tree or release packages.
- `modinfo -k KERNEL_RELEASE -n ipu3_cio2` selected the replacement under
  `extra/hp-elite-x2-camera/` on both kernels.
- The installed package loaded on 7.2.11 through ordinary `modprobe`, enabled
  `hp_front_only=1` through packaged configuration and captured 30 frames at
  approximately 30 fps with Secure Boot enabled.
- Neither tested initramfs contained CIO2, so no initramfs modification was needed.
- After a reboot into 7.3.4, the packaged replacement loaded automatically with
  `hp_front_only=1` and Secure Boot enabled. A fresh 30-frame capture passed at
  approximately 30 fps. This validates a real kernel-update boot, not just a build.
- Suspend/resume on 7.3.4 passed. Kernel logs confirmed s2idle entry and exit;
  the resumed camera captured another 30 frames at approximately 30 fps.
- PipeWire capture after the kernel update and suspend/resume completed
  60 buffers and reached EOS successfully.

The GNOME preview showed a live color image. A 1280x960 capture reported
the full 2592x1944 active sensor area.

## Windows comparison and front image quality, 2026-11-25

The user verified that Windows produces live front and rear camera images.
Front framing looks similar in Windows and Fedora, resolving the earlier concern
about a Linux-specific viewing angle. The Windows front image is sharper and has
better color according to the user; there is no matched image pair for an
objective image-quality measurement.

With libcamera 0.7.2-6.fc45 and PipeWire 1.6.9-1.fc45:

- PipeWire advertises the front camera only up to 1280x720. This is the advertised
  maximum, not evidence of the resolution previously negotiated by Snapshot.
- Direct libcamera viewfinder configuration accepts 1920x1080. A live SDL preview
  configured at that size streamed from the 2592x1944 sensor mode, but the user
  still reported a soft, washed-out image. The test preview was then stopped.
- A still configuration request for 2592x1944 adjusts to 2560x1920. This was a
  configuration check, not a validated still capture.
- libcamera reports missing ov5675.yaml and falls back to uncalibrated.yaml.
  It also reports unverified default sensor delays.

libcamera's IPU3 viewfinder/video stream format range ends at its 1280x720
initial configuration, while direct configuration can accept larger sizes:
https://github.com/libcamera-org/libcamera/blob/v0.7.2/src/libcamera/pipeline/ipu3/ipu3.cpp

Resolution and image processing are separate concerns. No persistent userspace
changes or speculative tuning values were installed. Matching Windows image
quality remains unverified and unresolved; the kernel workaround does not
provide sensor calibration.
