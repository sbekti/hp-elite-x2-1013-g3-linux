VERSION := 0.1.0
PROJECT := hp-elite-x2-1013-g3-camera
RPMDIR := $(CURDIR)/.build/rpmbuild

.PHONY: rpm
rpm:
	mkdir -p $(RPMDIR)/SOURCES $(RPMDIR)/SPECS dist
	tar --transform='s,^,$(PROJECT)-$(VERSION)/,' -czf $(RPMDIR)/SOURCES/$(PROJECT)-$(VERSION).tar.gz LICENSE README.md VALIDATION.md SOURCE.json Makefile src patches packaging
	cp packaging/hp-elite-x2-camera-kmod.spec $(RPMDIR)/SPECS/
	rpmbuild -ba --define '_topdir $(RPMDIR)' $(RPMDIR)/SPECS/hp-elite-x2-camera-kmod.spec
	find $(RPMDIR)/RPMS -name '*.rpm' -exec cp {} dist/ \;
