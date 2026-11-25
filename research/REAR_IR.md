# Rear and IR investigation

Status: evidence collection, not a working rear/IR fix. The installed v0.1.0
front-camera package has not been changed. No regulator, GPIO, sensor or emitter
settings have been written by this investigation.

## Live firmware evidence

On the HP, a temporary signed module evaluated a fixed list of camera metadata
objects and GNVS fields, then was unloaded. Secure Boot stayed enabled. The
sanitized results are in `firmware-summary.json`; firmware bus indices are not
Linux I2C adapter numbers.

- CLP0 is TPS68470, type 2, control-version 0x50, address 0x4d on firmware I2C2.
- CLDB is `0102005000000000000000000003000000000000000000000000000000000000`.
- Its six additional configuration bytes are `00 00 00 00 00 03`.
- Rear OV8856: CSI port 0, four lanes, 19.2 MHz, sensor address 0x10.
- Front OV5675: CSI port 1, two lanes, 19.2 MHz, address 0x36, separate DSC1.
- IR OV7251: CSI port 2, one lane, 19.2 MHz, address 0x60 on firmware I2C3.

This establishes topology and requested clocks. It does not establish regulator
consumer wiring, final per-sensor voltage settings or reset polarity. A board
entry must not be populated with guesses or another machine's settings.

## Windows driver analysis

HP SP98898 matches the previously collected Windows driver versions. Offline
analysis identified the control-version 0x50 branch in the TPS68470 constructor
and the associated CrdG2TiSensor implementation. It contains per-sensor voltage
configuration methods as well as constructor defaults. Those defaults alone
are not evidence of the settings ultimately used on this HP.

The referenced control-driver PDB was not available at its exact Microsoft
symbol-server identifier. No Windows binaries or full disassembly are included
in this repository. Further native-Windows evidence may be needed before a
hardware-changing patch is justified.

## Windows fallback preparation

- The existing native FAT32 WIN11 USB boots. It was not reformatted or replaced
  with Ventoy.
- Every ordinary installer file path and size matches the Microsoft ISO;
  install.wim is replaced by three split SWM files.
- Full wimlib verification of the split installation image passed.
- SHA256 of EFI bootx64.efi, bootmgr.efi, sources/boot.wim and sources/setup.exe
  matches the ISO.
- The boot image includes stornvme.sys, USBXHCI.SYS, USBSTOR.SYS and uaspstor.sys.
- Linux exposes a native Samsung NVMe controller, PCI 144d:a808, rather than an
  RST/VMD storage controller.
- The reported Setup failure is the early missing-media-driver dialog, not an
  established empty disk-selection screen. The connected USB path includes a hub.

The next physical test is to retry without the hub, then use Shift+F10 and
read-only DiskPart `list disk` and `list volume`. Do not change storage mode or
load an unrelated RST driver without evidence.

HP camera-driver installer, read-only inventory script, and Fedora recovery
metadata were added under HP-Camera on the USB. Recovery metadata was also copied
off the HP. These copies cover partition layout, EFI contents and boot entries,
not all user files. If Windows is installed, only the existing approximately
150 GiB unallocated region is authorized; preserve Fedora's three partitions.

## Remaining gates

1. Establish actual board supply/reset configuration; then add a model-specific
   TPS68470 entry and test the rear sensor before IR capture.
2. Validate illumination without assuming USB-emitter tooling applies to this
   I2C-controlled camera assembly.
3. Use existing packaged software only for face login. If it cannot consume the
   IR frames, document that limitation rather than add a custom capture adapter.
   Keep PAM unchanged until IR capture and a supported authentication path work.
