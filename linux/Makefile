BIN_NAME := cln_fwtool
BIN_TARGETS := $(BIN_NAME) $(BIN_NAME)_s
LIB_NAME := libclnfw
LIB_TARGETS := $(addprefix lib/,$(join $(LIB_NAME) $(LIB_NAME),.so.$(VERSION) .a))
INC_TARGETS := $(addprefix include/, \
	       cln_fw.h \
	       err_status.h \
	       eee.h \
	       )
TARGETS := $(BIN_TARGETS) $(LIB_TARGETS) $(INC_TARGETS)

OBJS_$(BIN_NAME) := \
		    cln_fwtool_main.o \
		    cmd_help.o \
		    cmd_sbembed.o \
		    cmd_show.o \
		    cmd_capsule.o
OBJS_$(LIB_NAME) := $(addprefix lib/, \
		    mfh.o \
		    platform_data.o \
		    capsule.o \
		    crc32.o \
		    parser.o \
		    handle.o \
		    util.o \
		    buffer_stream.o \
		    linux.o \
		    )
OBJS := $(OBJS_$(BIN_NAME)) $(OBJS_$(LIB_NAME))

LDFLAGS := --warn-common --no-undefined --fatal-warnings
CFLAGS := -O2 -Wall -ggdb -fPIC -DVERSION=\"$(VERSION)\" \
	  -I$(TOPDIR)/linux/include $(addprefix $(join -Wl,,),$(LDFLAGS))

.DEFAULT_GOAL := all
.PHONE: all clean install

all: $(TARGETS) Makefile

$(BIN_NAME): $(OBJS_$(BIN_NAME)) lib/$(LIB_NAME).so.$(VERSION)
	$(CC) $(CFLAGS) $^ -o $@

$(BIN_NAME)_s: $(OBJS_$(BIN_NAME)) lib/$(LIB_NAME).a
	$(CC) $(CFLAGS) -static $^ -o $@

lib/$(LIB_NAME).so.$(VERSION): $(OBJS_$(LIB_NAME))
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-soname,$(LIB_NAME).so.$(MAJOR_VERSION) \
		-DBYTE_STREAM_ERROR_BASE=0x10000 \
		-DCLN_FLASH_ERROR_BASE=0x20000 \
		-shared

lib/$(LIB_NAME).a: $(OBJS_$(LIB_NAME))
	$(AR) rcs $@ $^

clean:
	@$(RM) $(OBJS) $(BIN_TARGETS) $(LIB_TARGETS)

install: all
	$(INSTALL) -d -m 755 $(DESTDIR)$(bindir)
	$(foreach x, $(BIN_TARGETS), \
		$(INSTALL) -m 755 $(x) $(DESTDIR)$(bindir);)
	$(INSTALL) -d -m 755 $(DESTDIR)$(libdir)
	$(foreach x, $(LIB_TARGETS), \
		$(INSTALL) -m 755 $(x) $(DESTDIR)$(libdir);)
	ln -sfn $(LIB_NAME).so.$(VERSION) \
		$(DESTDIR)$(libdir)/$(LIB_NAME).so
	ln -sfn $(LIB_NAME).so.$(VERSION) \
		$(DESTDIR)$(libdir)/$(LIB_NAME).so.$(MAJOR_VERSION)
	ln -sfn $(LIB_NAME).so.$(VERSION) \
		$(DESTDIR)$(libdir)/$(LIB_NAME).so.$(MAJOR_VERSION).$(MINOR_VERSION)
	$(INSTALL) -d -m 755 $(DESTDIR)$(includedir)
	$(INSTALL) -m 644 $(INC_TARGETS) $(DESTDIR)$(includedir)