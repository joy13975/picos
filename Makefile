CC			= mpicc
CFLAGS		= -O3 -Wall -std=c99 -g
LD			=
SONAME 		= libpicos
USONAME		= libpicosu
VERSION 	= 1
SOFLAGS 	= -shared -Wl,-soname,$(SONAME).$(VERSION).so
SO_EXT  	= so
PREFIX  	= .
DEF 		= -DUSE_MPI

PLATFORM	= $(shell (uname -s))

ifeq ($(PLATFORM), Darwin)
	SOFLAGS 	= -dynamiclib -install_name "$(SONAME).$(VERSION).dylib" -current_version $(VERSION) -compatibility_version $(VERSION).0
	SO_EXT  	= dylib
endif

COMPILE 	= $(CC) $(DEF) $(CFLAGS)

all: src/libpicos

install_libpicos:
	mkdir -p $(PREFIX)/include
	cp src/picos.h $(PREFIX)/include/
	cp src/picos_types.h $(PREFIX)/include/
	mkdir -p $(PREFIX)/lib
	cp src/$(SONAME).$(SO_EXT) $(PREFIX)/lib/
	rm -rf $(PREFIX)/lib/$(SONAME).$(SO_EXT).$(VERSION)
	ln -s $(PREFIX)/lib/$(SONAME).$(SO_EXT) $(PREFIX)/lib/$(SONAME).$(VERSION).$(SO_EXT)

install: all install_libpicos

uninstall:
	rm -rf $(PREFIX)/include/*picos* $(PREFIX)/lib/*picos*

examples/%: examples/%.c
	$(COMPILE) -I./include -L./lib $^ -o $@ $(LD) -lpicos

examples: install examples/simple

%.o: %.c
	$(COMPILE) -c -fPIC $< -o $@ $(LD)

src/$(SONAME): src/$(SONAME).c src/util/vma-iter.c src/util/checksum.c src/util/corrupt.c src/util/bits.c
	$(COMPILE) -fPIC $(SOFLAGS)  $^ -o $@.$(SO_EXT) $(LD)

clean:
	rm -rf src/*.o src/*.so src/*.dylib src/util/*.o include lib
