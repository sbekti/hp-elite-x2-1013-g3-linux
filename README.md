# HP Elite x2 1013 G3 on Fedora

Two independently installable Fedora akmods for the HP Elite x2 1013 G3.

- **Front camera.** Enables the OV5675 front color camera. The CIO2 patch
  registers the front sensor while the rear and infrared cameras remain
  unavailable. Image processing uses libcamera's generic IPU3 tuning.
- **Hotkeys.** Restores brightness down, brightness up, and microphone mute on
  board 8414. These keys share AT keyboard scancode `0x81`. The module reads
  the last-hotkey ID through HP BIOS WMI and emits the corresponding Linux key
  event. Overlapping presses can lose events because the firmware reports
  only the most recent hotkey.

## Build on Fedora Workstation

```bash
sudo dnf install akmods kmodtool rpm-build gcc make elfutils-libelf-devel git
git clone https://github.com/sbekti/hp-elite-x2-1013-g3-linux.git
cd hp-elite-x2-1013-g3-linux
make rpm
```

Use `make camera-rpm` or `make hotkeys-rpm` to build one package.

### Secure Boot

Akmods signs modules using a local key. If this machine has no enrolled akmods
key, generate and request enrollment of one.

```bash
sudo kmodgenca -a
sudo mokutil --import /etc/pki/akmods/certs/public_key.der
```

Choose a temporary password when prompted. Reboot and select **Enroll MOK →
Continue → Yes** in MOK Manager. Enter the password and reboot again.

### Front camera

```bash
sudo dnf install libcamera-tools pipewire-plugin-libcamera ./dist/camera/*.rpm
sudo akmods --akmod hp-elite-x2-camera --kernels "$(uname -r)"
modinfo -n ipu3_cio2
```

The module path should include `extra/hp-elite-x2-camera/`. Reboot to activate
it. If the initramfs includes CIO2, regenerate it with `sudo dracut --force`
before rebooting. To test without rebooting, close camera apps, then run:

```bash
sudo rmmod ipu3_cio2
sudo modprobe ipu3_cio2
cam -l
cam -c 1 -s role=viewfinder,width=640,height=480 -C30
```

Using `rmmod` keeps the shared IPU bridge loaded.

### Hotkeys

```bash
sudo dnf install ./dist/hotkeys/*.rpm
sudo akmods --akmod hp-elite-x2-hotkeys --kernels "$(uname -r)"
sudo modprobe hp_elite_x2_hotkeys
```

The package loads the module at boot. The two sun keys should adjust the
built-in display and the microphone key should toggle mute independently.

## Kernel updates

[Akmods](https://packages.fedoraproject.org/pkgs/akmods/akmods/) builds and signs
the modules for new kernels. Wait for the build to finish before rebooting.
Future kernel API changes may require source updates. Build logs are in
`/var/cache/akmods/`. See [validation](VALIDATION.md) for tested kernels and
runtime checks.

## Uninstall

For the front camera, remove the packages and reboot. Regenerate the initramfs
first if you previously added the replacement CIO2 module to it.

```bash
sudo dnf remove hp-elite-x2-camera-kmod akmod-hp-elite-x2-camera 'kmod-hp-elite-x2-camera*'
sudo depmod -a
```

For the hotkeys, unload the module and remove the packages.

```bash
sudo modprobe -r hp_elite_x2_hotkeys
sudo dnf remove hp-elite-x2-hotkeys-kmod akmod-hp-elite-x2-hotkeys 'kmod-hp-elite-x2-hotkeys*'
```

## Camera research

See [rear camera research](research/REAR_IR.md) and the [userspace RPM experiment](packaging/userspace/README.md).

## Source and license

GPL-2.0-only. [SOURCE.json](SOURCE.json) records the Fedora kernel source and
hashes for the CIO2 files under `src/`. The camera RPM applies the patch under
`patches/`. The hotkey module is under `hotkeys/`.
