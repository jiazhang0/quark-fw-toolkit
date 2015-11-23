MAJOR_VERSION := 0
MINOR_VERSION := 1
REVISION := 2
VERSION := $(MAJOR_VERSION).$(MINOR_VERSION).$(REVISION)

CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
INSTALL ?= install

DESTDIR ?=
prefix ?= /usr/local
libdir ?= $(prefix)/lib
bindir ?= $(prefix)/bin
includedir ?= $(prefix)/include

TOPDIR := $(shell pwd)
SUBDIRS := linux

.DEFAULT_GOAL := all
.PHONE: all clean install tag

all clean install:
	@set -e
	@for x in $(SUBDIRS); do $(MAKE) -C $$x $@; done

tag:
	@git tag -a $(VERSION) refs/heads/master

export VERSION MAJOR_VERSION MINOR_VERSION \
       TOPDIR DESTDIR prefix libdir bindir includedir \
       CC LD AR INSTALL