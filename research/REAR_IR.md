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

## Native Windows baseline, 2026-11-25

Windows is now a permanent dual boot, using the previously unallocated roughly
150 GiB. Fedora's three partitions were preserved. Fedora GRUB is the default
boot path, with a visible five-second menu and a Windows Boot Manager entry;
the user verified booting both systems.

The installer USB's missing-media-driver problem was resolved by changing its
existing FAT32 partition type from EFI System to Microsoft Basic Data and
assigning a drive letter. No format or RST/VMD driver was needed. Before this,
all ordinary installer file paths/sizes, key boot-file hashes and the complete
split-WIM image had passed verification against the Microsoft ISO.

The read-only Windows inventory collected from the USB reports all eight
camera-related devices OK, with ProblemCode 0. The installed Intel ISP, control
logic and AVStream drivers are 30.18305.6.9990; the OV8856, OV5675 and OV7251 sensor
drivers are 30.15063.8317.1, matching the HP package analyzed earlier.

The dependency properties confirm OV8856 and OV7251 depend on CLP0, while OV5675
depends on DSC1. The user separately verified working Windows front and rear
capture; OK status alone is not proof of working IR capture or illumination.

The inventory lists Intel-iaLPSS-I2C, Intel-iaLPSS2-I2C and
Microsoft-Windows-SPB-HIDI2C tracing providers. It contains no register-level
power trace, regulator values or reset sequence. Provider presence does not
establish that its events expose the required I2C payloads. The full report is
kept privately because device properties contain machine-specific identifiers.

Partition-layout and EFI backups are retained off the HP, with copies under
HP-Camera/Recovery on the USB. These cover boot metadata, not all user files.

## Remaining gates

1. Establish actual board supply/reset configuration; then add a model-specific
   TPS68470 entry and test the rear sensor before IR capture.
2. Validate illumination without assuming USB-emitter tooling applies to this
   I2C-controlled camera assembly.
3. Use existing packaged software only for face login. If it cannot consume the
   IR frames, document that limitation rather than add a custom capture adapter.
   Keep PAM unchanged until IR capture and a supported authentication path work.
