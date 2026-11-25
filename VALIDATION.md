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

The GNOME preview showed a live color image. The user reported a higher-looking
viewing angle; a 4:3 comparison did not resolve this. A 1280x960 capture reported
the full 2592x1944 active sensor area. No same-angle Windows reference is available.
This remains an open image-quality concern, not a claimed solved issue.
