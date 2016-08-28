CC		= gcc
CFLAGS	= -O3 -Wall
LD		=
SONAME 	= libpicos
VERSION = 1
SOFLAGS = -shared -Wl,-soname,$(SONAME).so.$(VERSION)
SO_EXT  = so

PLATFORM= $(shell (uname -s))

ifeq ($(PLATFORM), Darwin)
	SOFLAGS = -dynamiclib -install_name "$(SONAME).$(VERSION).dylib" -current_version $(VERSION) -compatibility_version $(VERSION).0
	SO_EXT  = dylib
endif

all: src/libpicos

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -fPIC $< -o $@ $(LD)

src/libpicos: src/libpicos.c
	$(CC) $(CFLAGS) -fPIC $(SOFLAGS) $^ -o $@.$(SO_EXT) $(LD)

clean:
	rm -rf src/*.o src/*.so src/*.dylib