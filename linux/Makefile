TARGETS = cln_fwtool cln_fwtool_s
OBJS_cln_fwtool = \
	cln_fwtool_main.o \
	cmd_sbembed.o \
	cmd_show.o
OBJS = $(OBJS_cln_fwtool)

INC_TARGETS = include/cln_fw.h \
	include/err_status.h \
	include/eee.h

.DEFAULT_GOAL := all
.PHONE: all clean install

all: $(TARGETS) Makefile

clean:
	@$(RM) $(OBJS) $(TARGETS)

cln_fwtool: $(OBJS_cln_fwtool) lib/libclnfw.so.$(VERSION)
	$(CC) $(CFLAGS) $^ -o $@

cln_fwtool_s: $(OBJS_cln_fwtool) lib/libclnfw.a
	$(CC) $(CFLAGS) -static $^ -o $@

install:
	$(INSTALL) -d -m 755 $(DESTDIR)$(bindir)
	$(foreach x, $(TARGETS), $(INSTALL) -m 755 $(x) $(DESTDIR)$(bindir);)