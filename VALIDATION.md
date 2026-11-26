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
  approximately 30 fps. This verifies loading on a kernel-update boot.
- Suspend/resume on 7.3.4 passed. Kernel logs confirmed s2idle entry and exit;
  the resumed camera captured another 30 frames at approximately 30 fps.
- PipeWire capture after the kernel update and suspend/resume completed
  60 buffers and reached EOS successfully.

The GNOME preview showed live color. A 1280x960 capture reported a 2592x1944
active sensor area.

## Hotkeys

- On board 8414, brightness down, brightness up, and microphone mute all
  produced AT scancode `0x81`. The HP BIOS WMI LastHotKey query returned
  `0x0a0001b6`, `0x0a0001b7`, and `0x270`, respectively, on two rounds.
- On Fedora 45 with GNOME 51 and kernel 7.3.4, the module emitted Linux
  keycodes 224, 225, and 248. The screen backlight changed from 804 to 656
  and back to 804, and only the microphone key changed the mute state.
- Akmods built signed packages for 7.2.11 and 7.3.4. The 7.3.4 package
  loaded through `modprobe` with Secure Boot enabled.
