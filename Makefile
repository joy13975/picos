CC		= gcc
CFLAGS	= -O3 -Wall -std=c99 -g
LD		=
SONAME 	= libpicos
VERSION = 1
SOFLAGS = -shared -Wl,-soname,$(SONAME).so.$(VERSION)
SO_EXT  = so
PREFIX  = .

PLATFORM= $(shell (uname -s))

ifeq ($(PLATFORM), Darwin)
	SOFLAGS = -dynamiclib -current_version $(VERSION) -compatibility_version $(VERSION).0
	SO_EXT  = dylib
endif

all: src/libpicos src/libpicosu

install_libpicos:
	mkdir -p $(PREFIX)/include
	cp src/picos.h $(PREFIX)/include/
	mkdir -p $(PREFIX)/lib
	cp src/$(SONAME).$(SO_EXT) $(PREFIX)/lib/
	rm -rf $(PREFIX)/lib/$(SONAME).$(SO_EXT).$(VERSION)
	ln -s $(PREFIX)/lib/$(SONAME).$(SO_EXT) $(PREFIX)/lib/$(SONAME).$(VERSION).$(SO_EXT)

install_libpicosu:
	mkdir -p $(PREFIX)/include
	cp src/picosu.h $(PREFIX)/include/
	mkdir -p $(PREFIX)/lib
	cp src/$(SONAME)u.$(SO_EXT) $(PREFIX)/lib/
	rm -rf $(PREFIX)/lib/$(SONAME)u.$(SO_EXT).$(VERSION)
	ln -s $(PREFIX)/lib/$(SONAME)u.$(SO_EXT) $(PREFIX)/lib/$(SONAME)u.$(VERSION).$(SO_EXT)

install_picos_types:
	cp src/picos_types.h $(PREFIX)/include/

install: all install_libpicos install_libpicosu install_picos_types

uninstall:
	rm -rf $(PREFIX)/include/*picos* $(PREFIX)/lib/*picos*

examples/%: examples/%.c
	$(CC) $(CFLAGS) -I./include -L./lib $^ -o $@ $(LD) -lpicos

examples: install examples/simple

%.o: %.c
	$(CC) $(CFLAGS) -c -fPIC $< -o $@ $(LD)

src/$(SONAME)u: src/util/checksum.o src/util/corrupt.o
	$(CC) $(CFLAGS) -fPIC $(SOFLAGS) -install_name "$(SONAME)u.$(VERSION).dylib" $^ -o $@.$(SO_EXT) $(LD)

src/$(SONAME): src/$(SONAME).c
	$(CC) $(CFLAGS) -fPIC $(SOFLAGS) -install_name "$(SONAME).$(VERSION).dylib" $^ -o $@.$(SO_EXT) $(LD)

clean:
	rm -rf src/*.o src/*.so src/*.dylib src/util/*.o include lib
