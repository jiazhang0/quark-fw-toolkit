include version.mk

TOPDIR := $(shell pwd)

export TOPDIR

SUBDIRS := linux

.DEFAULT_GOAL := all
.PHONE: all clean install tag

all clean install:
	@set -e
	@for x in $(SUBDIRS); do $(MAKE) -C $$x $@; done

tag:
	@git tag -a $(VERSION) -m $(VERSION) refs/heads/master
