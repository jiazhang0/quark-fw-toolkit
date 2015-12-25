include $(TOPDIR)/common.mk
include $(TOPDIR)/version.mk

LIB_NAME := libclnfw
LIB_TARGETS := $(LIB_NAME).so.$(VERSION)
TARGETS := $(LIB_TARGETS) $(LIB_NAME).a
OBJS_$(LIB_NAME) = \
	parser.o \
	mfh.o \
	platform_data.o \
	capsule.o \
	crc32.o \
	buffer_stream.o \
	linux.o \
	util.o \
	handle.o \
	class.o
OBJS := $(OBJS_$(LIB_NAME))

CFLAGS += -fpic -DBYTE_STREAM_ERROR_BASE=0x10000 \
	  -DCLN_FLASH_ERROR_BASE=0x20000 \
	  -DCLASS_ERROR_BASE=0x30000

.DEFAULT_GOAL := all
.PHONE: all clean install

all: $(TARGETS) Makefile

clean:
	@$(RM) $(TARGETS) $(OBJS) *.so.*

install: all
	$(INSTALL) -d -m 755 $(DESTDIR)$(libdir)
	$(foreach x, $(LIB_TARGETS), $(INSTALL) -m 755 $(x) $(DESTDIR)$(libdir); \
		ln -sfn $(x) $(DESTDIR)$(libdir)/$(patsubst %.$(VERSION),%,$(x)); \
		ln -sfn $(x) $(DESTDIR)$(libdir)/$(patsubst %.$(VERSION),%.$(MAJOR_VERSION),$(x)); \
		ln -sfn $(x) $(DESTDIR)$(libdir)/$(patsubst %.$(VERSION),%.$(MAJOR_VERSION).$(MINOR_VERSION),$(x));)

$(LIB_NAME).so.$(VERSION): $(OBJS_$(LIB_NAME))
	$(CC) $(CFLAGS) -shared -o $@ $^ -Wl,-soname,$(patsubst %.$(VERSION),%,$@)

$(LIB_NAME).a: $(OBJS_$(LIB_NAME))
	$(AR) rcs $@ $^