LIB_TARGETS = libclnfw.so.$(VERSION)
TARGETS = $(LIB_TARGETS) libclnfw.a
OBJS_libclnfw = \
	parser.o \
	mfh.o \
	platform_data.o \
	crc32.o \
	buffer_stream.o \
	linux.o \
	util.o \
	handle.o

OBJS = $(OBJS_libclnfw)

CFLAGS += -fpic -DBYTE_STREAM_ERROR_BASE=0x10000 \
	  -DCLN_FLASH_ERROR_BASE=0x20000
LDFLAGS += -shared

.DEFAULT_GOAL := all
.PHONE: all clean install

all: $(TARGETS) Makefile

clean:
	@$(RM) $(OBJS) $(TARGETS) %.so.$(MAJOR_VERSION) %.so.$(MAJOR_VERSION).$(MINOR_VERSION)

install:
	$(INSTALL) -d -m 755 $(DESTDIR)$(libdir)
	$(foreach x, $(LIB_TARGETS), $(INSTALL) -m 755 $(x) $(DESTDIR)$(libdir); \
		ln -sfn $(x) $(DESTDIR)$(libdir)/$(patsubst %.$(VERSION),%,$(x)); \
		ln -sfn $(x) $(DESTDIR)$(libdir)/$(patsubst %.$(VERSION),%.$(MAJOR_VERSION),$(x)); \
		ln -sfn $(x) $(DESTDIR)$(libdir)/$(patsubst %.$(VERSION),%.$(MAJOR_VERSION).$(MINOR_VERSION),$(x));)

libclnfw.so.$(VERSION): $(OBJS_libclnfw)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -Wl,-soname,$(patsubst %.$(VERSION),%,$@)

libclnfw.a: $(OBJS_libclnfw)
	$(AR) rcs $@ $(OBJS_libclnfw)