MAJOR_VERSION = 0
MINOR_VERSION = 1
REVISION = 2
VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(REVISION)

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
AR = $(CROSS_COMPILE)ar
INSTALL ?= install

TOPDIR := $(shell pwd)

DESTDIR ?=
libdir ?= /usr/local/lib
bindir ?= /usr/local/bin

CFLAGS = -O2 -Wall -ggdb -DVERSION=\"$(VERSION)\" \
	-I$(TOPDIR)/linux/include
LDFLAGS = -Wl,--warn-common -Wl,--no-undefined -Wl,--fatal-warnings

TARGETS = \
	linux/lib/libclnfw.so.$(VERSION) \
	linux/lib/libclnfw.a \
	linux/cln_fwtool \
	linux/cln_fwtool_s

.DEFAULT_GOAL := all
.PHONE: all clean install tag

all: $(TARGETS) Makefile

clean:
	@$(MAKE) -C linux clean
	@$(MAKE) -C linux/lib clean

install:
	@$(MAKE) -C linux install
	@$(MAKE) -C linux/lib install

linux/cln_fwtool linux/cln_fwtool_s:
	$(MAKE) -C linux

linux/lib/libclnfw.so.$(VERSION) linux/lib/libclnfw.a:
	$(MAKE) -C linux/lib

tag:
	git tag -a $(VERSION) refs/heads/master

export CC LD AR INSTALL CFLAGS LDFLAGS \
	VERSION MAJOR_VERSION MINOR_VERSION \
	DESTDIR libdir bindir
