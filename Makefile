VERSION := 0.1.0
CAMERA_SOURCE := hp-elite-x2-1013-g3-camera-$(VERSION)
HOTKEYS_SOURCE := hp-elite-x2-hotkeys-$(VERSION)
CAMERA_BUILD := $(CURDIR)/.build/camera-rpm
HOTKEYS_BUILD := $(CURDIR)/.build/hotkeys-rpm

.PHONY: rpm camera-rpm hotkeys-rpm
rpm: camera-rpm hotkeys-rpm

camera-rpm:
	mkdir -p "$(CAMERA_BUILD)/SOURCES" "$(CAMERA_BUILD)/SPECS" dist/camera
	tar --transform='s,^,$(CAMERA_SOURCE)/,' -czf "$(CAMERA_BUILD)/SOURCES/$(CAMERA_SOURCE).tar.gz" \
		LICENSE README.md VALIDATION.md SOURCE.json src patches \
		packaging/hp-elite-x2-camera.conf packaging/hp-elite-x2-camera-depmod.conf
	cp packaging/hp-elite-x2-camera-kmod.spec "$(CAMERA_BUILD)/SPECS/"
	rpmbuild -ba --define '_topdir $(CAMERA_BUILD)' "$(CAMERA_BUILD)/SPECS/hp-elite-x2-camera-kmod.spec"
	find "$(CAMERA_BUILD)/RPMS" \( -name 'hp-elite-x2-camera-kmod-*.rpm' -o -name 'akmod-hp-elite-x2-camera-*.rpm' \) -exec cp {} dist/camera/ \;

hotkeys-rpm:
	mkdir -p "$(HOTKEYS_BUILD)/SOURCES" "$(HOTKEYS_BUILD)/SPECS" dist/hotkeys
	tar --transform='s,^,$(HOTKEYS_SOURCE)/,' -czf "$(HOTKEYS_BUILD)/SOURCES/$(HOTKEYS_SOURCE).tar.gz" \
		LICENSE hotkeys/Makefile hotkeys/hp_elite_x2_hotkeys.c packaging/hp-elite-x2-hotkeys.conf
	cp packaging/hp-elite-x2-hotkeys-kmod.spec "$(HOTKEYS_BUILD)/SPECS/"
	rpmbuild -ba --define '_topdir $(HOTKEYS_BUILD)' "$(HOTKEYS_BUILD)/SPECS/hp-elite-x2-hotkeys-kmod.spec"
	find "$(HOTKEYS_BUILD)/RPMS" \( -name 'hp-elite-x2-hotkeys-kmod-*.rpm' -o -name 'akmod-hp-elite-x2-hotkeys-*.rpm' \) -exec cp {} dist/hotkeys/ \;
