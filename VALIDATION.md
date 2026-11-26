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

## Windows comparison and front image quality, 2026-11-25

Windows produced live front and rear camera images. The front camera's framing
looked similar in Fedora and Windows during a live comparison. Matched reference
captures for quantitative comparison are pending.

With libcamera 0.7.2-6.fc45 and PipeWire 1.6.9-1.fc45:

- PipeWire advertises the front camera up to 1280x720.
- Direct libcamera viewfinder configuration accepts 1920x1080. A live SDL preview
  at that size streamed from the 2592x1944 sensor mode.
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

## Front parity experiments

- A separate libcamera 0.8.0 build captured 90 NV12 frames at about 30 fps at
  both 1920x1080 and 2560x1920. Metadata confirmed the locally generated HP
  profile applied a non-identity color matrix.
- A stock-versus-tuned 1080p live comparison on the HP found similar
  sharpness and a red cast with the imported profile.
- libcamera reports an unsupported sensor rectangle and missing sensor
  control delays. The capture logs also show zero effective exposure on the
  first two frames. CIO2 sometimes logs a short DMA payload when closing
  the preview. Neither warning was reproduced as sustained frame loss in
  these short captures.
- The Fedora libcamera 0.8.0 RPMs and a matching plug-in for PipeWire
  1.6.9 built successfully. On the HP, an isolated PipeWire and WirePlumber
  session advertised 1920x1080 and delivered 90 NV12 buffers at that size
  through GStreamer in 3.16 seconds. No experimental tuning was loaded.
- A DNF dry run resolved the four candidate packages without changing the
  PipeWire audio packages. Testing used extracted RPMs. The desktop still
  uses the installed Fedora packages.
