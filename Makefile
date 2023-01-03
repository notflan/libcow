# Generic C and C++ Makefile project template
# Contains targets for `release', `debug', and `clean'.

PROJECT=cow
AUTHOR=Avril (Flanchan) <flanchan@cumallover.me>

VERSION_MAJOR=0
VERSION_MINOR=3.0
VERSION=$(VERSION_MAJOR).$(VERSION_MINOR)

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

SRC_C   = $(wildcard src/*.c)
SRC_CXX = $(wildcard src/*.cpp)

INCLUDE=include

COMMON_FLAGS= -W -Wall -Wextra -Wstrict-aliasing -pedantic -fno-strict-aliasing "-DVERSION=$(VERSION)" $(addprefix -I,$(INCLUDE))

TARGET_CPU?=native
OPT_FLAGS?= $(addprefix -march=,$(TARGET_CPU)) -fgraphite -fopenmp -floop-parallelize-all -ftree-parallelize-loops=4 \
	    -floop-interchange -ftree-loop-distribution -floop-strip-mine -floop-block \
	    -fno-stack-check

CXX_OPT_FLAGS?= $(OPT_FLAGS) 

CFLAGS   += $(COMMON_FLAGS) --std=gnu11
CXXFLAGS += $(COMMON_FLAGS) --std=gnu++20 -felide-constructors
LDFLAGS  += 

STRIP=strip

ifneq ($(TARGET_SPEC_FLAGS),no)
	RELEASE_CFLAGS?=   -O3 -flto $(OPT_FLAGS)
	RELEASE_CXXFLAGS?= -O3 -flto $(CXX_OPT_FLAGS)
	RELEASE_LDFLAGS?=  -Wl,-O3 -Wl,-flto

	DEBUG_CFLAGS?=	-O0 -g
	DEBUG_CXXFLAGS?=-O0 -g
	DEBUG_LDFLAGS?=
endif

DEBUG_CFLAGS+=-DDEBUG
DEBUG_CXXFLAGS+=-DDEBUG

RELEASE_CFLAGS+=-DRELEASE
RELEASE_CXXFLAGS+=-DRELEASE

# Objects

OBJ_C   = $(addprefix obj/c/,$(SRC_C:.c=.o))
OBJ_CXX = $(addprefix obj/cxx/,$(SRC_CXX:.cpp=.o))
OBJ = $(OBJ_C) $(OBJ_CXX)

# Phonies

.PHONY: release
release: | dirs
	$(MAKE) lib$(PROJECT).a
	@$(MAKE) clean-rebuild >> /dev/null
	@$(MAKE) dirs >> /dev/null
	$(MAKE) lib$(PROJECT).so

.PHONY: debug
debug: | dirs
	$(MAKE) lib$(PROJECT)-debug.a
	@$(MAKE) clean-rebuild >> /dev/null
	@$(MAKE) dirs >> /dev/null
	$(MAKE) lib$(PROJECT)-debug.so

# Rebuild both release and debug targets from scratch
.PHONY: all
all: | clean
	@$(MAKE) release
	@$(MAKE) clean-rebuild
	@$(MAKE) debug

.PHONY: install
.PHONY: uninstall

.PHONY: test
test:
	@rm -f $(PROJECT)-cpp-test
	@$(MAKE) $(PROJECT)-cpp-test

# Targets

dirs:
	@mkdir -p obj/c{,xx}/src

obj/c/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@ $(LDFLAGS)

obj/cxx/%.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@ $(LDFLAGS)

lib$(PROJECT)-release.a: CFLAGS+= $(RELEASE_CFLAGS)
lib$(PROJECT)-release.a: CXXFLAGS += $(RELEASE_CXXFLAGS)
lib$(PROJECT)-release.a: LDFLAGS += $(RELEASE_LDFLAGS)
lib$(PROJECT)-release.a: $(OBJ)
	ar rcs $@ $^
	ranlib $@

lib$(PROJECT)-debug.a: CFLAGS+= $(DEBUG_CFLAGS)
lib$(PROJECT)-debug.a: CXXFLAGS += $(DEBUG_CXXFLAGS)
lib$(PROJECT)-debug.a: LDFLAGS += $(DEBUG_LDFLAGS)
lib$(PROJECT)-debug.a: $(OBJ)
	ar rcs $@ $^
	ranlib $@

lib$(PROJECT)-release.so: CFLAGS+= $(RELEASE_CFLAGS) -fPIC
lib$(PROJECT)-release.so: CXXFLAGS += $(RELEASE_CXXFLAGS) -fPIC
lib$(PROJECT)-release.so: LDFLAGS += $(RELEASE_LDFLAGS)
lib$(PROJECT)-release.so: $(OBJ)
	$(CXX) -shared $^ $(CXXFLAGS) -o $@ $(LDFLAGS)
	$(STRIP) $@

lib$(PROJECT)-debug.so: CFLAGS+= $(DEBUG_CFLAGS) -fPIC
lib$(PROJECT)-debug.so: CXXFLAGS += $(DEBUG_CXXFLAGS) -fPIC
lib$(PROJECT)-debug.so: LDFLAGS += $(DEBUG_LDFLAGS)
lib$(PROJECT)-debug.so: $(OBJ)
	$(CXX) -shared $^ $(CXXFLAGS) -o $@ $(LDFLAGS)

lib$(PROJECT).a: lib$(PROJECT)-release.a
	ln -f $< $@

lib$(PROJECT).so: LDFLAGS+= -Wl,-soname,lib$(PROJECT).so.$(VERSION_MAJOR)
lib$(PROJECT).so: lib$(PROJECT)-release.so
	ln -f $< $@.$(VERSION)
	ln -sf $@.$(VERSION) $@.$(VERSION_MAJOR)
	ln -sf $@.$(VERSION_MAJOR) $@

clean-rebuild:
	rm -rf obj

clean: clean-rebuild
	rm -f lib$(PROJECT){,-{release,debug,pgo}}.{a,so{,.*}}

install:
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 lib$(PROJECT).a $(DESTDIR)$(PREFIX)/lib/
	install -s -m 755 lib$(PROJECT).so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/
	ln -sf lib$(PROJECT).so.$(VERSION) $(DESTDIR)$(PREFIX)/lib/lib$(PROJECT).so.$(VERSION_MAJOR)
	ln -sf lib$(PROJECT).so.$(VERSION_MAJOR) $(DESTDIR)$(PREFIX)/lib/lib$(PROJECT).so
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 $(wildcard $(INCLUDE)/*.*) $(DESTDIR)$(PREFIX)/include/
	install -d $(DESTDIR)$(PREFIX)/include/$(PROJECT)/
	install -m 644 $(wildcard $(INCLUDE)/$(PROJECT)/*.*) $(DESTDIR)$(PREFIX)/include/$(PROJECT)/

uninstall:
	-rm $(DESTDIR)$(PREFIX)/lib/lib$(PROJECT).{a,so{,.*}}
	cd $(INCLUDE) && find . -type f | xargs -I {} rm "$(DESTDIR)$(PREFIX)/include/{}"
	-rmdir $(DESTDIR)$(PREFIX)/include/$(PROJECT)

$(PROJECT)-cpp-test: lib$(PROJECT).so
	g++ -O3 -flto --std=gnu++20 -Iinclude/ -g -Wall -Wextra src/test/*.cpp -o $@ -l:$< -Wl,-flto -Wl,-O3
	-valgrind ./$@
