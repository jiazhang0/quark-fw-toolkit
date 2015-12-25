CROSS_COMPILE ?=
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar
INSTALL ?= install

EXTRA_CFLAGS ?=
EXTRA_LDFLAGS ?=

LDFLAGS := --warn-common --no-undefined --fatal-warnings \
	   $(patsubst $(join -Wl,,)%,%,$(EXTRA_LDFLAGS))
CFLAGS := -O2 -Wall -ggdb -fPIC -I$(TOPDIR)/linux/include \
	  $(EXTRA_CFLAGS) $(addprefix $(join -Wl,,),$(LDFLAGS))

DESTDIR ?=
prefix ?= /usr/local
libdir ?= $(prefix)/lib
bindir ?= $(prefix)/bin
includedir ?= $(prefix)/include