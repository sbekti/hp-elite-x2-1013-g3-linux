# HP Elite x2 1013 G3 front camera

A small Linux driver workaround that enables the **OV5675 front color camera**
on the HP Elite x2 1013 G3. Packaged for Fedora Workstation using akmods so the
module is rebuilt and signed locally when compatible kernel updates arrive.

This is a community workaround, not complete camera support. The rear OV8856
and IR OV7251 remain unavailable. Image tuning uses libcamera's generic IPU3
defaults. A user reported higher-than-expected framing; changing aspect ratio
did not resolve it, and no Windows comparison is available. Suspend/resume
validation is still pending.

## What changes

The front sensor and its power controller already work. The rear/IR TPS68470
controller fails with `No board-data found for this model`. CIO2 waits for all
three sensors before exposing camera links, leaving even the front unavailable.

The patch adds an opt-in `hp_front_only` parameter to the existing `ipu3_cio2`
module. It registers only the verified `OVTI5675-1` front-camera connection.
The option is restricted to DMI vendor `HP`, product `HP Elite x2 1013 G3`.
No sensor registers, power sequences, firmware, or TPS68470 settings are changed.

The RPM installs the rebuilt module under `extra/hp-elite-x2-camera/`, enables
the option through modprobe configuration, and gives this copy priority through
depmod configuration. Fedora's packaged driver stays in place for rollback.
There are no custom daemons, full kernel builds, or download hooks.

## Install on Fedora Workstation

```bash
sudo dnf install akmods kmodtool rpm-build gcc make elfutils-libelf-devel git libcamera-tools pipewire-plugin-libcamera
git clone https://github.com/sbekti/hp-elite-x2-1013-g3-camera.git
cd hp-elite-x2-1013-g3-camera
make rpm
```

### Secure Boot

Akmods uses a local signing key. If its key has not been generated yet:

```bash
sudo kmodgenca -a
sudo mokutil --import /etc/pki/akmods/certs/public_key.der
```

Choose a temporary password, reboot, and select **Enroll MOK → Continue → Yes**
in MOK Manager. Enter that password and reboot again. Keep Secure Boot enabled.
Skip enrollment if this same akmods certificate is already trusted. Existing
enrolled keys can be reused by configuring akmods to use the matching local key
and certificate. Never commit or share the private key.

### Install and build the module

```bash
sudo dnf install ./dist/*.rpm
sudo akmods --force --akmod hp-elite-x2-camera --kernels "$(uname -r)"
modinfo -n ipu3_cio2
```

The module path should contain `extra/hp-elite-x2-camera/`. Reboot to activate it.
If your initramfs already includes CIO2, regenerate it after installation with
`sudo dracut --force`; otherwise no initramfs change is needed.

For a live test without rebooting, close camera apps, then run:

```bash
sudo rmmod ipu3_cio2
sudo modprobe ipu3_cio2
cam -l
```

Use `rmmod`, not `modprobe -r`: the latter may remove the shared ipu-bridge
dependency. Do not force an unload. `libcamera-tools` supplies `cam`; desktop
camera apps also need `pipewire-plugin-libcamera`.

## Kernel and OS updates

[Akmods](https://packages.fedoraproject.org/pkgs/akmods/akmods/) builds modules
after kernel installation and checks them at boot. Signing uses the enrolled
local key. Wait for the build to finish before rebooting after a kernel update.
Check a particular kernel with:

```bash
sudo akmods --kernels KERNEL_RELEASE --akmod hp-elite-x2-camera
modinfo -k KERNEL_RELEASE -n ipu3_cio2
modinfo -k KERNEL_RELEASE -F signer ipu3_cio2
```

This survives normal updates **while this source remains compatible**. Linux
does not promise a stable internal driver API. A future kernel may require a
source update; automatic rebuilding cannot fix that. Errors are recorded under
`/var/cache/akmods/hp-elite-x2-camera/`. Keep a working older kernel available.
After a Fedora release upgrade, verify the package, headers, build and signing
again. Other distributions, Atomic variants and custom kernels are untested.

The long-term solution is an upstream kernel fix, eliminating the replacement
module. This repository vendors just the two CIO2 source files, rather than
downloading and patching an entire kernel at every update. Maintainers should
refresh them from the target kernel when relevant upstream fixes or API changes
arrive, then revalidate the small patch.

## Validation

```bash
cat /sys/module/ipu3_cio2/parameters/hp_front_only
cam -l
cam -c 1 -s role=viewfinder,width=640,height=480 -C30
```

On the tested HP, the prototype and installed package loaded with Secure Boot enabled:

| Check | Result |
|---|---|
| Kernel 7.2.11-300.fc45.x86_64 | Front camera enumerates and captures |
| Kernel 7.3.4-300.fc45.x86_64 | Loaded automatically after reboot; captures at approximately 30 fps |
| Libcamera 640×480 | 30 frames, approximately 30 fps |
| Libcamera 1280×960 | Captures at approximately 30 fps |
| PipeWire 640×480 | 60 buffers completed successfully |
| GNOME Camera (Snapshot) | User confirmed a live image |
| Suspend/resume | Not yet tested |
| Rear / IR cameras | Not supported by this workaround |

See `VALIDATION.md` for package and update-build results. A successful build
against another kernel is not proof of runtime camera support on that kernel.

## Uninstall

```bash
sudo dnf remove hp-elite-x2-camera-kmod akmod-hp-elite-x2-camera 'kmod-hp-elite-x2-camera*'
sudo depmod -a
```

Reboot to return to Fedora's driver. If you previously regenerated an initramfs
containing this workaround, regenerate it again after uninstalling. No firmware
or sensor-power changes need reverting.

## Source and license

GPL-2.0-only, with the kernel authors' copyright notices retained. `SOURCE.json`
records the original Fedora kernel source and file hashes. `src/` is unmodified
kernel source; `patches/` contains the workaround applied during RPM preparation.
Signing keys, personal firmware dumps and captured images are not distributed.
